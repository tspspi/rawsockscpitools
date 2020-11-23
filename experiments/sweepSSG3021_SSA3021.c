#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <unistd.h>

#include "../include/labtypes.h"
#include "../include/siglent_ssa3021x.h"
#include "../include/siglent_ssg3021x.h"

static void printUsage(int argc, char* argv[]) {
    printf("Usage:\n\n\t %s ADDRESS_SSG ADDRESS_SSA STARTHZ ENDHZ STEPS TARGETFILE AVGSAMPLES\n", argv[0]);
    return;
}

int main(int argc, char* argv[]) {
    enum labError e;
    unsigned long int hzStart;
    unsigned long int hzEnd;
    unsigned long int nAvg;
    unsigned long int nSteps;
    struct spectrumDataTrace* lpTrace;
    unsigned long int i;

    unsigned long int dwSweepFrequency;

    struct siglentSSA3021x* lpSSA3021X;
    struct siglentSSG3021x* lpSSG3021X;

    FILE* fHandle = NULL;

    if(argc < 8) {
        printUsage(argc, argv);
        return 1;
    }

    /* Read required parameters ... */
    if(sscanf(argv[3], "%lu", &hzStart) != 1) {
        printf("Unknown start frequency %s\n", argv[3]);
        printUsage(argc, argv);
        return 1;
    }
    if(sscanf(argv[4], "%lu", &hzEnd) != 1) {
        printf("Unknown end frequency %s\n", argv[4]);
        printUsage(argc, argv);
        return 1;
    }
    if(sscanf(argv[5], "%lu", &nSteps) != 1) {
        printf("Unknown number of steps %s\n", argv[4]);
        printUsage(argc, argv);
        return 1;
    }
    if(sscanf(argv[7], "%lu", &nAvg) != 1) {
        printf("Unknown average sample count %s\n", argv[6]);
        printUsage(argc, argv);
        return 1;
    }

    if(hzEnd <= hzStart) { printf("End frequency has to be higher than start frequency"); }
    if(hzEnd > 2100000000) { printf("Upper frequency limit is 2.1 GHz"); }
    if(hzStart > 2100000000) { printf("Upper frequency limit is 2.1 GHz"); }

    /* Connect to devices */
    e = siglentSSG3021xConnect(&lpSSG3021X, argv[1]);
    if(e != labE_Ok) {
        printf("Failed to connect to signal generator\n");
        return 2;
    }

    e = siglentSSA3021xConnect(&lpSSA3021X, argv[2]);
    if(e != labE_Ok) {
        lpSSG3021X->vtbl->disconnect(lpSSG3021X);
        printf("Failed to connect to signal analyzer\n");
        return 2;
    }

    dwSweepFrequency = hzStart;

    if((fHandle = fopen(argv[6], "w")) == NULL) {
        printf("Failed to open file %s\n", argv[6]);
        goto cleanup;
    }

    /* Setup ... */
    printf("Initializing spectrum analyzer:\n");
    printf("\tCenter (%lu Hz) ... ", (hzStart+hzEnd) >> 1); if((e = lpSSA3021X->vtbl->setFrequencyCenter(lpSSA3021X, (hzStart + hzEnd)/2.0)) != labE_Ok) { printf("failed\n"); goto cleanup; }
    printf("ok\n\tSpan ... ");                              if((e = lpSSA3021X->vtbl->setSpan(lpSSA3021X, (hzEnd-hzStart))) != labE_Ok) { printf("failed\n"); goto cleanup; } /* Set to full span */
    printf("ok\n\tAverages ... ");                          if((e = lpSSA3021X->vtbl->setAverageCount(lpSSA3021X, nAvg)) != labE_Ok) { printf("failed\n"); goto cleanup; } /* Set averaging ... */
    printf("ok\n");

    /* Prepare generator */
    printf("Initializing signal generator:\n");
    printf("\tFrequency (%lu Hz) ... ", hzStart);   if((e = lpSSG3021X->vtbl->rfSetFrequency(lpSSG3021X, hzStart)) != labE_Ok) { printf("failed\n"); goto cleanup; }
    printf("ok\n\tPower ... ");                     if((e = lpSSG3021X->vtbl->rfSetPower(lpSSG3021X, 0.0)) != labE_Ok) { printf("failed\n"); goto cleanup; } /* Set 0 dBm power */
    printf("ok\n\tRF on ... ");                     if((e = lpSSG3021X->vtbl->rfOutEnable(lpSSG3021X, true)) != labE_Ok) { printf("failed\n"); goto cleanup; } /* Enable output ... */
    printf("ok\n");


    printf("Starting sweep from %lu to %lu\n", hzStart, hzEnd);
    double dStep = ((double)(hzEnd - hzStart) / (double)nSteps);
    fprintf(fHandle, "# AnalyzerHz    GeneratorHz     Amplitude");
    for(dwSweepFrequency = hzStart; dwSweepFrequency < hzEnd; dwSweepFrequency = dwSweepFrequency + dStep) {
        if((e = lpSSG3021X->vtbl->rfSetFrequency(lpSSG3021X, dwSweepFrequency)) != labE_Ok) { goto cleanup; }
        usleep(750000);

        if((e = lpSSA3021X->vtbl->queryTraceData(lpSSA3021X, &lpTrace)) != labE_Ok) { goto cleanup; }

        for(i = 0; i < lpTrace->dwDataPoints; i=i+1) {
            fprintf(fHandle, "%lf %lf %lf\n", lpTrace->data[i].frq, (double)dwSweepFrequency, lpTrace->data[i].value);
        }

        free(lpTrace);
    }

    e = labE_Ok;
cleanup:
    if(fHandle != NULL) {
        fclose(fHandle);
    }

    lpSSG3021X->vtbl->rfOutEnable(lpSSG3021X, false); /* Disable RF output ... */

    lpSSG3021X->vtbl->disconnect(lpSSG3021X);
    lpSSA3021X->vtbl->disconnect(lpSSA3021X);

    if(e == labE_Ok) {
        return 0;
    } else {
        return 3;
    }
}

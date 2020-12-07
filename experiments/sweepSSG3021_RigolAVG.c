#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#include <unistd.h>

#include "../include/labtypes.h"
#include "../include/siglent_ssg3021x.h"
#include "../include/rigolmso5000.h"

static void printUsage(int argc, char* argv[]) {
    printf("Usage:\n\n\t %s ADDRESS_SSG ADDRESS_RIGOL STARTHZ ENDHZ STEPS TARGETFILE AVGSAMPLES\n", argv[0]);
    return;
}

int main(int argc, char* argv[]) {
    enum labError e;
    unsigned long int hzStart;
    unsigned long int hzEnd;
    unsigned long int nAvg;
    unsigned long int nSteps;
    unsigned long int i;
    struct oscilloscopeWaveformData* lpWaveform[2];

    unsigned long int dwSweepFrequency;

    struct siglentSSG3021x* lpSSG3021X;
    struct rigolMso5000* lpMSO5000;

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

    e = rigolMso5000Connect(&lpMSO5000, argv[2]);
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
    printf("Initializing oscilloscope:\n");
    printf("\tEnabling channel 1 ... ");                    if((e = lpMSO5000->vtbl->setChannelEnable(lpMSO5000, 1, true)) != labE_Ok)  { printf("failed\n"); goto cleanup; }
    printf("ok\n\tEnabling channel 2 ... ");                if((e = lpMSO5000->vtbl->setChannelEnable(lpMSO5000, 2, true)) != labE_Ok)  { printf("failed\n"); goto cleanup; }
    printf("ok\n\tDisabling channel 3 ... ");               if((e = lpMSO5000->vtbl->setChannelEnable(lpMSO5000, 3, false)) != labE_Ok)  { printf("failed\n"); goto cleanup; }
    printf("ok\n\tDisabling channel 4 ... ");               if((e = lpMSO5000->vtbl->setChannelEnable(lpMSO5000, 4, false)) != labE_Ok)  { printf("failed\n"); goto cleanup; }
    printf("ok\n");

    /* Prepare generator */
    printf("Initializing signal generator:\n");
    printf("\tFrequency (%lu Hz) ... ", hzStart);   if((e = lpSSG3021X->vtbl->rfSetFrequency(lpSSG3021X, hzStart)) != labE_Ok) { printf("failed\n"); goto cleanup; }
    printf("ok\n\tPower ... ");                     if((e = lpSSG3021X->vtbl->rfSetPower(lpSSG3021X, 10.0)) != labE_Ok) { printf("failed\n"); goto cleanup; } /* Set 0 dBm power */
    printf("ok\n\tRF on ... ");                     if((e = lpSSG3021X->vtbl->rfOutEnable(lpSSG3021X, true)) != labE_Ok) { printf("failed\n"); goto cleanup; } /* Enable output ... */
    printf("ok\n");


    printf("Starting sweep from %lu to %lu\n", hzStart, hzEnd);
    double dStep = ((double)(hzEnd - hzStart) / (double)nSteps);

    fprintf(fHandle, "# GeneratorHz     Amplitude average WS        Amplitude average ANT       Sigma WS            Sigma ANT       AmplitudeSquared WS     AmplitudeSquared ANT        SigmaAmplitudeSquared WS     SigmaAmplitudeSquared ANT");
    for(dwSweepFrequency = hzStart; dwSweepFrequency < hzEnd; dwSweepFrequency = dwSweepFrequency + dStep) {
        if((e = lpSSG3021X->vtbl->rfSetFrequency(lpSSG3021X, dwSweepFrequency)) != labE_Ok) { goto cleanup; }
        usleep(500000);
        // sleep(1);

        if((e = lpMSO5000->vtbl->queryWaveform(lpMSO5000, 1, &(lpWaveform[0]))) != labE_Ok) { goto cleanup; }
        if((e = lpMSO5000->vtbl->queryWaveform(lpMSO5000, 2, &(lpWaveform[1]))) != labE_Ok) { goto cleanup; }

        {
            double dAvg1 = 0.0;
            double dAvg2 = 0.0;
            double dAvgSQ1 = 0.0;
            double dAvgSQ2 = 0.0;


            double dSigma1 = 0.0;
            double dSigma2 = 0.0;
            double dSigmaSQ1 = 0.0;
            double dSigmaSQ2 = 0.0;

            for(i = 0; i < lpWaveform[0]->dwDataPoints; i=i+1) {
                dAvg1 = dAvg1 + lpWaveform[0]->dData[i];
            }
            for(i = 0; i < lpWaveform[1]->dwDataPoints; i=i+1) {
                dAvg2 = dAvg2 + lpWaveform[1]->dData[i];
            }
            for(i = 0; i < lpWaveform[0]->dwDataPoints; i=i+1) {
                dAvgSQ1 = dAvgSQ1 + fabsl(lpWaveform[0]->dData[i]);
            }
            for(i = 0; i < lpWaveform[1]->dwDataPoints; i=i+1) {
                dAvgSQ2 = dAvgSQ2 + fabsl(lpWaveform[1]->dData[i]);
            }
            dAvg1 = dAvg1 / ((double)lpWaveform[0]->dwDataPoints);
            dAvg2 = dAvg2 / ((double)lpWaveform[1]->dwDataPoints);
            dAvgSQ1 = dAvgSQ1 / ((double)lpWaveform[0]->dwDataPoints);
            dAvgSQ2 = dAvgSQ2 / ((double)lpWaveform[1]->dwDataPoints);

            for(i = 0; i < lpWaveform[0]->dwDataPoints; i=i+1) {
                dSigma1 = dSigma1 + (dAvg1 - lpWaveform[0]->dData[i])*(dAvg1 - lpWaveform[0]->dData[i]);
            }
            for(i = 0; i < lpWaveform[1]->dwDataPoints; i=i+1) {
                dSigma2 = dSigma2 + (dAvg2 - lpWaveform[1]->dData[i])*(dAvg2 - lpWaveform[1]->dData[i]);
            }
            for(i = 0; i < lpWaveform[0]->dwDataPoints; i=i+1) {
                dSigmaSQ1 = dSigmaSQ1 + (dAvgSQ1 - lpWaveform[0]->dData[i])*(dAvgSQ1 - lpWaveform[0]->dData[i]);
            }
            for(i = 0; i < lpWaveform[1]->dwDataPoints; i=i+1) {
                dSigmaSQ2 = dSigmaSQ2 + (dAvgSQ2 - lpWaveform[1]->dData[i])*(dAvgSQ2 - lpWaveform[1]->dData[i]);
            }

            dSigma1 = sqrt(dSigma1 / ((double)lpWaveform[0]->dwDataPoints));
            dSigma2 = sqrt(dSigma2 / ((double)lpWaveform[1]->dwDataPoints));
            dSigmaSQ1 = sqrt(dSigmaSQ1 / ((double)lpWaveform[0]->dwDataPoints));
            dSigmaSQ2 = sqrt(dSigmaSQ2 / ((double)lpWaveform[1]->dwDataPoints));

            fprintf(fHandle, "%lu %lf %lf %lf %lf %lf %lf %lf %lf\n", dwSweepFrequency, dAvg1, dAvg2, dSigma1, dSigma2, dAvgSQ1, dAvgSQ2, dSigmaSQ1, dSigmaSQ2);
            fflush(fHandle);
            printf("Sampled at %lu HZ, measured %lf (stddev %lf) and %lf (dev %lf)\n", dwSweepFrequency, dAvg1, dSigma1, dAvg2, dSigma2); fflush(stdout);
            printf("\tabsolute: measured %lf (stddev %lf) and %lf (dev %lf)\n", dAvgSQ1, dSigmaSQ1, dAvgSQ2, dSigmaSQ2); fflush(stdout);
        }

        free(lpWaveform[0]);
        free(lpWaveform[1]);
    }

    e = labE_Ok;
cleanup:
    if(fHandle != NULL) {
        fclose(fHandle);
    }

    lpSSG3021X->vtbl->rfOutEnable(lpSSG3021X, false); /* Disable RF output ... */

    lpSSG3021X->vtbl->disconnect(lpSSG3021X);
    lpMSO5000->vtbl->disconnect(lpMSO5000);

    if(e == labE_Ok) {
        return 0;
    } else {
        return 3;
    }
}

#include <stdio.h>
#include <stdlib.h>

#include "../../include/labtypes.h"
#include "../../include/rigolmso5000.h"

static void printUsage(int argc, char* argv[]) {
    printf("Usage:\n\n");
    printf("\t%s ADDRESS CHANNEL TARGETFILE\n\n", argv[0]);
    printf("ADDRESS\n\tAddress (IPv4, IPv6 or domain name) of the oscilloscope\n");
    printf("CHANNEL\n\tChannel number (1,2,3,4) that should be queried\n");
    printf("TARGETFILE\n\tFilename of the target file (overwritten if exists)\n");
}

int main(int argc, char* argv[]) {
    enum labError e;
    struct rigolMso5000* lpMSO5000;
    FILE* fHandle;

    unsigned long int dwChannel;
    unsigned long int iSample;

    struct oscilloscopeWaveformData* lpWaveform;

    if(argc < 4) { printUsage(argc, argv); return 1; }

    /* Parse channel number */
    if(sscanf(argv[2], "%lu", &dwChannel) != 1) {
        printf("Unknown channel %s\n", argv[2]);
        printUsage(argc, argv);
        return 1;
    }

    /* First connect to the oscilloscope */
    e = rigolMso5000Connect(&lpMSO5000, argv[1]);
    if(e != labE_Ok) {
        printf("Failed to connect\n");
        return 2;
    }

    /* Ask for waveform data */
    e = lpMSO5000->vtbl->queryWaveform(lpMSO5000, dwChannel, &lpWaveform);
    if(e != labE_Ok) {
        printf("Failed to receive waveform (%u)\n", e);
        lpMSO5000->vtbl->disconnect(lpMSO5000);
        return 3;
    }

    lpMSO5000->vtbl->disconnect(lpMSO5000);

    /* Write into target file */
    if((fHandle = fopen(argv[3], "w")) == NULL) {
        printf("Failed to open output file %s\n", argv[3]);
        perror(NULL);
        printf("\n");
        return 4;
    }

    for(iSample = 0; iSample < lpWaveform->dwDataPoints; iSample = iSample + 1) {
        fprintf(fHandle, "%lf\n", lpWaveform->dData[iSample]);
    }

    fclose(fHandle);

    return 0;
}

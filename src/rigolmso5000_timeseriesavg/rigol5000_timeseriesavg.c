#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../../include/labtypes.h"
#include "../../include/rigolmso5000.h"

static void printUsage(int argc, char* argv[]) {
    printf("Averaging timeseries for Rigol5000\n\n");
    printf("Uses the Rigol5000 as an (expensive) ethernet controllable digital multimeter.\nTakes a sample every DELAY seconds (STEPS times), calculates the average and writes data to disk\n\n");
    printf("Usage:\n\n");
    printf("\t%s ADDRESS CHANNEL TARGETFILE STEPS DELAY\n\n", argv[0]);
    printf("ADDRESS\n\tAddress (IPv4, IPv6 or domain name) of the oscilloscope\n");
    printf("CHANNEL\n\tChannel number (1,2,3,4) that should be queried\n");
    printf("TARGETFILE\n\tFilename of the target file (overwritten if exists)\n");
    printf("STEPS\n\tNumber of steps to make\n");
    printf("DELAY\n\tSeconds (1 or more) to wait\n");
}

static void printTime(unsigned long int dt) {
    unsigned long int secs = dt % 60;
    unsigned long int mins = (dt / 60) % 60;
    unsigned long int hrs = (dt / 3600) % 24;
    unsigned long int days = (dt / 86400);

    if(days != 0) { printf("%lu days, ", days); }
    if(hrs != 0) { printf("%lu hours, ", hrs); }
    if(mins != 0) { printf("%lu minutes, ", mins); }
    printf("%lu seconds", secs);
}

int main(int argc, char* argv[]) {
    enum labError e;
    struct rigolMso5000* lpMSO5000;
    FILE* fHandle;

    unsigned long int dwChannel;
    unsigned long int iSample;
    unsigned long int nSteps;
    unsigned long int uDelay;

    unsigned long int iStep;

    struct oscilloscopeWaveformData* lpWaveform;

    if(argc < 6) { printUsage(argc, argv); return 1; }

    /* Parse channel number */
    if(sscanf(argv[2], "%lu", &dwChannel) != 1) {
        printf("Unknown channel %s\n", argv[2]);
        printUsage(argc, argv);
        return 1;
    }
    if(sscanf(argv[4], "%lu", &nSteps) != 1) {
        printf("Unknown step count %s\n", argv[4]);
        printUsage(argc, argv);
        return 1;
    }
    if(sscanf(argv[5], "%lu", &uDelay) != 1) {
        printf("Unknown delay %s\n", argv[5]);
        printUsage(argc, argv);
        return 1;
    }

    /* First connect to the oscilloscope */
    e = rigolMso5000Connect(&lpMSO5000, argv[1]);
    if(e != labE_Ok) {
        printf("Failed to connect\n");
        return 2;
    }

    /* Open target file */
    if((fHandle = fopen(argv[3], "w")) == NULL) {
        lpMSO5000->vtbl->disconnect(lpMSO5000);
        printf("Failed to open output file %s\n", argv[3]);
        perror(NULL);
        printf("\n");
        return 4;
    }

    printf("Ready. Press any key to start measurement for ");
    printTime(nSteps * uDelay);
    printf("\n");
    getchar();

    for(iStep = 0; iStep < nSteps; iStep = iStep + 1) {
        printf("Sample %lu ... ", iStep);

        /* Ask for waveform data */
        e = lpMSO5000->vtbl->queryWaveform(lpMSO5000, dwChannel, &lpWaveform);
        if(e != labE_Ok) {
            printf("Failed to receive waveform (%u)\n", e);
            lpMSO5000->vtbl->disconnect(lpMSO5000);
            return 3;
        }

        /* Should do a more stable average though ... */
        double avgSum = 0.0;
        for(iSample = 0; iSample < lpWaveform->dwDataPoints; iSample = iSample + 1) {
            avgSum = avgSum + lpWaveform->dData[iSample];
        }
        avgSum = avgSum / ((double)lpWaveform->dwDataPoints);

        printf("%lf ... ", avgSum);
        fprintf(fHandle, "%lu %lf\n", (iStep * uDelay * 1000), avgSum);


        printTime(uDelay * (nSteps - iStep)); printf(" remaining\n");
        usleep(uDelay * 1000000);
    }

    lpMSO5000->vtbl->disconnect(lpMSO5000);
    fclose(fHandle);

    printf("\n\nDone after "); printTime(uDelay * nSteps); printf("\n\n");

    return 0;
}

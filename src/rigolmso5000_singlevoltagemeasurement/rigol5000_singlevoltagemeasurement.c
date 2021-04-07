#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../../include/labtypes.h"
#include "../../include/rigolmso5000.h"

static void printUsage(int argc, char* argv[]) {
    printf("Doing a single min/max/avg voltage measurement for Rigol5000\n\n");
    printf("Uses the Rigol5000 as an (expensive) ethernet controllable digital multimeter.\nCalculates the average and writes data to standard output\n\n");
    printf("Usage:\n\n");
    printf("\t%s ADDRESS CHANNEL\n\n", argv[0]);
    printf("ADDRESS\n\tAddress (IPv4, IPv6 or domain name) of the oscilloscope\n");
    printf("CHANNEL\n\tChannel number (1,2,3,4) that should be queried\n");
}

int main(int argc, char* argv[]) {
    enum labError e;
    struct rigolMso5000* lpMSO5000;

    unsigned long int dwChannel;
    unsigned long int iSample;

    struct oscilloscopeWaveformData* lpWaveform;

    if(argc < 3) { printUsage(argc, argv); return 1; }

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

    /* Should do a more stable average though ... */
    double avgSum = 0.0;
    double dMin;
    double dMax;
    dMin = lpWaveform->dData[0];
    dMax = lpWaveform->dData[0];
    for(iSample = 0; iSample < lpWaveform->dwDataPoints; iSample = iSample + 1) {
        avgSum = avgSum + lpWaveform->dData[iSample];
        if(lpWaveform->dData[iSample] > dMax) { dMax = lpWaveform->dData[iSample]; }
        if(lpWaveform->dData[iSample] < dMin) { dMin = lpWaveform->dData[iSample]; }
    }
    avgSum = avgSum / ((double)lpWaveform->dwDataPoints);

    printf("# AVG MIN MAX\n");
    printf("%lf %lf %lf\n", avgSum, dMin, dMax);

    lpMSO5000->vtbl->disconnect(lpMSO5000);

    return 0;
}

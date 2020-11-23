#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "../../include/labtypes.h"
#include "../../include/siglent_ssa3021x.h"

static void printUsage(int argc, char* argv[]) {
    printf("Usage:\n\n");
    printf("\t%s ADDRESS AVGSAMPLES\n\nAVGSAMPLES\n\tNumber of samples to use for averaging\n\tSet to 1 to disable averaging\n", argv[0]);
}

int main(int argc, char* argv[]) {
    enum labError e;
    struct siglentSSA3021x* lpSSA3021X;
    unsigned long int dwNSamples;

    if(argc < 3) { printUsage(argc, argv); return 1; }

    if(sscanf(argv[2], "%lu", &dwNSamples) != 1) {
        printf("Unknown sample count %s\n", argv[2]);
        printUsage(argc, argv);
        return 1;
    }
    if(dwNSamples < 1) { printf("Less than 1 sample is not supported\n"); return 1; }
    if(dwNSamples > 999) { printf("More than 999 samples are not supported for averaging\n"); return 1; }

    /* First connect to the oscilloscope */
    e = siglentSSA3021xConnect(&lpSSA3021X, argv[1]);
    if(e != labE_Ok) {
        printf("Failed to connect\n");
        return 2;
    }

    lpSSA3021X->vtbl->setAverageCount(lpSSA3021X, dwNSamples);

    e = lpSSA3021X->vtbl->disconnect(lpSSA3021X);
    if(e != labE_Ok) {
        printf("Disconnection failed, terminating anyways (%u)\n", e);
    }

    return 0;
}

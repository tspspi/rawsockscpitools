#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "../../include/labtypes.h"
#include "../../include/siglent_ssa3021x.h"

static void printUsage(int argc, char* argv[]) {
    printf("Usage:\n\n");
    printf("\t%s ADDRESS SPANHZ\n\nSPANHZ\n\nSpan in HZ", argv[0]);
}

int main(int argc, char* argv[]) {
    enum labError e;
    struct siglentSSA3021x* lpSSA3021X;
    double dFrequencyHz;

    if(argc < 3) { printUsage(argc, argv); return 1; }

    if(sscanf(argv[2], "%lf", &dFrequencyHz) != 1) {
        printf("Unknown frequency span %s\n", argv[2]);
        printUsage(argc, argv);
        return 1;
    }
    if(dFrequencyHz < 0) { printf("Spans below 0 are not supported\n"); return 1; }
    if(dFrequencyHz > 2100000000.0) { printf("Spans above above 2.1 GHz are not supported\n"); return 1; }

    /* First connect to the oscilloscope */
    e = siglentSSA3021xConnect(&lpSSA3021X, argv[1]);
    if(e != labE_Ok) {
        printf("Failed to connect\n");
        return 2;
    }

    lpSSA3021X->vtbl->setSpan(lpSSA3021X, dFrequencyHz);

    e = lpSSA3021X->vtbl->disconnect(lpSSA3021X);
    if(e != labE_Ok) {
        printf("Disconnection failed, terminating anyways (%u)\n", e);
    }

    return 0;
}

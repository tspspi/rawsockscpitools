#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "../../include/labtypes.h"
#include "../../include/siglent_ssg3021x.h"

static void printUsage(int argc, char* argv[]) {
    printf("Usage:\n\n");
    printf("\t%s ADDRESS FREQUENCYHZ\n\nFREQUENCYHZ\n\nRF output frequency in HZ", argv[0]);
}

int main(int argc, char* argv[]) {
    enum labError e;
    struct siglentSSG3021x* lpSSG3021X;
    unsigned long int dwFrequencyHz;

    if(argc < 3) { printUsage(argc, argv); return 1; }

    if(sscanf(argv[2], "%lu", &dwFrequencyHz) != 1) {
        printf("Unknown frequency %s\n", argv[2]);
        printUsage(argc, argv);
        return 1;
    }
    if(dwFrequencyHz < 1000000) { printf("Frequencies below 1 MHz are not supported on RF output\n"); return 1; }
    if(dwFrequencyHz > 2100000000) { printf("Frequencies below above 2.1 GHz are not supported on RF output\n"); return 1; }

    /* First connect to the oscilloscope */
    e = siglentSSG3021xConnect(&lpSSG3021X, argv[1]);
    if(e != labE_Ok) {
        printf("Failed to connect\n");
        return 2;
    }

    lpSSG3021X->vtbl->rfSetFrequency(lpSSG3021X, dwFrequencyHz);

    e = lpSSG3021X->vtbl->disconnect(lpSSG3021X);
    if(e != labE_Ok) {
        printf("Disconnection failed, terminating anyways (%u)\n", e);
    }

    return 0;
}

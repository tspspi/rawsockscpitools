#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "../../include/labtypes.h"
#include "../../include/siglent_ssg3021x.h"

static void printUsage(int argc, char* argv[]) {
    printf("Usage:\n\n");
    printf("\t%s ADDRESS POWER\n\nPOWER\n\nRF output power in dBm", argv[0]);
}

int main(int argc, char* argv[]) {
    enum labError e;
    struct siglentSSG3021x* lpSSG3021X;
    float fPower;

    if(argc < 3) { printUsage(argc, argv); return 1; }

    if(sscanf(argv[2], "%f", &fPower) != 1) {
        printf("Unknown power %s\n", argv[2]);
        printUsage(argc, argv);
        return 1;
    }
    if(fPower < -110.0) { printf("Powers below -110.0 dBm are not supported on the RF output\n"); return 1; }
    if(fPower > 20.0) { printf("Powers above 20.0 dBm are not supported on RF output\n"); return 1; }

    /* First connect to the oscilloscope */
    e = siglentSSG3021xConnect(&lpSSG3021X, argv[1]);
    if(e != labE_Ok) {
        printf("Failed to connect\n");
        return 2;
    }

    lpSSG3021X->vtbl->rfSetPower(lpSSG3021X, fPower);

    e = lpSSG3021X->vtbl->disconnect(lpSSG3021X);
    if(e != labE_Ok) {
        printf("Disconnection failed, terminating anyways (%u)\n", e);
    }

    return 0;
}

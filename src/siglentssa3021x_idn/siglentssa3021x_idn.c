#include <stdio.h>
#include <stdlib.h>

#include "../../include/labtypes.h"
#include "../../include/siglent_ssa3021x.h"

static void printUsage(int argc, char* argv[]) {
    printf("Usage:\n\n");
    printf("\t%s ADDRESS\n\n", argv[0]);
}

int main(int argc, char* argv[]) {
    enum labError e;
    struct siglentSSA3021x* lpSSA3021X;

    if(argc < 2) { printUsage(argc, argv); return 1; }

    /* First connect to the oscilloscope */
    e = siglentSSA3021xConnect(&lpSSA3021X, argv[1]);
    if(e != labE_Ok) {
        printf("Failed to connect\n");
        return 2;
    }

    {
        char* lpIDNString;
        unsigned long int dwIDNStringLen;
        e = lpSSA3021X->vtbl->idn(lpSSA3021X, &lpIDNString, &dwIDNStringLen);

        if(e == labE_Ok) {
            printf("%.*s\n", (int)dwIDNStringLen, lpIDNString);
        } else {
            printf("Failed to receive identity string (%u)\n", e);
        }
    }

    e = lpSSA3021X->vtbl->disconnect(lpSSA3021X);
    if(e != labE_Ok) {
        printf("Disconnection failed, terminating anyways (%u)\n", e);
    }

    return 0;
}

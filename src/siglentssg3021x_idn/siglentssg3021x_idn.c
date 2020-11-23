#include <stdio.h>
#include <stdlib.h>

#include "../../include/labtypes.h"
#include "../../include/siglent_ssg3021x.h"

static void printUsage(int argc, char* argv[]) {
    printf("Usage:\n\n");
    printf("\t%s ADDRESS\n\n", argv[0]);
}

int main(int argc, char* argv[]) {
    enum labError e;
    struct siglentSSG3021x* lpSSG3021X;

    if(argc < 2) { printUsage(argc, argv); return 1; }

    /* First connect to the oscilloscope */
    e = siglentSSG3021xConnect(&lpSSG3021X, argv[1]);
    if(e != labE_Ok) {
        printf("Failed to connect\n");
        return 2;
    }

    {
        char* lpIDNString;
        unsigned long int dwIDNStringLen;
        e = lpSSG3021X->vtbl->idn(lpSSG3021X, &lpIDNString, &dwIDNStringLen);

        if(e == labE_Ok) {
            printf("%.*s\n", (int)dwIDNStringLen, lpIDNString);
        } else {
            printf("Failed to receive identity string (%u)\n", e);
        }
    }

    e = lpSSG3021X->vtbl->disconnect(lpSSG3021X);
    if(e != labE_Ok) {
        printf("Disconnection failed, terminating anyways (%u)\n", e);
    }

    return 0;
}

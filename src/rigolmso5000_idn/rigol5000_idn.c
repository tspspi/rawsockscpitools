#include <stdio.h>
#include <stdlib.h>

#include "../../include/labtypes.h"
#include "../../include/rigolmso5000.h"

static void printUsage(int argc, char* argv[]) {
    printf("Usage:\n\n");
    printf("\t%s ADDRESS\n\n", argv[0]);
}

int main(int argc, char* argv[]) {
    enum labError e;
    struct rigolMso5000* lpMSO5000;

    if(argc < 2) { printUsage(argc, argv); return 1; }

    /* First connect to the oscilloscope */
    e = rigolMso5000Connect(&lpMSO5000, argv[1]);
    if(e != labE_Ok) {
        printf("Failed to connect\n");
        return 2;
    }

    {
        char* lpIDNString;
        unsigned long int dwIDNStringLen;
        e = lpMSO5000->vtbl->idn(lpMSO5000, &lpIDNString, &dwIDNStringLen);

        if(e == labE_Ok) {
            printf("%.*s\n", (int)dwIDNStringLen, lpIDNString);
        } else {
            printf("Failed to receive identity string (%u)\n", e);
        }
    }

    e = lpMSO5000->vtbl->disconnect(lpMSO5000);
    if(e != labE_Ok) {
        printf("Disconnection failed, terminating anyways (%u)\n", e);
    }

    return 0;
}

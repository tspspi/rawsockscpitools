#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "../../include/labtypes.h"
#include "../../include/siglent_ssg3021x.h"

static void printUsage(int argc, char* argv[]) {
    printf("Usage:\n\n");
    printf("\t%s ADDRESS STATE\n\nSTATE\n\nEither ON or OFF to enable or disable the RF output", argv[0]);
}

int main(int argc, char* argv[]) {
    enum labError e;
    struct siglentSSG3021x* lpSSG3021X;
    bool bOnOff;

    if(argc < 3) { printUsage(argc, argv); return 1; }

    {
        unsigned long int i;

        for(i = 0; i < strlen(argv[2]); i=i+1) {
            argv[2][i] = tolower(argv[2][i]);
        }

        if(strcmp(argv[2], "on") == 0) { bOnOff = true; }
        else if(strcmp(argv[2], "off") == 0) { bOnOff = false; }
        else {
            printf("Unknown state %s\n", argv[2]);
            return 1;
        }
    }

    /* First connect to the oscilloscope */
    e = siglentSSG3021xConnect(&lpSSG3021X, argv[1]);
    if(e != labE_Ok) {
        printf("Failed to connect\n");
        return 2;
    }

    lpSSG3021X->vtbl->rfOutEnable(lpSSG3021X, bOnOff);

    e = lpSSG3021X->vtbl->disconnect(lpSSG3021X);
    if(e != labE_Ok) {
        printf("Disconnection failed, terminating anyways (%u)\n", e);
    }

    return 0;
}

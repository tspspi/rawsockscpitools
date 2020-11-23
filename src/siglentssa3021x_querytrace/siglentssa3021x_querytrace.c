#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "../../include/labtypes.h"
#include "../../include/siglent_ssa3021x.h"

static void printUsage(int argc, char* argv[]) {
    printf("Usage:\n\n");
    printf("\t%s ADDRESS\n\n", argv[0]);
}

int main(int argc, char* argv[]) {
    enum labError e;
    struct siglentSSA3021x* lpSSA3021X;
    struct spectrumDataTrace* lpTrace;
    unsigned long int i;

    if(argc < 2) { printUsage(argc, argv); return 1; }

    e = siglentSSA3021xConnect(&lpSSA3021X, argv[1]);
    if(e != labE_Ok) {
        printf("Failed to connect\n");
        return 2;
    }

    e = lpSSA3021X->vtbl->queryTraceData(lpSSA3021X, &lpTrace);
    if(e != labE_Ok) {
        printf("Failed to query trace data (%u)\n", e);
        lpSSA3021X->vtbl->disconnect(lpSSA3021X);
        return 3;
    }

    /* Output trace data ... */
    printf("# Start frequency: %lf\n", lpTrace->frqStart);
    printf("# Stop frequency: %lf\n", lpTrace->frqEnd);
    printf("# Step size: %lf\n", lpTrace->frqStep);
    printf("# Center frequency: %lf\n", lpTrace->frqCenter);
    printf("# Points: %lu\n", lpTrace->dwDataPoints);
    for(i = 0; i < lpTrace->dwDataPoints; i=i+1) {
        printf("%lf\t%lf\n", lpTrace->data[i].frq, lpTrace->data[i].value);
    }


    e = lpSSA3021X->vtbl->disconnect(lpSSA3021X);
    if(e != labE_Ok) {
        printf("Disconnection failed, terminating anyways (%u)\n", e);
    }

    return 0;
}

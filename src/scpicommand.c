#ifndef BUFFER_JUNK_SIZE
    #define BUFFER_JUNK_SIZE 4096
#endif

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#ifdef DEBUG
    #include <stdio.h>
#endif

#include "../include/labtypes.h"

enum labError labScpiCommand_NoReply(
    int hSocket,
    char* lpCommand,
    unsigned long int dwCommandLen
) {
    ssize_t sentLength;
    unsigned long int dwLenToGo;

    if(hSocket < 0) { return labE_InvalidParam; }
    if((lpCommand == NULL) || (dwCommandLen == 0)) { return labE_Ok; } /* No-operation */

    dwLenToGo = dwCommandLen;
    while(dwLenToGo > 0) {
        sentLength = send(hSocket, &(lpCommand[dwCommandLen-dwLenToGo]), dwLenToGo, 0);
        if(sentLength < 0) {
            return labE_Failed;
        }
        dwLenToGo = dwLenToGo - sentLength;
    }

    return labE_Ok;
}

enum labError labScpiCommand(
    int hSocket,
    char* lpCommand,
    unsigned long int dwCommandLen,

    char** lpReceivedOut,
    unsigned long int* lpReceivedLen
) {
    ssize_t sentLength;
    ssize_t recvLength;
    char* lpRes;
    unsigned long int dwSentCommand;
    unsigned long int dwResSize, dwResUsed;

    if(lpReceivedOut != NULL) { (*lpReceivedOut) = NULL; }
    if(lpReceivedLen != NULL) { (*lpReceivedLen) = 0; }

    if((lpReceivedOut == NULL) || (lpCommand == NULL) || (dwCommandLen == 0) || (hSocket < 0)) { return labE_InvalidParam; }


    /* Send the single command ... */
    dwSentCommand = 0;
    while(dwSentCommand < dwCommandLen) {
        sentLength = send(hSocket, &(lpCommand[dwSentCommand]), dwCommandLen-dwSentCommand, 0);
        if(sentLength < 0) {
            return labE_Failed;
        }
        dwSentCommand = dwSentCommand + sentLength;
    }

    /*
        Transmitted command, now read reply till end of line ...

        Currently we assume that the end of line will be received at the end
        of an TCP packet.
    */
    lpRes = malloc(BUFFER_JUNK_SIZE);
    dwResSize = BUFFER_JUNK_SIZE;
    dwResUsed = 0;

    for(;;) {
        /* Grow buffer if required ... */
        if(dwResSize-dwResUsed == 0) {
            lpRes = realloc(lpRes, BUFFER_JUNK_SIZE + dwResSize);
            memset(&(lpRes[dwResSize]), 0, BUFFER_JUNK_SIZE);
            dwResSize = dwResSize + BUFFER_JUNK_SIZE;
        }

        recvLength = recv(hSocket, &(lpRes[dwResUsed]), dwResSize - dwResUsed, 0);
        if(recvLength <= 0) {
            /* Error ... */
            free(lpRes);
            return labE_Failed;
        }
        dwResUsed = dwResUsed + recvLength;
        if(lpRes[dwResUsed-1] == '\n') {
            if(lpReceivedLen != NULL) { (*lpReceivedLen) = dwResUsed; }
            lpRes[dwResUsed-1] = 0x00;

            (*lpReceivedOut) = lpRes;
            return labE_Ok;
        }
    }
}

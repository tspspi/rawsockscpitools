#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "../include/labtypes.h"
#include "../include/siglent_ssg3021x.h"

struct siglentSSG3021xImpl {
    struct siglentSSG3021x              objGenerator;

    int hSocket;
    int adrFamily;
    union {
        struct sockaddr_in sin;
        struct sockaddr_in6 sin6;
    } adr;
};

static enum labError siglentSSG3021xImpl__Disconnect(
    struct siglentSSG3021x* lpDevice
) {
    struct siglentSSG3021xImpl* lpThis;

    if(lpDevice == NULL) { return labE_InvalidParam; }
    lpThis = (struct siglentSSG3021xImpl*)(lpDevice->lpReserved);

    close(lpThis->hSocket);
    free(lpThis);

    return labE_Ok;
}

static char* siglentSSG3021xImpl__IDN__SCPI_IDN = "*IDN?\n";
static enum labError siglentSSG3021xImpl__IDN(
    struct siglentSSG3021x* lpDevice,

    char** lpResultOut,
    unsigned long int* lpResultLenOut
) {
    enum labError e;
    struct siglentSSG3021xImpl* lpThis;

    if(lpResultOut != NULL) { (*lpResultOut) = NULL; }
    if(lpResultLenOut != NULL) { (*lpResultLenOut) = 0; }

    if((lpDevice == NULL) || (lpResultOut == NULL)) { return labE_InvalidParam; }
    lpThis = (struct siglentSSG3021xImpl*)(lpDevice->lpReserved);

    e = labScpiCommand(lpThis->hSocket, siglentSSG3021xImpl__IDN__SCPI_IDN, strlen(siglentSSG3021xImpl__IDN__SCPI_IDN), lpResultOut, lpResultLenOut);

    return e;
}

static enum labError siglentSSG3021xImpl___RFOutputEnable(
    struct siglentSSG3021x* lpDevice,
    bool bEnable
) {
    enum labError e;
    struct siglentSSG3021xImpl* lpThis;
    char buffer[128];

    if(lpDevice == NULL) { return labE_InvalidParam; }
    lpThis = (struct siglentSSG3021xImpl*)(lpDevice->lpReserved);

    if(bEnable != false) {
        sprintf(buffer, ":OUTP ON\n");
    } else {
        sprintf(buffer, ":OUTP OFF\n");
    }

    e = labScpiCommand_NoReply(lpThis->hSocket, buffer, strlen(buffer));
    return e;
}



static struct siglentSSG3021x_Vtbl siglentSSG3021xImpl__DefaultVTBL = {
    siglentSSG3021xImpl__Disconnect,
    &siglentSSG3021xImpl__IDN,

    &siglentSSG3021xImpl___RFOutputEnable
};


static char* siglentSSG3021xConnect__Signature = "Siglent Technologies,SSG3021X,";

enum labError siglentSSG3021xConnect(
    struct siglentSSG3021x** lpOut,
    char* lpAddress
) {
    struct siglentSSG3021xImpl* lpDev;
    struct hostent* dnsResult;
    char readableAddress[128];
    enum labError e;

    /* Input parameter validation */
    if(lpOut == NULL) { return labE_InvalidParam; }
    (*lpOut) = NULL;

    if(lpAddress == NULL) { return labE_InvalidParam; }

    /* Allocate and initialize structure */
    lpDev = (struct siglentSSG3021xImpl*)malloc(sizeof(struct siglentSSG3021xImpl));
    if(lpDev == NULL) { return labE_OutOfMemory; }

    /*
        Convert string to network address:
            * Try IPv6 address
            * Try IPv4 address
            * Try DNS lookup
    */
    for(;;) {
        /* IPv6 */
        lpDev->adr.sin6.sin6_len = sizeof(struct sockaddr_in6);
        lpDev->adr.sin6.sin6_family = AF_INET6;
        if(inet_pton(AF_INET6, lpAddress, &(lpDev->adr.sin6.sin6_addr)) == 1) {
            lpDev->adrFamily = AF_INET6;
            #ifdef DEBUG
                printf("Connecting to IPv6 address %s\n", lpAddress);
            #endif
            break;
        }

        /* IPv4 */
        lpDev->adr.sin.sin_len = sizeof(struct sockaddr_in);
        lpDev->adr.sin.sin_family = AF_INET;
        if(inet_pton(AF_INET, lpAddress, &(lpDev->adr.sin.sin_addr)) == 1) {
            lpDev->adrFamily = AF_INET;
            #ifdef DEBUG
                printf("Connecting to IPv4 address %s\n", lpAddress);
            #endif
            break;
        }

        /* Hostent (DNS query) */
        #ifdef DEBUG
            printf("Looking up %s (IPv6): ", lpAddress);
        #endif
        if((dnsResult = gethostbyname2(lpAddress, AF_INET6)) != NULL) {
            if(dnsResult->h_length < 1) {
                free(dnsResult);
            } else {
                inet_ntop(
                    AF_INET6,
                    ((struct in_addr**)(dnsResult->h_addr_list))[0],
                    readableAddress,
                    sizeof(readableAddress)/sizeof(char)
                );
                lpDev->adrFamily = AF_INET6;
                memcpy(&(lpDev->adr.sin6), ((struct in_addr**)(dnsResult->h_addr_list))[0], sizeof(lpDev->adr.sin6));
                lpDev->adr.sin6.sin6_len = sizeof(struct sockaddr_in6);
                #ifdef DEBUG
                    printf("%s\n", readableAddress);
                #endif
                break;
            }
        }
        #ifdef DEBUG
            printf("failed\n");
        #endif

        if((dnsResult = gethostbyname2(lpAddress, AF_INET)) != NULL) {
            if(dnsResult->h_length < 1) {
                free(dnsResult);
            } else {
                inet_ntop(
                    AF_INET,
                    ((struct in_addr**)(dnsResult->h_addr_list))[0],
                    readableAddress,
                    sizeof(readableAddress)/sizeof(char)
                );
                lpDev->adrFamily = AF_INET;
                memcpy(&(lpDev->adr.sin), ((struct in_addr**)(dnsResult->h_addr_list))[0], sizeof(lpDev->adr.sin));
                lpDev->adr.sin.sin_len = sizeof(struct sockaddr_in);
                #ifdef DEBUG
                    printf("%s\n", readableAddress);
                #endif
                break;
            }
        }
        #ifdef DEBUG
            printf("failed\n");

            printf("Address %s not resolveable ...\n", lpAddress);
        #endif
        free(lpDev);
        return labE_AddressNotResolvable;
    }

    /*
        We will now connect with the given address in an synchronous fashion
    */
    #ifdef DEBUG
        printf("Connecting ... ");
    #endif

    switch(lpDev->adrFamily) {
        case AF_INET:
            lpDev->hSocket = socket(PF_INET, SOCK_STREAM, 0);
            if(lpDev->hSocket == -1) {
                #ifdef DEBUG
                    printf("failed\n");
                #endif
                free(lpDev);
                return labE_Failed;
            }
            lpDev->adr.sin.sin_port = htons(5025);
            break;
        case AF_INET6:
            lpDev->hSocket = socket(PF_INET6, SOCK_STREAM, 0);
            if(lpDev->hSocket == -1) {
                #ifdef DEBUG
                    printf("failed\n");
                #endif
                free(lpDev);
                return labE_Failed;
            }
            lpDev->adr.sin6.sin6_port = htons(5025);
            break;
        default:
            free(lpDev);
            return labE_Failed;
    }

    if(connect(lpDev->hSocket, (struct sockaddr*)(&(lpDev->adr.sin)), lpDev->adr.sin.sin_len) == -1) {
        close(lpDev->hSocket);
        free(lpDev);
        #ifdef DEBUG
            printf("failed\n");
            printf("%s\n", strerror(errno));
        #endif
        return labE_ConnectionFailed;
    }

    #ifdef DEBUG
        printf("connected\n");
    #endif

    lpDev->objGenerator.lpReserved = (void*)lpDev;
    lpDev->objGenerator.vtbl = &siglentSSG3021xImpl__DefaultVTBL;

    (*lpOut) = &(lpDev->objGenerator);


    /*
        Now perform an IDN command and verify that we are really one of the
        known MSO5000 devices
    */
    {
        char* lpIDNString;
        unsigned long int dwIDNStringLen;

        e = lpDev->objGenerator.vtbl->idn(&(lpDev->objGenerator), &lpIDNString, &dwIDNStringLen);
        if(e != labE_Ok) {
            close(lpDev->hSocket);
            free(lpDev);
            return e;
        }

        /* Now do a signature match */

        for(;;) {
            if(strncmp(siglentSSG3021xConnect__Signature, lpIDNString, strlen(siglentSSG3021xConnect__Signature)) == 0) {
                break;
            }

            /* Unknown */
            close(lpDev->hSocket);
            free(lpDev);
            return labE_UnknownDevice;
        }
    }

    return labE_Ok;
}

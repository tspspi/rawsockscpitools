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
#include "../include/rigolmso5000.h"

enum mso5000DeviceFamily {
    mso5000DeviceFamily_MSO5072
};

struct mso5000DeviceImpl {
    struct rigolMso5000 objMso5000;

    enum mso5000DeviceFamily devType;

    int hSocket;
    int adrFamily;
    union {
        struct sockaddr_in sin;
        struct sockaddr_in6 sin6;
    } adr;
};

static char* mso5000_SCPI__IDN = "*IDN?\n";

static enum labError (mso5000DeviceImpl__Disconnect)(
    struct rigolMso5000* lpDevice
) {
    struct mso5000DeviceImpl* lpThis;

    if(lpDevice == NULL) { return labE_InvalidParam; }

    lpThis = (struct mso5000DeviceImpl*)(lpDevice->lpReserved);

    close(lpThis->hSocket);
    free(lpThis);

    return labE_Ok;
}

static enum labError (mso5000DeviceImpl__IDN)(
    struct rigolMso5000* lpDevice,

    char** lpResultOut,
    unsigned long int* lpResultLenOut
) {
    enum labError e;
    struct mso5000DeviceImpl* lpThis;

    if(lpDevice == NULL) { return labE_InvalidParam; }

    if(lpResultOut == NULL) { return labE_InvalidParam; }
    (*lpResultOut) = NULL;

    lpThis = (struct mso5000DeviceImpl*)(lpDevice->lpReserved);

    e = labScpiCommand(lpThis->hSocket, mso5000_SCPI__IDN, strlen(mso5000_SCPI__IDN), lpResultOut, lpResultLenOut);

    return e;
}


static struct rigolMso5000_Vtbl rigolMso5000_DefaultVTBL = {
    &mso5000DeviceImpl__Disconnect,
    &mso5000DeviceImpl__IDN,
    NULL,
    NULL
};

static char* rigolMso5000__Signature_MSO5072 = "RIGOL TECHNOLOGIES,MSO5072,";

enum labError rigolMso5000Connect(
    struct rigolMso5000** lpOut,
    char* lpAddress
) {
    struct mso5000DeviceImpl* lpDev;
    struct hostent* dnsResult;
    char readableAddress[128];
    enum labError e;

    /* Input parameter validation */
    if(lpOut == NULL) { return labE_InvalidParam; }
    (*lpOut) = NULL;

    if(lpAddress == NULL) { return labE_InvalidParam; }

    /* Allocate and initialize structure */
    lpDev = (struct mso5000DeviceImpl*)malloc(sizeof(struct mso5000DeviceImpl));
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
            lpDev->adr.sin.sin_port = htons(5555);
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
            lpDev->adr.sin6.sin6_port = htons(5555);
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

    lpDev->objMso5000.lpReserved = (void*)lpDev;
    lpDev->objMso5000.vtbl = &rigolMso5000_DefaultVTBL;

    (*lpOut) = &(lpDev->objMso5000);


    /*
        Now perform an IDN command and verify that we are really one of the
        known MSO5000 devices
    */
    {
        char* lpIDNString;
        unsigned long int dwIDNStringLen;

        e = lpDev->objMso5000.vtbl->idn(&(lpDev->objMso5000), &lpIDNString, &dwIDNStringLen);
        if(e != labE_Ok) {
            close(lpDev->hSocket);
            free(lpDev);
            return e;
        }

        /* Now do a signature match */
        for(;;) {
            if(strncmp(rigolMso5000__Signature_MSO5072, lpIDNString, strlen(rigolMso5000__Signature_MSO5072)) == 0) {
                /* We are an MSO5072 */
                lpDev->devType = mso5000DeviceFamily_MSO5072;

                /* Extract the serial number - ToDo */
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

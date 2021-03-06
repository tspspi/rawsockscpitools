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
#include "../include/siglent_ssa3021x.h"

struct siglentSSA3021xImpl {
    struct siglentSSA3021x              objAnalyzer;

    int hSocket;
    int adrFamily;
    union {
        struct sockaddr_in sin;
        struct sockaddr_in6 sin6;
    } adr;
};

static enum labError siglentSSA3021xImpl__Disconnect(
    struct siglentSSA3021x* lpDevice
) {
    struct siglentSSA3021xImpl* lpThis;

    if(lpDevice == NULL) { return labE_InvalidParam; }
    lpThis = (struct siglentSSA3021xImpl*)(lpDevice->lpReserved);

    close(lpThis->hSocket);
    free(lpThis);

    return labE_Ok;
}

static char* siglentSSA3021xImpl__IDN__SCPI_IDN = "*IDN?\n";
static enum labError siglentSSA3021xImpl__IDN(
    struct siglentSSA3021x* lpDevice,

    char** lpResultOut,
    unsigned long int* lpResultLenOut
) {
    enum labError e;
    struct siglentSSA3021xImpl* lpThis;

    if(lpResultOut != NULL) { (*lpResultOut) = NULL; }
    if(lpResultLenOut != NULL) { (*lpResultLenOut) = 0; }

    if((lpDevice == NULL) || (lpResultOut == NULL)) { return labE_InvalidParam; }
    lpThis = (struct siglentSSA3021xImpl*)(lpDevice->lpReserved);

    e = labScpiCommand(lpThis->hSocket, siglentSSA3021xImpl__IDN__SCPI_IDN, strlen(siglentSSA3021xImpl__IDN__SCPI_IDN), lpResultOut, lpResultLenOut);

    return e;
}

static enum labError siglentSSA3021xImpl__SetFrequencyCenter(
    struct siglentSSA3021x* lpDevice,
    double dFrequencyHz
) {
    enum labError e;
    struct siglentSSA3021xImpl* lpThis;
    char buffer[128];

    if(lpDevice == NULL) { return labE_InvalidParam; }
    lpThis = (struct siglentSSA3021xImpl*)(lpDevice->lpReserved);

    if(dFrequencyHz < 0) { return labE_InvalidParam; } /* Support down to DC ... (ToDo) */
    if(dFrequencyHz > 2100000000) { return labE_InvalidParam; } /* Clamp at 2.1 GHz */

    sprintf(buffer, ":FREQ:CENT %lf MHz\n", dFrequencyHz/1000000.0);
    e = labScpiCommand_NoReply(lpThis->hSocket, buffer, strlen(buffer));
    return e;
}
static enum labError siglentSSA3021xImpl__SetSpan(
    struct siglentSSA3021x* lpDevice,
    double dSpanHz
) {
    enum labError e;
    struct siglentSSA3021xImpl* lpThis;
    char buffer[128];

    if(lpDevice == NULL) { return labE_InvalidParam; }
    lpThis = (struct siglentSSA3021xImpl*)(lpDevice->lpReserved);

    if(dSpanHz < 0) { return labE_InvalidParam; } /* Support down to DC ... (ToDo) */
    if(dSpanHz > 2100000000) { return labE_InvalidParam; } /* Clamp at 2.1 GHz */

    sprintf(buffer, ":FREQ:SPAN %lf Hz\n", dSpanHz);
    e = labScpiCommand_NoReply(lpThis->hSocket, buffer, strlen(buffer));
    return e;
}
static char* siglentSSA3021xImpl__SetAverageCount__MODE_WRITE = ":TRAC1:MODE WRIT\n";
static char* siglentSSA3021xImpl__SetAverageCount__MODE_AVERAGE = ":TRAC1:MODE AVER\n";
static char* siglentSSA3021xImpl__SetAverageCount__AVG_CLEAR = ":AVER:TRAC1:CLE\n";
static enum labError siglentSSA3021xImpl__SetAverageCount(
    struct siglentSSA3021x* lpDevice,
    unsigned long int dwCount
) {
    enum labError e;
    struct siglentSSA3021xImpl* lpThis;
    char buffer[128];

    if(lpDevice == NULL) { return labE_InvalidParam; }
    lpThis = (struct siglentSSA3021xImpl*)(lpDevice->lpReserved);

    if(dwCount < 1) { return labE_InvalidParam; }
    if(dwCount > 999) { return labE_InvalidParam; }

    if(dwCount == 1) {
        /* Set trace mode to normal ... disables averaging */
        if((e = labScpiCommand_NoReply(lpThis->hSocket, siglentSSA3021xImpl__SetAverageCount__MODE_WRITE, strlen(siglentSSA3021xImpl__SetAverageCount__MODE_WRITE))) != labE_Ok) { return e; }
    } else {
        if((e = labScpiCommand_NoReply(lpThis->hSocket, siglentSSA3021xImpl__SetAverageCount__MODE_AVERAGE, strlen(siglentSSA3021xImpl__SetAverageCount__MODE_AVERAGE))) != labE_Ok) { return e; }
        sprintf(buffer, ":AVER:TRAC1:COUN %lu\n", dwCount);
        if((e = labScpiCommand_NoReply(lpThis->hSocket, buffer, strlen(buffer))) != labE_Ok) { return e; }
        if((e = labScpiCommand_NoReply(lpThis->hSocket, siglentSSA3021xImpl__SetAverageCount__AVG_CLEAR, strlen(siglentSSA3021xImpl__SetAverageCount__AVG_CLEAR))) != labE_Ok) { return e; }
    }

    return labE_Ok;
}

static char* siglentSSA3021xImpl__QueryTraceData__SCPI_StartFreq = ":FREQ:STAR?\n";
static char* siglentSSA3021xImpl__QueryTraceData__SCPI_StopFreq = ":FREQ:STOP?\n";
static char* siglentSSA3021xImpl__QueryTraceData__SCPI_QueryTrace = ":TRAC:DATA? 1\n";

static enum labError siglentSSA3021xImpl__QueryTraceData(
    struct siglentSSA3021x* lpDevice,
    struct spectrumDataTrace** lpTraceOut
) {
    enum labError e;
    struct siglentSSA3021xImpl* lpThis;
    struct spectrumDataTrace* lpResult;

    unsigned long int i;
    unsigned long int idxMeasurement;

    char* lpRecvTemp;
    unsigned long int dwRecvTemp;

    double dStartFreq;
    double dStopFreq;
    double dFrqStep;
    unsigned long int measCount;

    if(lpTraceOut != NULL) { (*lpTraceOut) = NULL; }

    if((lpDevice == NULL) || (lpTraceOut == NULL)) { return labE_InvalidParam; }
    lpThis = (struct siglentSSA3021xImpl*)(lpDevice->lpReserved);

    /* Query required parameters ... */
    if((e = labScpiCommand(lpThis->hSocket, siglentSSA3021xImpl__QueryTraceData__SCPI_StartFreq, strlen(siglentSSA3021xImpl__QueryTraceData__SCPI_StartFreq), &lpRecvTemp, &dwRecvTemp)) != labE_Ok) { return e; }
    if(sscanf(lpRecvTemp, "%lf", &dStartFreq) != 1) { free(lpRecvTemp); return labE_Failed; } else { free(lpRecvTemp); }
    if((e = labScpiCommand(lpThis->hSocket, siglentSSA3021xImpl__QueryTraceData__SCPI_StopFreq, strlen(siglentSSA3021xImpl__QueryTraceData__SCPI_StopFreq), &lpRecvTemp, &dwRecvTemp)) != labE_Ok) { return e; }
    if(sscanf(lpRecvTemp, "%lf", &dStopFreq) != 1) { free(lpRecvTemp); return labE_Failed; } else { free(lpRecvTemp); }

    if((e = labScpiCommand(lpThis->hSocket, siglentSSA3021xImpl__QueryTraceData__SCPI_QueryTrace, strlen(siglentSSA3021xImpl__QueryTraceData__SCPI_QueryTrace), &lpRecvTemp, &dwRecvTemp)) != labE_Ok) { return e; }

    /*
        Parse trace data by simply using sscanf
        replace , by 0; scan after every 0 till the end ...
    */
    measCount = 0;
    for(i = 0; i < dwRecvTemp; i=i+1) {
        if(lpRecvTemp[i] == ',') {
            lpRecvTemp[i] = 0x00;
            measCount = measCount + 1;
        }
    }

    dFrqStep = (dStopFreq-dStartFreq)/(measCount-1);

    lpResult = (struct spectrumDataTrace*)malloc(sizeof(struct spectrumDataTrace) + sizeof(((struct spectrumDataTrace*)0)->data[0])*measCount);
    if(lpResult == NULL) {
        free(lpRecvTemp);
        return labE_OutOfMemory;
    }

    lpResult->dwDataPoints = measCount;
    lpResult->frqStep = dFrqStep;
    lpResult->frqStart = dStartFreq;
    lpResult->frqEnd = dStopFreq;
    lpResult->frqCenter = (dStartFreq + dStopFreq) / 2.0;

    idxMeasurement = 0;
    sscanf(lpRecvTemp, "%lf", &(lpResult->data[idxMeasurement].value));
    lpResult->data[idxMeasurement].frq = dStartFreq;
    idxMeasurement = idxMeasurement + 1;

    for(i = 0; i < dwRecvTemp; i=i+1) {
        if((lpRecvTemp[i] == 0x00) && ((dwRecvTemp-i) > 2)) {
            sscanf(&(lpRecvTemp[i+1]), "%lf", &(lpResult->data[idxMeasurement].value));
            lpResult->data[idxMeasurement].frq = dStartFreq + i * dFrqStep;
            idxMeasurement = idxMeasurement + 1;
        }
    }

    free(lpRecvTemp);
    (*lpTraceOut) = lpResult;
    return labE_Ok;
}


static struct siglentSSA3021x_Vtbl siglentSSA3021xImpl__DefaultVTBL = {
    &siglentSSA3021xImpl__Disconnect,
    &siglentSSA3021xImpl__IDN,

    &siglentSSA3021xImpl__SetFrequencyCenter,
    &siglentSSA3021xImpl__SetSpan,
    &siglentSSA3021xImpl__SetAverageCount,

    &siglentSSA3021xImpl__QueryTraceData
};

static char* siglentSSA3021xConnect__Signature = "Siglent Technologies,SSA3021X,";

enum labError siglentSSA3021xConnect(
    struct siglentSSA3021x** lpOut,
    char* lpAddress
) {
    struct siglentSSA3021xImpl* lpDev;
    struct hostent* dnsResult;
    char readableAddress[128];
    enum labError e;

    /* Input parameter validation */
    if(lpOut == NULL) { return labE_InvalidParam; }
    (*lpOut) = NULL;

    if(lpAddress == NULL) { return labE_InvalidParam; }

    /* Allocate and initialize structure */
    lpDev = (struct siglentSSA3021xImpl*)malloc(sizeof(struct siglentSSA3021xImpl));
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

    lpDev->objAnalyzer.lpReserved = (void*)lpDev;
    lpDev->objAnalyzer.vtbl = &siglentSSA3021xImpl__DefaultVTBL;

    (*lpOut) = &(lpDev->objAnalyzer);


    /*
        Now perform an IDN command and verify that we are really one of the
        known MSO5000 devices
    */
    {
        char* lpIDNString;
        unsigned long int dwIDNStringLen;

        e = lpDev->objAnalyzer.vtbl->idn(&(lpDev->objAnalyzer), &lpIDNString, &dwIDNStringLen);
        if(e != labE_Ok) {
            close(lpDev->hSocket);
            free(lpDev);
            return e;
        }

        /* Now do a signature match */

        for(;;) {
            if(strncmp(siglentSSA3021xConnect__Signature, lpIDNString, strlen(siglentSSA3021xConnect__Signature)) == 0) {
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

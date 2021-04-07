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


static enum labError (mso5000DeviceImpl__SetChannelEnable)(
    struct rigolMso5000* lpDevice,

    unsigned long int dwChannel,
    bool bEnabled
) {
    enum labError e;
    struct mso5000DeviceImpl* lpThis;
    char buffer[128];

    if(lpDevice == NULL) { return labE_InvalidParam; }

    lpThis = (struct mso5000DeviceImpl*)(lpDevice->lpReserved);

    switch(lpThis->devType) {
        case mso5000DeviceFamily_MSO5072:
            if(dwChannel > 4) {
                return labE_InvalidParam;
            }
            break;
        default:
            return labE_NotImplemented;
    }

    sprintf(buffer, ":CHAN%lu:DISP %s\n", dwChannel, ((bEnabled == true) ? "ON" : "OFF"));

    e = labScpiCommand_NoReply(lpThis->hSocket, buffer, strlen(buffer));

    return e;
}

static char* mso5000DeviceImpl__QueryWaveform__SCPICmd_WavModeNorm = ":WAV:MODE NORM\n";
static char* mso5000DeviceImpl__QueryWaveform__SCPICmd_WavFormAscii = ":WAV:FORM ASCii\n";
static char* mso5000DeviceImpl__QueryWaveform__SCPICmd_WavData = ":WAV:DATA?\n";

static enum labError (mso5000DeviceImpl__QueryWaveform)(
    struct rigolMso5000* lpDevice,

    unsigned long int dwChannel,
    struct oscilloscopeWaveformData** lpWaveformOut
) {
    enum labError e;
    struct mso5000DeviceImpl* lpThis;

    unsigned long int dwASCIILen;
    unsigned long int i;
    unsigned long int measCount;
    unsigned long int idxMeasurement;
    char* lpASCIIData;
    struct oscilloscopeWaveformData* lpTrace;
    char bCommand[128];

    if(lpWaveformOut != NULL) { (*lpWaveformOut) = NULL; }

    if(lpDevice == NULL) { return labE_InvalidParam; }
    if(lpWaveformOut == NULL) { return labE_InvalidParam; }

    lpThis = (struct mso5000DeviceImpl*)(lpDevice->lpReserved);

    switch(lpThis->devType) {
        case mso5000DeviceFamily_MSO5072:
            if(dwChannel > 4) {
                return labE_InvalidParam;
            }
            break;
        default:
            return labE_NotImplemented;
    }

    /*
        First we set the waveform source to the desired channel
    */
    sprintf(bCommand, ":WAV:SOUR CHAN%lu\n", dwChannel);
    if((e = labScpiCommand_NoReply(lpThis->hSocket, bCommand, strlen(bCommand))) != labE_Ok) { return e; }

    /*
        Set waveform mode to normal and ASCII (ToDo: Binary would be better)
    */
    if((e = labScpiCommand_NoReply(lpThis->hSocket, mso5000DeviceImpl__QueryWaveform__SCPICmd_WavModeNorm, strlen(mso5000DeviceImpl__QueryWaveform__SCPICmd_WavModeNorm))) != labE_Ok) { return e; }
    if((e = labScpiCommand_NoReply(lpThis->hSocket, mso5000DeviceImpl__QueryWaveform__SCPICmd_WavFormAscii, strlen(mso5000DeviceImpl__QueryWaveform__SCPICmd_WavFormAscii))) != labE_Ok) { return e; }

    /*
        Now receive waveform data
    */
    if((e = labScpiCommand(lpThis->hSocket, mso5000DeviceImpl__QueryWaveform__SCPICmd_WavData, strlen(mso5000DeviceImpl__QueryWaveform__SCPICmd_WavData), &lpASCIIData, &dwASCIILen)) != labE_Ok) { return e; }

    /*
        Parse trace data by simply using sscanf
        replace , by 0; scan after every 0 till the end ...
    */
    measCount = 0;
    for(i = 11; i < dwASCIILen; i=i+1) {
        if(lpASCIIData[i] == ',') {
            lpASCIIData[i] = 0x00;
            measCount = measCount + 1;
        }
    }

    lpTrace = (struct oscilloscopeWaveformData*)malloc(sizeof(struct oscilloscopeWaveformData) + sizeof(double)*measCount);
    lpTrace->dwDataPoints = measCount;

    idxMeasurement = 0;
    for(i = 10; i < dwASCIILen; i=i+1) {
        if((lpASCIIData[i] == 0x00) && ((dwASCIILen-i) > 2)) {
            sscanf(&(lpASCIIData[i+1]), "%lf", &(lpTrace->dData[idxMeasurement]));
            idxMeasurement = idxMeasurement + 1;
        }
    }

    free(lpASCIIData);

    (*lpWaveformOut) = lpTrace;
    return labE_Ok;
}

static char* mso5000_SCPI__TFORce = ":TFOR\n";

static enum labError (mso5000DeviceImpl__TriggerForce)(
    struct rigolMso5000* lpDevice
) {
    enum labError e;
    struct mso5000DeviceImpl* lpThis;

    if(lpDevice == NULL) { return labE_InvalidParam; }

    lpThis = (struct mso5000DeviceImpl*)(lpDevice->lpReserved);

    if((e = labScpiCommand_NoReply(lpThis->hSocket, mso5000_SCPI__TFORce, strlen(mso5000_SCPI__TFORce))) != labE_Ok) { return e; }

    return e;
}

static char* mso5000_SCPI__TSINGLE = ":SING\n";

static enum labError (mso5000DeviceImpl__TriggerSingle)(
    struct rigolMso5000* lpDevice
) {
    enum labError e;
    struct mso5000DeviceImpl* lpThis;

    if(lpDevice == NULL) { return labE_InvalidParam; }

    lpThis = (struct mso5000DeviceImpl*)(lpDevice->lpReserved);

    if((e = labScpiCommand_NoReply(lpThis->hSocket, mso5000_SCPI__TSINGLE, strlen(mso5000_SCPI__TSINGLE))) != labE_Ok) { return e; }

    return e;
}

static struct rigolMso5000_Vtbl rigolMso5000_DefaultVTBL = {
    &mso5000DeviceImpl__Disconnect,
    &mso5000DeviceImpl__IDN,
    &mso5000DeviceImpl__SetChannelEnable,
    &mso5000DeviceImpl__QueryWaveform,

    &mso5000DeviceImpl__TriggerForce,
    &mso5000DeviceImpl__TriggerSingle
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

#ifndef __is_included__874757e8_2d6e_11eb_89c2_b499badf00a1
#define __is_included__874757e8_2d6e_11eb_89c2_b499badf00a1 1

#ifdef __cplusplus
    extern "C" {
#endif

#ifndef __cplusplus
    #ifndef true
        #define true 1
        #define false 0
        typedef int bool;
    #endif
#endif

enum labError {
    labE_Ok                         = 0,

    labE_Failed,
    labE_ConnectionFailed,
    labE_InvalidParam,
    labE_OutOfMemory,

    labE_AddressNotResolvable,

    labE_UnknownDevice,
};


struct oscilloscopeWaveformData {
    unsigned long int dwDataPoints;
    double dData[];
};



/*
    SCPI helper functions used by the libraries themselves

    No public API!
*/

enum labError labScpiCommand_NoReply(
    int hSocket,
    char* lpCommand,
    unsigned long int dwCommandLen
);

enum labError labScpiCommand(
    int hSocket,
    char* lpCommand,
    unsigned long int dwCommandLen,

    char** lpReceivedOut,
    unsigned long int* lpReceivedLen
);

#ifdef __cplusplus
    } /* extern "C" { */
#endif


#endif /* #ifndef __is_included__874757e8_2d6e_11eb_89c2_b499badf00a1 */

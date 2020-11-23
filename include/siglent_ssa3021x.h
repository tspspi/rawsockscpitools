#ifndef __is_included__a0d9ddd4_2d85_11eb_89c2_b499badf00a1
#define __is_included__a0d9ddd4_2d85_11eb_89c2_b499badf00a1 1

#ifdef __cplusplus
    extern "C" {
#endif

struct siglentSSA3021x;
struct siglentSSA3021x_Vtbl;

typedef enum labError (*lpfnSiglentSSA3021X_Disconnect)(
    struct siglentSSA3021x* lpDevice
);
typedef enum labError (*lpfnSiglentSSA3021X_IDN)(
    struct siglentSSA3021x* lpDevice,

    char** lpResultOut,
    unsigned long int* lpResultLenOut
);

typedef enum labError (*lpfnSiglentSSA3021X_SetFrequencyCenter)(
    struct siglentSSA3021x* lpDevice,
    double dFrequencyHz
);
typedef enum labError (*lpfnSiglentSSA3021X_SetSpan)(
    struct siglentSSA3021x* lpDevice,
    double dSpanHz
);
typedef enum labError (*lpfnSiglentSSA3021X_SetAverageCount)(
    struct siglentSSA3021x* lpDevice,
    unsigned long int dwCount
);

typedef enum labError (*lpfnSiglentSSG3021X_QueryTraceData)(
    struct siglentSSA3021x* lpDevice,
    struct spectrumDataTrace** lpTraceOut
);

struct siglentSSA3021x_Vtbl {
    lpfnSiglentSSA3021X_Disconnect          disconnect;
    lpfnSiglentSSA3021X_IDN                 idn;

    lpfnSiglentSSA3021X_SetFrequencyCenter  setFrequencyCenter;
    lpfnSiglentSSA3021X_SetSpan             setSpan;
    lpfnSiglentSSA3021X_SetAverageCount     setAverageCount;
    lpfnSiglentSSG3021X_QueryTraceData      queryTraceData;
};

struct siglentSSA3021x {
    struct siglentSSA3021x_Vtbl*            vtbl;
    void*                                   lpReserved;
};

enum labError siglentSSA3021xConnect(
    struct siglentSSA3021x** lpOut,
    char* lpAddress
);

#ifdef __cplusplus
    } /* extern "C" { */
#endif

#endif /* #ifndef __is_included__a0d9ddd4_2d85_11eb_89c2_b499badf00a1 */

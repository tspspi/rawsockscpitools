#ifndef __is_included__5d177c48_2d6e_11eb_89c2_b499badf00a1
#define __is_included__5d177c48_2d6e_11eb_89c2_b499badf00a1 1

#ifdef __cplusplus
    extern "C" {
#endif

struct rigolMso5000;
struct rigolMso5000_Vtbl;

typedef enum labError (*lpfnRigolMSO5000_Disconnect)(
    struct rigolMso5000* lpDevice
);
typedef enum labError (*lpfnRigolMSO5000_IDN)(
    struct rigolMso5000* lpDevice,

    char** lpResultOut,
    unsigned long int* lpResultLenOut
);
typedef enum labError (*lpfnRigolMSO5000_SetChannelEnable)(
    struct rigolMso5000* lpDevice,

    unsigned long int dwChannel,
    bool bEnabled
);
typedef enum labError (*lpfnRigolMSO5000_QueryWaveform)(
    struct rigolMso5000* lpDevice,

    unsigned long int dwChannel,
    struct oscilloscopeWaveformData** lpWaveformOut
);


struct rigolMso5000_Vtbl {
    lpfnRigolMSO5000_Disconnect             disconnect;
    lpfnRigolMSO5000_IDN                    idn;

    lpfnRigolMSO5000_SetChannelEnable       setChannelEnable;
    lpfnRigolMSO5000_QueryWaveform          queryWaveform;
};
struct rigolMso5000 {
    struct rigolMso5000_Vtbl                *vtbl;
    void*                                   lpReserved;
};

enum labError rigolMso5000Connect(
    struct rigolMso5000** lpOut,
    char* lpAddress
);

#ifdef __cplusplus
    } /* extern "C" { */
#endif

#endif /* #ifndef __is_included__5d177c48_2d6e_11eb_89c2_b499badf00a1 */

#ifndef __is_included__1b8a9934_2d7e_11eb_89c2_b499badf00a1
#define __is_included__1b8a9934_2d7e_11eb_89c2_b499badf00a1 1

#ifdef __cplusplus
    extern "C" {
#endif

struct siglentSSG3021x;
struct siglentSSG3021x_Vtbl;

typedef enum labError (*lpfnSiglentSSG3021X_Disconnect)(
    struct siglentSSG3021x* lpDevice
);
typedef enum labError (*lpfnSiglentSSG3021X_IDN)(
    struct siglentSSG3021x* lpDevice,

    char** lpResultOut,
    unsigned long int* lpResultLenOut
);




struct siglentSSG3021x_Vtbl {
    lpfnSiglentSSG3021X_Disconnect              disconnect;
    lpfnSiglentSSG3021X_IDN                     idn;
};
struct siglentSSG3021x {
    struct siglentSSG3021x_Vtbl*                vtbl;
    void*                                       lpReserved;
};

enum labError siglentSSG3021xConnect(
    struct siglentSSG3021x** lpOut,
    char* lpAddress
);

#ifdef __cplusplus
    } /* extern "C" { */
#endif


#endif /* #ifndef __is_included__1b8a9934_2d7e_11eb_89c2_b499badf00a1 */
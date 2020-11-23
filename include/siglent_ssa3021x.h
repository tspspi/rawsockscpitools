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





struct siglentSSA3021x_Vtbl {
    lpfnSiglentSSA3021X_Disconnect  disconnect;
    lpfnSiglentSSA3021X_IDN         idn;
};

struct siglentSSA3021x {
    struct siglentSSA3021x_Vtbl*    vtbl;
    void*                           lpReserved;
};

enum labError siglentSSA3021xConnect(
    struct siglentSSA3021x** lpOut,
    char* lpAddress
);

#ifdef __cplusplus
    } /* extern "C" { */
#endif

#endif /* #ifndef __is_included__a0d9ddd4_2d85_11eb_89c2_b499badf00a1 */

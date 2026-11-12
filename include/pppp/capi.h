#ifndef PPPP_CAPI_H
#define PPPP_CAPI_H

#include "pppp/version.h" /* IWYU pragma: export */
#include <stddef.h>

#if defined(_WIN32) || defined(__CYGWIN__)
#if defined(PPPP_C_BUILD)
#define PPPP_C_API __declspec(dllexport)
#elif defined(PPPP_C_USE_DLL)
#define PPPP_C_API __declspec(dllimport)
#else
#define PPPP_C_API
#endif
#elif defined(__GNUC__) && __GNUC__ >= 4
#define PPPP_C_API __attribute__((visibility("default")))
#else
#define PPPP_C_API
#endif

#if defined(_MSC_VER) && _MSC_VER < 1310
typedef __int64 PpppInt64;
typedef unsigned __int64 PpppUint64;
#elif defined(__GNUC__)
__extension__ typedef long long PpppInt64;
__extension__ typedef unsigned long long PpppUint64;
#else
typedef long long PpppInt64;
typedef unsigned long long PpppUint64;
#endif

typedef int PpppInt32;
typedef unsigned int PpppUint32;
typedef PpppInt32 PpppBool;

#ifdef __cplusplus
extern "C" {
#endif

PPPP_C_API const char* Pppp_Version(void);
PPPP_C_API PpppUint32 Pppp_VersionNumber(void);
PPPP_C_API const char* Pppp_Backend(void);

#ifdef __cplusplus
}
#endif

#endif

#ifndef funs_h
#define funs_h 1

#ifdef __cplusplus
extern "C" { 			/* Assume C declarations for C++   */
#endif  /* __cplusplus */

#if defined(WIN32) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#include <sqlext.h>
#else
#include <sql.h>
#endif

SQLRETURN allocEnv(SQLHENV *Handle);

#ifdef __cplusplus
}                                    /* End of extern "C" { */
#endif  /* __cplusplus */

#endif 

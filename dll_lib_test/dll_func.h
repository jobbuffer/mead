#ifndef __DLL_FUNC_H__
#define __DLL_FUNC_H__

#include <stdio.h>
#include <string.h>

#ifdef DLL_IMPLEMENT
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllexport)
#endif

#ifdef __cplusplus
extern "C"{
#endif

DLL_API int dll_int_(int a);

#ifdef __cplusplus
}
#endif

#endif
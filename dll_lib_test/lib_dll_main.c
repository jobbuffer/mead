/*=============================================
* c use cpp lib/dll
==============================================*/
#include <stdio.h>
#include <windows.h>
#include "dll_func.h"
#include "lib_func.h"

#pragma comment(lib,"dll_test.lib")

int c_main()
{
	int lib_re = lib_int_(10);
	int dll_re = dll_int_(11);

	printf("lib:%d, dll:%d\n", lib_re, dll_re);

	system("pause");
	return 0;
}
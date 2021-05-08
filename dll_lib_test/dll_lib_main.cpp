/*=============================================
* .cpp use cpp lib/dll
==============================================*/
#include <stdio.h>
#include <Windows.h>

#include "dll_func.h"
#include "lib_func.h"

#pragma comment(lib,"dll_test.lib")
#pragma comment(lib,"lib_test.lib")

int cpp_main()
{
	int lib_re = lib_int_(12);
	int dll_re = dll_int_(13);

	printf("lib:%d, dll:%d\n", lib_re, dll_re);

	system("pause");
	return 0;
}
#include "mead.h"
#include "base_func.h"

int mead_main()
{
	uint64_t time = mead_time_ms();
	printf("%llu", time);
	return 0;
}
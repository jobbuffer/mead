#include <stdio.h>
#include <zlib.h>
#include <Windows.h>
#include <string.h>

#include "base_func.h"

#define RAWLEN 1024*1024*10*10
#pragma warning(disable:4996)

static int mead_zlib_compress(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen, int level)
{
	z_stream stream;
	int err;
	const uInt max = (uInt)-1;
	uLong left;

	left = *destLen;
	*destLen = 0;

	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;
	stream.opaque = (voidpf)0;

	err = deflateInit(&stream, level);
	if (err != Z_OK) return err;

	stream.next_out = dest;
	stream.avail_out = 0;
	stream.next_in = (z_const Bytef *)source;
	stream.avail_in = 0;

	do {
		if (stream.avail_out == 0) {
			stream.avail_out = left > (uLong)max ? max : (uInt)left;
			left -= stream.avail_out;
		}
		if (stream.avail_in == 0) {
			stream.avail_in = sourceLen > (uLong)max ? max : (uInt)sourceLen;
			sourceLen -= stream.avail_in;
		}
		err = deflate(&stream, sourceLen ? Z_NO_FLUSH : Z_FINISH);
	} while (err == Z_OK);

	*destLen = stream.total_out;
	deflateEnd(&stream);
	return err == Z_STREAM_END ? Z_OK : err;
}

int main()
{
	uint8_t *in_buf = NULL;
	uint8_t *out_buf = NULL;
	uint8_t *un_buf = NULL;
	uint64_t time_tmp = 0;
	uint64_t compress_time = 0;
	uint64_t uncompress_time = 0;

	int destLen = 0;
	int unLen = 0;
	int srcLen = 0;
	int ret = 0;
	int i_loop = 0;

	FILE *in_fp = NULL;
	FILE *out_fp = NULL;
	FILE *un_fp = NULL;

	in_fp = fopen("G:/project/4/mead/zlib/yuv420p_320x240.yuv", "rb+");
	out_fp = fopen("G:/project/4/mead/zlib/z02.zlib", "wb+");
	un_fp = fopen("G:/project/4/mead/zlib/z02_un.zlib", "wb+");

	if (!in_fp || !out_fp || !un_fp)
	{
		if (in_fp)fclose(in_fp);
		if (out_fp)fclose(out_fp);
		if (un_fp)fclose(un_fp);
		return 2;
	}
	destLen = RAWLEN + 1000;
	srcLen = RAWLEN;
	unLen = RAWLEN + 1000;

	in_buf = (uint8_t*)malloc(srcLen);
	out_buf = (uint8_t*)malloc(destLen);
	un_buf = (uint8_t*)malloc(unLen);

	while (!feof(in_fp))
	{
		destLen = RAWLEN + 1000;
		unLen = RAWLEN + 1000;
		i_loop++;
		ret = fread(in_buf, 1, RAWLEN, in_fp);
		if (ret > 0)
		{
			time_tmp = mead_time_ms();
			compress2(out_buf, &destLen, in_buf, ret, Z_BEST_COMPRESSION);
			compress_time += mead_time_ms() - time_tmp;
			
			fwrite(out_buf, 1, destLen, out_fp);
			time_tmp = mead_time_ms();
			uncompress2(un_buf, &unLen, out_buf, &destLen);
			uncompress_time += mead_time_ms() - time_tmp;
			fwrite(un_buf, 1, unLen, un_fp);
		}
	}
	
	printf("cost : i_loop:%d, compress:%llu, uncompress:%llu\n", i_loop, compress_time, uncompress_time);

	fclose(in_fp); in_fp = NULL;
	fclose(out_fp); out_fp = NULL;
	fclose(un_fp); un_fp = NULL;

	free(un_buf);
	free(out_buf);
	free(in_buf);

	system("pause");
	return 0;
}
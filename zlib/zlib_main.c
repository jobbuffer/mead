#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <zlib.h>

#define EXCESS 256
#define RAWLEN 8

static int partcompress(FILE *in, z_streamp def)
{
	int ret, flush;
	uint8_t raw[RAWLEN];
	flush = Z_NO_FLUSH;
	do{
		def->avail_in = fread(raw, 1, RAWLEN, in);
		if (error(in))
			return Z_ERRNO;
		def->next_in = raw;
		if (feof(in))
			flush = Z_FINISH;
		ret = deflate(def, flush);
		assert(ret != Z_STREAM_ERROR);
	} while (def->avail_out != 0 && flush == Z_NO_FLUSH);
	return ret;
}

static int recompress(z_streamp inf, z_streamp def)
{
	int ret, flush;
	uint8_t raw[RAWLEN];

	flush = Z_NO_FLUSH;
	do{
		inf->avail_out = RAWLEN;
		inf->next_out = raw;
		ret = inflate(inf, Z_NO_FLUSH);
		if (ret == Z_MEM_ERROR)
			return ret;

		def->avail_in = RAWLEN - inf->avail_out;
		def->next_in = raw;
		if (inf->avail_out != 0)
			flush = Z_FINISH;
		ret = deflate(def, flush);
	} while (ret != Z_STREAM_END&& def->avail_out != 0);
	return ret;
}

int main()
{
	int ret = 0;
	int size = 2048, have;
	uint8_t *blk;
	uint8_t *tmp;

	z_stream def, inf;
	
	blk = malloc(4096);
	def.zalloc = Z_NULL;
	def.zfree = Z_NULL;
	def.opaque = Z_NULL;
	ret = deflateInit(&def, Z_DEFAULT_COMPRESSION);
	
	if (ret != Z_OK || blk == NULL)
		printf("out of memory!\n");

	def.avail_out = size + EXCESS;
	def.next_out = blk;
	ret = partcompress(stdin, &def);
	if (ret == Z_ERRNO)
		printf("error reading input\n");

	if (ret == Z_STREAM_END && def.avail_out >= EXCESS)
	{
		have = size + EXCESS - def.avail_out;
		if (fwrite(blk, 1, have, stdout) != have || ferror(stdout))
			printf("error writing output!\n");

		ret = deflateEnd(&def);
		assert(ret != Z_STREAM_ERROR);
		free(blk);
		return 0;
	}

	inf.zalloc = Z_NULL;
	inf.zfree = Z_NULL;
	inf.opaque = Z_NULL;
	inf.avail_in = 0;
	inf.next_in = Z_NULL;
	ret = inflateInit(&inf);
	tmp = malloc(size + EXCESS);

	if (ret != Z_OK || tmp == NULL)
		printf("out of memory!\n");

	ret = deflateReset(&def);
	assert(ret != Z_STREAM_ERROR);
	inf.avail_in = size + EXCESS;
	inf.next_in = blk;
	def.avail_out = size + EXCESS;
	def.next_out = tmp;

	ret = recompress(&inf, &def);
	if (ret == Z_MEM_ERROR)
		printf("out of memory!\n");

///////////////////////////////////////////////
	ret = inflateReset(&inf);
	assert(ret != Z_STREAM_ERROR);
	ret = deflateReset(&def);
	assert(ret != Z_STREAM_ERROR);

	inf.avail_in = size - RAWLEN;
	inf.next_in = tmp;
	def.avail_out = size;
	def.next_out = blk;
	ret = recompress(&inf, &def);
	if (ret == Z_MEM_ERROR)
		printf("out of memory!\n");

	free(tmp);
	ret = inflateEnd(&inf);
	ret = deflateEnd(&def);
	free(blk);
	return 0;
}
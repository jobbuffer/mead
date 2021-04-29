#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "base_func.h"
#include "zstd.h"

#pragma warning(disable:4996)

static void compress(const char *name_in, const char *name_out, const int cLevel)
{
	FILE *fin = NULL;
	FILE *fout = NULL;
	
	size_t buffInSize = ZSTD_CStreamInSize();
	size_t buffOutSize = ZSTD_CStreamOutSize();
	void*  buffIn = malloc(buffInSize);	
	void*  buffOut = malloc(buffOutSize);

	ZSTD_CCtx *cctx = ZSTD_createCCtx();
	ZSTD_inBuffer input;
	ZSTD_outBuffer output;
	ZSTD_EndDirective mode;
	size_t toRead = buffInSize;
	size_t remaining = 0;
	size_t read = 0;
	int finished = 0;
	int lastChunk = 0;
	int write = 0;

	fout = fopen(name_out, "wb+");
	fin = fopen(name_in, "rb+");
	if (!fin || !fout)
	{
		if (fin) fclose(fin);
		if (fout) fclose(fout);
		return 2;
	}

	ZSTD_CCtx_setParameter(cctx, ZSTD_c_compressionLevel, cLevel);
	ZSTD_CCtx_setParameter(cctx, ZSTD_c_checksumFlag, 1);
	ZSTD_CCtx_setParameter(cctx, ZSTD_c_nbWorkers, 4);

	for (;;) 
	{
		read = fread(buffIn, 1, toRead, fin);
		lastChunk = (read < toRead);
		mode = lastChunk ? ZSTD_e_end : ZSTD_e_continue;
		ZSTD_inBuffer input = { buffIn, read, 0 };		
		do
		{
			ZSTD_outBuffer output = { buffOut, buffOutSize, 0 };

			remaining = ZSTD_compressStream2(cctx, &output, &input, mode);
			write = fwrite(buffOut, 1, output.pos, fout);
			printf("write length:%d\n", write);
			finished = lastChunk ? (remaining == 0) : (input.pos == input.size);
		} while (!finished);
		
		if (lastChunk) {
			break;
		}
	}

	ZSTD_freeCCtx(cctx);
	fclose(fout); fout = NULL;
	fclose(fin); fin = NULL;
	free(buffIn);
	free(buffOut);
}


static void decompress(const char *name_in,const char *name_out)
{
	FILE *fin = NULL;
	FILE *fout = NULL;

	size_t buffInSize = ZSTD_DStreamInSize();
	void*  buffIn = malloc(buffInSize);
	
	size_t buffOutSize = ZSTD_DStreamOutSize(); 
	void*  buffOut = malloc(buffOutSize);

	ZSTD_DCtx *dctx = ZSTD_createDCtx();
	ZSTD_inBuffer input;
	ZSTD_outBuffer output;

	size_t toRead = buffInSize;
	size_t read;
	size_t lastRet = 0;
	int isEmpty = 1;
	int write = 0;
	int ret = 0;

	fin = fopen(name_in, "rb+");
	fout = fopen(name_out, "wb+");
	if (!fin || !fout)
	{
		if (fin) fclose(fin);
		if (fout) fclose(fout);
		return 2;
	}

	do
	{
		read = fread(buffIn, 1, buffInSize, fin);
		isEmpty = 0;
		input.src = buffIn;
		input.size = read;
		input.pos = 0;

		while (input.pos < input.size)
		{
			output.dst = buffOut;
			output.size = buffOutSize;;
			output.pos = 0;

			ret = ZSTD_decompressStream(dctx, &output, &input);
			write = fwrite(buffOut, 1, output.pos, fout);
			printf("write: out pos:%llu, write length:%d\n", output.pos, write);
			lastRet = ret;
		}
	} while (read == buffInSize);
	
	if (isEmpty) {
		fprintf(stderr, "input is empty\n");
		exit(1);
	}

	if (lastRet != 0) {
		fprintf(stderr, "EOF before end of stream: %zu\n", lastRet);
		exit(1);
	}

	ZSTD_freeDCtx(dctx);
	fclose(fin); fin = NULL;
	fclose(fout); fout = NULL;
	free(buffIn);
	free(buffOut);
}

int main()
{
	compress("G:/project/4/mead/zstd/yuv420p_320x240.yuv", "G:/project/4/mead/zstd/compress.zst", 1);
	decompress("G:/project/4/mead/zstd/compress.zst","G:/project/4/mead/zstd/dst.yuv");
	system("pause");
	return 0;
}
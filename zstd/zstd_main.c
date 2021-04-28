#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "base_func.h"
#include "zstd.h"


static void compress(const char *name_in, const char *name_out, const int cLevel)
{
	FILE *fin = NULL;
	FILE *fout = NULL;
	
	size_t buffInSize = ZSTD_CStreamInSize();
	size_t buffOutSize = ZSTD_CStreamOutSize();
	void*  buffIn = malloc(buffInSize);	
	void*  buffOut = malloc(buffOutSize);

	ZSTD_CCtx *cctx = ZSTD_createCCtx();
	size_t toRead = buffInSize;

	fout = fopen(name_out, "wb");
	fin = fopen(name_in, "rb");
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
		size_t read = fread(buffIn, 1, toRead, fin);
		int const lastChunk = (read < toRead);
		ZSTD_EndDirective mode = lastChunk ? ZSTD_e_end : ZSTD_e_continue;
		ZSTD_inBuffer input = { buffIn, read, 0 };
		int finished;

		do
		{
			ZSTD_outBuffer output = { buffOut, buffOutSize, 0 };
			size_t remaining = ZSTD_compressStream2(cctx, &output, &input, mode);
			int write = fwrite(buffOut, 1, output.pos, fout);
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
	
	size_t toRead = buffInSize;
	size_t read;
	size_t lastRet = 0;
	int isEmpty = 1;

	fin = fopen(name_in, "wb");
	fout = fopen(name_out, "rb");
	if (!fin || !fout)
	{
		if (fin) fclose(fin);
		if (fout) fclose(fout);
		return 2;
	}

	while ((read = fread_orDie(buffIn, toRead, fin)))
	{
		isEmpty = 0;
		ZSTD_inBuffer input = { buffIn, read, 0 };
		while (input.pos < input.size) 
		{
			ZSTD_outBuffer output = { buffOut, buffOutSize, 0 };
			size_t ret = ZSTD_decompressStream(dctx, &output, &input);
			fwrite_orDie(buffOut, output.pos, fout);
			lastRet = ret;
		}
	}

	if (isEmpty) {
		fprintf(stderr, "input is empty\n");
		exit(1);
	}

	if (lastRet != 0) {
		fprintf(stderr, "EOF before end of stream: %zu\n", lastRet);
		exit(1);
	}

	ZSTD_freeDCtx(dctx);
	fclsoe(fin); fin = NULL;
	fclsoe(fout); fout = NULL;
	free(buffIn);
	free(buffOut);
}

int main()
{
	compress("yuv420p_320x240.yuv", "compress.zst", 1);
	decompress("compress.zst","dst.yuv");
	system("pause");
	return 0;
}
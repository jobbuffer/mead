/* fitblk.c: example of fitting compressed output to a specified size
   Not copyrighted -- provided to the public domain
   Version 1.1  25 November 2004  Mark Adler */

/* Version history:
   1.0  24 Nov 2004  First version
   1.1  25 Nov 2004  Change deflateInit2() to deflateInit()
                     Use fixed-size, stack-allocated raw buffers
                     Simplify code moving compression to subroutines
                     Use assert() for internal errors
                     Add detailed description of approach
 */

/* Approach to just fitting a requested compressed size:

   fitblk performs three compression passes on a portion of the input
   data in order to determine how much of that input will compress to
   nearly the requested output block size.  The first pass generates
   enough deflate blocks to produce output to fill the requested
   output size plus a specfied excess amount (see the EXCESS define
   below).  The last deflate block may go quite a bit past that, but
   is discarded.  The second pass decompresses and recompresses just
   the compressed data that fit in the requested plus excess sized
   buffer.  The deflate process is terminated after that amount of
   input, which is less than the amount consumed on the first pass.
   The last deflate block of the result will be of a comparable size
   to the final product, so that the header for that deflate block and
   the compression ratio for that block will be about the same as in
   the final product.  The third compression pass decompresses the
   result of the second step, but only the compressed data up to the
   requested size minus an amount to allow the compressed stream to
   complete (see the MARGIN define below).  That will result in a
   final compressed stream whose length is less than or equal to the
   requested size.  Assuming sufficient input and a requested size
   greater than a few hundred bytes, the shortfall will typically be
   less than ten bytes.

   If the input is short enough that the first compression completes
   before filling the requested output size, then that compressed
   stream is return with no recompression.

   EXCESS is chosen to be just greater than the shortfall seen in a
   two pass approach similar to the above.  That shortfall is due to
   the last deflate block compressing more efficiently with a smaller
   header on the second pass.  EXCESS is set to be large enough so
   that there is enough uncompressed data for the second pass to fill
   out the requested size, and small enough so that the final deflate
   block of the second pass will be close in size to the final deflate
   block of the third and final pass.  MARGIN is chosen to be just
   large enough to assure that the final compression has enough room
   to complete in all cases.
 */

#include <stdio.h>
#include <stdlib.h>
#include "zlib.h"

#define local static

#pragma warning(disable:4996)

/* print nastygram and leave */
local void quit(char *why)
{
    fprintf(stderr, "fitblk abort: %s\n", why);
    exit(1);
}

#define RAWLEN 4096    /* intermediate uncompressed buffer size */

/* compress from file to def until provided buffer is full or end of
   input reached; return last deflate() return value, or Z_ERRNO if
   there was read error on the file */

local int partcompress(FILE *in, z_streamp def)
{
    int ret, flush;
    unsigned char raw[RAWLEN];

    flush = Z_NO_FLUSH;
    do {
        def->avail_in = fread(raw, 1, RAWLEN, in);
        //if (ferror(in))
          //  return Z_ERRNO;
        def->next_in = raw;
        if (feof(in))
            flush = Z_FINISH;
        ret = deflate(def, flush);
    } while (def->avail_out != 0 && flush == Z_NO_FLUSH);
    return ret;
}


/* recompress from inf's input to def's output; the input for inf and
   the output for def are set in those structures before calling;
   return last deflate() return value, or Z_MEM_ERROR if inflate()
   was not able to allocate enough memory when it needed to */
local int recompress(z_streamp inf, z_streamp def)
{
    int ret, flush;
    unsigned char raw[RAWLEN];

    flush = Z_NO_FLUSH;
    do {
        /* decompress */
        inf->avail_out = RAWLEN;
        inf->next_out = raw;
        ret = inflate(inf, Z_NO_FLUSH);
        
        if (ret == Z_MEM_ERROR)
            return ret;

        /* compress what was decompresed until done or no room */
        def->avail_in = RAWLEN - inf->avail_out;
        def->next_in = raw;
        if (inf->avail_out != 0)
            flush = Z_FINISH;
        ret = deflate(def, flush);
		
    } while (ret != Z_STREAM_END && def->avail_out != 0);
    return ret;
}

#define EXCESS 256      /* empirically determined stream overage */
#define MARGIN 8        /* amount to back off for completion */

/* compress from stdin to fixed-size block on stdout */
int zlib_main(int argc, char **argv)
{
	int ret = 1024*1024*100;             /* return code */
    unsigned size;          /* requested fixed output block size */
    unsigned have;          /* bytes written by deflate() call */
    unsigned char *blk;     /* intermediate and final stream */
    unsigned char *tmp;     /* close to desired size stream */
	FILE *in_fp=NULL;
	FILE *out_fp = NULL;
    z_stream def, inf;      /* zlib deflate and inflate states */
	int in_size = 0;

	in_fp = fopen("G:/project/4/mead/zlib/yuv420p_320x240.yuv", "rb+");
	out_fp = fopen("G:/project/4/mead/zlib/z01.zlib", "wb+");
	if (!in_fp || !out_fp)
	{
		if (in_fp)fclose(in_fp);
		if (out_fp)fclose(out_fp);
		return 2;
	}

	fseek(in_fp, 0L, SEEK_END);
	in_size = ftell(in_fp);
	fseek(in_fp, 0L, SEEK_SET);
	ret = in_size + 1024 * 10;

	size = (unsigned)ret;

	blk = malloc(size + EXCESS);
	def.zalloc = Z_NULL;
	def.zfree = Z_NULL;
	def.opaque = Z_NULL;
	ret = deflateInit(&def, Z_DEFAULT_COMPRESSION);
	if (ret != Z_OK || blk == NULL)
		quit("out of memory");

	def.avail_out = size + EXCESS;
	def.next_out = blk;

    ret = partcompress(in_fp, &def);
    if (ret == Z_ERRNO)
        quit("error reading input");

    if (ret == Z_STREAM_END && def.avail_out >= EXCESS) {
        have = size + EXCESS - def.avail_out;
		if (fwrite(blk, 1, have, out_fp) != have || ferror(stdout))
            quit("error writing output");

        ret = deflateEnd(&def);
        free(blk);
        fprintf(stderr,
                "%u bytes unused out of %u requested (all input)\n",
                size - have, size);
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
        quit("out of memory");
    ret = deflateReset(&def);

    inf.avail_in = size + EXCESS;
    inf.next_in = blk;
    def.avail_out = size + EXCESS;
    def.next_out = tmp;
    ret = recompress(&inf, &def);
    if (ret == Z_MEM_ERROR)
        quit("out of memory");

    ret = inflateReset(&inf);
    ret = deflateReset(&def);

    inf.avail_in = size - MARGIN;  
    inf.next_in = tmp;
    def.avail_out = size;
    def.next_out = blk;
    ret = recompress(&inf, &def);
    if (ret == Z_MEM_ERROR)
        quit("out of memory");

    have = size - def.avail_out;
    if (fwrite(blk, 1, have, stdout) != have || ferror(stdout))
        quit("error writing output");

    free(tmp);
    ret = inflateEnd(&inf);
    
    ret = deflateEnd(&def);
    
    free(blk);
    fprintf(stderr,
            "%u bytes unused out of %u requested (%lu input)\n",
            size - have, size, def.total_in);
	
	fclose(in_fp); in_fp = NULL;
	fclose(out_fp); out_fp = NULL;

	system("pause");
    return 0;
}

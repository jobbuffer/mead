#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <x264.h>
#include <x264_config.h>

//#include <libavcodec/avcodec.h>
//#include <libswscale/swscale.h>
//#include <libavformat/avformat.h>
//#include <libavutil/adler32.h>


#ifdef __cplusplus
extern "C"
{
#endif

//#include "libavutil/imgutils.h"
//#include "libavutil/timestamp.h"

#ifdef __cplusplus
}
#endif


#pragma warning(disable:4996)


static int mead_264_encode(char *src_name, char *dst_name, int width, int height)
{
	x264_param_t param;
	x264_picture_t pic;
	x264_picture_t pic_out;
	x264_nal_t *nal;
	x264_t *h;
	int i_frame = 0;
	int i_frame_size = 0;
	int i_nal;
	
	FILE *src_fp = fopen(src_name, "rb+");
	FILE *dst_fp = fopen(dst_name, "wb+");

	if (!src_fp || !dst_fp)
	{
		if (src_fp) fclose(src_fp);
		if (dst_fp) fclose(dst_fp);
		return 2;
	}

	if (x264_param_default_preset(&param, "medium", NULL) < 0)
		goto fail;

	param.i_bitdepth = 8;
	param.i_csp = X264_CSP_I420;
	param.i_width = width;
	param.i_height = height;
	param.b_vfr_input = 0;
	param.b_repeat_headers = 1;
	param.b_annexb = 1;

	if (x264_param_apply_profile(&param, "high") < 0)
		goto fail;

	if (x264_picture_alloc(&pic, param.i_csp, param.i_width, param.i_height) < 0)
		goto fail;

	h = x264_encoder_open(&param);
	if (!h)
		goto fail;

	int luma_size = width*height;
	int chroma_size = luma_size / 4;

	for (;; i_frame++)
	{
		if (fread(pic.img.plane[0], 1, luma_size, src_fp) != (unsigned)luma_size)
			break;
		if (fread(pic.img.plane[1], 1, chroma_size, src_fp) != (unsigned)chroma_size)
			break;
		if (fread(pic.img.plane[2], 1, chroma_size, src_fp) != (unsigned)chroma_size)
			break;

		pic.i_pts = i_frame;
		i_frame_size = x264_encoder_encode(h, &nal, &i_nal, &pic, &pic_out);
		if (i_frame_size < 0)
			goto fail;
		else if (i_frame_size)
		{
			if (!fwrite(nal->p_payload, i_frame_size, 1, dst_fp))
				goto fail;
		}
	}

	while (x264_encoder_delayed_frames(h))
	{
		i_frame_size = x264_encoder_encode(h, &nal, &i_nal, NULL, &pic_out);
		if (i_frame_size < 0)
			goto fail;
		else if (i_frame_size)
		{
			if (!fwrite(nal->p_payload, i_frame_size, 1, dst_fp))
				goto fail;
		}
	}

	x264_encoder_close(h);
	x264_picture_clean(&pic);
	return 0;
fail:
	return 1;
}
//
//static int mead_264_decode(const char *input_filename)
//{
//	AVCodec *codec = NULL;
//	AVCodecContext *ctx = NULL;
//	AVCodecParameters *origin_par = NULL;
//	AVFrame *fr = NULL;
//	uint8_t *byte_buffer = NULL;
//	AVPacket pkt;
//	AVFormatContext *fmt_ctx = NULL;
//	int number_of_written_bytes;
//	int video_stream;
//	int got_frame = 0;
//	int byte_buffer_size;
//	int i = 0;
//	int result;
//	int end_of_stream = 0;
//
//	result = avformat_open_input(&fmt_ctx, input_filename, NULL, NULL);
//	if (result < 0) {
//		//av_log(NULL, AV_LOG_ERROR, "Can't open file\n");
//		return result;
//	}
//
//	result = avformat_find_stream_info(fmt_ctx, NULL);
//	if (result < 0) {
//		//av_log(NULL, AV_LOG_ERROR, "Can't get stream info\n");
//		return result;
//	}
//
//	video_stream = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
//	if (video_stream < 0) {
//		//av_log(NULL, AV_LOG_ERROR, "Can't find video stream in input file\n");
//		return -1;
//	}
//
//	origin_par = fmt_ctx->streams[video_stream]->codecpar;
//
//	codec = avcodec_find_decoder(origin_par->codec_id);
//	if (!codec) {
//		//av_log(NULL, AV_LOG_ERROR, "Can't find decoder\n");
//		return -1;
//	}
//
//	ctx = avcodec_alloc_context3(codec);
//	if (!ctx) {
//		//av_log(NULL, AV_LOG_ERROR, "Can't allocate decoder context\n");
//		return AVERROR(ENOMEM);
//	}
//
//	result = avcodec_parameters_to_context(ctx, origin_par);
//	if (result) {
//		//av_log(NULL, AV_LOG_ERROR, "Can't copy decoder context\n");
//		return result;
//	}
//
//	result = avcodec_open2(ctx, codec, NULL);
//	if (result < 0) {
//		//av_log(ctx, AV_LOG_ERROR, "Can't open decoder\n");
//		return result;
//	}
//
//	fr = av_frame_alloc();
//	if (!fr) {
//		//av_log(NULL, AV_LOG_ERROR, "Can't allocate frame\n");
//		return AVERROR(ENOMEM);
//	}
//
//	byte_buffer_size = av_image_get_buffer_size(ctx->pix_fmt, ctx->width, ctx->height, 16);
//	byte_buffer = av_malloc(byte_buffer_size);
//	if (!byte_buffer) {
//		//av_log(NULL, AV_LOG_ERROR, "Can't allocate buffer\n");
//		return AVERROR(ENOMEM);
//	}
//
//	printf("#tb %d: %d/%d\n", video_stream, fmt_ctx->streams[video_stream]->time_base.num, fmt_ctx->streams[video_stream]->time_base.den);
//	i = 0;
//	av_init_packet(&pkt);
//	do {
//		if (!end_of_stream)
//			if (av_read_frame(fmt_ctx, &pkt) < 0)
//				end_of_stream = 1;
//		if (end_of_stream) {
//			pkt.data = NULL;
//			pkt.size = 0;
//		}
//		if (pkt.stream_index == video_stream || end_of_stream) {
//			got_frame = 0;
//			if (pkt.pts == AV_NOPTS_VALUE)
//				pkt.pts = pkt.dts = i;
//			result = avcodec_decode_video2(ctx, fr, &got_frame, &pkt);
//			if (result < 0) {
//				//av_log(NULL, AV_LOG_ERROR, "Error decoding frame\n");
//				return result;
//			}
//			if (got_frame) {
//				number_of_written_bytes = av_image_copy_to_buffer(byte_buffer, byte_buffer_size,
//					(const uint8_t* const *)fr->data, (const int*)fr->linesize,
//					ctx->pix_fmt, ctx->width, ctx->height, 1);
//				if (number_of_written_bytes < 0) {
//					//av_log(NULL, AV_LOG_ERROR, "Can't copy image to buffer\n");
//					return number_of_written_bytes;
//				}
//				printf("%d, %s, %s, %8"PRId64", %8d, 0x%08lx\n", video_stream,
//					av_ts2str(fr->pts), av_ts2str(fr->pkt_dts), fr->pkt_duration,
//					number_of_written_bytes, av_adler32_update(0, (const uint8_t*)byte_buffer, number_of_written_bytes));
//			}
//			av_packet_unref(&pkt);
//			av_init_packet(&pkt);
//		}
//		i++;
//	} while (!end_of_stream || got_frame);
//
//	av_packet_unref(&pkt);
//	av_frame_free(&fr);
//	avcodec_close(ctx);
//	avformat_close_input(&fmt_ctx);
//	avcodec_free_context(&ctx);
//	av_freep(&byte_buffer);
//	return 0;
//}

int main()
{
	int width = 320, height = 240;

	//mead_264_encode("yuv420p_320x240.yuv", "dst_h264.h264", width, height);
	//mead_264_decode("dst_h264.h264");
	
	system("pause");
	return 0;
}
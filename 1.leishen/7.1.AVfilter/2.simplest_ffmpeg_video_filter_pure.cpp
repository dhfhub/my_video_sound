/** 
 * ��򵥵Ļ���FFmpeg��AVFilter���� - ������
 * Simplest FFmpeg AVfilter Example - Pure
 *
 * ������ʹ��FFmpeg��AVfilterʵ����YUV�������ݵ��˾��������ܡ�
 * ���Ը�YUV�������Ӹ�����Ч���ܡ�
 * ����򵥵�FFmpeg��AVFilter����Ľ̡̳�
 * �ʺ�FFmpeg�ĳ�ѧ�ߡ�
 *
 * libavfilter���Զ�����libavcodec�����Ľӿں�����Ϊһ�������⡱������Ƶ��Ч������ʹ�á�
 * ���ļ�¼�ġ������桱��avfilter�����Ӽ�ʵ����һ���������Ƶ��Ч���ӵĹ��ܡ�
 * ����������Ϊһ��YUV�ļ������Ҳ��һ��YUV�ļ���
 * ͨ��avfilter�Ĺ��ܿ��Դ�����YUV�ļ�ʵ��ȥɫ����ģ����ˮƽ��ת���ü����ӷ��򡢵������ֵȹ��ܡ�
 *
 */
#include <stdio.h>
#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
#define snprintf _snprintf
//Windows
extern "C"
{
#include "libavfilter/avfiltergraph.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
};
#else
//Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#ifdef __cplusplus
};
#endif
#endif

//avfilter_register_all()��ע������AVFilter��
//avfilter_graph_alloc()��ΪFilterGraph�����ڴ档
//avfilter_graph_create_filter()����������FilterGraph������һ��Filter��
//avfilter_graph_parse_ptr()����һ��ͨ���ַ���������Graph���ӵ�FilterGraph�С�
//avfilter_graph_config()�����FilterGraph�����á�
//av_buffersrc_add_frame()����FilterGraph�м���һ��AVFrame��
//av_buffersink_get_frame()����FilterGraph��ȡ��һ��AVFrame��

//yuvͼƬģʽ��������ļ���yuvplayer�鿴
int main(int argc, char* argv[])
{
	int ret;
	AVFrame *frame_in;
	AVFrame *frame_out;
	unsigned char *frame_buffer_in;
	unsigned char *frame_buffer_out;

	AVFilterContext *buffersink_ctx;
	AVFilterContext *buffersrc_ctx;
	AVFilterGraph *filter_graph;
	static int video_stream_index = -1;

	//1.�������ļ�
	FILE *fp_in=fopen("/root/www/video_sound_data/1.yuv_data/sintel_480x272_yuv420p.yuv", "rb+");
	if(fp_in==NULL){
		printf("Error open input file.\n");
		return -1;
	}
	int in_width=480;
	int in_height=272;

	//2.������ļ�
	FILE *fp_out=fopen("2.yuv","wb+");
	if(fp_out==NULL){
		printf("Error open output file.\n");
		return -1;
	}

	//3.ָ��filter����
	//const char *filter_descr = "lutyuv='u=128:v=128'";
	const char *filter_descr = "boxblur";
	//const char *filter_descr = "hflip";
	//const char *filter_descr = "hue='h=60:s=-3'";
	//const char *filter_descr = "crop=2/3*in_w:2/3*in_h";
	//const char *filter_descr = "drawbox=x=100:y=100:w=100:h=100:color=pink@0.5";
	//const char *filter_descr = "drawtext=fontfile=arial.ttf:fontcolor=green:fontsize=30:text='Lei Xiaohua'";

	//4.
	avfilter_register_all();

	//5.����filter
	char args[512];
	AVFilter *buffersrc  = avfilter_get_by_name("buffer");
	AVFilter *buffersink = avfilter_get_by_name("buffersink");
	//6.��������/�����filter����
	AVFilterInOut *outputs = avfilter_inout_alloc();
	AVFilterInOut *inputs  = avfilter_inout_alloc();
	enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };
	AVBufferSinkParams *buffersink_params;

	//7.����graph
	filter_graph = avfilter_graph_alloc();

	//8.��Ƶ����Դ:������frame������뵽����
	/* buffer video source: the decoded frames from the decoder will be inserted here. */
	snprintf(args,
			sizeof(args),
			"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
			in_width,in_height, AV_PIX_FMT_YUV420P, 1, 25, 1, 1);
	ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in", args, NULL, filter_graph);
	if (ret < 0) {
		printf("Cannot create buffer source\n");
		return ret;
	}

	//9.��Ƶ����ˮ��:
	/* buffer video sink: to terminate the filter chain. */
	buffersink_params = av_buffersink_params_alloc();
	buffersink_params->pixel_fmts = pix_fmts;
	ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out", NULL, buffersink_params, filter_graph);
	av_free(buffersink_params);
	if (ret < 0) {
		printf("Cannot create buffer sink\n");
		return ret;
	}

	//10.
	/* Endpoints for the filter graph. */
	outputs->name       = av_strdup("in");
	outputs->filter_ctx = buffersrc_ctx;
	outputs->pad_idx    = 0;
	outputs->next       = NULL;

	inputs->name       = av_strdup("out");
	inputs->filter_ctx = buffersink_ctx;
	inputs->pad_idx    = 0;
	inputs->next       = NULL;

	//11.��һ��ͨ���ַ���������Graph���ӵ�FilterGraph�С�
	if ((ret = avfilter_graph_parse_ptr(filter_graph, filter_descr, &inputs, &outputs, NULL)) < 0)
		return ret;

	//12.���FilterGraph������
	if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
		return ret;

	frame_in=av_frame_alloc();
	//av_image_get_buffer_size():����ָ���ߴ磬�����ļ������������ֽڣ�
	frame_buffer_in=(unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, in_width,in_height,1));
	av_image_fill_arrays(frame_in->data,
						frame_in->linesize,
						frame_buffer_in,
						AV_PIX_FMT_YUV420P,in_width, in_height,1);

	frame_out=av_frame_alloc();
	frame_buffer_out=(unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, in_width,in_height,1));
	av_image_fill_arrays(frame_out->data,
						frame_out->linesize,
						frame_buffer_out,
						AV_PIX_FMT_YUV420P, in_width, in_height,1);

	frame_in->width=in_width;
	frame_in->height=in_height;
	frame_in->format=AV_PIX_FMT_YUV420P;

	while (1) {
		if(fread(frame_buffer_in, 1, in_width*in_height*3/2, fp_in)!= in_width*in_height*3/2){
			break;
		}
		//input Y,U,V
		frame_in->data[0]=frame_buffer_in;
		frame_in->data[1]=frame_buffer_in+in_width*in_height;
		frame_in->data[2]=frame_buffer_in+in_width*in_height*5/4;
		//��frame���ӵ�buffersrc_ctx
		if (av_buffersrc_add_frame(buffersrc_ctx, frame_in) < 0) {
			printf( "Error while add frame.\n");
			break;
		}
		//filter������ȡ��frame
		/* pull filtered pictures from the filtergraph */
		ret = av_buffersink_get_frame(buffersink_ctx, frame_out);
		if (ret < 0){
			break;
		}
		//output Y,U,V
		if(frame_out->format==AV_PIX_FMT_YUV420P){
			for(int i=0;i<frame_out->height;i++){
				fwrite(frame_out->data[0]+frame_out->linesize[0]*i,1,frame_out->width,fp_out);
			}
			for(int i=0;i<frame_out->height/2;i++){
				fwrite(frame_out->data[1]+frame_out->linesize[1]*i,1,frame_out->width/2,fp_out);
			}
			for(int i=0;i<frame_out->height/2;i++){
				fwrite(frame_out->data[2]+frame_out->linesize[2]*i,1,frame_out->width/2,fp_out);
			}
		}
		printf("Process 1 frame!\n");
		av_frame_unref(frame_out);
	}

	fclose(fp_in);
	fclose(fp_out);

	av_frame_free(&frame_in);
	av_frame_free(&frame_out);
	avfilter_graph_free(&filter_graph);

	return 0;
}

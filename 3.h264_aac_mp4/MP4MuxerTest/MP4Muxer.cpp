/*
 * Car eye ��������ƽ̨: www.car-eye.cn
 * Car eye ��Դ��ַ: https://github.com/Car-eye-team
 * MP4Muxer.cpp
 *
 * Author: Wgj
 * Date: 2019-03-04 22:37
 * Copyright 2019
 *
 * MP4��������
 */
 
#include <iostream>
#include "CEDebug.h"
#include "MP4Muxer.h"

// av_read_frame��ȡAAC��������С 1K�ֽ�
#define AUDIO_BUFFER_SIZE		2048
// AAC���������
#define AUDIO_QUEUE_SIZE		100
// AACÿ�������������С
#define AUDIO_ITEM_SIZE			2048
// AAC���б�ʶ
#define AUDIO_QUEUE_FLAG		0x10

// av_read_frame��ȡH264��������С 1K�ֽ�
#define VIDEO_BUFFER_SIZE		40960
// H264���������
#define VIDEO_QUEUE_SIZE		100
// H264ÿ�������������С
#define VIDEO_ITEM_SIZE			10240
// H264���б�ʶ
#define VIDEO_QUEUE_FLAG		0x20

// ������Ƶ�������ڵ���
//#define SAVE_AUDIO_FILE

// �Ƿ��Ѿ���ʼ����FFMPEG��
static bool mIsInitFFMPEG = false;

// aac��������, �������MP4�ļ�ͷ
const uint8_t AAC_MUTE_DATA[361] = {
	0xFF,0xF1,0x6C,0x40,0x2D,0x3F,0xFC,0x00,0xE0,0x34,0x20,0xAD,0xF2,0x3F,0xB5,0xDD,
	0x73,0xAC,0xBD,0xCA,0xD7,0x7D,0x4A,0x13,0x2D,0x2E,0xA2,0x62,0x02,0x70,0x3C,0x1C,
	0xC5,0x63,0x55,0x69,0x94,0xB5,0x8D,0x70,0xD7,0x24,0x6A,0x9E,0x2E,0x86,0x24,0xEA,
	0x4F,0xD4,0xF8,0x10,0x53,0xA5,0x4A,0xB2,0x9A,0xF0,0xA1,0x4F,0x2F,0x66,0xF9,0xD3,
	0x8C,0xA6,0x97,0xD5,0x84,0xAC,0x09,0x25,0x98,0x0B,0x1D,0x77,0x04,0xB8,0x55,0x49,
	0x85,0x27,0x06,0x23,0x58,0xCB,0x22,0xC3,0x20,0x3A,0x12,0x09,0x48,0x24,0x86,0x76,
	0x95,0xE3,0x45,0x61,0x43,0x06,0x6B,0x4A,0x61,0x14,0x24,0xA9,0x16,0xE0,0x97,0x34,
	0xB6,0x58,0xA4,0x38,0x34,0x90,0x19,0x5D,0x00,0x19,0x4A,0xC2,0x80,0x4B,0xDC,0xB7,
	0x00,0x18,0x12,0x3D,0xD9,0x93,0xEE,0x74,0x13,0x95,0xAD,0x0B,0x59,0x51,0x0E,0x99,
	0xDF,0x49,0x98,0xDE,0xA9,0x48,0x4B,0xA5,0xFB,0xE8,0x79,0xC9,0xE2,0xD9,0x60,0xA5,
	0xBE,0x74,0xA6,0x6B,0x72,0x0E,0xE3,0x7B,0x28,0xB3,0x0E,0x52,0xCC,0xF6,0x3D,0x39,
	0xB7,0x7E,0xBB,0xF0,0xC8,0xCE,0x5C,0x72,0xB2,0x89,0x60,0x33,0x7B,0xC5,0xDA,0x49,
	0x1A,0xDA,0x33,0xBA,0x97,0x9E,0xA8,0x1B,0x6D,0x5A,0x77,0xB6,0xF1,0x69,0x5A,0xD1,
	0xBD,0x84,0xD5,0x4E,0x58,0xA8,0x5E,0x8A,0xA0,0xC2,0xC9,0x22,0xD9,0xA5,0x53,0x11,
	0x18,0xC8,0x3A,0x39,0xCF,0x3F,0x57,0xB6,0x45,0x19,0x1E,0x8A,0x71,0xA4,0x46,0x27,
	0x9E,0xE9,0xA4,0x86,0xDD,0x14,0xD9,0x4D,0xE3,0x71,0xE3,0x26,0xDA,0xAA,0x17,0xB4,
	0xAC,0xE1,0x09,0xC1,0x0D,0x75,0xBA,0x53,0x0A,0x37,0x8B,0xAC,0x37,0x39,0x41,0x27,
	0x6A,0xF0,0xE9,0xB4,0xC2,0xAC,0xB0,0x39,0x73,0x17,0x64,0x95,0xF4,0xDC,0x33,0xBB,
	0x84,0x94,0x3E,0xF8,0x65,0x71,0x60,0x7B,0xD4,0x5F,0x27,0x79,0x95,0x6A,0xBA,0x76,
	0xA6,0xA5,0x9A,0xEC,0xAE,0x55,0x3A,0x27,0x48,0x23,0xCF,0x5C,0x4D,0xBC,0x0B,0x35,
	0x5C,0xA7,0x17,0xCF,0x34,0x57,0xC9,0x58,0xC5,0x20,0x09,0xEE,0xA5,0xF2,0x9C,0x6C,
	0x39,0x1A,0x77,0x92,0x9B,0xFF,0xC6,0xAE,0xF8,0x36,0xBA,0xA8,0xAA,0x6B,0x1E,0x8C,
	0xC5,0x97,0x39,0x6A,0xB8,0xA2,0x55,0xA8,0xF8
};

/*
* Comments: ����һ��ָ����С�Ķ���
* Param aCount: ������ĸ���
* Param aItemSize: ����ÿ����ֽ���
* @Return �����Ķ���, ʧ�ܷ���nullptr
*/
static CESimpleQueue* CreateQueue(int aCount, int aItemSize)
{
	CESimpleQueue* queue = (CESimpleQueue *)malloc(sizeof(CESimpleQueue));
	if (queue == nullptr)
	{
		return nullptr;
	}

	queue->Items = (CEQueueItem *)malloc(sizeof(CEQueueItem) * aCount);
	if (queue->Items == nullptr)
	{
		// �ͷ�������ڴ�
		free(queue);
		return nullptr;
	}
	for (int i = 0; i < aCount; i++)
	{
		queue->Items[i].Bytes = (uint8_t *)malloc(aItemSize);
		queue->Items[i].TotalSize = aItemSize;
		queue->Items[i].EnqueueSize = 0;
		queue->Items[i].DequeueSize = 0;
	}
	queue->Count = aCount;
	queue->DequeueIndex = 0;
	queue->EnqueueIndex = 0;

	return queue;
}

/*
* Comments: ���ٶ���
* Param aQueue: Ҫ���ٵĶ���
* @Return void
*/
static void DestroyQueue(CESimpleQueue* aQueue)
{
	if (aQueue == nullptr)
	{
		return;
	}

	// �ͷŶ�������������ڴ�
	for (int i = 0; i < aQueue->Count; i++)
	{
		free(aQueue->Items[i].Bytes);
	}

	free(aQueue->Items);
	free(aQueue);
}

/*
* Comments: ����������Ӳ���
* Param aQueue: ����ʵ��, �����ڲ������пռ��
* Param aBytes: Ҫ��ӵ�����
* Param aSize: Ҫ��ӵ������ֽ���
* @Return ��ӳɹ����
*/
static bool Enqueue(CESimpleQueue* aQueue, uint8_t* aBytes, int aSize)
{
	if (aBytes == NULL || aSize < 1)
	{
		return false;
	}

	int tail = aQueue->EnqueueIndex;
	int size = FFMIN(aSize, aQueue->Items[tail].TotalSize);
	while (size > 0)
	{
		memcpy(aQueue->Items[tail].Bytes, aBytes, size);
		aQueue->Items[tail].EnqueueSize = size;
		aQueue->Items[tail].DequeueSize = 0;
		aQueue->EnqueueIndex = (aQueue->EnqueueIndex + 1) % aQueue->Count;
		tail = aQueue->EnqueueIndex;
		// ���ݹ���, �������
		aSize -= size;
		aBytes += size;
		size = FFMIN(aSize, aQueue->Items[tail].TotalSize);
	}

	return true;
}

/*
* Comments: ��ȡý���ڴ�������FFMPEG�ص����� ���Ӳ���
* Param aHandle: ���о��
* Param aBytes: [���] Ҫ��ȡ������
* Param aSize: Ҫ��ȡ���ֽ���
* @Return ��ȡ����ʵ���ֽڸ���
*/
static int ReadBytesCallback(void *aHandle, uint8_t *aBytes, int aSize)
{
	CESimpleQueue *pQueue = (CESimpleQueue *)aHandle;

	if (pQueue->DequeueIndex == pQueue->EnqueueIndex)
	{
		//		DEBUG_W("Queue[%02X] head index equal tail index: %d.", pQueue->Flag, pQueue->DequeueIndex);
		return 0;
	}

	int head = pQueue->DequeueIndex;
// 	DEBUG_V("Queue[%02X]%d deqSize:%d, enqSize:%d, aSize=%d.", pQueue->Flag, head,
// 		pQueue->Items[head].DequeueSize, pQueue->Items[head].EnqueueSize, aSize);
	int size = FFMIN(aSize, pQueue->Items[head].EnqueueSize);

	// �������ݳ���
	memcpy(aBytes, pQueue->Items[head].Bytes + pQueue->Items[head].DequeueSize, size);

	pQueue->Items[head].DequeueSize += size;
	pQueue->Items[head].EnqueueSize -= size;

	if (pQueue->Items[head].EnqueueSize <= 0)
	{
		pQueue->Items[head].DequeueSize = 0;
		pQueue->DequeueIndex = (pQueue->DequeueIndex + 1) % pQueue->Count;
	}

	return size;
}

/*
* Comments: MP4��������ʼ��
* Param : 
* @Return void
*/
MP4Muxer::MP4Muxer()
{
	this->mInputAudioContext = nullptr;
	this->mInputVideoContext = nullptr;
	this->mOutputFrmtContext = nullptr;
	this->mAudioFilter = nullptr;
	this->mVideoFilter = nullptr;
	this->mAudioBuffer = nullptr;
	this->mVideoBuffer = nullptr;
	this->mIsStartAudio = false;
	this->mIsStartVideo = false;
	this->mAudioQueue = nullptr;
	this->mVideoQueue = nullptr;
	this->mAudioIndex = -1;
	this->mVideoIndex = -1;
	this->mAudioOutIndex = -1;
	this->mVideoOutIndex = -1;
	this->mFrameIndex = 0;
}

MP4Muxer::~MP4Muxer()
{
}

/*
* Comments: ��������ʼ�� ʵ���������ȵ��ø÷��������ڲ������ĳ�ʼ��
* Param aFileName: Ҫ������ļ���
* @Return �Ƿ�ɹ�
*/
bool MP4Muxer::Start(std::string aFileName)
{
	if (this->mOutputFrmtContext)
	{
		// �Ѿ���ʼ������.
		return true;
	}

	if (!mIsInitFFMPEG)
	{
		mIsInitFFMPEG = true;
		av_register_all();
	}

	if (aFileName.empty())
	{
		// �ļ���Ϊ��
		return false;
	}

	DEBUG_V("Start muxer: %s.", aFileName.c_str());

	// �ֱ𴴽�AAC��H264���ݶ���
	if (this->mAudioQueue == nullptr)
	{
		this->mAudioQueue = CreateQueue(AUDIO_QUEUE_SIZE, AUDIO_ITEM_SIZE);
		if (this->mAudioQueue == nullptr)
		{
			return false;
		}
		this->mAudioQueue->Flag = AUDIO_QUEUE_FLAG;
	}
	if (this->mVideoQueue == nullptr)
	{
		this->mVideoQueue = CreateQueue(VIDEO_QUEUE_SIZE, VIDEO_ITEM_SIZE);
		if (this->mVideoQueue == nullptr)
		{
			return false;
		}
		this->mVideoQueue->Flag = VIDEO_QUEUE_FLAG;
	}

	if (!mAudioFilter)
	{
		// ����AAC�˲���
		mAudioFilter = av_bitstream_filter_init("aac_adtstoasc");
		if (!mAudioFilter)
		{
			// ����ʧ��
			DEBUG_E("Init aac filter fail.");
			return false;
		}
	}

	if (!mVideoFilter)
	{
		// ����H264�˲���
		mVideoFilter = av_bitstream_filter_init("h264_mp4toannexb");
		if (!mVideoFilter)
		{
			// ����ʧ��
			DEBUG_E("Init h264 filter fail.");
			return false;
		}
	}

	this->mFrameIndex = 0;
	this->mFileName = aFileName;

	// ����������ĳ�ʼ��
	int ret = avformat_alloc_output_context2(&this->mOutputFrmtContext, NULL, "mp4", this->mFileName.c_str());
	if (!this->mOutputFrmtContext)
	{
		DEBUG_E("Alloc output format from file extension failed: %d!", ret);
		return false;
	}

	return true;
}

/*
* Comments: �رղ��ͷ������ʽ������
* Param : None
* @Return None
*/
void MP4Muxer::Stop(void)
{
	// lock
	std::lock_guard<std::mutex> lockGuard(this->mLock);

	DEBUG_V("Stop muxer: %s.", this->mFileName.c_str());

	if (this->mOutputFrmtContext)
	{
		if (this->mIsStartVideo)
		{
			// д���ļ�β
			av_write_trailer(this->mOutputFrmtContext);
		}

		// ����Ѵ����ļ�����йر�
		if (!(this->mOutputFrmtContext->oformat->flags & AVFMT_NOFILE))
		{
			avio_closep(&this->mOutputFrmtContext->pb);
		}

		// �ͷ�������
		avformat_free_context(this->mOutputFrmtContext);
		this->mOutputFrmtContext = nullptr;
	}

	// �ͷ��˲���
	if (mAudioFilter)
	{
		av_bitstream_filter_close(mAudioFilter);
		mAudioFilter = nullptr;
	}
	if (mVideoFilter)
	{
		av_bitstream_filter_close(mVideoFilter);
		mVideoFilter = nullptr;
	}

	// �ͷ�������
	if (mInputAudioContext)
	{
		avio_context_free(&mInputAudioContext->pb);
		avformat_close_input(&mInputAudioContext);
		mInputAudioContext = nullptr;
		mAudioBuffer = nullptr;
	}
	else if (mAudioBuffer)
	{
		av_freep(&mAudioBuffer);
	}
	if (mInputVideoContext)
	{
		avio_context_free(&mInputVideoContext->pb);
		avformat_close_input(&mInputVideoContext);
		mInputVideoContext = nullptr;
		mVideoBuffer = nullptr;
	}
	else if (mVideoBuffer)
	{
		av_freep(&mVideoBuffer);
	}

	// �ͷŶ���
	if (this->mAudioQueue)
	{
		DestroyQueue(mAudioQueue);
		mAudioQueue = nullptr;
	}
	if (this->mVideoQueue)
	{
		DestroyQueue(mVideoQueue);
		mVideoQueue = nullptr;
	}

}

/*
* Comments: д��H264��Ƶ֡���ļ�
* Param : None
* @Return void
*/
void MP4Muxer::WriteVideoFrame(void)
{
	int ret = -1;
	int count = 0;
	AVStream *in_stream, *out_stream;
	// ��ý�����ݰ�
	AVPacket pkt;

	while (1)
	{
		// ��ȡһ�����ݰ�
		ret = av_read_frame(mInputVideoContext, &pkt);
//		DEBUG_V("Read[%d] video frame[%d] ret=-%08X.", count++, mVideoQueue->DequeueIndex, -ret);
		if (ret < 0)
		{
			// AVERROR_EOF��ʾ����ȡ��
			if (ret != AVERROR_EOF)
			{
				DEBUG_W("Read video frame ret = -%08X.", -ret);
			}
			return;
		}
//		DEBUG_V("read index: %d, video index: %d, %d.", pkt.stream_index, mVideoIndex, mVideoOutIndex);
		in_stream = mInputVideoContext->streams[mVideoIndex];
		out_stream = mOutputFrmtContext->streams[mVideoOutIndex];

		// ���û����ʾʱ����Լ�����ʱ������ҽ���ʾʱ�����ֵ������ʱ���
		if (pkt.pts == AV_NOPTS_VALUE)
		{
			//Write PTS
			AVRational time_base1 = in_stream->time_base;
			//Duration between 2 frames (us)
			int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(in_stream->r_frame_rate);
			//Parameters
			pkt.pts = (double)(mFrameIndex * calc_duration) / (double)(av_q2d(time_base1)*AV_TIME_BASE);
			pkt.dts = pkt.pts;
			pkt.duration = (double)calc_duration / (double)(av_q2d(time_base1)*AV_TIME_BASE);
			mFrameIndex++;
		}

		// H264��Ƶ����
		av_bitstream_filter_filter(mVideoFilter, in_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);

		//Convert PTS/DTS
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.stream_index = mVideoOutIndex;
		pkt.pos = -1;

		//Write
		if (av_interleaved_write_frame(mOutputFrmtContext, &pkt) < 0)
		{
			av_free_packet(&pkt);
			DEBUG_E("Error muxing packet.");
			break;
		}
		av_free_packet(&pkt);
		//printf("Write %8d frames to output file\n",frame_index);
	}
}

/*
* Comments: ׷��һ֡H264��Ƶ���ݵ�������
* Param aBytes: Ҫ׷�ӵ��ֽ�����
* Param aSize: ׷�ӵ��ֽ���
* @Return �ɹ����
*/
bool MP4Muxer::AppendVideo(uint8_t* aBytes, int aSize)
{
	std::lock_guard<std::mutex> lockGuard(this->mLock);

	if (aBytes == nullptr || aSize < 1)
	{
		return false;
	}

	if (!Enqueue(this->mVideoQueue, aBytes, aSize))
	{
		return false;
	}

	if (!this->mIsStartVideo)
	{
		if (OpenVideo() < 0)
		{
			return false;
		}
		this->mIsStartVideo = true;
	}
	else
	{
		WriteVideoFrame();
	}

	return true;
}

/*
* Comments: ��H264��Ƶ��
* Param : None
* @Return 0�ɹ�, ����ʧ��
*/
int MP4Muxer::OpenVideo(void)
{
	int result;
	AVIOContext* pb = nullptr;
	AVInputFormat* inputFrmt = nullptr;

// 	if (mInputVideoContext)
// 	{
// 		// ��������
// 		avio_context_free(&mInputVideoContext->pb);
// 		avformat_close_input(&mInputVideoContext);
// 		mInputVideoContext = nullptr;
// 		mVideoBuffer = nullptr;
// 	}

	if (mInputVideoContext == nullptr)
	{
		if (mVideoBuffer == nullptr)
		{
			// ������Ƶ���ݻ�����
			mVideoBuffer = (uint8_t*)av_malloc(VIDEO_BUFFER_SIZE);
			if (mVideoBuffer == nullptr)
			{
				return -1;
			}
		}

		// ����FFMPEG�Ķ�ȡ�ص�����, ͨ���ص���ʽ��ȡ�ڴ�����
		pb = avio_alloc_context(mVideoBuffer, VIDEO_BUFFER_SIZE, 0, (void *)this->mVideoQueue, ReadBytesCallback, NULL, NULL);
		mInputVideoContext = avformat_alloc_context();
		if (mInputVideoContext == nullptr)
		{
			// ʧ�����ͷ�������
			avio_context_free(&pb);
			mVideoBuffer = nullptr;
			DEBUG_E("Allock video format context fail.");
			return -1;
		}
		mInputVideoContext->pb = pb;
	}

	if (this->mVideoQueue->EnqueueIndex - this->mVideoQueue->DequeueIndex < 10)
	{
		// �ȴ���Ƶ֡
		return -1;
	}

	if ((result = av_probe_input_buffer(mInputVideoContext->pb, &inputFrmt, "", NULL, 0, 0)) < 0)
	{
		DEBUG_E("Probe video context fail: -%08X.", -result);
		return result;
	}

	if (strcmp(inputFrmt->name, "h264") != 0)
	{
		DEBUG_E("Error input video format name: %s.", inputFrmt->name);
		return -1;
	}

	// ��������
	if ((result = avformat_open_input(&mInputVideoContext, "", inputFrmt, NULL)) != 0)
	{
		if (!mInputVideoContext)
		{
			mVideoBuffer = nullptr;
			if (pb)
			{
				avio_context_free(&pb);
			}
		}
		DEBUG_E("Couldn't open input video stream -%08X.", -result);
		return result;
	}

	// ��������Ϣ
	if ((result = avformat_find_stream_info(mInputVideoContext, NULL)) < 0)
	{
		DEBUG_E("Failed to retrieve input video stream information -%08X.", -result);
		return result;
	}

	// ��������ý���������ʽ
	for (unsigned int i = 0; i < mInputVideoContext->nb_streams; i++)
	{
		AVStream* input_stream = mInputVideoContext->streams[i];
		if (input_stream->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			if (input_stream->codec == nullptr)
			{
				DEBUG_E("Input video codec is null.");
				return -2;
			}
			if (input_stream->codec->coded_height < 1
				|| input_stream->codec->coded_width < 1)
			{
				// ��Ƶ�ߴ���Ч
				DEBUG_E("Input video size invalid: %d, %d.",
					input_stream->codec->coded_width, input_stream->codec->coded_height);
// 				input_stream->codec->width = input_stream->codec->coded_width = 352;
// 				input_stream->codec->height = input_stream->codec->coded_height = 288;
				return -2;
			}
			mVideoIndex = i;
			AVStream* output_stream = avformat_new_stream(this->mOutputFrmtContext, input_stream->codec->codec);
			if (!output_stream)
			{
				DEBUG_E("Create new video stream format failed!");
				return -2;
			}

			mVideoOutIndex = output_stream->index;
			if ((result = avcodec_copy_context(output_stream->codec, input_stream->codec)) < 0)
			{
				DEBUG_E("Failed to copy video context from input to output stream codec context: -%08X!", -result);
				return result;
			}

			output_stream->codec->codec_tag = 0;
			if (this->mOutputFrmtContext->oformat->flags & AVFMT_GLOBALHEADER)
			{
				output_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
			}
			break;
		}
	}

	if (!this->mIsStartAudio)
	{
		// û�еȴ���Ƶ֡�����Ĭ�ϵ���Ƶ֡
		Enqueue(this->mAudioQueue, (uint8_t *)AAC_MUTE_DATA, sizeof(AAC_MUTE_DATA));
		OpenAudio(true);
		this->mIsStartAudio = true;
	}

	// ������Ҫ������ļ�
	if (!(this->mOutputFrmtContext->oformat->flags & AVFMT_NOFILE))
	{
		if ((result = avio_open(&this->mOutputFrmtContext->pb, this->mFileName.c_str(), AVIO_FLAG_WRITE)) < 0)
		{
			DEBUG_E("Can't open file: %s: -%08X.", this->mFileName.c_str(), -result);
			return result;
		}
	}

	// д���ļ�ͷ
	if ((result = avformat_write_header(this->mOutputFrmtContext, NULL)) < 0)
	{
		DEBUG_E("Error occurred when opening output file: -%08X!", -result);
		return result;
	}

	return 0;
}

/*
* Comments: д��AAC����֡���ļ�
* Param : None
* @Return void
*/
void MP4Muxer::WriteAudioFrame(void)
{
	int result = -1;
	AVStream *in_stream, *out_stream;
	// ��ý�����ݰ�
	AVPacket pkt;

	while (1)
	{
		AVStream *in_stream, *out_stream;
		// ��ȡһ�����ݰ�
		result = av_read_frame(mInputAudioContext, &pkt);
		if (result < 0)
		{
			// AVERROR_EOF��ʾ����ȡ��
// 			if (result != AVERROR_EOF)
// 			{
// 				DEBUG_W("Read audio frame result = -%08X.", -result);
// 			}
			return;
		}
		in_stream = mInputAudioContext->streams[mAudioIndex];
		out_stream = mOutputFrmtContext->streams[mAudioOutIndex];

		// ���û����ʾʱ����Լ�����ʱ������ҽ���ʾʱ�����ֵ������ʱ���
		if (pkt.pts == AV_NOPTS_VALUE)
		{
			//Write PTS
			AVRational time_base1 = in_stream->time_base;
			//Duration between 2 frames (us)
			int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(in_stream->r_frame_rate);
			//Parameters
			pkt.pts = (double)(mFrameIndex * calc_duration) / (double)(av_q2d(time_base1)*AV_TIME_BASE);
			pkt.dts = pkt.pts;
			pkt.duration = (double)calc_duration / (double)(av_q2d(time_base1)*AV_TIME_BASE);
			mFrameIndex++;
		}

		av_bitstream_filter_filter(mAudioFilter, out_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);

		// Convert PTS/DTS
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.stream_index = mAudioOutIndex;
		pkt.pos = -1;
		// Write
		if (av_interleaved_write_frame(mOutputFrmtContext, &pkt) < 0)
		{
			av_free_packet(&pkt);
			printf("Error muxing packet\n");
			break;
		}
		av_free_packet(&pkt);
		//printf("Write %8d frames to output file\n",frame_index);
	}
}

/*
* Comments: ׷��һ֡AAC���ݵ�������
* Param aBytes: Ҫ׷�ӵ��ֽ�����
* Param aSize: ׷�ӵ��ֽ���
* @Return �ɹ����
*/
bool MP4Muxer::AppendAudio(uint8_t* aBytes, int aSize)
{
	std::lock_guard<std::mutex> lockGuard(this->mLock);

// 	if (!this->mIsStartVideo)
// 	{
// 		return false;
// 	}

	if (aBytes == nullptr || aSize < 1)
	{
		return false;
	}

// 	static int i = 0;
// 	char file_name[30];
// 
// 	sprintf(file_name, "./audio/aac%d.audio", i);
// 	i++;
// 	fopen_s(&pFile, file_name, "w");

// 	fopen_s(&pFile, "./aac.audio", "a");
// 
// 	fwrite(aBytes, 1, aSize, pFile);
// 
// 	fclose(pFile);

	if (!Enqueue(this->mAudioQueue, aBytes, aSize))
	{
		return false;
	}

	if (!this->mIsStartAudio)
	{
		if (OpenAudio(false) < 0)
		{
			return false;
		}
		this->mIsStartAudio = true;
	}
	else if (this->mIsStartVideo)
	{
		WriteAudioFrame();
	}

	return true;
}

/*
* Comments: ��AAC����������
* Param : None
* @Return 0�ɹ�, ����ʧ��
*/
int MP4Muxer::OpenAudio(bool aNew)
{
	int result;
	AVIOContext* pb = nullptr;
	AVInputFormat* inputFrmt = nullptr;

	if (aNew && mInputAudioContext)
	{
		// ��������
		avio_context_free(&mInputAudioContext->pb);
		avformat_close_input(&mInputAudioContext);
		mInputAudioContext = nullptr;
		mAudioBuffer = nullptr;
	}

	if (mInputAudioContext == nullptr)
	{
		if (mAudioBuffer == nullptr)
		{
			// ������Ƶ���ݻ�����
			mAudioBuffer = (uint8_t*)av_malloc(AUDIO_BUFFER_SIZE);
			if (mAudioBuffer == nullptr)
			{
				return -1;
			}
		}

		// ����FFMPEG�Ķ�ȡ�ص�����, ͨ���ص���ʽ��ȡ�ڴ�����
		pb = avio_alloc_context(mAudioBuffer, AUDIO_BUFFER_SIZE, 0, (void *)this->mAudioQueue, ReadBytesCallback, NULL, NULL);
		mInputAudioContext = avformat_alloc_context();
		if (mInputAudioContext == nullptr)
		{
			// ʧ�����ͷ�������
			avio_context_free(&pb);
			mAudioBuffer = nullptr;
			DEBUG_E("Alloc audio format context fail.");
			return -1;
		}
		mInputAudioContext->pb = pb;
	}

// 	inputFrmt = av_find_input_format("aac");
// 	if (inputFrmt == nullptr)
// 	{
// 		DEBUG_E("Find aac input format fail.");
// 		return -1;
// 	}

	if ((result = av_probe_input_buffer(mInputAudioContext->pb, &inputFrmt, "", NULL, 0, 0)) < 0)
	{
		DEBUG_E("Probe audio context fail: -%08X.", -result);
		return result;
	}

	if (strcmp(inputFrmt->name, "aac") != 0)
	{
		DEBUG_E("Error input audio format name: %s.", inputFrmt->name);
		return -1;
	}

	// ��������
	if ((result = avformat_open_input(&mInputAudioContext, "", inputFrmt, NULL)) != 0)
	{
		if (!mInputAudioContext)
		{
			mAudioBuffer = nullptr;
			if (pb)
			{
				avio_context_free(&pb);
			}
		}
		DEBUG_E("Couldn't open input audio stream -%08X.", -result);
		return result;
	}

	// ��������Ϣ
	if ((result = avformat_find_stream_info(mInputAudioContext, NULL)) < 0)
	{
		DEBUG_E("Failed to retrieve input audio stream information -%08X.", -result);
		return result;
	}

	// ��������ý���������ʽ
	for (unsigned int i = 0; i < mInputAudioContext->nb_streams; i++)
	{
		AVStream* input_stream = mInputAudioContext->streams[i];
		if (input_stream->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			mAudioIndex = i;
			AVStream* output_stream = avformat_new_stream(this->mOutputFrmtContext, input_stream->codec->codec);
			if (!output_stream)
			{
				DEBUG_E("Create new audio stream format failed!");
				return -2;
			}

			mAudioOutIndex = output_stream->index;
			if ((result = avcodec_copy_context(output_stream->codec, input_stream->codec)) < 0)
			{
				DEBUG_E("Failed to copy audio context from input to output stream codec context: -%08X!", -result);
				return result;
			}

			output_stream->codec->codec_tag = 0;
			if (this->mOutputFrmtContext->oformat->flags & AVFMT_GLOBALHEADER)
			{
				output_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
			}
			break;
		}
	}

	return  mAudioIndex >= 0 ? 0 : -1;
}

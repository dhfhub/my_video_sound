/*
 * Car eye ��������ƽ̨: www.car-eye.cn
 * Car eye ��Դ��ַ: https://github.com/Car-eye-team
 * MP4Muxer.h
 *
 * Author: Wgj
 * Date: 2019-03-04 22:21
 * Copyright 2019
 *
 * MP4�������������ļ�
 */

#ifndef __MP4_MUXER_H__
#define __MP4_MUXER_H__

#define _TIMESPEC_DEFINED
#include <thread>
#include <string>
#include <mutex>

#ifdef __cplusplus
extern "C" {
#endif

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"

#ifdef __cplusplus
}
#endif

// ���ڴ洢�����ݵĶ�����ṹ
typedef struct _CE_QUEUE_ITEM_T_
{
	// ����Ļ�����ָ��
	uint8_t* Bytes;
	// ����Ļ�������С
	int TotalSize;
	// ������������ֽ���
	int EnqueueSize;
	// ���Ӷ�ȡ�������ֽ���
	int DequeueSize;
}CEQueueItem;

// һ���򵥵Ķ��нṹ, �����ݴ�������
typedef struct _CE_QUEUE_T_
{
	// ������ָ��
	CEQueueItem *Items;
	// ���еı�ʶ, ʹ���߿����Զ����ʶ���ж�������
	uint8_t Flag;
	// ���������
	int Count;
	// ���Ӽ�������
	int DequeueIndex;
	// �������
	int EnqueueIndex;
}CESimpleQueue;

/*!
 * \class MP4Muxer
 *
 * \brief MP4������, �ϳ�H264��AAC����Ƶ
 *
 * \author Wgj
 * \date 2019-02-24
 */
class MP4Muxer
{
public:
	MP4Muxer();
	~MP4Muxer();

	/*
	* Comments: ��������ʼ�� ʵ���������ȵ��ø÷��������ڲ������ĳ�ʼ��
	* Param aFileName: Ҫ������ļ���
	* @Return �Ƿ�ɹ�
	*/
	bool MP4Muxer::Start(std::string aFileName);

	/*
	* Comments: ׷��һ֡AAC���ݵ�������
	* Param aBytes: Ҫ׷�ӵ��ֽ�����
	* Param aSize: ׷�ӵ��ֽ���
	* @Return �ɹ����
	*/
	bool MP4Muxer::AppendAudio(uint8_t* aBytes, int aSize);

	/*
	* Comments: ׷��һ֡H264��Ƶ���ݵ�������
	* Param aBytes: Ҫ׷�ӵ��ֽ�����
	* Param aSize: ׷�ӵ��ֽ���
	* @Return �ɹ����
	*/
	bool MP4Muxer::AppendVideo(uint8_t* aBytes, int aSize);

	/*
	* Comments: �رղ��ͷ������ʽ������
	* Param : None
	* @Return None
	*/
	void MP4Muxer::Stop(void);

private:
	/*
	* Comments: ��AAC����������
	* Param : None
	* @Return 0�ɹ�, ����ʧ��
	*/
	int MP4Muxer::OpenAudio(bool aNew = false);
	/*
	* Comments: д��AAC����֡���ļ�
	* Param : None
	* @Return void
	*/
	void MP4Muxer::WriteAudioFrame(void);
	/*
	* Comments: ��H264��Ƶ��
	* Param : None
	* @Return 0�ɹ�, ����ʧ��
	*/
	int MP4Muxer::OpenVideo(void);
	/*
	* Comments: д��H264��Ƶ֡���ļ�
	* Param : None
	* @Return void
	*/
	void MP4Muxer::WriteVideoFrame(void);

private:
	// Ҫ�����MP4�ļ�·��
	std::string mFileName;
	// ����AAC��Ƶ������
	AVFormatContext* mInputAudioContext;
	// ����H264��Ƶ������
	AVFormatContext* mInputVideoContext;
	// ���ý���ʽ������
	AVFormatContext* mOutputFrmtContext;
	// AAC��Ƶλ��ɸѡ������
	AVBitStreamFilterContext* mAudioFilter;
	// H264��Ƶλ��ɸѡ������
	AVBitStreamFilterContext* mVideoFilter;
	// ��ý�����ݰ�
	AVPacket mPacket;
	// AAC���ݻ�����
	uint8_t* mAudioBuffer;
	// AAC���ݶ�д����
	CESimpleQueue* mAudioQueue;
	// H264���ݻ�����
	uint8_t* mVideoBuffer;
	// H264���ݶ�д����
	CESimpleQueue* mVideoQueue;
	// AAC�Ļص���ȡ�Ƿ������ɹ�
	bool mIsStartAudio;
	// H264�Ļص���ȡ�Ƿ������ɹ�
	bool mIsStartVideo;
	// ��Ƶ��mInputAudioContext->streams���е�����
	int mAudioIndex;
	// ��Ƶ��mInputVideoContext->streams���е�����
	int mVideoIndex;
	// ���������Ƶ֡������
	int mAudioOutIndex;
	// �������Ƶ֡������
	int mVideoOutIndex;
	// ���֡��������
	int mFrameIndex;

	// �̲߳���������
	std::mutex mLock;

};

#endif


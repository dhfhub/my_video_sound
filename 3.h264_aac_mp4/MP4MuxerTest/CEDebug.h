/*
 * Car eye ��������ƽ̨: www.car-eye.cn
 * Car eye ��Դ��ַ: https://github.com/Car-eye-team
 * CEDebug.h
 *
 * Author: Wgj
 * Date: 2019-03-04 22:21
 * Copyright 2019
 *
 * ������Ϣ�ּ����, ����Android�����ʽ
 */
 
#ifndef __CE_DEBUG_H_
#define __CE_DEBUG_H_

#include <stdio.h>

 // Verbose����, ��ϸ��Ϣ, ���м������Ϣ�������
#define D_VERBOSE		0
// Debug����, �����Լ����ϵȼ���Ϣ���
#define D_DEBUG			5
// Info����, ֪ͨ��Ϣ�����ϵȼ���Ϣ���
#define D_INFO			10
// Warn����, ������Ϣ�����ϵȼ���Ϣ���
#define D_WARN			15
// Error����, ������Ϣ�Լ����ϵȼ���Ϣ���
#define D_ERROR			20
// DISABLE����, ��ֹ���е�����Ϣ�ӵ��Կ����
#define D_DISABLE		99

// DEBUGѡ��ʱ���������Ϣѡ��
#ifdef _DEBUG
#define DEBUG_LEVEL	D_VERBOSE
#else
// ��ǰ�ĵ�����Ϣ�������, ֻ����ü������ϼ������Ϣ
#define DEBUG_LEVEL	D_INFO
#endif

// ���VERBOSE��ϸ������Ϣ
#if DEBUG_LEVEL > D_VERBOSE
#define DEBUG_V(aFrmt, ...)
#define _DEBUG_V(aFrmt, ...)
#else
#define DEBUG_V(aFrmt, ...)		fprintf(stdout, "V:[%s:%d] "##aFrmt##"\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define _DEBUG_V(aFrmt, ...)	fprintf(stdout, aFrmt, ##__VA_ARGS__)
#endif

// ���DEBUG���Լ�����Ϣ
#if DEBUG_LEVEL > D_DEBUG
#define DEBUG_D(aFrmt, ...)
#define _DEBUG_D(aFrmt, ...)
#else
#define DEBUG_D(aFrmt, ...)		fprintf(stdout, "D:[%s:%d] "##aFrmt##"\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define _DEBUG_D(aFrmt, ...)	fprintf(stdout, aFrmt, ##__VA_ARGS__)
#endif

// ���INFO֪ͨ������Ϣ
#if DEBUG_LEVEL > D_INFO
#define DEBUG_I(aFrmt, ...)
#define _DEBUG_I(aFrmt, ...)
#else
#define DEBUG_I(aFrmt, ...)		fprintf(stdout, "I:[%s:%d] "##aFrmt##"\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define _DEBUG_I(aFrmt, ...)	fprintf(stdout, aFrmt, ##__VA_ARGS__)
#endif

// ���WARN���漶����Ϣ
#if DEBUG_LEVEL > D_WARN
#define DEBUG_W(aFrmt, ...)
#define _DEBUG_W(aFrmt, ...)
#else
#define DEBUG_W(aFrmt, ...)		fprintf(stderr, "W:[%s:%d] "##aFrmt##"\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define _DEBUG_W(aFrmt, ...)	fprintf(stderr, aFrmt, ##__VA_ARGS__)
#endif

// ���ERROR���󼶱���Ϣ
#if DEBUG_LEVEL > D_ERROR
#define DEBUG_E(aFrmt, ...)
#define _DEBUG_E(aFrmt, ...)
#else
#define DEBUG_E(aFrmt, ...)		fprintf(stderr, "E:[%s:%d] "##aFrmt##"\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define _DEBUG_E(aFrmt, ...)	fprintf(stderr, aFrmt, ##__VA_ARGS__)
#endif

#endif
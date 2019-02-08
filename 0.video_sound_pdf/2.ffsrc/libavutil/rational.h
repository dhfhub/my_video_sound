#ifndef RATIONAL_H
#define RATIONAL_H

//��������ȷ��ʾ������
//����Ŀ�����һ��float ��double ��������ʾ������
//�����Ǿ�ȷ��ʾ������Ҫ��ԱȽϾ�ȷ�����ʱ��Ϊ����Ǿ�ȷ��ʾ�����ļ�������������������ȷ��ʾ
typedef struct AVRational
{
    int num; // numerator   // ����
    int den; // denominator // ��ĸ
} AVRational;

static inline double av_q2d(AVRational a)
{
    return a.num / (double)a.den;
}

#endif

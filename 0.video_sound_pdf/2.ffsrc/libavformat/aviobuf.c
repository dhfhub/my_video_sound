//�л���Ĺ����ļ�ByteIOContext ��ص��ļ�����������open�� read�� close�� seek �ȵ�
#include "avformat.h"
#include "avio.h"
#include <stdarg.h>

#define IO_BUFFER_SIZE 32768

//��ʼ�������ļ� ByteIOContext �ṹ��һЩ�򵥵ĸ�ֵ����
int init_put_byte(ByteIOContext *s,
		unsigned char *buffer,
		int buffer_size,
		int write_flag,
		void *opaque,
		int(*read_buf)(void *opaque, uint8_t *buf, int buf_size),
		int(*write_buf)(void *opaque, uint8_t *buf, int buf_size),
		offset_t(*seek)(void *opaque, offset_t offset, int whence))
{
	s->buffer = buffer;
	s->buffer_size = buffer_size;
	s->buf_ptr = buffer;
	s->write_flag = write_flag;
	if (!s->write_flag)
		s->buf_end = buffer;// ��ʼ����£�������û��Ч���ݣ�����buf_end ָ�򻺴��׵�ַ��
	else
		s->buf_end = buffer + buffer_size;
	s->opaque = opaque;
	s->write_buf = write_buf;
	s->read_buf = read_buf;
	s->seek = seek;
	s->pos = 0;
	s->must_flush = 0;
	s->eof_reached = 0;
	s->error = 0;
	s->max_packet_size = 0;

	return 0;
}

//�����ļ� ByteIOContext ��seek ������
//��������� sΪ�����ļ������ offsetΪƫ������ whence Ϊ��λ��ʽ��
//�����������Թ����ļ���ʼ��ƫ����
offset_t url_fseek(ByteIOContext *s, offset_t offset, int whence)
{
	printf("url_fseek ByteIOContext * addresss:%x, offset_t:%d, whence:%d\n",
			s, offset, whence);
	offset_t offset1;

	//ֻ֧��SEEK_CUR ��SEEK_SET ��λ��ʽ�� ��֧��SEEK_END ��ʽ��
	//SEEK_CUR:���ļ���ǰ��дλ��Ϊ��׼ƫ�� offset�ֽڡ�
	//SEEK_SET:���ļ���ʼλ��ƫ�� offset�ֽڡ�
	if (whence != SEEK_CUR && whence != SEEK_SET)
		return  - EINVAL;

	//ffplay ��SEEK_CUR ��SEEK_SET ͳһ��SEEK_SET ��ʽ����
	//���������SEEK_CUR ��ʽ��Ҫת����SEEK_SET ��ƫ������
	//offset1 = s->pos - (s->buf_end - s->buffer) + (s->buf_ptr - s->buffer) ��ʽ��ϵ�����3.6�ڵ�ʾ��ͼ��
	//��ʾ�����ļ��ĵ�ǰʵ��ƫ��
	if (whence == SEEK_CUR)
	{
		printf("SEEK_CUR\n");
		printf("s->pos:%d, s->buf_end:%d ,s->buffer:%d ,s->buf_ptr:%d , s->buffer:%d\n",
				s->pos, s->buf_end  ,s->buffer ,s->buf_ptr , s->buffer);

		offset1 = s->pos - (s->buf_end - s->buffer) + (s->buf_ptr - s->buffer);
		printf("url_fseek, offset1:%d\n", offset1);
		if (offset == 0)
			return offset1;// ���ƫ����Ϊ0������ʵ��ƫ��λ��
		//����ʵ��ƫ������ͳһ����Թ����ļ���ʼ�ľ���ƫ����
		//�������ƫ��������ֵ��offset
		offset += offset1;
	}
	//�������ƫ������Ե�ǰ�����ƫ��������ֵ��offset1��
	offset1 = offset - (s->pos - (s->buf_end - s->buffer));
	printf("offset1:%d\n", offset1);//0
	//�жϾ���ƫ�����Ƿ��ڵ�ǰ�����У�����ڵ�ǰ�����У��ͼ򵥵��޸�buf_ptrָ��
	if (offset1 >= 0 && offset1 <= (s->buf_end - s->buffer))
	{
		printf("can do the seek inside the buffer\n");
		printf("s->buf_ptr:%x, s->buffer:%x\n", s->buf_ptr, s->buffer);//s->buf_ptr:10003b40, s->buffer:10003b10
		s->buf_ptr = s->buffer + offset1; // can do the seek inside the buffer
	}
	else
	{
		if (!s->seek)
			return  - EPIPE;
		//���õײ������ļ�ϵͳ��seek�������ʵ�ʵ�seek ��������ʱ���������³�ʼ����
		//buf_end����ָ�򻺴��׵�ַ�����޸�pos����Ϊ�����ļ���ǰʵ��ƫ������
		s->buf_ptr = s->buffer;
		s->buf_end = s->buffer;
		if (s->seek(s->opaque, offset, SEEK_SET) == (offset_t) - EPIPE)
			return  - EPIPE;
		s->pos = offset;
	}
	s->eof_reached = 0;
	//���ع����ļ���ǰ��ʵ��ƫ����
	return offset;
}

//�����ļ� ByteIOContext�ĵ�ǰʵ��ƫ������ƫ�� offset�ֽڣ����� url_fseekʵ�֡�
void url_fskip(ByteIOContext *s, offset_t offset)
{
	url_fseek(s, offset, SEEK_CUR);
}

//���ع����ļ� ByteIOContext�ĵ�ǰʵ��ƫ������
offset_t url_ftell(ByteIOContext *s)
{
	return url_fseek(s, 0, SEEK_CUR);
}

//���ع����ļ� ByteIOContext�Ĵ�С
offset_t url_fsize(ByteIOContext *s)
{
	offset_t size;
	//�жϵ�ǰ�����ļ� ByteIOContext�Ƿ��� seek��������ܾͷ��ش���
	if (!s->seek)
		return  - EPIPE;

	//���õײ��seek����ȡ���ļ���С
	size = s->seek(s->opaque,  - 1, SEEK_END) + 1;
	//ע��seek�����ı��˶�ָ�룬����Ҫ����seek ����ǰ��ָ��λ��
	s->seek(s->opaque, s->pos, SEEK_SET);
	return size;
}

//�жϵ�ǰ�����ļ� ByteIOContext�Ƿ�ĩβ
int url_feof(ByteIOContext *s)
{
	return s->eof_reached;
}

int url_ferror(ByteIOContext *s)
{
	return s->error;
}

// Input stream
//�������ļ� ByteIOContext �ڲ������ݻ�����
static void fill_buffer(ByteIOContext *s)
{
	printf("fill_buffer()\n");
	int len;

	//������˹����ļ� ByteIOContext ĩβ��ֱ�ӷ���
	if (s->eof_reached)
		return ;
	//���õײ��ļ�ϵͳ�Ķ�����ʵ�ʶ���������棬ע�����ﾭ���˺ü�����ת�ŵ��ײ��������
	//������ת�� url_read_buf()����������ת��url_read()������ת��ʵ���ļ�Э��Ķ�������ɶ�����
	len = s->read_buf(s->opaque, s->buffer, s->buffer_size);
	printf("url_read_buf return len:%d\n", len);//32768
	if (len <= 0)
	{   // do not modify buffer if EOF reached so that a seek back can be done without rereading data
		// ����ǵ����ļ�ĩβ�Ͳ�Ҫ�� buffer�����������������¶����ݾͿ�����seek back����
		s->eof_reached = 1;

		//���ô����룬���ڷ�����λ��
		if (len < 0)
			s->error = len;
	}
	else
	{
		printf("before:\n");
		printf("s->pos:%d\n",s->pos);
		printf("s->buf_ptr:%x\n", s->buf_ptr);
		printf("s->buf_end:%x\n", s->buf_end);

		//�����ȷ��ȡ���޸�һ�»����������μ� 3.6���е�ʾ��ͼ
		s->pos += len;
		s->buf_ptr = s->buffer;
		s->buf_end = s->buffer + len;

		printf("after\n");
		printf("s->pos:%d\n",s->pos);
		printf("s->buf_ptr:%x\n", s->buf_ptr);
		printf("s->buf_end:%x\n", s->buf_end);
	}
}//end fill_buffer()

//�ӹ����ļ� ByteIOContext �ж�ȡһ���ֽڡ�
int get_byte(ByteIOContext *s) // NOTE: return 0 if EOF, so you cannot use it if EOF handling is necessary
{
	//printf("get_byte(), s->buf_ptr:%x , s->buf_end:%x\n", s->buf_ptr , s->buf_end);
	if (s->buf_ptr < s->buf_end)
	{
		//��������ļ� ByteIOContext�ڲ����������ݣ����޸Ķ�ָ�룬���ض�ȡ������
		return  *s->buf_ptr++;
	}
	else
	{
		//��������ļ� ByteIOContext�ڲ�����û�����ݣ���������ڲ�����
		fill_buffer(s);
		//��������ļ� ByteIOContext �ڲ����������ݣ����޸Ķ�ָ�룬���ض�ȡ�����ݡ�
		//���û�����ݾ��� �����ļ�ĩβ������ 0��
		//NOTE: return 0 if EOF, so you cannot use it if EOF handling is necessary
		if (s->buf_ptr < s->buf_end)
			return  *s->buf_ptr++;
		else
			return 0;
	}
}

//�ӹ����ļ� ByteIOContext ����С�˷�ʽ��ȡ�����ֽ�,ʵ�ִ����ָ���get_byte()����
unsigned int get_le16(ByteIOContext *s)
{
	unsigned int val;
	val = get_byte(s);
	val |= get_byte(s) << 8;
	return val;
}

//�ӹ����ļ� ByteIOContext ����С�˷�ʽ��ȡ�ĸ��ֽ�,ʵ�ִ����ָ���get_le16()����
unsigned int get_le32(ByteIOContext *s)
{
	unsigned int val;
	val = get_le16(s);
	val |= get_le16(s) << 16;
	return val;
}

#define url_write_buf NULL

//����ת����������
static int url_read_buf(void *opaque, uint8_t *buf, int buf_size)
{
	URLContext *h = (URLContext *)opaque;
	return url_read(h, buf, buf_size);
}

static offset_t url_seek_buf(void *opaque, offset_t offset, int whence)
{
	URLContext *h = (URLContext *)opaque;
	return url_seek(h, offset, whence);
}

//���ò���������ļ� ByteIOContext�ڲ�����Ĵ�С�������Ӧ�����޸��ڲ������С���ϡ�
int url_setbufsize(ByteIOContext *s, int buf_size) // must be called before any I/O
{
	uint8_t *buffer;
	//��������ļ� ByteIOContext�ڲ����档
	buffer =(uint8_t *) av_malloc(buf_size);
	if (!buffer)
		return  - ENOMEM;

	//�ͷŵ�ԭ�������ļ� ByteIOContext���ڲ����棬����һ�����յĲ���
	av_free(s->buffer);
	//���ù����ļ� ByteIOContext �ڲ�������ز���
	s->buffer = buffer;
	s->buffer_size = buf_size;
	s->buf_ptr = buffer;
	if (!s->write_flag)
		s->buf_end = buffer;// ��Ϊ��ʱֻ�Ƿ������ڴ�,��û�ж�������,����buf_end ָ���׵�ַ
	else
		s->buf_end = buffer + buf_size;
	return 0;
}

//�򿪹����ļ� ByteIOContext
int url_fopen(ByteIOContext *s, const char *filename, int flags)
{
	URLContext *h;
	uint8_t *buffer;
	int buffer_size, max_packet_size;
	int err;

	err = url_open(&h, filename, flags);
	if (err < 0){
		printf("url_open fail:%d\n",err);
		return err;
	}
	//��ȡ�ײ��ļ�ϵͳ֧�ֵ�������С������� 0��������Ϊ�ڲ�����Ĵ�С��
	//�����ڲ���������ΪĬ�ϴ�С IO_BUFFER_SIZE(32768 �ֽ�)
	max_packet_size = url_get_max_packet_size(h);
	if (max_packet_size)
	{
		buffer_size = max_packet_size; // no need to bufferize more than one packet
	}
	else
	{
		buffer_size = IO_BUFFER_SIZE;
	}

	//��������ļ� ByteIOContext�ڲ����棬�������͹ر��ļ����ش�����
	buffer = (uint8_t *)av_malloc(buffer_size);
	if (!buffer)
	{
		url_close(h);
		return  - ENOMEM;
	}

	//��ʼʼ�������ļ� ByteIOContext���ݽṹ���������͹ر��ļ����ͷ��ڲ����棬���ش�����
	if (init_put_byte(s,
				buffer,
				buffer_size,
				(h->flags & URL_WRONLY || h->flags & URL_RDWR),
				h,
				url_read_buf,
				url_write_buf,
				url_seek_buf) < 0)
	{
		url_close(h);
		av_free(buffer);
		return AVERROR_IO;
	}

	//����������С
	s->max_packet_size = max_packet_size;

	return 0;
}

//�رչ����ļ� ByteIOContext�������ͷŵ��ڲ�ʹ�õĻ��棬
//�ٰ��Լ����ֶ���0�����ת��ײ��ļ�ϵͳ�Ĺرպ���ʵ���Թر��ļ�
int url_fclose(ByteIOContext *s)
{
	URLContext *h = (URLContext *)s->opaque;
	av_free(s->buffer);
	memset(s, 0, sizeof(ByteIOContext));
	return url_close(h);
}

//�����ļ� ByteIOContext��������ע��˺�����get_buffer ����������
//�����к������ܣ�ҲΪ���걸�����ļ�������������
int url_fread(ByteIOContext *s, unsigned char *buf, int size) // get_buffer
{
	printf("url_fread()\n");
	int len, size1;
	//���ǵ�size ���ܱȻ����е����ݴ�ö࣬ ��ʱ�Ͷ�ζ����棬
	//������size1 ����Ҫ��ȡ�����ֽ�����size������Ϊ����Ҫ��ȡ���ֽ���
	size1 = size;
	printf("size:%d\n", size);
	printf("size1:%d\n", size1);

	//�������Ҫ�����ֽ�������0���ͽ���ѭ��������
	while (size > 0)
	{
		//���㵱��ѭ��Ӧ�ö�ȡ���ֽ��� len����������lenΪ�ڲ��������ݳ��ȣ�
		//�ٺ���Ҫ�����ֽ��� size �ȣ� ���������� len��ֵ��
		len = s->buf_end - s->buf_ptr;
		printf("before len:%d\n", len);
		if (len > size)
			len = size;
		printf("after len:%d\n", len);
		//����ڲ�����û������
		if (len == 0)
		{
			printf("buffer_size:%d\n", s->buffer_size);//32768
			//���Ҫ��ȡ�����������ڲ�������������
			//�͵��õײ㺯����ȡ�����ƹ��ڲ�����ֱ�ӵ�Ŀ�껺��
			//��������ͺ��ѳ���
			if (size > s->buffer_size)
			{
				len = s->read_buf(s->opaque, buf, size);
				//����ײ��ļ�ϵͳ�����������ļ�ĩβ��Ǻʹ����룬
				//����ѭ��������ʵ�ʶ������ֽ���
				if (len <= 0)
				{
					s->eof_reached = 1;
					if (len < 0)
						s->error = len;
					break;
				}
				else
				{
					//����ײ��ļ�ϵͳ��ȷ�����޸���ز�����������һ��ѭ�����ر�ע��˴����ļ��ƹ����ڲ����档
					s->pos += len;
					size -= len;
					buf += len;// ��Ϊ�ƹ����ڲ����棬�ر�ע��˴����޸�
					s->buf_ptr = s->buffer;
					s->buf_end = s->buffer /* + len*/;// ��Ϊ�ƹ����ڲ����棬�ر�ע��˴����޸�
				}
			}
			else
			{
				//���Ҫ��ȡ�����������ڲ�����������С���͵��õײ㺯����ȡ���ݵ��ڲ����棬�ж϶��ɹ���
				fill_buffer(s);
				len = s->buf_end - s->buf_ptr;
				if (len == 0)
					break;
			}
		}
		else
		{
			printf("len !=0 ,size:%d, len:%d\n", size, len);
			//����ڲ����������ݣ� �Ϳ���len ���ȵ����ݵ��������� ���޸���ز����� ������һ��ѭ���������жϡ�
			memcpy(buf, s->buf_ptr, len);
			buf += len;
			s->buf_ptr += len;
			size -= len;
		}
	}
	//����ʵ�ʶ�ȡ���ֽ�����
	return size1 - size;
}

//识别文件格式和媒体格式部分使用的一些工具类函数。

#include "../berrno.h"
#include "avformat.h"
#include <assert.h>

#define UINT_MAX  (0xffffffff)

#define PROBE_BUF_MIN 2048
#define PROBE_BUF_MAX 131072

//注册文件容器格式。 ffplay 把所有支持的文件容器格式用链表串联起来，表头是first_iformat。
AVInputFormat *first_iformat = NULL;

void av_register_input_format(AVInputFormat *format)
{
	AVInputFormat **p;
	p = &first_iformat;
	//循环移动节点指针到最后一个文件容器格式。
	while (*p != NULL)
		p = &(*p)->next;
	//直接挂接要注册的文件容器格式。
	*p = format;
	format->next = NULL;
}

//比较文件的扩展名来识别文件类型。
int match_ext(const char *filename, const char *extensions)
{
	const char *ext,  *p;
	char ext1[32],  *q;

	if (!filename)
		return 0;

	//用'.'号作为扩展名分割符，在文件名中找扩展名分割符。
	ext = strrchr(filename, '.');
	if (ext)
	{
		ext++;
		p = extensions;
		for (;;)
		{
			//文件名中可能有多个标点符号，
			//取两个标点符号间或一个标点和一个结束符间的字符串和扩展名比较来判断文件类型，
			//所以可能要多次比较，所以这里有一个循环
			q = ext1;
			//定位下一个标点符号或字符串结束符，把这之间的字符拷贝到扩展名字符数组中。
			while (*p != '\0' &&  *p != ',' && q - ext1 < sizeof(ext1) - 1)
				*q++ =  *p++;
			//添加扩展名字符串结束标记 0。
			*q = '\0';
			//比较识别的扩展名是否后给定的扩展名相同，如果相同就返回 1，否则继续。
			if (!strcasecmp(ext1, ext))
				return 1;
			//判断是否到了文件名末尾，如果是就返回，否则进入下一个循环
			if (*p == '\0')
				break;
			p++;
		}
	}
	//如果在前面的循环中没有匹配到扩展名，就是不识别的文件类型，返回 0
	return 0;
}

//探测输入的文件容器格式，返回识别出来的文件格式。如果没有识别出来，就返回初始值NULL。
AVInputFormat *av_probe_input_format(AVProbeData *pd, int is_opened)
{
	AVInputFormat *fmt1,  *fmt;
	int score, score_max;

	fmt = NULL;
	//score， score_max 可以理解识别文件容器格式的正确级别。
	//文件容器格式识别结果，如果完全正确可以设定为100，如果可能正确可以设定为50，没识别出来设定为0。
	//识别方法不同导致等级不同
	score_max = 0;
	for (fmt1 = first_iformat; fmt1 != NULL; fmt1 = fmt1->next)
	{
		if (!is_opened)
			continue;

		score = 0;
		if (fmt1->read_probe)
		{
			//读取文件头，判断文件头的内容来识别文件容器格式，这种识别方法非常可靠，设定score为100。
			score = fmt1->read_probe(pd);
		}
		else if (fmt1->extensions)
		{
			//通过文件扩展名来识别文件容器格式，因为文件扩展名任何人都可以改，如果改变扩展名，这种方法
			//就错误，如果不改变扩展名，这种识别方法有点可靠，综合等级为50
			if (match_ext(pd->filename, fmt1->extensions))
				score = 50;
		}
		//如果识别出来的等级大于最大要求的等级，就认为正确识别，相关参数赋值后，
		//进下一个循环，最后 返回最高级别对应的文件容器格式。
		if (score > score_max)
		{
			score_max = score;
			fmt = fmt1;
		}
	}
	//返回文件容器格式，如果没有识别出来，返回的是初始值NULL。
	return fmt;
}

//打开输入流，其中AVFormatParameters *ap参数在瘦身后的 ffplay中没有用到， 保留为了不改变接口。
int av_open_input_stream(AVFormatContext **ic_ptr, ByteIOContext *pb, const char *filename,
		AVInputFormat *fmt, AVFormatParameters *ap)
{
	int err;
	AVFormatContext *ic;
	AVFormatParameters default_ap;

	if (!ap)
	{
		ap = &default_ap;
		memset(ap, 0, sizeof(default_ap));
	}
	//分配 AVFormatContext内存，部分成员变量在接下来的程序代码中赋值，
	//部分成员变量在下面调用的 ic->iformat->read_header(ic, ap)函数中赋值。
	ic = (AVFormatContext*) av_mallocz(sizeof(AVFormatContext));
	if (!ic)
	{
		err = AVERROR_NOMEM;
		goto fail;
	}
	//关联 AVFormatContext和 AVInputFormat
	ic->iformat = fmt;

	//关联 AVFormatContext和广义文件 ByteIOContext
	if (pb)
		ic->pb =  *pb;

	//分配 priv_data指向的内存。
	if (fmt->priv_data_size > 0)
	{
		ic->priv_data = av_mallocz(fmt->priv_data_size);
		if (!ic->priv_data)
		{
			err = AVERROR_NOMEM;
			goto fail;
		}
	}
	else
	{
		ic->priv_data = NULL;
	}

	//读取文件头，识别媒体流格式。
	err = ic->iformat->read_header(ic, ap);
	if (err < 0)
		goto fail;

	*ic_ptr = ic;
	return 0;

fail:
	if (ic)
		av_freep(&ic->priv_data);

	av_free(ic);
	*ic_ptr = NULL;
	return err;
}//end  av_open_input_stream()

//打开输入文件，并识别文件格式，然后调用函数识别媒体流格式。
int av_open_input_file(AVFormatContext **ic_ptr, const char *filename, AVInputFormat *fmt,
		int buf_size, AVFormatParameters *ap)
{
	printf("av_open_input_file()\n");
	int err, must_open_file, file_opened, probe_size;
	AVProbeData probe_data,  *pd = &probe_data;
	ByteIOContext pb1,  *pb = &pb1;

	file_opened = 0;
	pd->filename = "";
	if (filename)
		pd->filename = filename;
	pd->buf = NULL;
	pd->buf_size = 0;

	must_open_file = 1;

	if (!fmt || must_open_file)
	{
		//打开输入文件，关联 ByteIOContext， 经过跳转几次后才实质调用文件系统open()函数实质打开文件。
		if (url_fopen(pb, filename, URL_RDONLY) < 0)
		{
			err = AVERROR_IO;
			goto fail;
		}
		file_opened = 1;
		//如果程序指定 ByteIOContext内部使用的缓存大小，就重新设置内部缓存大小。通常不指定大小。
		if (buf_size > 0)
			url_setbufsize(pb, buf_size);
		//先读 PROBE_BUF_MIN(2048)字节文件开始数据识别文件格式，如果不能识别文件格式，就把识别文
		//件缓存以 2倍的增长扩大再读文件开始数据识别，直到识别出文件格式或者超过 131072字节缓存。
		for (probe_size = PROBE_BUF_MIN; probe_size <= PROBE_BUF_MAX && !fmt; probe_size <<= 1)
		{
			//重新分配缓存，重新读文件开始数据。
			pd->buf = (unsigned char*) av_realloc(pd->buf, probe_size);
			pd->buf_size = url_fread(pb, pd->buf, probe_size);
			printf("url_fread():%d\n", pd->buf_size);
			//把文件读指针 seek到文件开始处，便于下一次读。
			if (url_fseek(pb, 0, SEEK_SET) == (offset_t) - EPIPE)
			{
				//如果 seek错误，关闭文件，再重新打开。
				url_fclose(pb);
				if (url_fopen(pb, filename, URL_RDONLY) < 0)
				{
					//重新打开文件出错，设置错误码，跳到错误处理。
					file_opened = 0;
					err = AVERROR_IO;
					goto fail;
				}
			}
			//重新识别文件格式，因为一次比一次数据多，数据少的时候可能识别不出，数据多了可能就可以了。
			fmt = av_probe_input_format(pd, 1);
		}
		av_freep(&pd->buf);
	}

	if (!fmt)
	{
		err = AVERROR_NOFMT;
		goto fail;
	}

	//识别出文件格式后，调用函数识别流 av_open_input_stream格式。
	err = av_open_input_stream(ic_ptr, pb, filename, fmt, ap);
	if (err)
		goto fail;
	return 0;

fail:
	av_freep(&pd->buf);
	if (file_opened)
		url_fclose(pb);
	*ic_ptr = NULL;
	return err;
}//end 识别出文件格式后，调用函数识别流 av_open_input_stream格式。

//一次读取一个数据包，在瘦身后的ffplay中，一次读取一个完整的数据帧，数据包。
int av_read_packet(AVFormatContext *s, AVPacket *pkt)
{
	return s->iformat->read_packet(s, pkt);
}

//添加索引到索引表。
//有些媒体文件为便于seek，有音视频数据帧有索引， ffplay 把这些索引以时间排序放到一个数据中。
//返回值添加项的索引
int av_add_index_entry(AVStream *st, int64_t pos, int64_t timestamp, int size, int distance, int flags)
{
	AVIndexEntry *entries,  *ie;
	int index;

	//索引项越界判断，如果占有内存达到 UINT_MAX时，返回。
	if ((unsigned)st->nb_index_entries + 1 >= UINT_MAX / sizeof(AVIndexEntry)) // 越界判断
		return  - 1;
	//重新分配索引内存。
	//注意av_fast_realloc()函数并不是每次调用就一定会重新分配内存，那样效率就太低了。
	entries =(AVIndexEntry *) av_fast_realloc(st->index_entries,(unsigned int *) &st->index_entries_allocated_size, (st->nb_index_entries + 1) * sizeof(AVIndexEntry));
	if (!entries)
		return  - 1;

	//保持重新分配内存后，索引的首地址。
	st->index_entries = entries;

	//以时间为顺序查找当前索引应该插在索引表的位置。
	index = av_index_search_timestamp(st, timestamp, AVSEEK_FLAG_ANY);

	if (index < 0)	// 后续
	{
		//续补，既接着最后一个插入，索引计算加 1，取得索引项指针，便于后面赋值操作。
		index = st->nb_index_entries++;
		ie = &entries[index];
		assert(index == 0 || ie[ - 1].timestamp < timestamp);
	}
	else			// 中插
	{
		//中插，既插入索引表的中间，取得索引项指针，便于后面赋值操作。
		ie = &entries[index];
		if (ie->timestamp != timestamp)
		{
			if (ie->timestamp <= timestamp)
				return  - 1;
			//把索引项后面的项全部后移一项，空出当前索引项。
			memmove(entries + index + 1, entries + index,
					sizeof(AVIndexEntry)*(st->nb_index_entries - index));
			st->nb_index_entries++;
		}
	}
	//修改索引项参数，完成排序添加。
	ie->pos = pos;
	ie->timestamp = timestamp;
	ie->size = size;
	ie->flags = flags;
	//返回索引。
	return index;
}

//以时间为关键字查找当前索引应排在索引表中的位置。
int av_index_search_timestamp(AVStream *st, int64_t wanted_timestamp, int flags)
{
	AVIndexEntry *entries = st->index_entries;
	int nb_entries = st->nb_index_entries;
	int a, b, m;
	int64_t timestamp;

	a =  - 1;
	b = nb_entries;
	//以时间为关键字折半查找位置，请仔细理解。
	while (b - a > 1) //并没有记录idx值，采用的是折半查找
	{
		m = (a + b) >> 1;
		timestamp = entries[m].timestamp;
		if (timestamp >= wanted_timestamp)
			b = m;
		if (timestamp <= wanted_timestamp)
			a = m;
	}

	m = (flags &AVSEEK_FLAG_BACKWARD) ? a : b;

	if (!(flags &AVSEEK_FLAG_ANY))
	{
		//Seek 时，找关键帧，从关键帧开始解码，注意有些帧解码但不显示。
		while (m >= 0 && m < nb_entries && !(entries[m].flags &AVINDEX_KEYFRAME))
		{
			m += (flags &AVSEEK_FLAG_BACKWARD) ?  - 1: 1;
		}
	}
	if (m == nb_entries)
		return  - 1;

	//返回找到的位置。
	return m;
}

//关闭输入媒体文件，一大堆的关闭释放操作。
void av_close_input_file(AVFormatContext *s)
{
	int i;
	AVStream *st;

	if (s->iformat->read_close)
		s->iformat->read_close(s);

	for (i = 0; i < s->nb_streams; i++)
	{
		st = s->streams[i];
		av_free(st->index_entries);
		av_free(st->actx);
		av_free(st);
	}

	url_fclose(&s->pb);

	av_freep(&s->priv_data);
	av_free(s);
}

//new 一个新的媒体流，返回AVStream 指针
AVStream *av_new_stream(AVFormatContext *s, int id)
{
	AVStream *st;
	//判断媒体流的数目是否超限，如果超过就丢弃当前流返回 NULL。
	if (s->nb_streams >= MAX_STREAMS)
		return NULL;

	st = (AVStream *)av_mallocz(sizeof(AVStream));
	if (!st)
		return NULL;
	//通过 avcodec_alloc_context分配一块 AVFormatContext内存，并关联到 AVStream。
	st->actx = avcodec_alloc_context();

	//关联 AVFormatContext 和AVStream。
	s->streams[s->nb_streams++] = st;
	return st;
}

//设置计算 pts时钟的相关参数。
void av_set_pts_info(AVStream *s, int pts_wrap_bits, int pts_num, int pts_den)
{
	s->time_base.num = pts_num;
	s->time_base.den = pts_den;
}

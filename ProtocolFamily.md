## ProtocolFamily ##

ProtocolFamily是对服务器/客户端传输数据的封装。包括协议Protocol协议和ProtocolHeader协议头的。

**1. ProtocolHeader协议头**

框架对协议头的调用非常简单，只需要很少的几个方法：
```
class ProtocolHeader
{
public:
	virtual ~ProtocolHeader(){}
	
	//协议头长度
	virtual int get_header_length()=0;

	//编码协议头.成功返回true,失败返回false
	//buf:存放协议头编码后数据的缓冲区
	//body_length:协议体长度
	virtual bool encode(char *buf, int body_length)=0;

	//解码协议头.成功返回true,失败返回false
	//io_buffer:待解码的协议头数据
	//body_length:解码后返回的协议体长度
	virtual bool decode(const char *buf, int &body_length)=0;
};
```
> 其中encode方法用来编码协议头数据到buf缓冲区，其要求传入一个协议体的长度。而decode方法则用来从缓冲区buf解码协议头数据，编码成功后返回一个协议体长度body\_length，用来获取协议体数据。

**2. Protocol协议**
> 每个协议都包含一个协议头和协议体数据。协议头的编/解码由ProtocolHeader完成，而协议体数据的编/解码则由encode\_body和decode\_body方法完成：
```
//编码协议体数据到byte_buffer,成功返回true,失败返回false.
virtual bool encode_body(ByteBuffer *byte_buffer)=0;

//解码大小为size的协议体数据buf.成功返回true,失败返回false.
virtual bool decode_body(const char *buf, int size)=0;
```
> 每个具体的协议都需要实现这个两个接口方法来完成编解码。当发送协议时，框架（NetInterface)主动调用Protocol的encode方法，对协议头和协议体进行编码，得到编码后的数据，然后进行发送。encode方法如下：
```
inline
bool Protocol::encode()
{
	if(m_raw_data != NULL) //已经编码过(或者attach_raw_data过)
		return true;

	assert(m_protocol_header != NULL);
	int header_length = m_protocol_header->get_header_length();
	m_raw_data = new ByteBuffer;
	m_raw_data->get_append_buffer(header_length);
	m_raw_data->set_append_size(header_length);
	//编码协议体
	if(encode_body(m_raw_data) == false)
	{
		delete m_raw_data;
		m_raw_data = NULL;
		return false;
	}
	//编码协议头
	int body_length = m_raw_data->size()-header_length;
	char *header_buffer = m_raw_data->get_data();
	if(m_protocol_header->encode(header_buffer, body_length) == false)
	{
		delete m_raw_data;
		m_raw_data = NULL;
		return false;
	}
	return true;
}
```

**3. ProtocolFamily协议族**
> 协议族是一个工厂类，用来创建/销毁具体的协议。开发者需要实现如下几个接口方法：
```
class ProtocolFamily
{
public:
	virtual ~ProtocolFamily(){}
	//创建协议头
	virtual ProtocolHeader* create_protocol_header()=0;
	//销毁协议头
	virtual void destroy_protocol_header(ProtocolHeader *header)=0;

	//创建协议(根据协议头包含的信息创建具体的协议)
	virtual Protocol* create_protocol_by_header(ProtocolHeader *header)=0;
	//销毁协议
	virtual void destroy_protocol(Protocol *protocol)=0;
};
```
> 在应用中，协议的创建和销毁是非常频繁的操作，通过实现不同创建协议的策略，比如使用内存管理模块来控制内存的分配释放，能够避免产生内存碎片，提供效率。关于在协议族中使用内存管理模块分配空间可见 [多任务/多线程下载例子](MThreadDownload.md)。

**4. 其他**
> 为了方便数据的编解码，我们提供了几个有用的宏定义用来编码字符、整数、字符串等：
```
////编码字符
#define ENCODE_CHAR(c) do{ \
	if(!byte_buffer->append(c)) return false; \
}while(0)
////解码字符
#define DECODE_CHAR(c) do{ \
	if(size < sizeof(c)) return false; \
	c = buf[0]; ++buf; --size; \
}while(0)

////编码整数
#define ENCODE_INT(i) do{ \
	if(!byte_buffer->append((const char*)&i, sizeof(i))) return false; \
}while(0)

////解码整数
#define DECODE_INT(i) do{ \
	if(size < sizeof(i)) return false; \
	i = *(int*)buf; buf+=sizeof(i); size-=sizeof(i); \
}while(0)

////编码64位整数
#define ENCODE_INT64(i) do{ \
	if(!byte_buffer->append((const char*)&i, sizeof(i))) return false; \
}while(0)

////解码整数
#define DECODE_INT64(i) do{ \
	if(size < sizeof(i)) return false; \
	i = *(int64_t*)buf; buf+=sizeof(i); size-=sizeof(i); \
}while(0)

////编码字符串
#define ENCODE_STRING(str) do{ \
	int len = str.size(); \
	if(!byte_buffer->append((const char*)&len, sizeof(len))) return false; \
	if(len>0 && !byte_buffer->append(str.c_str())) return false; \
}while(0)

////解码字符串
#define DECODE_STRING(str) do{ \
	int len = 0; \
	DECODE_INT(len); \
	if(len<0 || size<len) return false; \
	if(len > 0) {str.assign(buf, len); buf+=len; size-=len;}\
}while(0)

////编码C风格字符串
#define ENCODE_STRING_C(c_str) do{ \
	int len = strlen(c_str); \
	if(!byte_buffer->append((const char*)&len, sizeof(len))) return false; \
	if(len>0 && !byte_buffer->append(c_str)) return false; \
}while(0)

////解码C风格字符串
#define DECODE_STRING_C(c_str) do{ \
	int len = 0; \
	DECODE_INT(len); \
	if(len<0 || size<len) return false; \
	if(len > 0) {c_str=buf; buf+=len; size-=len;} \
	else c_str = NULL; \
}while(0)
```
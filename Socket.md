# Introduction #

  * **Socket**
对底层socket的封装.纯抽象类

  * **ListenSocket**
用于监听socket

  * **TransSocket**
用于连接,收发数据的socket

# Details #

  * **Socket的方法**
  * 构造函数
```
//socket_handle:socket描述符
//port:监听/连接的端口号
//ip:连接的地址
//block_mode:阻塞模式. BLOCK(阻塞)/NOBLOCK(非阻塞)
Socket(SocketHandle socket_handle=SOCKET_INVALID, int port=-1, const char *ip=NULL, BlockMode block_mode=NOBLOCK);
```

  * 赋值函数
```
bool assign(SocketHandle socket_handle, int port, const char *ip, BlockMode block_mode);
```

  * 获取属性
```
SocketHandle get_handle(){return m_socket_handle;}
int get_port(){return m_port;}
const char* get_ip(){return m_ip;}
BlockMode get_block_mode(){return m_block_mode;}
```

  * 打开socket
```
//成功返回true, 失败返回false
//timeout_out: 打开socket的超时时间.
virtual bool open(int timeout_ms=2000)=0;
```
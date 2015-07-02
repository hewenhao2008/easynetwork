## NetInterface ##

NetInterface是服务器框架的接口，其实对链接的请求、创建等进行管理，同时处理协议的编/解码、数据的发送。NetInterface继承于ConnectAccepter和EventHandler类。


# 成员方法 #
  * **实现ConnectAccepter**
```
public:
	//实现ConnectAccepter:接收一个新的连接请求
	virtual bool accept(SocketHandle trans_fd);
```
> accept方法被调用后，将收到的链接注册到IODemuxer进行监听，由注册的事件处理句柄处理链接的相应事件。

  * **实现EventHandler的接口**
```
public:
	//重写EventHandler:实现trans socket的读写
	virtual HANDLE_RESULT on_readable(int fd);
	virtual HANDLE_RESULT on_writeable(int fd);
	virtual HANDLE_RESULT on_timeout(int fd);  //to do deal with timeout
	virtual HANDLE_RESULT on_error(int fd); //to do deal with error
```
    * 当某个链接的读事件发生时，on\_readable方法将被调用。on\_readable方法查找链接对应的TranSocket对象，然后开始接收一个协议数据：<br>1. 接收协议头数据并解码；<br>2. 接收协议体数据并解码；<br>3. 调用接口方法on_recv_protocol向应用层分发协议；<br>4. 如果协议数据未接收完整，接收到的数据将保存在TransSocket等待下一次的读事件发生；<br><br>
<ul><li>当某个链接的写事件发生时，on_writeable方法被调用。on_writeable方法查找链接对应的TransSocket对象，然后开始发送一个协议数据：<br>1. 从链接的待发送协议队列取一个协议；<br>将协议进行编码；<br>发送编码后的协议数据；<br>根据发送成功/失败调用on_protocol_send_succ/on_socket_handle_error接口上报应用层。<br><br>
</li><li>当某个链接超时时，on_timeout方法将被调用。on_timeout将会取消该链接上的所有待发送协议，然后调用on_socket_handle_timeout上报应用层。<br><br>
</li><li>当某个链接出现错误时将调用on_error方法。on_error将会取消该链接上的所有待发送协议，然后调用on_socket_handle_error上报给应用层。</li></ul></li></ul>

<ul><li><b>提供给应用层的接口方法</b>
</li></ul><blockquote>根据上面的分析，NetInterface提供给应用层的接口方法主要有：<br>
<pre><code>protected:<br>
    //返回值:true:成功, false:失败<br>
    ////由应用层实现 -- 接收协议函数<br>
    virtual bool on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, bool &amp;detach_protocol)=0;<br>
    ////由应用层实现 -- 协议发送错误处理函数<br>
    virtual bool on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol)=0;<br>
    ////由应用层实现 -- 协议发送成功处理函数<br>
    virtual bool on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)=0;<br>
    ////由应用层实现 -- 连接错误处理函数<br>
    virtual bool on_socket_handle_error(SocketHandle socket_handle)=0;<br>
    ////由应用层实现 -- 连接超时处理函数<br>
    virtual bool on_socket_handle_timeout(SocketHandle socket_handle)=0;<br>
    ////由应用层实现 -- 已经收到一个新的连接请求<br>
    virtual bool on_socket_handler_accpet(SocketHandle socket_handle)=0;<br>
</code></pre></blockquote>

<ul><li><b>NetInterface的初始化</b>
</li></ul><blockquote>!NetINterface组合了IODemuxer、SocketManager以及ProtocolFamily，因此在!NetINterface在被创建或者被使用前必须创造出这三者的实例。!NetINterface默认使用IODemuxerEpoll和SocketManager，但是ProtocolFamily是由具体的应用所决定，因此开发者需要实现create_protocol_family和destroy_protocol_family接口来创建/销毁具体的ProtocolFamily：<br>
<pre><code>protected:<br>
	////由应用层实现 -- 创建具体的协议族<br>
	virtual ProtocolFamily* create_protocol_family()=0;<br>
	////由应用层实现 -- 销毁协议族<br>
	virtual void delete_protocol_family(ProtocolFamily* protocol_family)=0;<br>
</code></pre>
换句话说，协议族的创建是放到NetInterface的子类中创建。既然这样，就不能在NetInterface的构造函数中去创建协议族实例，而需要另外提供一个时机来创建协议族实例。init_net_interface方法用来创建IODemuxer、SocketManger和ProtocolFamily的实例：<br>
<pre><code>bool NetInterface::init_net_interface()<br>
{<br>
	if(m_io_demuxer == NULL)<br>
		m_io_demuxer = create_io_demuxer();<br>
	if(m_socket_manager == NULL)<br>
		m_socket_manager = create_socket_manager();<br>
	if(m_protocol_family == NULL)<br>
		m_protocol_family = create_protocol_family();<br>
	return true;<br>
}<br>
</code></pre>
当NetInterface的子类实例被创建后，需要显示调用该方法来完成NetInterface的初始化。我们提供了一个start_server的接口方法，强制开发者显示调用该方法来启动NetInterface，因此在start_server中调用init_net_interface会是一个好的方案，框架在enetlib工具生产框架类代码的时候，显示生成了start_server对init_net_interface的调用：<br>
<pre><code>public:<br>
	////由应用层实现 -- net interface实例启动入口<br>
	virtual bool start_server()=0;<br>
...<br>
...<br>
<br>
//框架生成的代码<br>
bool STAppTemplate::start_server()<br>
{<br>
	////Init NetInterface<br>
	init_net_interface();<br>
<br>
	////Add your codes here<br>
	///////////////////////<br>
<br>
	return true;<br>
}<br>
</code></pre></blockquote>

<ul><li><b>其他成员方法</b>
</li></ul><blockquote>其他成员方法，用来管理链接、发送/接收协议数据：<br>
<pre><code>//获取主动链接<br>
virtual SocketHandle get_active_trans_socket(const char *ip, int port);<br>
//添加协议到发送队列.成功返回0.失败返回-1,需要自行处理protocol.<br>
virtual int send_protocol(SocketHandle socket_handle, Protocol *protocol, bool has_resp=false);<br>
//获取等待队列中待发送的协议<br>
virtual Protocol* get_wait_to_send_protocol(SocketHandle socket_handle);<br>
//获取等待队列中待发送的协议个数<br>
virtual int get_wait_to_send_protocol_number(SocketHandle socket_handle);<br>
//取消所有待发送协议,同时调用on_protocol_send_error通知应用层<br>
virtual int cancal_wait_to_send_protocol(SocketHandle socket_handle);<br>
</code></pre>
  * **服务器开发需要做什么**
    1. 当然你需要了解/熟悉/甚至精通网络编程,socket等等,这是基础
    1. 监听服务器端口
    1. 接受连接请求,管理这些链接(包括创建,销毁等等)
    1. 收/发协议数据.比如协议头编解码, 协议数据编解码(或者说序列化/反序列化).
    1. 处理这些协议等(主要跟服务器的业务逻辑有关).

  * **什么是框架**
> > 每个应用程序或者服务器server的开发, 都需要做一些基础的东西,然后再在这些基础的东西上完成业务逻辑层.可能每开发一个应用程序,你都需要重复做这些基础的东西.
> > 比如客户端开发,你需要创建一个对话框.或许你会用win32的api来完成,但是你可以直接使用MFC来完成你的工作,因为那更快(可能也更可靠).你们MFC就是这样一个框架,帮你完成了底层的一些东西.
> > 同样,服务端开发,你同样需要重复的做一些基础的东西:创建监听端口、接收连接请求、管理连接请求、sockket数据的读取，数据的编解码，数据的发送等等.或许你会考虑为这些过程开发响应的模块.
> > 作为应用层,应该把精力集中在业务逻辑的处理上, 或许根本就不需要去考虑连接怎么创建,怎么处理;数据怎么接收,怎么发送;怎么编解码等等这些问题.由你开发的这些模块来完成.那么由这些模块组成的一个有机体,可以称为框架吧.

  * **什么是EasyNetwork网络开发框架**
> > EasyNetWork网络开发框架,就是能够帮你完成创建连接,管理连接,完成数据收发,编解码的一个开发框架.只要实现框架的接口, 就可以接收协议请求,发送协议,能够让你集中于业务逻辑的开发.

  * **EasyNetwork框架扩展性如何**
> > EasyNetWork是可扩展的, 只要继承响应模块,并实现接口方法,就可以实现扩展.如IODemuxer, 你可以封装libevent来实现.如Protocol, 应用层可以封装自己的协议格式等.

  * **EasyNetwork怎么管理连接**
> > EasyNetWork由[SocketManager](SocketManager.md)模块来管理连接.

  * **EasyNetwork什么时候收/发数据**
> > EasyNetwork有IODemuxer模块来触发socket的读、写、超时、错误事件.并通过EventHandler回调类响应这些事件.当事件发送时,就可以进行数据的读写了.
> > 目前EasyNetwork用epoll实现了IODemuxer.

  * **EasyNetwork怎么收发数据**
> > 当socket可读时,由[TransSocket](TransSocket.md)完成数据的读取.如trans\_socket->recv\_buffer();
> > 当socket可写时,由[TransSocket](TransSocket.md)完成数据的发送,如trans\_socket->send\_buffer();

  * **EasyNetwork怎么编/解码(序列化/反序列化)数据**
> > 当socket读取到数据时. 通过trans\_socket->get\_recv\_buffer()获取到数据缓冲区,然后利用[protocol](protocol.md)来对数据进行解码.
> > 当需要发协议数据时, 由protocol来进行编码,然后把编码数据写入trans\_socket的write\_buffer中,进行发送数据.

  * **EasyNetwork支持自定义协议格式吗**
> > 可以,继承并实现Protocol的方法.具体见[StringProtocol](StringProtocol.md)例子

  * **EasyNetwork怎么处理粘包现象**
> > 由于数据是存在trans\_socket的IOBuffer中的,每次解码时从buffer中取出完整的数据进行解码,然后清掉这些完整的数据.如果buffer里面的数据不够一个完整的包,那么这些不完整的数据会留在IOBuffer中,直到收到完整的数据并被正确解码位置.

  * **EasyNetwork怎么处理高并发要求**
> > EasyNetwork实现了一个ConnectThread/ConnectThreadPool的多线程框架模型.当收到新的连接请求时,把新的连接分配到某个线程中去,由该线程处理此连接的数据.
> > 应用server继承并实现ConnectThread/ConnectThreadPool相应的接口,即可完成该多线程模型.

  * **EasyNetwork怎么处理高吞吐量要求**
> > EasyNetwork不关心次要求, 当收到协议protocol之后, 通过应用层实现的接口函数on\_receive\_protocol()把protocol发给应用层, 由应用层来负责解决高吞吐量要求.
> > 当然,应用层可以利用框架的Thread/ThreadPool来完成这个要求.

  * **EasyNetwork内存管理与优化**
> > 面对高并发(如每秒成千上万的连接请求),高吞吐量的问题(如每秒收发成千上万甚至更多的协议数据).EasyNetwork需要对TransSocket,Protocol等做优化,以防止内存碎片的产生.
> > 框架有MemManager模块完成内存的管理和优化.框架的Socket,Protocol等由MemManager产生和释放.

  * **如何利用EasyNetwork框架搭建自己的服务器**
> > 服务器可以继承NetInterface完成一个单线程的server,也可以实现ConnectThread和ConnectThreadPool完成多线程的server.具体例子见[AppServerSamples](AppServerSamples.md).

  * **服务器框架自动化生成工具server\_tool**
> > [敬请期待...]
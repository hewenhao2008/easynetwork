## TransProtocol ##

> TransProtocol主要为客户端或者不需要使用框架的地方提供一个编解码协议、传输数据的小工具类。使用TransProtocol提供的方法可以快速完成协议数据的发送和接收。
```
class TransProtocol
{
public:
	static bool send_protocol(TransSocket *trans_socket, Protocol *protocol);
	static bool recv_protocol(TransSocket *trans_socket, Protocol *protocol);
};
```
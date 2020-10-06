#include"chatserver.hpp"
#include"chatservice.hpp"

ChatServer::ChatServer(EventLoop* loop,const InetAddress& listenAddr,const string& nameArg):_server(loop,listenAddr,nameArg)
{
    //设置客户端登入与断开连接回调
    _server.setConnectionCallback(bind(&ChatServer::onconnection,this,_1));
    //设置客户端与服务端通信回调
    _server.setMessageCallback(bind(&ChatServer::onmessage,this,_1,_2,_3));
    //设置服务器线程数量
    _server.setThreadNum(4);
}
//开启服务端的服务
void ChatServer::start()
{
    _server.start();
}

void ChatServer::onconnection(const TcpConnectionPtr& conn)
{
    if(conn->connected())
    {
        cout<<conn->peerAddress().toIpPort()<<"->"<<conn->localAddress().toIpPort()<<" status:online"<<endl;
    }
    else
    {
        ChatService::instance()->clientCloseException(conn);
        cout<<conn->peerAddress().toIpPort()<<"->"<<conn->localAddress().toIpPort()<<" status:offnline"<<endl;
        conn->shutdown();
    }
}
//{"msgid":1,"id":1,"password":"123456"}
//{"msgid":3,"name":"li si","password":"123456"}
//{"msgid":1,"id":2,"password":"123456"}
//{"msgid":5,"to":2,"from":"zhouli","meg":"哈哈哈哈","id":1}
//{"msgid":5,"to":1,"from":"li si","meg":"挺好的!","id":2}
//{"msgid":6,"userid":1,"friendid":2}
void ChatServer::onmessage(const TcpConnectionPtr& conn,Buffer* buf, Timestamp time)
{
    string buffer = buf->retrieveAllAsString();
    json js = json::parse(buffer);
    ChatService::instance()->getHandler(js["msgid"].get<int>())(conn,js,time);
}
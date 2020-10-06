#ifndef CHATSERVER_H
#define CHATSERVER_H

#include<iostream>
#include<string>
#include<functional>
#include<memory>
#include<muduo/net/TcpServer.h>
#include<muduo/net/EventLoop.h>

using namespace std;
using namespace placeholders;
using namespace muduo;
using namespace muduo::net;


class ChatServer
{
    public:
        ChatServer(EventLoop* loop,const InetAddress& listenAddr,const string& nameArg);
        void start();

    private:

        void onconnection(const TcpConnectionPtr& conn);
        void onmessage(const TcpConnectionPtr& conn,Buffer* buf, Timestamp time);

        TcpServer _server;
        EventLoop *loop;
};
#endif
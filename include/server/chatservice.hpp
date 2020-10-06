#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include<muduo/net/TcpConnection.h>
#include<unordered_map>
#include<mutex>
#include<string>
#include<functional>
#include<iostream>
#include"usermodel.hpp"
#include"json.hpp"
#include"offlinemessage.hpp"
#include"groupmodel.hpp"
#include"friendmodel.hpp"
#include"redis.hpp"
using namespace muduo::net;
using namespace muduo;
using namespace std;

using json=nlohmann::json;
using MsgHandler = function<void(const TcpConnectionPtr &conn,json &js,Timestamp Timer)>;


class ChatService
{
    public:
        //获取唯一实例
        static ChatService *instance();

        //获取消息id对应的函数
        MsgHandler getHandler(int msgid);
        //登入业务
        void login(const TcpConnectionPtr &conn,json &js,Timestamp Timer);
        //注册业务
        void reg(const TcpConnectionPtr &conn,json &js,Timestamp Timer);
        //处理客户端异常退出
        void clientCloseException(const TcpConnectionPtr &conn);
        //一对一聊天业务
        void oneChat(const TcpConnectionPtr &conn,json &js,Timestamp Timer);
        //服务器接受重置用户状态
        void reset();
        //添加好友
        void addFrined(const TcpConnectionPtr &conn,json &js,Timestamp Timer);
        //添加群组
        void createGroup(const TcpConnectionPtr &conn,json &js,Timestamp Timer);
        //添加好友
        void addGroup(const TcpConnectionPtr &conn,json &js,Timestamp Timer);
        //添加好友
        void groupChat(const TcpConnectionPtr &conn,json &js,Timestamp Timer);
        //注销用户业务
        void loginout(const TcpConnectionPtr &conn,json &js,Timestamp Timer);
        //redis事件上调
        void handleRedisSubscribeMessage(int,string);
    private:
        ChatService();
        //存放消息id与业务函数之间的关系
        unordered_map<int ,MsgHandler> _msgHandlerMap; 
        //用户的登入与注册功能
        userModel _usermodel;
        //保存用户登入成功后的连接信息
        unordered_map<int,TcpConnectionPtr> _userConnMap;
        //用户访问连接信息的互斥锁
        mutex _connMutex;
        //离线业务
        OfflineMessage _offlineMsgModel;
        //好友业务
        FriendModel _frinedModel;
        //群组业务
        GroupModel _groupModel;
        //redis业务
        Redis _redis;
        
};


#endif
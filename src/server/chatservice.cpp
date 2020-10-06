#include"chatservice.hpp"
#include"public.hpp"
#include<muduo/base/Logging.h>
#include<vector>
using namespace muduo;
using namespace std;

//获取唯一实例
ChatService::ChatService()
{
    //将LOGIN_MSG对应的函数存入map中
    _msgHandlerMap.insert({LOGIN_MSG,bind(&ChatService::login,this,_1,_2,_3)});
    //将REG_MSG对应的函数存入map中
    _msgHandlerMap.insert({REG_MSG,bind(&ChatService::reg,this,_1,_2,_3)});
    //将ONE_CHAT_MSG对应的函数存入map中
    _msgHandlerMap.insert({ONE_CHAT_MSG,bind(&ChatService::oneChat,this,_1,_2,_3)});
    //将ONE_CHAT_MSG对应的函数存入map中
    _msgHandlerMap.insert({ADD_FRINED_MEG,bind(&ChatService::addFrined,this,_1,_2,_3)});
     //将CREATE_GROUP_MSG对应的函数存入map中
    _msgHandlerMap.insert({CREATE_GROUP_MSG,bind(&ChatService::createGroup,this,_1,_2,_3)});
     //将ADD_GROUP_MSG对应的函数存入map中
    _msgHandlerMap.insert({ADD_GROUP_MSG,bind(&ChatService::addGroup,this,_1,_2,_3)});
     //将GROUP_CHAT_MSG对应的函数存入map中
    _msgHandlerMap.insert({GROUP_CHAT_MSG,bind(&ChatService::groupChat,this,_1,_2,_3)});
    //将LOGINOUT_MSG对应的函数存入map中
    _msgHandlerMap.insert({LOGINOUT_MSG,bind(&ChatService::loginout,this,_1,_2,_3)});

    if(_redis.connect())
    {
        _redis.init_notify_handler(bind(&ChatService::handleRedisSubscribeMessage,this,_1,_2));
    }

}

 //获取消息id对应的函数
MsgHandler ChatService::getHandler(int msgid)
{
    if(_msgHandlerMap.find(msgid)==_msgHandlerMap.end())
    {
        return [=](const TcpConnectionPtr &conn,json &js,Timestamp Timer){
            LOG_ERROR<<"msgid:"<< msgid <<" is not found!";
        };
    }
    else
        return _msgHandlerMap[msgid];
}
//网络模块的唯一实例
ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}
//登入业务
void ChatService::login(const TcpConnectionPtr &conn,json &js,Timestamp Timer)
{
    int id = js["id"].get<int>();
    string pwd = js["password"];
    User user = _usermodel.query(id);
   
    if(user.getId()==id&&user.getPassword()==pwd) //检查用户的密码是否正确
    {
        if(user.getState()=="online") //检查用户是否在线
        {
            json jsonsend;
            jsonsend["msgid"]="LOGIN_MSG_ACKK";
            jsonsend["errno"]=2;
            jsonsend["errmsg"] = "用户重复登入！";
            conn->send(jsonsend.dump());
        }
        else    //将用户state信息更新并回送信息
        {

            {
                //对连接信息进行互斥访问
                lock_guard<mutex> lock(_connMutex);
                _userConnMap.insert({id,conn});
            }

            _redis.subscribe(id);

            user.setState("online");
            _usermodel.updatestate(user);
            json jsonsend;
            jsonsend["msgid"]="LOGIN_MSG_ACK";
            jsonsend["errno"]=0;
            jsonsend["id"]=user.getId();
            jsonsend["name"]=user.getName();
            vector<string> vec = _offlineMsgModel.query(id);
            if(!vec.empty())
            {
                jsonsend["offlinemsg"]=vec;
                _offlineMsgModel.remove(id);
            }

            vector<User> userVec =  _frinedModel.query(id);
            if(!userVec.empty())
            {
                vector<string> vec2;
                for(auto &user:userVec)
                {
                    json js;
                    js["id"]=user.getId();
                    js["name"]=user.getName();
                    js["state"]=user.getState();
                    vec2.push_back(js.dump());
                }
                jsonsend["friends"] = vec2;
            }

            vector<Group> groupVec = _groupModel.queryGroups(id);
            if(!groupVec.empty())
            {
                vector<string> vec3;
                for(auto &group : groupVec)
                {
                    json js;
                    js["id"] = group.getId();
                    js["name"] = group.getName();
                    js["desc"] = group.getDesc();
                    vector<string> vec4;
                    for(auto &user : group.getUsers())
                    {
                        json userjs;
                        userjs["id"] = user.getId();
                        userjs["name"] = user.getName();
                        userjs["state"] = user.getState();
                        userjs["role"] = user.getRole();
                        vec4.push_back(userjs.dump());
                    }
                    js["users"] = vec4;
                    vec3.push_back(js.dump());
                }
                jsonsend["groups"] = vec3;
            }
            conn->send(jsonsend.dump());
        }
        
    }
    else    //登入失败
    {
        json jsonsend;
        jsonsend["msgid"]="LOGIN_MSG_ACK";
        jsonsend["errno"]=1;
        jsonsend["errmsg"] = "登录失败！密码或者用户名错误！";
     
        conn->send(jsonsend.dump());
    }
}

//注销用户业务
void ChatService::loginout(const TcpConnectionPtr &conn,json &js,Timestamp Timer)
{
     int userid = js["id"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if (it != _userConnMap.end())
        {
            _userConnMap.erase(it);
        }
        
    }

    _redis.unsubscribe(userid);

   User user(userid,"","","offline");
    user.setState("offline");
    _usermodel.updatestate(user);
}

//注册业务
void ChatService::reg(const TcpConnectionPtr &conn,json &js,Timestamp)
{
    string name = js["name"];
    string password = js["password"];
    User user;
    user.setName(name);
    user.setPassword(password);
    bool state = _usermodel.insert(user);
    if(state)
    {
        json jsonsend;
        jsonsend["msgid"]="REG_MEG_ACK";
        jsonsend["errno"]=0;
        jsonsend["id"]=user.getId();
        conn->send(jsonsend.dump());
    }
    else
    {
        json jsonsend;
        jsonsend["msgid"]="REG_MEG_ACK";
        jsonsend["errno"]=1;
        jsonsend["errmsg"] = "注册失败！";
        conn->send(jsonsend.dump());
    }
    
}
//处理客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr& conn){
    User user;
    {
        lock_guard<mutex> lock(_connMutex);
        for(auto it=_userConnMap.begin();it!=_userConnMap.end();++it)
        {
            if(it->second==conn)
            {
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }
    _redis.unsubscribe(user.getId());
    if(user.getId()!=-1)
    {
        user.setState("offline");
        _usermodel.updatestate(user);
    }
}
//一对一聊天
void ChatService::oneChat(const TcpConnectionPtr &conn,json &js,Timestamp)
{
    int chat_to = js["toid"].get<int>();

    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(chat_to);
        if(it!=_userConnMap.end())
        {
            it->second->send(js.dump());
            return;
        }
    }
    User user = _usermodel.query(chat_to);
    if(user.getState()=="online")
    {
        _redis.publish(chat_to,js.dump());
        return;
    }
    _offlineMsgModel.insert(chat_to,js.dump());
}

void ChatService::reset()
{
    _usermodel.resetState();
}
//添加好友
void ChatService::addFrined(const TcpConnectionPtr &conn,json &js,Timestamp Timer)
{
    int userid = js["userid"].get<int>();
    int friendid = js["friendid"].get<int>();
    _frinedModel.insert(userid,friendid);
}

 //添加群组
void ChatService::createGroup(const TcpConnectionPtr &conn,json &js,Timestamp Timer)
{
    int userid = js["id"].get<int>();
    string groupname = js["groupname"];
    string groupdesc = js["groupdesc"];
    Group group(-1,groupname,groupdesc);
    if(_groupModel.createGroup(group))
    {
        _groupModel.addGroup(userid,group.getId(),"creator");
    }
}
//将用户添加到群组中去
void ChatService::addGroup(const TcpConnectionPtr &conn,json &js,Timestamp Timer)
{
    int userid = js["id"];
    int groupid = js["groupid"];
    _groupModel.addGroup(userid,groupid,"normal");

}
//群组聊天
void ChatService::groupChat(const TcpConnectionPtr &conn,json &js,Timestamp Timer)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> vec =  _groupModel.queryGroupUsers(userid,groupid);
    lock_guard<mutex> lock(_connMutex);
    for(int &id : vec)
    {
        auto it = _userConnMap.find(id);
        if(it!=_userConnMap.end())
        {
            it->second->send(js.dump());
        }
        else
        {
            User user = _usermodel.query(id);
            if(user.getState()=="online")
            {
                _redis.publish(id,js.dump());
                return;
            }
            _offlineMsgModel.insert(id,js.dump());
        }
        
    }
}
void ChatService::handleRedisSubscribeMessage(int userid,string msg)
{
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if(it != _userConnMap.end())
    {
        it->second->send(msg);
        return;
    }
    _offlineMsgModel.insert(userid,msg);
}
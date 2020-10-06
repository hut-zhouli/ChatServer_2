#include<iostream>
#include<string>
#include<thread>
#include<chrono>
#include<ctime>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include"json.hpp"

using namespace std;
using json=nlohmann::json;
#include"group.hpp"
#include"user.hpp"
#include"public.hpp"

//记录当前登入的用户信息
User g_currentUser;
//记录当前用户的好友列表信息
vector<User> g_currentUserFriendList;
//记录当前用户的组列表信息
vector<Group> g_currentUserGroupList;
//显示当前用户的基本信息
void showCurrentUserData();
//接受信息线程的函数
void readTaskHandler(int clientfd);
//获取系统时间
string getCurrentTime();
//主聊天页面程序
void mainMenu(int clientfd);
// 控制主菜单页面程序
bool isMainMenuRunning = false;



int main(int argc,char *argv[])
{
    if(argc<3)
    {
        cerr << "参数输入错误，请重新输入，例如：./ChatClient 127.0.0.1 8888" <<endl;
        exit(-1);
    }
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);
    int clientfd = socket(AF_INET,SOCK_STREAM,0);
    if(clientfd == -1)
    {
        cerr << "客户端套接字创建失败！" <<endl;
        exit(-1);
    }
    sockaddr_in server;
    memset(&server,0,sizeof(server));
    server.sin_family=AF_INET;
    server.sin_port=htons(port);
    server.sin_addr.s_addr=inet_addr(ip);
    
    
    if(connect(clientfd,(sockaddr *)&server,sizeof(server))==-1)
    {
        cerr<<"连接服务器失败！"<<endl;
        close(clientfd);
        exit(-1);
    }


    while(1)
    {
        cout << "========================" << endl;
        cout << "1. 登录" << endl;
        cout << "2. 注册" << endl;
        cout << "3. 退出" << endl;
        cout << "========================" << endl;
        cout << "choice:";
        int choice =0;
        cin >> choice;
        cin.get();

        switch (choice)
        {
            case 1:
                {
                    int id =0;
                    char pwd[50] ={0};
                    cout<<"用户id：";
                    cin>>id;
                    cin.get();

                    cout<<"密码：";
                    cin.getline(pwd,50);

                    json js;
                    js["msgid"] = LOGIN_MSG;
                    js["id"] = id;
                    js["password"] = pwd;
                    string senddata = js.dump();

                    int len = send(clientfd,senddata.c_str(),strlen(senddata.c_str())+1,0);
                    if(len==-1)
                    {
                        cerr<<"客户端发送登录信息失败！"<<senddata<<endl;
                    }
                    else
                    {
                        char buf[1024] = {};
                        len = recv(clientfd,buf,sizeof(buf),0);
                        if(len==-1)
                        {
                            cerr<<"客户端收到登录信息失败！"<<endl;
                        }
                        else
                        {
                            json response;
                            response = json::parse(buf);
                            if(response["errno"].get<int>()!=0)
                            {
                                 cerr<<response["errmsg"]<<endl; 
                            }
                            else
                            {
                                g_currentUser.setId(response["id"].get<int>());
                                g_currentUser.setName(response["name"]);
                                
                                if(response.contains("friends"))
                                {
                                    g_currentUserFriendList.clear();
                                    vector<string> vec = response["friends"];
                                    for(string &str : vec)
                                    {
                                        json js;
                                        js = json::parse(str);
                                        User user;
                                        user.setId(js["id"].get<int>());
                                        user.setName(js["name"]);
                                        user.setState(js["state"]);
                                        g_currentUserFriendList.push_back(user);
                                    }

                                }

                                 if(response.contains("groups"))
                                {
                                    g_currentUserGroupList.clear();
                                    vector<string> vec = response["groups"];
                                    for(string &str : vec)
                                    {
                                        json js;
                                        js = json::parse(str);
                                        Group group;
                                        group.setId(js["id"].get<int>());
                                        group.setName(js["name"]);
                                        group.setDesc(js["desc"]);
                                        
                                        vector<string> vec2 = js["users"];
                                        for(auto &groupuser : vec2)
                                        {
                                            json groupjs = json::parse(groupuser);
                                            GroupUser user;
                                            user.setId(groupjs["id"].get<int>());
                                            user.setName(groupjs["name"]);
                                            user.setState(groupjs["state"]);
                                            user.setRole(groupjs["role"]);
                                            group.getUsers().push_back(user);
                                        }
                                        g_currentUserGroupList.push_back(group);
                                    }
                                    
                                }
                            
                                showCurrentUserData(); 
                                if(response.contains("offlinemsg"))
                                {
                                    vector<string> vec = response["offlinemsg"];
                                    for(auto &str : vec)
                                    {
                                        json js = json::parse(str);

                                        if (ONE_CHAT_MSG == js["msgid"].get<int>())
                                        {
                                            cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                                                << " said: " << js["msg"].get<string>() << endl;
                                        }
                                        else
                                        {
                                            cout << "群消息[" << js["groupid"] << "]:" << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                                                << " said: " << js["msg"].get<string>() << endl;
                                        }
                                    }
                                }
                                static int threadnumber = 0;
                                if(threadnumber==0)
                                {
                                    thread thd(readTaskHandler, clientfd);
                                    thd.detach();
                                    threadnumber++;  
                                }
                                
                                isMainMenuRunning=true;
                                mainMenu(clientfd);                
                            }
                        }
                    }
                }
                break;
            case 2:
                {
                    char name[50] = {0};
                    char pwd[50] = {0};
                    cout<<"用户名：";
                    cin.getline(name,50);

                    cout<<"密码：";
                    cin.getline(pwd,50);

                    json js;
                    js["msgid"] = REG_MSG;
                    js["name"] = name;
                    js["password"] = pwd;
                    string senddata = js.dump();
                    int len = send(clientfd,senddata.c_str(),strlen(senddata.c_str())+1,0);

                    if(len==-1)
                    {
                        cerr<<"客户端发送注册信息失败！"<<senddata<<endl;
                    }
                    else
                    {
                        char buf[1024]={0};
                        len = recv(clientfd,buf,sizeof(buf),0);

                        if(len==-1)
                        {
                            cerr<<"客户端收到注册会送信息失败！"<<endl;
                        }
                        else
                        {
                            json response;
                            response = json::parse(buf);
                            if(response["errno"].get<int>()!=0)
                            {
                                  cerr<<"用户名已存在！注册失败！"<< response["name"]<<endl; 
                            }
                            else
                            {
                                cout <<"注册成功！"<<"这是你的id号："<<response["id"]<<"  不要忘记id了哦！"<<endl;
                            }
                            
                        }
                        
                    }
                    
                }
                break;
            case 3:
                close(clientfd);
                exit(0);
            default:
                cerr<<"输入选项错误！请重新输入！"<<endl;
                break;
        }
    }
    return 0;
}
//显示当前用户的基本信息
void showCurrentUserData()
{
     cout << "======================login user======================" << endl;
    cout << "current login user => id:" << g_currentUser.getId() << " name:" << g_currentUser.getName() << endl;
    cout << "----------------------friend list---------------------" << endl;
    if (!g_currentUserFriendList.empty())
    {
        for (User &user : g_currentUserFriendList)
        {
            cout << user.getId() << " " << user.getName() << " " << user.getState() << endl;
        }
    }
    cout << "----------------------group list----------------------" << endl;
    if (!g_currentUserGroupList.empty())
    {
        for (Group &group : g_currentUserGroupList)
        {
            cout << group.getId() << " " << group.getName() << " " << group.getDesc() << endl;
            for (GroupUser &user : group.getUsers())
            {
                cout << user.getId() << " " << user.getName() << " " << user.getState()
                     << " " << user.getRole() << endl;
            }
        }
    }
    cout << "======================================================" << endl;
}

void help(int = 0,string = "");
void chat(int,string);
void addfriend(int,string);
void creategroup(int,string);
void addgroup(int,string);
void loginout(int,string);
void groupchat(int,string);

//获取系统时间
string getCurrentTime()
{
    return "2020:10:3";
}

unordered_map<string, string> commandMap = {
    {"help", "显示所有支持的命令，格式help"},
    {"chat", "一对一聊天，格式chat:friendid:message"},
    {"addfriend", "添加好友，格式addfriend:friendid"},
    {"creategroup", "创建群组，格式creategroup:groupname:groupdesc"},
    {"addgroup", "加入群组，格式addgroup:groupid"},
    {"groupchat", "群聊，格式groupchat:groupid:message"},
    {"loginout", "注销，格式loginout"}
};

unordered_map<string,function<void(int,string)>> commodHandlerMap = 
{
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"loginout", loginout}
};

void readTaskHandler(int clientfd)
{
    while(1)
    {
        char buf[1024]={0};
        int len = recv(clientfd,buf,1024,0);
        if(len==-1||len==0)
        {
            cerr<<"客户端接受数据失败！"<<__LINE__<<endl;
            close(clientfd);
            exit(-1);
        }
        else
        {
            json js = json::parse(buf);
            int msgtype = js["msgid"].get<int>();
            if (ONE_CHAT_MSG == msgtype)
            {
                cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                    << " said: " << js["msg"].get<string>() << endl;
                    continue;
            }
            if(GROUP_CHAT_MSG == msgtype)
            {
                cout << "群消息[" << js["groupid"] << "]:" << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                 << " said: " << js["msg"].get<string>() << endl;
                continue;
            }
        }
    }
    
}

//主聊天页面程序
void mainMenu(int clientfd)
{
    help();
    char buf[1024]={0};
    while(isMainMenuRunning)
    {
        cin.getline(buf,1024);
        string commandBuf(buf);
        string command;
        int index = commandBuf.find(":");
        if(index == -1)
        {
            command = commandBuf;
        }
        else
        {
            command = commandBuf.substr(0,index);
        }
        auto it = commodHandlerMap.find(command);
        if(it==commodHandlerMap.end())
        {
            cerr<<"命令输入有误！请重新输入！"<<__LINE__<<endl;
            continue;
        }
        else
        {
            it->second(clientfd,commandBuf.substr(index+1,commandBuf.size()-index));
        }
    }
}

void help(int,string)
{
    cout<<"帮助文档"<<endl;
    for(auto &x : commandMap)
    {
        cout<<x.first<<":"<<x.second<<endl;
    }
    cout<<endl;
}
/* {"chat", "一对一聊天，格式chat:friendid:message"}*/
void chat(int clientfd,string str)
{
    int index = str.find(":");
    if(index==-1)
    {
        cerr<<"输入错误！请重新输入！"<<endl;
        return;
    }
    int friendid = atoi(str.substr(0,index).c_str());
    string message = str.substr(index+1,str.size()-index);
    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["name"] = g_currentUser.getName();
    js["id"] = g_currentUser.getId();
    js["toid"] = friendid;
    js["msg"] = message;
    js["time"] = getCurrentTime();

    string data = js.dump();
    int len = send(clientfd,data.c_str(),strlen(data.c_str())+1,0);

    if(len==-1)
    {
        cerr<<"客户端发送数据失败！"<<__LINE__<<endl;
    }
}
/*{"addfriend", "添加好友，格式addfriend:friendid"},*/
void addfriend(int clientfd,string str)
{
    if(str.empty())
    {
        cerr<<"输入错误！请重新输入！"<<endl;
        return;
    }
    json js;
    js["friendid"] = atoi(str.c_str());
    js["msgid"] = ADD_FRINED_MEG;
    js["userid"] = g_currentUser.getId();
    string data = js.dump();
    int len = send(clientfd,data.c_str(),strlen(data.c_str())+1,0);

    if(len==-1)
    {
        cerr<<"客户端发送数据失败！"<<__LINE__<<endl;
    }

}
/*    {"creategroup", "创建群组，格式creategroup:groupname:groupdesc"},*/
void creategroup(int clientfd,string str)
{
    int index = str.find(":");
    if(index==-1)
    {
        cerr<<"输入错误！请重新输入！"<<endl;
        return;
    }
    string groupname = str.substr(0,index);
    string groupdesc = str.substr(index+1,str.size()-index);
    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    js["id"] = g_currentUser.getId();

    string data = js.dump();

    int len = send(clientfd,data.c_str(),strlen(data.c_str())+1,0);

    if(len==-1)
    {
        cerr<<"客户端发送数据失败！"<<__LINE__<<endl;
    }
}
/*    {"addgroup", "加入群组，格式addgroup:groupid"},*/
void addgroup(int clientfd,string str)
{
    if(str.empty())
    {
        cerr<<"输入错误！请重新输入！"<<endl;
        return;
    }
    int groupid = atoi(str.c_str());
    json js;

    js["msgid"] = ADD_GROUP_MSG;
    js["groupid"] = groupid;
    js["id"] = g_currentUser.getId();
    string data = js.dump();
    int len = send(clientfd,data.c_str(),strlen(data.c_str())+1,0);

    if(len==-1)
    {
        cerr<<"客户端发送数据失败！"<<__LINE__<<endl;
    }
}
   
/*{"loginout", "注销，格式loginout"}*/
void loginout(int clientfd,string str)
{
    json js;
    js["msgid"] = LOGINOUT_MSG;
    js["id"] = g_currentUser.getId();
    string data = js.dump();
    int len = send(clientfd,data.c_str(),strlen(data.c_str())+1,0);
    if(len==-1)
    {
        cerr<<"客户端发送数据失败！"<<__LINE__<<endl;
    }else
    {
        isMainMenuRunning = false;
    }
    
}
 /*{"groupchat", "群聊，格式groupchat:groupid:message"}*/
void groupchat(int clientfd,string str)
{
    int index = str.find(":");
    if(index==-1)
    {
        cerr<<"输入错误！请重新输入！"<<endl;
        return;
    }
    int groupid = atoi(str.substr(0,index).c_str());
    string message = str.substr(index+1,str.size()-index);
    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["name"] = g_currentUser.getName();
    js["id"] = g_currentUser.getId();
    js["groupid"] = groupid;
    js["msg"] = message;
    js["time"] = getCurrentTime();

    string data = js.dump();

    int len = send(clientfd,data.c_str(),strlen(data.c_str())+1,0);

    if(len==-1)
    {
        cerr<<"客户端发送数据失败！"<<__LINE__<<endl;
    }
}

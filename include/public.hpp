#ifndef PUBLIC_H
#define PUBLIC_H

enum EnMsgType{
    LOGIN_MSG=1,//登入信号
    LOGIN_MSG_ACK,//登入回应信号
    LOGINOUT_MSG, //注销用户消息
    REG_MSG,//注册信号
    REG_MSG_ACK,//注册回应信息
    ONE_CHAT_MSG,//一对一聊天信号
    ADD_FRINED_MEG,//添加好友信号
    CREATE_GROUP_MSG,//创建群组信号
    ADD_GROUP_MSG,//将用户加入群组信号
    GROUP_CHAT_MSG//群组聊天


};

#define LOG(str) cout << __FILE__ << ":" << __LINE__<<" "<<__TIMESTAMP__<<" : "<< str <<endl;
#endif
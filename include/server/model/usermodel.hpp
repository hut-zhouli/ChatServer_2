#ifndef USERMODEL_H
#define USERMODEL_H
#include"user.hpp"
#include"db/Connection.hpp"

class userModel
{
    public:
        //注册用户时将用户消息插入数据库中
        bool insert(User &user);
        //登入是查询数据库检查用户是否正确
        User query(int id);
        //更新用户state消息
        bool updatestate(User user);
        //重置用户state信息
        void resetState();
    private:
       
};

#endif
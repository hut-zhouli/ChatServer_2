#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H
#include"user.hpp"
#include<vector>
class FriendModel
{
    public:
        //添加用户模块
        void insert(int userid,int friendid);

        //返回用户好友信息
        vector<User> query(int userid);
};

#endif
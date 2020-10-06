#include"friendmodel.hpp"
#include"db/Connection.hpp"
#include "CommonConnectionPool.hpp"

 //添加用户模块
void FriendModel::insert(int userid,int friendid)
{   
    char sql[1024]={0};
    sprintf(sql,"insert into friend values(%d,%d)",userid,friendid);
    ConnectionPool* cp = ConnectionPool::GetConnectionPool();
			
	shared_ptr<MySQL> conn = cp->getConnection();
    if(conn!=nullptr)
    {
        conn->update(sql);
    }
}

//返回用户好友信息
vector<User> FriendModel::query(int userid)
{
    char sql[1024]={0};
    sprintf(sql,"select a.id,a.name,a.state from user a inner join friend b on a.id=b.friendid where userid=%d ",userid);
    ConnectionPool* cp = ConnectionPool::GetConnectionPool();
			
	shared_ptr<MySQL> conn = cp->getConnection();
    vector<User> user_vec;
    if(conn!=nullptr)
    {
        MYSQL_RES* res = conn->query(sql);
        if(res!=nullptr)
        {
            MYSQL_ROW row;
            while (row = mysql_fetch_row(res))
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user_vec.push_back(user);
            }
            mysql_free_result(res);
            return user_vec;
        }
    }
    return user_vec;
}
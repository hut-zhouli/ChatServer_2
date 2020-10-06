#include "usermodel.hpp"
#include "Connection.hpp"
#include"CommonConnectionPool.hpp"
#include<iostream>
using namespace std;

//注册用户时将用户消息插入数据库中
 bool userModel::insert(User &user)
 {
    ConnectionPool* cp = ConnectionPool::GetConnectionPool();
			
	shared_ptr<MySQL> conn = cp->getConnection();
    char sql[1024];
    sprintf(sql,"insert into user(name,password,state) values('%s','%s','%s')",user.getName().c_str(),user.getPassword().c_str(),user.getState().c_str());
    if(conn!=nullptr)
    {
        if(conn->update(sql))
        {
            user.setId(mysql_insert_id(conn->getConn()));
            return true;
        }
    }
    return false;
 }

//登入是查询数据库检查用户是否正确
 User userModel::query(int id)
 {
    ConnectionPool* cp = ConnectionPool::GetConnectionPool();
			
	shared_ptr<MySQL> conn = cp->getConnection();
    char  sql[1024]={0}; 
    sprintf(sql,"select * from user where id=%d",id);
    
    MYSQL_RES *res = conn->query(sql);
    if(res!=nullptr)
    {
        MYSQL_ROW row = mysql_fetch_row(res);
        
        if(row!=nullptr)
        { 
        User user;
        user.setId(atoi(row[0]));
        user.setName(row[1]);
        user.setPassword(row[2]);
        user.setState(row[3]);

        return user;
        }
    }
     
     return User();
 }

//更新用户state消息
 bool userModel::updatestate(User user)
 {
     ConnectionPool* cp = ConnectionPool::GetConnectionPool();
			
	shared_ptr<MySQL> conn = cp->getConnection();
     char  sql[1024]={0}; 
     sprintf(sql,"update user set state='%s' where id=%d",user.getState().c_str(),user.getId());
     if(conn!=nullptr)
     {
         if(conn->update(sql))
            {
                user.setId(mysql_insert_id(conn->getConn()));
                return true;
            }
     }
     return false;
 }

//重置用户state信息
void userModel::resetState()
{
     ConnectionPool* cp = ConnectionPool::GetConnectionPool();
			
	shared_ptr<MySQL> conn = cp->getConnection();
     char  sql[1024]="update user set state='offline' where state='online'"; 
     if(conn!=nullptr)
     {
         conn->update(sql);
     }
}
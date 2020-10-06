#include "groupmodel.hpp"
#include "pch.hpp"
#include "db/Connection.hpp"
#include "CommonConnectionPool.hpp"
//创建群组
bool GroupModel::createGroup(Group &group)
{
    char sql[1024]={0};
    sprintf(sql,"insert into allgroup(groupname,groupdesc) values('%s','%s')",group.getName().c_str(),group.getDesc().c_str());
    ConnectionPool* cp = ConnectionPool::GetConnectionPool();
			
	shared_ptr<MySQL> conn = cp->getConnection();
    if(conn!=nullptr)
    {
        if(conn->update(sql))
        {
            group.setId(mysql_insert_id(conn->getConn()));
            return true;
        }
    }
    return false;
}
//加入群组
void GroupModel::addGroup(int userid,int groupid,string role)
{
    char sql[1024]={0};
    sprintf(sql,"insert into groupuser values(%d,%d,'%s')",groupid,userid,role.c_str());
    ConnectionPool* cp = ConnectionPool::GetConnectionPool();
			
	shared_ptr<MySQL> conn = cp->getConnection();
    if(conn!=nullptr)
    {
        conn->update(sql);
    } 
}
//查询用户所在群组信息;
vector<Group> GroupModel::queryGroups(int userid)
{
    char sql[1024] = {0};
    sprintf(sql,"select b.id,b.groupname,b.groupdesc from groupuser a inner join allgroup b on a.groupid=b.id where a.userid=%d",userid);
    ConnectionPool* cp = ConnectionPool::GetConnectionPool();
			
	shared_ptr<MySQL> conn = cp->getConnection();
    vector<Group> groupVec;
    if(conn!=nullptr)
    {
        MYSQL_RES * res = conn->query(sql);
        if(res!=nullptr)
        {
            MYSQL_ROW row; 
            while((row = mysql_fetch_row(res))!=nullptr)
            {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                groupVec.push_back(group);
            }
        }
        mysql_free_result(res);
    }

    for(auto &group : groupVec)
    {
        sprintf(sql,"select a.id,a.name,a.state,b.grouprole from user a inner join groupuser b on b.userid=a.id where b.groupid=%d",group.getId());
        MYSQL_RES* res = conn->query(sql);
        if(res!=nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res))!=nullptr)
            {
                GroupUser users;
                users.setId(atoi(row[0]));
                users.setName(row[1]);
                users.setState(row[2]);
                users.setRole(row[3]);
                group.getUsers().push_back(users);
            }
            mysql_free_result(res);
        }
    }
    return groupVec;
}
//根据指定的groupid查询群组用户的id列表，除userid自己，主要用于用户群发消息
vector<int> GroupModel::queryGroupUsers(int userid,int groupid)
{
    char sql[1024] = {0};
    sprintf(sql,"select userid from groupuser where groupid=%d and userid!=%d",groupid,userid);
    ConnectionPool* cp = ConnectionPool::GetConnectionPool();
			
	shared_ptr<MySQL> conn = cp->getConnection();
    vector<int> vec;
    if(conn!=nullptr)
    {
        MYSQL_RES *res = conn->query(sql);
        if(res!=nullptr)
        {
            MYSQL_ROW row;
            while((row=mysql_fetch_row(res))!=nullptr)
            {
                vec.push_back(atoi(row[0]));
            }
        }
        mysql_free_result(res);
    }
    return vec;
}
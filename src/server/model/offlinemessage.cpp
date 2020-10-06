#include "offlinemessage.hpp"
#include "db/Connection.hpp"
#include"CommonConnectionPool.hpp"

void OfflineMessage::insert(int userid,string meg)
{
    char sql[1024]={0};
    sprintf(sql,"insert into offlinemessage values(%d,'%s')",userid,meg.c_str());
    ConnectionPool* cp = ConnectionPool::GetConnectionPool();
			
	shared_ptr<MySQL> conn = cp->getConnection();
    if(conn!=nullptr)
    {
        conn->update(sql);
    }
}


 void OfflineMessage::remove(int userid)
{
    char sql[1024]={0};
    sprintf(sql,"delete from offlinemessage where userid=%d",userid);
    ConnectionPool* cp = ConnectionPool::GetConnectionPool();
			
	shared_ptr<MySQL> conn = cp->getConnection();
    if(conn!=nullptr)
    {
        conn->update(sql);
    }

}


vector<string> OfflineMessage::query(int userid)
{
    char sql[1024]={0};
    sprintf(sql,"select message from offlinemessage where userid=%d",userid);
    ConnectionPool* cp = ConnectionPool::GetConnectionPool();
			
	shared_ptr<MySQL> conn = cp->getConnection();
    vector<string> vec;
    if(conn!=nullptr)
    {
        MYSQL_RES *res = conn->query(sql);
        if(res!=nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res))!=nullptr)
            {
                vec.push_back(row[0]);
                cout<<row[0]<<endl;
            }
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}
#ifndef CONNECTION_H
#define CONNECTION_H

#include <mysql/mysql.h>
#include <string>
#include <ctime>
using namespace std;

class MySQL
{
public:
	MySQL();
	~MySQL();
	bool connect();
	bool update(string sql);
	MYSQL* getConn();
	MYSQL_RES* query(string sql);
	void setAliveTime() { _alivetime = clock(); }
	clock_t getAliveTime() { return _alivetime - clock(); }
private:
	MYSQL* _conn; 
	clock_t _alivetime;
};

#endif
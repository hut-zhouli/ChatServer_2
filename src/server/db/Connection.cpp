#include "pch.hpp"
#include "public.hpp"
#include "Connection.hpp"
#include <iostream>
using namespace std;


static string ip="localhost";
static string username="root";
static string password="123456";
static string dbname="chat";
static int port=3306;


MySQL::MySQL()
{
	_conn = mysql_init(nullptr);
}

MySQL::~MySQL()
{
	if (_conn != nullptr)
		mysql_close(_conn);
}

bool MySQL::connect()
{
	MYSQL* p = mysql_real_connect(_conn, ip.c_str(), username.c_str(),
		password.c_str(), dbname.c_str(), port, nullptr, 0);
	return p != nullptr;
}

bool MySQL::update(string sql)
{
	if (mysql_query(_conn, sql.c_str()))
	{
		LOG("update is fail!:" + sql);
		return false;
	}
	return true;
}

MYSQL_RES* MySQL::query(string sql)
{
	if (mysql_query(_conn, sql.c_str()))
	{
		LOG("query is fail!:" + sql);
		return nullptr;
	}
	return mysql_use_result(_conn);
}
MYSQL* MySQL::getConn()
{
    return _conn;
}

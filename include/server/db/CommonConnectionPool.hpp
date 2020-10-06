#pragma once
#include<queue>
#include<mutex>
#include<string>
#include"Connection.hpp"
#include"public.hpp"
#include <iostream>
#include<atomic>
#include<thread>
#include<functional>
#include<ctime>
#include<memory>
#include<condition_variable>
using namespace std;


class ConnectionPool
{
public:
	static ConnectionPool* GetConnectionPool();
	shared_ptr<MySQL> getConnection();
private:
	ConnectionPool();
	ConnectionPool(ConnectionPool&) = delete;
	ConnectionPool& operator=(ConnectionPool&) = delete;
	bool loadConfigFile();
	void produceConnectionTask();
	void scannerConnectionTask();

	string _ip;
	unsigned short _port;
	string username;
	string password;
	string _dbname;
	int _initSize;
	int _maxSize;
	int _maxIdleTime;
	int _connectionTimeout;

	//�������ݿ����ӵ�����
	queue<MySQL*> _connectionQue;
	//��֤�����̰߳�ȫ�Ļ�����
	mutex _queueMutex;
	//�������������������������������ߵ��̰߳�ȫ����
	condition_variable cv;
	//���ӳ����������ӵ�����
	atomic_int _connectionCnt;
};
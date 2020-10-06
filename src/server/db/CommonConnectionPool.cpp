#include"CommonConnectionPool.hpp"


ConnectionPool* ConnectionPool::GetConnectionPool()
{
	static ConnectionPool pool;
	return &pool;
}

//������ʼ�߳�
ConnectionPool::ConnectionPool()
{
	if (!loadConfigFile())
	{
		return;
	}

	for (int i = 0; i < _initSize; ++i)
	{
		MySQL* p = new MySQL();
		p->connect();
		p->setAliveTime();
		_connectionQue.push(p);
		_connectionCnt++;
	}

	thread produce(bind(&ConnectionPool::produceConnectionTask,this));
	produce.detach();
	thread scanner(bind(&ConnectionPool::scannerConnectionTask, this));
	scanner.detach();
}


bool ConnectionPool::loadConfigFile()
{
	/*FILE *pf = fopen("lib/mysql.ini","r");
	if (pf == nullptr)
	{
		LOG("mysql.ini file is not exist!");
		return false;
	}
	while (!feof(pf))
	{
		char line[1024] = { 0 };
		fgets(line , 1024, pf);
		string str = string(line);
		int idx = str.find("=",0);
		if (idx == -1)
		{
			continue;
		}
		int endidx = str.find("\n", idx);
		string key = str.substr(0, idx);
		string value = str.substr(idx + 1, endidx - idx - 1);
		//cout << key << ":" << value << endl;
		if (key == "ip")
		{
			_ip = value;
		} else if(key == "port")
		{
			_port = atoi(value.c_str());
		}
		else if (key == "username")
		{
			username = value;
		}
		else if (key == "password")
		{
			password = value;
		}
		else if (key == "dbname")
		{
			_dbname = value;
		}
		else if (key == "initSize")
		{
			_initSize = atoi(value.c_str());
		}
		else if (key == "maxSize")
		{
			_maxSize = atoi(value.c_str());
		}
		else if (key == "maxIdleTime")
		{
			_maxIdleTime = atoi(value.c_str());
		}
		else if (key == "connectionTimeout")
		{
			_connectionTimeout = atoi(value.c_str());
		}
	}
	return true;*/
	_ip="localhost";
	_port=3306;
	username="root";
	password="123456";
	_dbname="chat01";
	_initSize=10;
	_maxSize=1024;
	_maxIdleTime=60;
	_connectionTimeout=100;

	return true;

}


void ConnectionPool::produceConnectionTask()
{
	while (1)
	{
		unique_lock<mutex> lck(_queueMutex);
		
		while (!_connectionQue.empty())
		{
			cv.wait(lck);
		}
		
		if (_connectionCnt < _maxSize)
		{
			MySQL* cp = new MySQL();
			cp->connect();
			cp->setAliveTime();
			_connectionQue.push(cp);
			_connectionCnt++;
		}
		
		cv.notify_all();
	}
	
}


shared_ptr<MySQL> ConnectionPool::getConnection()
{
	unique_lock<mutex> lock(_queueMutex);
	while (_connectionQue.empty())
	{
		if (cv_status::timeout == cv.wait_for(lock, chrono::milliseconds(_connectionTimeout)))
		{
			if (_connectionQue.empty())
			{
				LOG("_connectionQue is empty");
				return nullptr;
			}
		}
	}
	
	shared_ptr <MySQL> cp(_connectionQue.front(), [&](MySQL* conn) {unique_lock<mutex> lock(_queueMutex); conn->setAliveTime(); _connectionQue.push(conn); });
	_connectionQue.pop();
	cv.notify_all();
	return cp;
}

void ConnectionPool::scannerConnectionTask()
{
	while (1)
	{
		this_thread::sleep_for(chrono::seconds(_maxSize));
		unique_lock<mutex> lock(_queueMutex);
		while (_connectionCnt > _initSize)
		{
			MySQL* p = _connectionQue.front();

			if (p->getAliveTime() >=(_maxIdleTime*1000))
			{
				_connectionQue.pop();
				_connectionCnt--;
				delete p;
			}
		}
	}
	
}
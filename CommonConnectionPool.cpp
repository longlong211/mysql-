#include<iostream>
#include"CommonConnectionPool.h"
#include"Connection.h"
#include"pch.h"
#include<thread>
#include<memory>
#include<condition_variable>

class ConnectionPool;
ConnectionPool* ConnectionPool::getConnectionPool()
{
	static ConnectionPool pool;
	return &pool;//懒汉模式，线程安全

}
ConnectionPool::ConnectionPool() 
{
	if (!loadConfigFile())//loading setting file 
	{
		return;
	}
	//create init connection
	for (int i = 0; i < _initSize; i++)
	{
		Connection* p = new Connection();
		p->connect(_ip, _port, _username, _password, _dbname);
		p->refreshAliveTime();
		_connectionQue.push(p);
		_connectionCnt++;//系统启动时候，没有线程安全问题，只有一个线程
	}
	//启动一个新的线程，作为生产者。
	thread produce(std::bind(&ConnectionPool::produceConnectionTask,this));
	//启动一个新线程，扫描多余空闲连接，进行回收
	produce.detach();
	thread scanner(std::bind(&ConnectionPool::scannerConnectionTask, this));
	scanner.detach();

  
}//1私有化构造
bool ConnectionPool::loadConfigFile()
{
	FILE* pf = fopen("mysql.ini", "r");
	if (pf == nullptr)
	{
		LOG("mysql.ini file is not exits");
		return false;
	}
	while (!feof(pf))
	{
		char line[1024] = { 0 };
		fgets(line, 1024, pf);
		string str = line;
		int idx = str.find('=', 0);
		if (idx == -1)
		{
			continue;
		}
		int endidx =str.find('\n',idx);
		string key = str.substr(0, idx);
		string value = str.substr(idx + 1, endidx - idx - 1);

		if (key == "ip")
		{
			_ip = value;
		}
		else if(key=="port")
		{
			_port = atoi(value.c_str());

		}
		else if (key == "username")
		{
			_username = value;
		}
		else if (key == "dbname")
		{
			_dbname = value;
		}
		else if (key == "password")
		{
			_password = value;
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
			_maxIdleTime= atoi(value.c_str());
		}
		else if (key == "connectionTimeout")
		{
			_connectionTimeout= atoi(value.c_str());
		}
		
	}
	return true;
}

void ConnectionPool::produceConnectionTask()
{
	for (;;)
	{
		unique_lock<mutex> lock(_queueMutex);
		while (!_connectionQue.empty())
		{
			_cv.wait(lock);//释放锁给消费者。
		}
			//连接数量没有达到上线。继续创建新的连接
			if (_connectionCnt < _maxSize)
			{
				//cout << "prouduce new connection" << endl;
				Connection* p = new Connection();
				p->connect(_ip, _port, _username, _password, _dbname);
				p->refreshAliveTime();
				_connectionQue.push(p);
				_connectionCnt++;//系统启动时候，没有线程安全问题，只有一个线程
			}
		_cv.notify_all();
		
	}

}
void ConnectionPool::scannerConnectionTask()
{
	for (;;)
	{
		this_thread::sleep_for(chrono::seconds(_maxIdleTime));
		unique_lock<mutex> lock(_queueMutex);

		//扫描队列
		while (_connectionCnt > _initSize)
		{

			Connection* p = _connectionQue.front();
			if (p->getAliveTime() >= (_maxIdleTime * 1000))
			{
				_connectionQue.pop();
				_connectionCnt--;

				delete p;//调用析构函数释放连接
			}
			else
			{
				break;//对头元素没有超时其他元素肯定没有超时
			}

		}
	}

}


shared_ptr<Connection> ConnectionPool::getConnection()//给外部提供接口，从连接池获取一个可以用的空闲连接
{
	unique_lock<mutex> lock(_queueMutex);
	while (_connectionQue.empty())
	{
		if (cv_status::timeout == _cv.wait_for(lock, chrono::microseconds(_connectionTimeout)))
		{//超时和非超时
			if (_connectionQue.empty())
			{
				LOG("获取空闲连接超时。。！");
				return nullptr;
			}

		}
	}
	
		/*shared_ptr 会把资源析构delete，需要自定义shared_ptr的释放资源的方式，把connection归还*/
		shared_ptr<Connection> sp(_connectionQue.front(), [&](Connection* pcon) {
			//归还到线程池中
			unique_lock<mutex> lock(_queueMutex);
			pcon->refreshAliveTime();
			_connectionQue.push(pcon);
			//_connectionCnt++;
			LOG("回收连接");

			
			});
		_connectionQue.pop();
		_cv.notify_all();//消费完，通知生产者检查队列为空 生产

		return sp;
	

}
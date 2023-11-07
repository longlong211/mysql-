#pragma once
/*连接池功能模块*/
#include<string>
#include<queue>
#include<mutex>
#include<atomic>
#include<functional>
class Connection;
using namespace std;
class ConnectionPool
{
public:
	static ConnectionPool* getConnectionPool();
	shared_ptr<Connection> getConnection();//给外部提供接口，从连接池获取一个可以用的空闲连接
	//shared_ptr自动管理connection

private:
	ConnectionPool();//1私有化构造

	void produceConnectionTask();
	void scannerConnectionTask();
	bool loadConfigFile();
	string _ip;
	unsigned short _port;
	string _username;
	string _dbname;
	string _password;
	int _initSize;
	int _maxSize;
	int _maxIdleTime;
	int _connectionTimeout;
	queue<Connection*> _connectionQue;//存储mysql连接的队列
	mutex _queueMutex;//维护连接队列的互斥锁
	atomic_int _connectionCnt;//++操作不安全
	condition_variable _cv;
};
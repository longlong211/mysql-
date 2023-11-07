#pragma once
/*���ӳع���ģ��*/
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
	shared_ptr<Connection> getConnection();//���ⲿ�ṩ�ӿڣ������ӳػ�ȡһ�������õĿ�������
	//shared_ptr�Զ�����connection

private:
	ConnectionPool();//1˽�л�����

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
	queue<Connection*> _connectionQue;//�洢mysql���ӵĶ���
	mutex _queueMutex;//ά�����Ӷ��еĻ�����
	atomic_int _connectionCnt;//++��������ȫ
	condition_variable _cv;
};
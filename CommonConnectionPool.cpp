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
	return &pool;//����ģʽ���̰߳�ȫ

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
		_connectionCnt++;//ϵͳ����ʱ��û���̰߳�ȫ���⣬ֻ��һ���߳�
	}
	//����һ���µ��̣߳���Ϊ�����ߡ�
	thread produce(std::bind(&ConnectionPool::produceConnectionTask,this));
	//����һ�����̣߳�ɨ�����������ӣ����л���
	produce.detach();
	thread scanner(std::bind(&ConnectionPool::scannerConnectionTask, this));
	scanner.detach();

  
}//1˽�л�����
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
			_cv.wait(lock);//�ͷ����������ߡ�
		
			//��������û�дﵽ���ߡ����������µ�����
			if (_connectionCnt < _maxSize)
			{
				//cout << "prouduce new connection" << endl;
				Connection* p = new Connection();
				p->connect(_ip, _port, _username, _password, _dbname);
				p->refreshAliveTime();
				_connectionQue.push(p);
				_connectionCnt++;//ϵͳ����ʱ��û���̰߳�ȫ���⣬ֻ��һ���߳�
			}
		_cv.notify_all();
		}
	}

}
void ConnectionPool::scannerConnectionTask()
{
	for (;;)
	{
		this_thread::sleep_for(chrono::seconds(_maxIdleTime));
		unique_lock<mutex> lock(_queueMutex);

		//ɨ�����
		while (_connectionCnt > _initSize)
		{
			cout << "collect  connection" << endl;


			Connection* p = _connectionQue.front();
			if (p->getAliveTime() >= (_maxIdleTime * 1000))
			{
				_connectionQue.pop();
				_connectionCnt--;

				delete p;//�������������ͷ�����
			}
			else
			{
				break;//��ͷԪ��û�г�ʱ����Ԫ�ؿ϶�û�г�ʱ
			}

		}
	}

}


shared_ptr<Connection> ConnectionPool::getConnection()//���ⲿ�ṩ�ӿڣ������ӳػ�ȡһ�������õĿ�������
{
	unique_lock<mutex> lock(_queueMutex);
	while (_connectionQue.empty())
	{
		if (cv_status::timeout == _cv.wait_for(lock, chrono::microseconds(_connectionTimeout)))
		{//��ʱ�ͷǳ�ʱ
			if (_connectionQue.empty())
			{
				LOG("��ȡ�������ӳ�ʱ������");
				return nullptr;
			}

		}
	}
	
		/*shared_ptr �����Դ����delete����Ҫ�Զ���shared_ptr���ͷ���Դ�ķ�ʽ����connection�黹*/
		shared_ptr<Connection> sp(_connectionQue.front(), [&](Connection* pcon) {
			//�黹���̳߳���
			unique_lock<mutex> lock(_queueMutex);
			pcon->refreshAliveTime();
			_connectionQue.push(pcon);
			//_connectionCnt++;
			LOG("��������");

			
			});
		_connectionQue.pop();
		_cv.notify_all();//�����֪꣬ͨ�����߼�����Ϊ�� ����

		return sp;
	

}
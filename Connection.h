#pragma once
#include<mysql.h>
#include<string>
#include"pch.h"
#include<ctime>
using namespace std;
/* mysql��������ͷ�ļ���lib����lib��������dll����ǰĿ¼��linux -link  mysql��ɾ�Ĳ�  ����Ŀ�������ã�lib��64λ��ѡ��64λ*/
class Connection {
public:
	Connection();
	~Connection();
	bool connect(string ip, unsigned short port, string username, string password, string dbname);
	bool update(string sql);
	MYSQL_RES* query(string sql);
	void refreshAliveTime() { _aliveTime = clock(); }
	clock_t getAliveTime()const { return clock() - _aliveTime; }
private:
	MYSQL* _conn;//��ʾ��mysql sever��һ������
	clock_t _aliveTime;
};

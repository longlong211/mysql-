#pragma once
#include<mysql.h>
#include<string>
#include"pch.h"
#include<ctime>
using namespace std;
/* mysql属性配置头文件，lib，和lib名，拷贝dll到当前目录，linux -link  mysql增删改查  对项目进行配置，lib是64位，选成64位*/
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
	MYSQL* _conn;//表示和mysql sever的一条连接
	clock_t _aliveTime;
};

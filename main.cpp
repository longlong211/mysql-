
#include<iostream>
#include"pch.h"
#include"CommonConnectionPool.h"
#include"Connection.h"
using namespace std;
int main()
{

//Connection conn;
//char sql[1024] = { 0 };
//sprintf_s(sql, "insert into user(name,age,sex) values('%s','%d','%s')",
//	"ongzu", 700, "male");
//conn.connect("127.0.0.1", 3306, "root", "123456", "chat");
//conn.update(sql);

	clock_t begin = clock();
	/*thread t1([]() {*/
		ConnectionPool* cp = ConnectionPool::getConnectionPool();

		for (int i = 0; i < 500; ++i)
		{
			char sql[1024] = { 0 };
			sprintf_s(sql, "insert into user(name,age,sex) values('%s','%d','%s')",
				"longzhu", 80, "male");
			shared_ptr<Connection> sp = cp->getConnection();
			//sp->connect("127.0.0.1", 3306, "root", "123456", "chat");
			sp->update(sql);



		}
	//thread t2([]() {
	//	ConnectionPool* cp = ConnectionPool::getConnectionPool();

	//	for (int i = 0; i < 500; ++i)
	//	{
	//		char sql[1024] = { 0 };
	//		sprintf_s(sql, "insert into user(name,age,sex) values('%s','%d','%s')",
	//			"longzhu", 80, "male");
	//		//sp->connect("127.0.0.1", 3306, "root", "123456", "chat");
	//		shared_ptr<Connection> sp = cp->getConnection();

	//		sp->update(sql);

	//	}});
	//thread t3([]() {
	//	ConnectionPool* cp = ConnectionPool::getConnectionPool();

	//	for (int i = 0; i < 500; ++i)
	//	{
	//		
	//		char sql[1024] = { 0 };
	//		sprintf_s(sql, "insert into user(name,age,sex) values('%s','%d','%s')",
	//			"longzhu", 80, "male");
	//		//sp->connect("127.0.0.1", 3306, "root", "123456", "chat");
	//		shared_ptr<Connection> sp = cp->getConnection();

	//		sp->update(sql);

	//	}});
	
	//t1.join();
	clock_t end = clock();
	cout << (end - begin) << "ms" << endl;


	return 0;
}
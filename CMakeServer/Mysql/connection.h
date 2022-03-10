#ifndef SERVER_CONNECTION_H
#define SERVER_CONNECTION_H

#include <string>
#include <mysql/mysql.h>
#include <mutex>
#include <list>
#include <condition_variable>

class MysqlConnection
{
public: 
	std::string url_;
	int port_;
	std::string user_;
	std::string password_;
	std::string dbName_;
	int closeLog_;

	void init(std::string url, std::string username, std::string password, int port, std::string dbName, int maxConn, int closeLog);
	MYSQL* getConnection();
	bool releaseConnection(MYSQL* conn);

private:
	~MysqlConnection();

	int maxConn_;
	int curConn_;
	int freeConn_;
	std::mutex mutex_;
	std::condition_variable cond_;
	std::list<MYSQL*> connList_;
};

#endif
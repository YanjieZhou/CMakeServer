#include "connection.h"
#include "../log/log.h"

void MysqlConnection::init(std::string url, std::string user, std::string password, int port, std::string dbName, int maxConn, int closeLog) 
{
	url_ = url;
	user_ = user;
	password_ = password;
	port_ = port;
	dbName_ = dbName;
	maxConn_ = maxConn;
	closeLog_ = closeLog;
	curConn_ = 0;
	freeConn_ = 0;

	for (int i = 0; i < maxConn; i++) {
		MYSQL* conn = nullptr;
		conn = mysql_init(conn);

		if (conn == NULL) {
			LOG_ERROR("mysql error");
			exit(1);
		}
		conn = mysql_real_connect(conn, url.c_str(), user.c_str(), password.c_str(), dbName.c_str(), port, NULL, 0);
		if (conn == NULL) {
			LOG_ERROR("mysql error");
			exit(1);
		}
		connList_.push_back(conn);
		++freeConn_;
	}
	maxConn_ = freeConn_;
}

MYSQL* MysqlConnection::getConnection()
{
	MYSQL* conn = nullptr;
	std::unique_lock<std::mutex> lock(mutex_);
	if (connList_.size() == 0) {
		return nullptr;
	}
	while (curConn_ <= 0) {
		cond_.wait(lock);
	}
	conn = connList_.front();
	connList_.pop_front();
	curConn_++;
	freeConn_--;

	return conn;
}

bool MysqlConnection::releaseConnection(MYSQL* conn)
{
	if (conn == nullptr) {
		return false;
	}
	{
		const std::lock_guard<std::mutex> lock(mutex_);
		connList_.push_back(conn);
		freeConn_++;
		curConn_--;
	}
	cond_.notify_all();
	return true;
}

MysqlConnection::~MysqlConnection()
{
	const std::lock_guard<std::mutex> lock(mutex_);
	for (MYSQL* conn : connList_) {
		mysql_close(conn);
	}
	connList_.clear();
}


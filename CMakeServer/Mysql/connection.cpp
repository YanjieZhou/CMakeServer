#include "connection.h"

MysqlConnection::MysqlConnection(std::string url, std::string user, std::string password, int port, std::string dbName, int maxConn, int closeLog): 
	url_(url), user_(user), password_(password), port_(port), dbName_(dbName), maxConn_(maxConn), closeLog_(closeLog) {
	curConn_ = 0;
	freeConn_ = 0;

	for (int i = 0; i < maxConn; i++) {
		MYSQL* conn = mysql_init();
	}
}

MysqlConnection& MysqlConnection::getInstance(std::string url, std::string user, std::string password, int port, std::string dbName, int maxConn, int closeLog) {
	static MysqlConnection instance{ url, user, password, port, dbName, maxConn, closeLog };
	return instance;

}
#ifndef SERVER_LOG_H
#define SERVER_LOG_H

#include <iostream>
#include <mutex>
#include "../lock/blockingqueue.h"
#include <memory>
#include <fstream>

using blocking_queue_p = std::unique_ptr<BlockingQueue<std::string>>;

class Log
{
public: 
	static Log& getInstance() {
		static Log instance;
		return instance;
	}
	void init(const char* fileName, int closeLog, int bufSize, int maxLines, int queueCapacity);

	void flush();

	void writeLog(int level, const char* format, ...);

private:
	char dirName_[128];
	char logName_[128];
	int maxLines_;
	int bufSize_;
	long long count_;
	int today_;
	std::fstream fp_;
	char* buf_;
	bool isAsync_;
	std::mutex mutex_;
	int closeLog_;
	blocking_queue_p bq_;
};

#define LOG_DEBUG(format, ...) if(0 == closeLog_) {Log::getInstance().writeLog(0, format, ##__VA_ARGS__); Log::getInstance().flush();}
#define LOG_INFO(format, ...) if(0 == closeLog_) {Log::getInstance().writeLog(1, format, ##__VA_ARGS__); Log::getInstance().flush();}
#define LOG_WARN(format, ...) if(0 == closeLog_) {Log::getInstance().writeLog(2, format, ##__VA_ARGS__); Log::getInstance().flush();}
#define LOG_ERROR(format, ...) if(0 == closeLog_) {Log::getInstance().writeLog(3, format, ##__VA_ARGS__); Log::getInstance().flush();}

#endif
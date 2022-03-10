#include "log.h"
#include <cstring>
#include <ctime>


void Log::init(const char* fileName, int closeLog, int bufSize, int maxLines, int queueCapacity)
{
	if (queueCapacity > 0) {
		isAsync_ = true;
		bq_ = blocking_queue_p(new BlockingQueue<std::string>(queueCapacity));

	}

	closeLog_ = closeLog;
	bufSize_ = bufSize;
	buf_ = new char[bufSize];
	memset(buf_, '/0', bufSize_);
	maxLines_ = maxLines;

	time_t t = std::time(NULL);
	tm* tm_p = localtime(&t);
	char logName[256] = { 0 };
	snprintf(logName, 255, "%d_%02d_%02d_%s", tm_p->tm_year + 1900, tm_p->tm_mon + 1, tm_p->tm_mday, fileName);
	today_ = tm_p->tm_mday;
	
	fp_.open(fileName, std::ios::app | std::ios::out);
}




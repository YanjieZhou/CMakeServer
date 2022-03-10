#ifndef SERVER_THREADPOOL_H
#define SERVER_THREADPOOL_H

#include "../Mysql/connection.h"
#include <thread>
#include <vector>

template<typename T>
class Threadpool
{
public:
	Threadpool(int actorModel, MysqlConnection* connPool, int numThreads=8);
	~Threadpool();
private:
	int numThreads_;
	MysqlConnection* connPool_;
	std::mutex mutex_;
	std::condition_variable cond_;
	std::vector<std::thread> threads_;
	int actorModel_;

	void run();
};

template<typename T>
Threadpool<T>::Threadpool(int actorModel, MysqlConnection* connPool, int numThreads) : actorModel_(actorModel), connPool_(connPool), numThreads_(numThreads)
{
	if (!threads) {
		throw std::exception();
	}
	for (int i = 0; i < numThreads; i++) {
		threads_.emplace_back(run);
		threads_[i].detach();
	}
}

template<typename T>
Threadpool<T>::~Threadpool()
{
	delete[] threads_;
}

#endif
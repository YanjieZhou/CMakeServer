#ifndef SERVER_THREADPOOL_H
#define SERVER_THREADPOOL_H

#include "../Mysql/connection.h"
#include <thread>
#include <vector>

template<typename T>
class Threadpool
{
public:
	Threadpool(int actorModel, MysqlConnection* connPool, int numThreads=8, int maxRequests=10000);
	~Threadpool();
	bool submit(T* request)
private:
	int numThreads_;
	int maxRequests_;
	MysqlConnection* connPool_;
	std::mutex mutex_;
	std::condition_variable cond_;
	std::vector<std::thread> threads_;
	int actorModel_;
	std::list<T*> workQueue_;

	void run();
};

template<typename T>
Threadpool<T>::Threadpool(int actorModel, MysqlConnection* connPool, int numThreads, int maxRequests) :
	actorModel_(actorModel), connPool_(connPool), numThreads_(numThreads), maxRequests_(maxRequests)
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

template<typename T>
void Threadpool<T>::run() 
{

}
template<typename T>
bool Threadpool<T>::submit(T* request)
{
	mutex_.lock();
	if (workQueue_.size() >= maxRequests_) {
		mutex_.unlock();
		return false;
	}
	workQueue_.push_back(request);
	mutex_.unlock();
	cond_.notify_all();
	return true;
}
#endif
#ifndef SERVER_BQ_H
#define SERVER_BQ_H

#include <cstdlib>
#include <mutex>
#include <condition_variable>

template<typename T>
class BlockingQueue
{
public:
    BlockingQueue(int maxSize = 1000)
    {
        if (maxSize <= 0)
        {
            exit(-1);
        }

        maxSize_ = maxSize;
        array_ = new T[maxSize];
        size_ = 0;
        front_ = -1;
        back_ = -1;
    }

    void clear()
    {
        mutex_.lock();
        size_ = 0;
        front_ = -1;
        back_ = -1;
        mutex_.unlock();
    }

    ~BlockingQueue()
    {
        const std::lock_guard<std::mutex> lock(mutex_);    
        if (array_ != NULL)
            delete[] array_;
    }

    //判断队列是否满了
    bool isFull()
    {
        const std::lock_guard<std::mutex> lock(mutex_);
        if (size_ >= maxSize_)
        {
            return true;
        }
        return false;
    }
    //判断队列是否为空
    bool isEmpty()
    {
        const std::lock_guard<std::mutex> lock(mutex_);
        if (0 == size_)
        {
            return true;
        }
        return false;
    }
    //返回队首元素
    bool front(T& value)
    {
        const std::lock_guard<std::mutex> lock(mutex_);
        if (0 == size_)
        {
            return false;
        }
        value = array_[front_];
        return true;
    }
    //返回队尾元素
    bool back(T& value)
    {
        const std::lock_guard<std::mutex> lock(mutex_);
        if (0 == size_)
        {
            return false;
        }
        value = array_[back_];
        return true;
    }

    int size()
    {
        int tmp = 0;
        const std::lock_guard<std::mutex> lock(mutex_);
        tmp = size_;

        return tmp;
    }

    int maxSize()
    {
        int tmp = 0;

        const std::lock_guard<std::mutex> lock(mutex_);
        tmp = maxSize_;

        return tmp;
    }
    //往队列添加元素，需要将所有使用队列的线程先唤醒
    //当有元素push进队列,相当于生产者生产了一个元素
    //若当前没有线程等待条件变量,则唤醒无意义
    bool push(const T& item)
    {

        std::unique_lock<std::mutex> lock(mutex_);
        if (size_ >= maxSize_)
        {
            lock.unlock();
            cond_.notify_all();
            return false;
        }

        back_ = (back_ + 1) % maxSize_;
        array_[back_] = item;

        size_++;

        lock.unlock();
        cond_.notify_all();
        return true;
    }
    //pop时,如果当前队列没有元素,将会等待条件变量
    bool pop(T& item)
    {

        std::unique_lock<std::mutex> lock(mutex_);
        while (size_ <= 0)
        {

            cond_.wait(lock);
        }

        front_ = (front_ + 1) % maxSize_;
        item = array_[front_];
        size_--;
        return true;
    }

private:
    std::mutex mutex_;
    std::condition_variable cond_;

    T* array_;
    int size_;
    int maxSize_;
    int front_;
    int back_;
};
#endif // !SERVER_BQ_H

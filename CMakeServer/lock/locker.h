
#include <stdexcept>
#include <pthread.h>
#include <time.h>

class Condition {
public:
	Condition(const Condition&) = delete;
	Condition& operator=(const Condition&) = delete;

	Condition() {
		if (pthread_cond_init(&cond_, NULL) != 0) {
			throw std::exception();
		}
	}

	~Condition() {
		pthread_cond_destroy(&cond_);
	}

	void wait() {
		pthread_cond_wait(&cond_, &mutex_);
	}

	void timedwait(timespec t) {
		pthread_cond_timedwait(&cond_, &mutex_, &t);
	}

private:
	pthread_cond_t cond_;
	pthread_mutex_t mutex_;
};
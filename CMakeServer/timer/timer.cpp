#include "timer.h"

int main()

{

	signal(SIGALRM, tick);

	alarm(1); // 1s的周期心跳

	// test

	addTimer(1, doTime1);

	addTimer(2, doTime2);

	addTimer(9, doTime9);

	while (1) pause();

	return 0;

}

template <class ForwardIterator, class T>
ForwardIterator remove(ForwardIterator first, ForwardIterator last, const T& val)
{
	ForwardIterator result = first;
	while (first != last) {
		if (!(*first == val)) {
			if (result != first)
				*result = move(*first);
			++result;
		}
		++first;
	}
	return result;
}

#include <cstdlib>
#include <csignal>
#include <unistd.h>
#include <iostream>

const static int csTimeWheelSize = 8;

typedef void (*func)(int data);

struct TimerNode{

	struct TimerNode* next;

	int rotation;

	func proc;

	int data;

};

struct TimerWheel {

	TimerNode* slot[csTimeWheelSize];

	int current;

};

static TimerWheel timer = { {0}, 0 };

void tick(int signal)

{

	// 使用二级指针进行单链表的删除

	TimerNode** cur = &timer.slot[timer.current];

	while (*cur) {

		TimerNode* curr = *cur;

		if (curr->rotation > 0) {

			curr->rotation--;

			cur = &curr->next;

		}
		else {

			curr->proc(curr->data);

			*cur = curr->next;

			free(curr);

		}

	}

	timer.current = (timer.current + 1) % csTimeWheelSize;

	alarm(1);

}

void addTimer(int len, func action)

{

	int pos = (len + timer.current) % csTimeWheelSize;

	TimerNode* node = (TimerNode*) malloc(sizeof(*node));

	// 插入到对应格子的链表头部即可, O(1)复杂度

	node->next = timer.slot[pos];

	timer.slot[pos] = node;

	node->rotation = len / csTimeWheelSize;

	node->data = 0;

	node->proc = action;

}

// test case1: 1s循环定时器

int g_sec = 0;

void doTime1(int data)

{

	printf("timer %s, %d\n", __FUNCTION__, g_sec++);

	addTimer(1, doTime1);

}

// test case2: 2s单次定时器

void doTime2(int data)

{

	printf("timer %s\n", __FUNCTION__);

	addTimer(2, doTime1);

}

// test case3: 9s循环定时器

void doTime9(int data)

{

	printf("timer %s\n", __FUNCTION__);

	addTimer(9, doTime9);

}



/*
 * Thread_Pool.h
 *
 *  Created on: 2013-8-30
 *      Author: casa
 */
/*
 * 没有任何容错和销毁线程的版本
 * */
#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <stdlib.h>

#include <iostream>
#include <pthread.h>
#include <vector>
using namespace std;

class Thread_Pool {
public:
	class Task{
		//这个地方就是想要Task的集合，这样写有什么不妥吗？
		//TODO: 有没有较为好点的写法
		friend class Thread_Pool;
	public:
		Task(){}
		virtual ~Task(){}

	private:
		void (*function)(void *);//函数指针
		void *argument;//传进函数的这个参数
	};

	Thread_Pool();
	virtual ~Thread_Pool();

	void Thread_Pool_init(int thread_count_in_pool_,int task_count_in_queue_);
	void Thread_Pool_add(void (*function_)(void *), void *argument_);

	static void * threadpool_thread(void *arg);

private:

	pthread_mutex_t lock;
	pthread_cond_t notify;
	pthread_t *thread_id_list;
	std::vector<Task*> task_list;

	int thread_count_in_pool;//线程池里面总共有多少个线程
	int task_count_in_queue;//任务队列中的任务个数

	int head_in_queue;//先插进queue的任务
	int tail_in_queue;//后插进queue的任务
	//以上两个参数其实是任务的索引，是FIFO的

	int pending_task_count;//在等待执行的任务
	int started_thread_count;//已经开始运行的线程，create之后知道exit的线程个数

};

#endif /* THREAD_POOL_H_ */

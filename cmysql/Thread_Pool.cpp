/*
 * Thread_Pool.cpp
 *
 *  Created on: 2013-8-30
 *      Author: casa
 */

#include "Thread_Pool.h"

Thread_Pool::Thread_Pool() {
	// TODO 自动生成的构造函数存根

}

Thread_Pool::~Thread_Pool() {
	// TODO 自动生成的析构函数存根
}

void Thread_Pool::Thread_Pool_init(int thread_count_in_pool_,int task_count_in_queue_){
	//这里第二个参数为NULL的原因是参数不用设置
	pthread_mutex_init(&lock,NULL);
	pthread_cond_init(&notify,NULL);
	thread_id_list=(pthread_t *)malloc(sizeof(pthread_t)*thread_count_in_pool_);

	for(int i=0;i<task_count_in_queue_;i++){
		Task *task=new Thread_Pool::Task();
		task_list.push_back(task);
	}

	thread_count_in_pool=0;
	task_count_in_queue=task_count_in_queue_;

	head_in_queue=tail_in_queue=0;
	pending_task_count=0;
	started_thread_count=0;

	for(int i=0;i<thread_count_in_pool_;i++){
		//这里的一定要搞清楚，因为传递的是本实例的指针，这里一定要搞懂？
		pthread_create(&thread_id_list[i],NULL,threadpool_thread,this);
		thread_count_in_pool++;
		//因为一创建线程就开始做了，所以要把开始做的线程加1
		started_thread_count++;
	}
}


void Thread_Pool::Thread_Pool_add(void (*function_)(void *), void *argument_){
	int next_task_todo=0;
	pthread_mutex_lock(&lock);
	next_task_todo=tail_in_queue+1;
	next_task_todo=(next_task_todo==task_count_in_queue)?0:next_task_todo;
	do{
		if(pending_task_count==task_count_in_queue)
			break;
		task_list[tail_in_queue]->function=function_;
		task_list[tail_in_queue]->argument=argument_;
		tail_in_queue=next_task_todo;
		pending_task_count++;
		pthread_cond_signal(&notify);
	}while(0);

	pthread_mutex_unlock(&lock);
}

void *Thread_Pool::threadpool_thread(void *arg){
	Thread_Pool *Pthis=(Thread_Pool*)arg;
	Task *task=new Task();
	//这是线程池创建的一个程序块，这里其实是一个for循环，一直在等待add函数添加一个函数去给执行
	while(1){
		pthread_mutex_lock(&(Pthis->lock));
		//如果还没有一个任务到来的话，证明进入这个锁之后的任务是第一个创建之后还没有得到得到任务的那个线程，
		//这样的话就一直循环等待，当没有任务的时候你肯定要等待，不然你拿什么去做，但是这个地方一定要这个循环吗？
		while(Pthis->pending_task_count==0){
			pthread_cond_wait(&(Pthis->notify),&(Pthis->lock));
		}
		task->function=Pthis->task_list.at(Pthis->head_in_queue)->function;
		task->argument=Pthis->task_list.at(Pthis->head_in_queue)->argument;
		Pthis->head_in_queue=Pthis->head_in_queue+1;
		//这里是一个循环使用task队列，如果满了计数就从头开始，有bug
		//TODO: 解决对面追上前面怎么办？会出现这样的情况吗？
		Pthis->head_in_queue=(Pthis->head_in_queue==Pthis->task_count_in_queue)?0:Pthis->head_in_queue;
		Pthis->pending_task_count=Pthis->pending_task_count-1;
		pthread_mutex_unlock(&(Pthis->lock));
		(*(task->function))(task->argument);
	}
	Pthis->started_thread_count=Pthis->started_thread_count-1;
	pthread_mutex_unlock(&(Pthis->lock));
	pthread_exit(NULL);
	return (NULL);
}


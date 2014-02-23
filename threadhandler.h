#ifndef __THREAD_HANDLER_H_
#define __THREAD_HANDLER_H_

#define BUFFSZ 512
#define BITSHIFT 12
#define CORES 16
#define COREMEM 32

struct threadLocal;

struct threadRecord
{
	int number;
	char path[BUFFSZ];
	struct threadRecord *next;
};

struct threadLocal
{
	int threadNumber;
	pthread_mutex_t threadLocalLock;
	void* localTree;
	void* optTree;
	struct threadLocal* prev;
	struct threadLocal* next;
};

struct threadGlobal
{
	int activeThreads;
	struct threadLocal* head;
	struct threadLocal* tail;
};

struct threadResources
{
	int threadNumber;
	struct threadRecord *records;
	void* globalTree;
	struct threadGlobal* globals;
	struct threadLocal* local;
};


#endif


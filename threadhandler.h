#ifndef __THREAD_HANDLER_H_
#define __THREAD_HANDLER_H_

#define BUFFSZ 512
#define BITSHIFT 12
#define CORES 16
#define COREMEM 32
#define MEMWIDTH 16

struct threadLocal;

struct threadRecord
{
	int number;
	char path[BUFFSZ];
	struct threadLocal *local;
	struct threadRecord *next;
	pthread_mutex_t lockToAddDeleteRecord;
};

struct threadLocal
{
	int threadNumber;
	long instructionCount;
	long faultCount;
	long prevInstructionCount;
	long prevFaultCount;
	void* localTree;
	void* optTree;
	struct threadLocal *prev;
	struct threadLocal *next;
	pthread_mutex_t threadLocalLock;
};

struct threadGlobal
{
	int activeThreads;
	struct threadLocal* head;
	struct threadLocal* tail;
	void* globalTree;
	pthread_mutex_t threadGlobalLock;
};

struct threadResources
{
	struct threadRecord *records;
	struct threadGlobal* globals;
	struct threadLocal* local;
};


#endif


#ifndef __THREAD_HANDLER_H_
#define __THREAD_HANDLER_H_

#define BUFFSZ 512
#define BITSHIFT 12
#define CORES 16
#define COREMEM 32768
#define MEMWIDTH 16
#define PAGESIZE (1 << BITSHIFT) 

struct ThreadLocal;

struct ThreadArray
{
	int threadNumber;
	pthread_t aPThread;
	struct ThreadArray* nextThread;
};

struct ThreadRecord
{
	int number;
	char path[BUFFSZ];
	struct ThreadLocal *local;
	struct ThreadRecord *next;
};

struct ThreadLocal
{
	int threadNumber;
	long instructionCount;
	long faultCount;
//	long prevFaultCount;
	long tickCount;
	long prevTickCount;
	void* localTree;
	void* optTree;
	pthread_mutex_t threadLocalLock;
};

struct ThreadGlobal
{
	struct ThreadRecord* head;
	void* globalTree;
	struct ThreadArray *threads;
	char* outputPrefix;
	pthread_mutex_t threadGlobalLock;
};

struct ThreadResources
{
	struct ThreadRecord *records;
	struct ThreadGlobal* globals;
	struct ThreadLocal* local;
};

struct PageChain {
	long page;
	struct PageChain *next;
};


void* startThreadHandler(void *resources);
void incrementActive(void);
void decrementActive(void);
void updateTickCount(struct ThreadLocal* local);

#endif


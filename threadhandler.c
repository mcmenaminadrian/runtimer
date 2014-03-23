#include <stdio.h>
#include <stdlib.h>
#include <expat.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#include "threadhandler.h"
#include "pages.h"
#include "insttree.h"
#include "opttree.h"

//launch a thread
static void spawnThread(int threadNo, struct ThreadGlobal* globals)
{
	printf("Spawning thread %i. at tick %li\n", threadNo,
		globals->totalTicks);
	char* threadOPT = (char*) malloc(BUFFSZ);
	if (!threadOPT) {
		fprintf(stderr,
			"Could not allocate memory to spawn thread %i.\n",
			threadNo);
		goto fail;
	}
	sprintf(threadOPT, "%s%i.bin", globals->outputPrefix, threadNo);

	//find the file that matches the thread number
	struct ThreadRecord* threadRecord = globals->head;
	
	while (threadRecord) {
		if (threadRecord->number == threadNo) {
			break;
		}
		threadRecord = threadRecord->next;
	}

	struct ThreadLocal* localThreadStuff = (struct ThreadLocal*)
		malloc(sizeof (struct ThreadLocal));
	if (!localThreadStuff) {
		fprintf(stderr, "Could not create local stuff for thread %i\n",
			threadNo);
		goto failTL;
	}

	localThreadStuff->threadNumber = threadNo;
	localThreadStuff->instructionCount = 0;
	localThreadStuff->tickCount = 0;
	localThreadStuff->prevTickCount = 0;
	localThreadStuff->faultCount = 0;
	localThreadStuff->prevInstructionCount = 0;
	localThreadStuff->prevFaultCount = 0;

	localThreadStuff->optTree = createOPTTree();
	if (!localThreadStuff->optTree) {
		fprintf(stderr, "Could not create OPT tree for thread %i\n",
			threadNo);
		goto failOPT;
	}

	int errL = pthread_mutex_init(&localThreadStuff->threadLocalLock, NULL);
	if (errL) {
		fprintf(stderr,
			"Error %i when initialising lock on thread %i\n",
			errL, threadNo);
		goto failLock;
	}

	readOPTTree(localThreadStuff->optTree, threadOPT);
	
	struct ThreadResources* threadResources = (struct ThreadResources*)
		malloc(sizeof (struct ThreadResources));
	if (!threadResources) {
		fprintf(stderr,
			"Could not allocate memory for ThreadResources for thread %i\n",
			threadNo);
		goto failTR;
	}

	threadResources->records = threadRecord;
	threadResources->globals = globals;
	threadResources->local = localThreadStuff;
	threadResources->records->local = localThreadStuff;

	struct ThreadArray* anotherThread = (struct ThreadArray*)
		malloc(sizeof (struct ThreadArray));
	if (!anotherThread) {
		fprintf(stderr,
			"Could not create pThread memory for thread %i\n",
			threadNo);
		goto failTA;
	}
	anotherThread->threadNumber = threadNo;
	anotherThread->nextThread = NULL;

	pthread_mutex_lock(&globals->threadGlobalLock);
	struct ThreadArray* tArray = globals->threads;	
	while (tArray) {
		if (tArray->nextThread == NULL) {
			tArray->nextThread = anotherThread;
			break;
		}
		tArray = tArray->nextThread;
	}
	pthread_mutex_unlock(&globals->threadGlobalLock);
	
	free(threadOPT);
	pthread_create(&anotherThread->aPThread, NULL, startThreadHandler,
		(void*)threadResources);
	return;

failTA:
	free(threadResources);
failTR:
failLock:
	free(localThreadStuff->optTree);
failOPT:
	free(localThreadStuff);
failTL:
	free(threadOPT);
fail:
	return;
}

static void removePage(long pageNumber, struct ThreadResources *thResources)
{
	/*printf("Thread: %i - instruction: %li ticks: %li faults: %li\n",
		thResources->local->threadNumber,
		thResources->local->instructionCount,
		thResources->local->tickCount,
		thResources->local->faultCount); */
		

	struct ThreadRecord* records = thResources->records;
	struct PageChain *headChain =
			getPageChain(thResources->globals->globalTree);
	struct PageChain* currentChain = NULL;
	void* minTree = createMinTree();
	while (records) {
		struct ThreadLocal* locals = records->local;
		if (!locals) {
			break;
		}
		currentChain = headChain;
		void* instructionTree = createInstructionTree();
		while (currentChain) {
			long instructionNext =
				nextInChain(currentChain->page,
				locals->instructionCount,
				locals->optTree);
			if (instructionNext != -1) {
				insertIntoTree(currentChain->page,
					instructionNext, instructionTree);
			} else {
				insertIntoTree(currentChain->page,
					LONG_MAX, instructionTree);
			}
			currentChain = currentChain->next;
		}

		pushToMinTree(minTree, instructionTree);
		records = records->next;
		freeInstTree(instructionTree);
	}

	removeFromPageTree(getPageToKill(minTree),
		thResources->globals->globalTree);
	killMinTree(minTree);		
}
		
static int faultPage(long pageNumber, struct ThreadResources *thResources)
{
	int countDown = (4096 * 100)/MEMWIDTH ;
	while (countDown) {
		if (locatePageTreePR(pageNumber,
			thResources->globals->globalTree) > 0) {
			return 0;
		}
		updateTickCount(thResources);
		countDown--;
	}
	thResources->local->faultCount++;
	return 1;
}

static void inGlobalTree(long pageNumber, struct ThreadResources *thResources,
	time_t *now)
{
	struct ThreadGlobal *globals = thResources->globals;
	pthread_mutex_unlock(&globals->threadGlobalLock);
	updateTickCount(thResources);
}

static void notInGlobalTree(long pageNumber,
	struct ThreadResources *thResources, time_t *now)
{
	struct ThreadGlobal *globals = thResources->globals;
	decrementCoresInUse();
	pthread_mutex_unlock(&globals->threadGlobalLock);
	if (faultPage(pageNumber, thResources) > 0) {
		pthread_mutex_lock(&globals->threadGlobalLock);
		if (countPageTree(globals->globalTree) >=
			CORES * COREMEM / PAGESIZE ) {
			removePage(pageNumber, thResources);
		}	
		insertIntoPageTree(pageNumber, globals->globalTree);
		pthread_mutex_unlock(&globals->threadGlobalLock);
	}
	incrementCoresInUse(thResources);
}
	
static void XMLCALL
threadXMLProcessor(void* data, const XML_Char *name, const XML_Char **attr)
{ 
	int i;
	long address, pageNumber;
	struct ThreadResources *thResources;
	struct ThreadGlobal *globals;
	struct ThreadLocal *local;
	thResources = (struct ThreadResources *)data;
	globals = thResources->globals;
	local = thResources->local;
	if (strcmp(name, "instruction") == 0 || strcmp(name, "load") == 0 ||
		strcmp(name, "modify") == 0 || strcmp(name, "store") == 0) {
		for (i = 0; attr[i]; i += 2) {
			if (strcmp(attr[i], "address") == 0) {
				time_t now = time(NULL);
				address = strtol(attr[i+1], NULL, 16);
				pageNumber = address >> BITSHIFT;
				//have address - is it already present?
				pthread_mutex_lock(&globals->threadGlobalLock);
				if (locatePageTreePR(pageNumber,
						globals->globalTree) > 0) {
					inGlobalTree(pageNumber, thResources,
						&now);
				} else {
					notInGlobalTree(pageNumber,
						thResources, &now);
				}
			}
		}
		if (strcmp(name, "modify") == 0) {
			//do it again
			time_t now = time(NULL);
			pthread_mutex_lock(&globals->threadGlobalLock);
			if (locatePageTreePR(pageNumber, globals->globalTree)){
				inGlobalTree(pageNumber, thResources, &now);
			} else {
				notInGlobalTree(pageNumber, thResources, &now);
			}
		}
		local->instructionCount++;
	} else {
		if (strcmp(name, "spawn") == 0) {
			for (i = 0; attr[i]; i += 2) {
				if (strcmp(attr[i], "thread") == 0) {
					int threadNo = atoi(attr[i+1]);
					spawnThread(threadNo,
						thResources->globals);
				}
			}
		}
	}
}

void* startThreadHandler(void *resources)
{
	FILE* inThreadXML;
	size_t len = 0;
	int done;
	char data[BUFFSZ];

	struct ThreadResources *thResources;
	thResources = (struct ThreadResources*)resources;
	//Setup the Parser
	XML_Parser p_threadParser = XML_ParserCreate("UTF-8");
	if (!p_threadParser) {
		fprintf(stderr, "Could not create parser for thread\n");
		return (void*)(-1);
	}
	//Pass the parser thread specific data
	XML_SetUserData(p_threadParser, resources);
	//Start the Parser
	XML_SetStartElementHandler(p_threadParser, threadXMLProcessor);
	inThreadXML = fopen(thResources->records->path, "r");
	if (inThreadXML == NULL) {
		fprintf(stderr, "Could not open %s\n",
			thResources->records->path);
		XML_ParserFree(p_threadParser);
		goto cleanup;
	}
	incrementCoresInUse(thResources);
	incrementActive();
	do { 
		len = fread(data, 1, sizeof(data), inThreadXML);
		done = len < sizeof(data);
		
		if (XML_Parse(p_threadParser, data, len, 0) == 0) {
			enum XML_Error errcde =
				XML_GetErrorCode(p_threadParser);
			printf("PARSE ERROR: %s\n", XML_ErrorString(errcde));
			printf("Error at column number %lu\n",
				XML_GetCurrentColumnNumber(p_threadParser));
			printf("Error at line number %lu\n",
				XML_GetCurrentLineNumber(p_threadParser));
			goto cleanup;
		}
	} while(!done);
	decrementActive();
	decrementCoresInUse();
	thResources->local->prevTickCount = 0;
	updateTickCount(thResources);
	printf("Thread %i finished at tick %li\n",
		thResources->local->threadNumber,
		thResources->globals->totalTicks);
	
	struct ThreadArray* aThread = thResources->globals->threads;
	while (aThread) {
		if (aThread->threadNumber != thResources->local->threadNumber){
			pthread_join(aThread->aPThread, NULL);
			aThread = aThread->nextThread;
		}
	}

cleanup:
	removeOPTTree(thResources->local->optTree);
	removePageTree(thResources->globals->globalTree);
	free(thResources->globals);
	free(thResources);
	
	return NULL;
}

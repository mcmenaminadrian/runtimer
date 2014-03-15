#include <stdio.h>
#include <stdlib.h>
#include <expat.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include "threadhandler.h"
#include "pages.h"
#include "insttree.h"
#include "opttree.h"

//launch a thread
static void spawnThread(int threadNo, struct ThreadGlobal* globals)
{
	char* threadName = NULL;
	//find the file that matches the thread number
	struct ThreadRecord* threadRecord = globals->head;
	while (threadRecord) {
		if (threadRecord->number == threadNo) {
			threadName = (char*) malloc(BUFFSZ);
			strcpy(threadName, threadRecord->path);
			break;
		}
		threadRecord = threadRecord->next;
	}
	if (!threadName) {
		fprintf(stderr, "Could not initialise record for thread %i\n",
			threadNo);
		return;
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
	localThreadStuff->localTree = createPageTree();
	if (!localThreadStuff->localTree) {
		fprintf(stderr, "Could not create local tree for thread %i\n",
			threadNo);
		goto failLocTree;
	}

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

	readOPTTree(localThreadStuff->optTree, threadName);
	
	struct ThreadResources* threadResources = (struct ThreadResources*)
		malloc(sizeof (struct ThreadResources));
	if (!threadResources) {
		fprintf(stderr,
			"Could not allocate memory for ThreadResources for thread %i\n",
			threadNo);
		goto failTR;
	}

	threadResources->records = startTR;
	threadResources->globals = globals;
	threadResources->local = localThreadStuff;

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
	
	free(threadName);
	pthread_create(&anotherThread->aPThread, NULL, startThreadHandler,
		(void*)threadResources);
	return;

failTA:
	free(threadResources);
failTR:
failLock:
	free(localThreadStuff->optTree);
failOPT:
	free(localThreadStuff->localTree);	
failLocTree:
	free(localThreadStuff);
failTL:
done:
	free(threadName);
	return;
}


//C code that parses a thread

static void removePage(long pageNumber, struct ThreadResources *thResources)
{
	printf("Introducing page %li\n", pageNumber);
	//find the page with the longest reuse distance,
	//otherwise find the page with the oldest date
	//either for this thread or all threads
	struct PageChain *currentChain =
		getPageChain(thResources->local->localTree);
	void* instructionTree = createInstructionTree();
	while (currentChain) {
		long instructionNext =
			nextInChain(currentChain->page,
			thResources->local->instructionCount,
			thResources->local->optTree);
		if (instructionNext == -1) {
			fprintf(stderr,
			"ERROR: Page %li in localTree but not in optTree",
			currentChain->page);
		} else {
			insertIntoTree(currentChain->page, instructionNext,
				instructionTree);
		}
		currentChain = currentChain->next;
	}
	//now get the maximum
	long maximumReuse = maxNode(instructionTree);
	printf("Maximum reuse page is %li\n", maximumReuse);
	if (maximumReuse && locatePageTreePR(maximumReuse,
		thResources->globals->globalTree)) {
		removeFromPageTree(maximumReuse,
			thResources->globals->globalTree);
		removeFromPageTree(maximumReuse,
			thResources->local->localTree);
	} else {
		//if we didn't find the page, kill the oldest page
		removeOldestFromPageTree(thResources->globals->globalTree);
	}
	freeInstTree(instructionTree);
}
		
static int faultPage(long pageNumber, struct ThreadResources *thResources)
{
	int countDown = 100 * MEMWIDTH;
	while (countDown) {
		if (locatePageTreePR(pageNumber,
			thResources->globals->globalTree)) {
			return 0;
		}
		updateTickCount(thResources->local);
		countDown--;
	}
	thResources->local->faultCount++;
	return 1;
}

static void inGlobalTree(long pageNumber, struct ThreadResources *thResources,
	time_t *now)
{
	struct ThreadGlobal *globals = thResources->globals;
	struct ThreadLocal *local = thResources->local;
	updateLRU(pageNumber, *now, globals->globalTree);
	pthread_mutex_unlock(&globals->threadGlobalLock);
	if (locatePageTreePR(pageNumber, local->localTree)) {
		updateLRU(pageNumber, *now, local->localTree);
	} else {
		insertIntoPageTree(pageNumber, *now, local->localTree);
	}
	updateTickCount(local);
}

static void notInGlobalTree(long pageNumber,
	struct ThreadResources *thResources, time_t *now)
{
	struct ThreadGlobal *globals = thResources->globals;
	struct ThreadLocal *local = thResources->local;
	pthread_mutex_unlock(&globals->threadGlobalLock);
	if (faultPage(pageNumber, thResources) > 0) {
		pthread_mutex_lock(&globals->threadGlobalLock);
		if (countPageTree(globals->globalTree) >=
			CORES * COREMEM / PAGESIZE ) {
			removePage(pageNumber, thResources);
		}	
		insertIntoPageTree(pageNumber, *now, globals->globalTree);
		pthread_mutex_unlock(&globals->threadGlobalLock);
		insertIntoPageTree(pageNumber, *now, local->localTree);
	}
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
						globals->globalTree)) {
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

	struct ThreadArray* aThread = thResources->globals->threads;
	while (aThread) {
		if (aThread->number != thResources->local->threadNumber) {
			pthread_join(aThread->aPThread);
			aThread = aThread->nextThread;
		}
	}

cleanup:
	removeOPTTree(thResources->local->optTree);
	removePageTree(thResources->local->localTree);
	removePageTree(thResources->globals->globalTree);
	free(thResources->globals);
	free(thResources);
	
	return NULL;
}

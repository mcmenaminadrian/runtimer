#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <expat.h>
#include <pthread.h>
#include "pages.h"
#include "threadhandler.h"
#include "opttree.h"

#define BARRIER 10

struct ThreadRecord *startTR = NULL;
static char outputprefix[BUFFSZ];
static pthread_mutex_t updateLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t barrierThreshold = PTHREAD_COND_INITIALIZER;
static int threadsActive = 0;
static int threadsLocked = 0;

void incrementActive(void)
{
	pthread_mutex_lock(&updateLock);
	threadsActive++;
	pthread_mutex_unlock(&updateLock);
}

void decrementActive(void)
{
	pthread_mutex_lock(&updateLock);
	threadsActive--;
	pthread_mutex_unlock(&updateLock);
}

struct ThreadRecord* createThreadRecord(int tNum, char* fileName)
{
	struct ThreadRecord* newRecord = malloc(sizeof (struct ThreadRecord));
	if (!newRecord) {
		fprintf(stderr, "Could not create ThreadRecord.\n");
		return NULL;
	}
	newRecord->number = tNum;
	strcpy(newRecord->path, fileName);
	newRecord->next = NULL;
	newRecord->local = NULL;
	return newRecord;
}

void mapThread(struct ThreadRecord **root, int tNum, char *fileName)
{
	if (*root) {
		struct ThreadRecord* thRecord = *root;
		while (thRecord->next) {
			thRecord = thRecord->next;
		}
		thRecord->next = createThreadRecord(tNum, fileName);
	} else {
		*root = createThreadRecord(tNum, fileName);
	}
}

void cleanThreadList(struct ThreadRecord *root)
{
	if (root == NULL)
		return;
	struct ThreadRecord *nextOne = root->next;
	free(root);
	cleanThreadList(nextOne);
}

void usage()
{
	printf("USAGE: runtimer controlfile prefix\n");
}

static void XMLCALL
	starthandler(void *data, const XML_Char *name, const XML_Char **attr)
{
	int i;
	int threadID = 0;
	char threadPath[BUFFSZ]; 
	if (strcmp(name, "file") == 0) {
		for (i = 0; attr[i]; i += 2) {
			if (strcmp(attr[i], "thread") == 0) {
				threadID = atoi(attr[i + 1]);
				break;
			}
		}
		for (i = 0; attr[i]; i += 2) {
			if (strcmp(attr[i], "path") == 0) {
				strcpy(threadPath, attr[i + 1]);
				break;
			}
		}
		mapThread(&startTR, threadID, threadPath);
	} 
}

void updateTickCount(struct ThreadLocal* local)
{
	local->tickCount++;
	if (local->tickCount - local->prevTickCount >= BARRIER) {
		pthread_mutex_lock(&updateLock);
		threadsLocked++;
		if (threadsLocked >= threadsActive) {
			pthread_cond_broadcast(&barrierThreshold);
			threadsLocked = 0;
		} else {
			pthread_cond_wait(&barrierThreshold, &updateLock);
		}
		pthread_mutex_unlock(&updateLock);
		local->prevTickCount = local->tickCount;
	}
}			

int startFirstThread(char* outputprefix)
{
	int errL, errG;
	struct ThreadLocal *firstThreadLocal;
	char threadname[BUFFSZ];
	struct ThreadResources *firstThreadResources;
	
	//start the first thread
	//first task is read the OPT string
	struct ThreadGlobal* globalThreadList =
		(struct ThreadGlobal*)malloc(sizeof (struct ThreadGlobal));
	if (!globalThreadList) {
		fprintf(stderr,
			"Could not allocate memory for threadGlobal.\n");
		goto failed;
	}

	globalThreadList->globalTree = createPageTree();
	if (!(globalThreadList->globalTree)) {
		fprintf(stderr,
			"Could not create global tree");
		goto failGlobalTree;
	}

	firstThreadLocal =
		(struct ThreadLocal*)malloc(sizeof(struct ThreadLocal));
	if (!firstThreadLocal) {
		fprintf(stderr,
			"Could not allocate memory for threadLocal.\n");
		goto failFirstThreadLocal;
	}
	startTR->local = firstThreadLocal;
	firstThreadLocal->prev = NULL;
	firstThreadLocal->next = NULL;	
	firstThreadLocal->instructionCount = 0;
	firstThreadLocal->tickCount = 0;
	firstThreadLocal->faultCount = 0;
	firstThreadLocal->localTree = createPageTree();
	if (!(firstThreadLocal->localTree)) {
		fprintf(stderr,
			"Could not initialise local tree.\n");
		goto failLocalTree;
	}

	firstThreadLocal->optTree = createOPTTree();
	if (!(firstThreadLocal->optTree)) {
		fprintf(stderr,
			"Could not initialise OPT tree.\n");
		goto failOPTTreeCreate;
	}

	firstThreadLocal->threadNumber = startTR->number;

	sprintf(threadname, "%s%i.bin",outputprefix, startTR->number);
	readOPTTree(firstThreadLocal->optTree, threadname);

	//prepare to start the thread
	firstThreadResources =
		(struct ThreadResources*)malloc(sizeof (struct ThreadResources));
	if (!firstThreadResources) {
		fprintf(stderr,
			"Could not allocate memory for threadResources.\n");
		goto failResources;
	}
	firstThreadResources->records = startTR;
	firstThreadResources->globals = globalThreadList;
	firstThreadResources->local = firstThreadLocal;
	globalThreadList->head = startTR;
	errL = pthread_mutex_init(&firstThreadLocal->threadLocalLock, NULL);
	errG = pthread_mutex_init(&globalThreadList->threadGlobalLock, NULL);
	if (errL || errG) {
		fprintf(stderr,
			"Mutex initialisation fails with %i and %i\n",
			errL, errG);
		goto failMutex;
	}
	pthread_t firstHandlerThread;
	pthread_create(&firstHandlerThread, NULL, startThreadHandler,
		(void *)firstThreadResources);
	pthread_join(firstHandlerThread, NULL);
	return 0;

failMutex:
	free(firstThreadResources);
failResources:
	removeOPTTree(firstThreadLocal->optTree);
failOPTTreeCreate:
	removePageTree(firstThreadLocal->localTree);
failLocalTree:
	free(firstThreadLocal);
failFirstThreadLocal:
	removePageTree(globalThreadList->globalTree);
failGlobalTree:
	free(globalThreadList);	
failed:
	free(startTR);
	return -1;
}


int main(int argc, char* argv[])
{
	FILE* inXML;
	char data[BUFFSZ]; 
	size_t len = 0;
	int done;	

	if (argc < 3) {
		usage();
		exit(-1);
	}

	strcpy(outputprefix, argv[2]);

	XML_Parser p_ctrl = XML_ParserCreate("UTF-8");
	if (!p_ctrl) {
		fprintf(stderr, "Could not create parser\n");
		exit(-1);
	}

	XML_SetStartElementHandler(p_ctrl, starthandler);
	inXML = fopen(argv[1], "r");
	if (inXML == NULL) {
		fprintf(stderr, "Could not open %s\n", argv[1]);
		XML_ParserFree(p_ctrl);
		exit(-1);
	}

	do {
		len = fread(data, 1, sizeof(data), inXML);
		done = len < sizeof(data);

		if (XML_Parse(p_ctrl, data, len, 0) == 0) {
			enum XML_Error errcde = XML_GetErrorCode(p_ctrl);
			printf("ERROR: %s\n", XML_ErrorString(errcde));
			printf("Error at column number %lu\n",
				XML_GetCurrentColumnNumber(p_ctrl));
			printf("Error at line number %lu\n",
				XML_GetCurrentLineNumber(p_ctrl));
			exit(-1);
		}
	} while(!done);

	XML_ParserFree(p_ctrl);
	fclose(inXML);
	if (startFirstThread(outputprefix) < 0) {
		printf("ERROR: thread parsing failed.\n");
		exit(-1);
	}
	cleanThreadList(startTR);
	return 0;
}

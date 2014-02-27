#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <expat.h>
#include <pthread.h>
#include "pages.h"
#include "threadhandler.h"
#include "opttree.h"

static struct threadRecord *startTR = NULL;
static char outputprefix[BUFFSZ];
static struct threadGlobal *globalThreadList = NULL;

struct threadRecord*
	mapThread(struct threadRecord *root, int tNum, char *fileName)
{
	if (root == NULL) {
		struct threadRecord *newThread =
			(struct threadRecord*)
				malloc( sizeof (struct threadRecord));
		newThread->number = tNum;
		strcpy(newThread->path, fileName);
		newThread->next = NULL;
		root = newThread;
		return root;
	} else 
		return mapThread(root->next, tNum, fileName);
}

void cleanThreadList(struct threadRecord *root)
{
	if (root == NULL)
		return;
	struct threadRecord *nextOne = root->next;
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
		struct threadRecord* endRecord = mapThread(startTR, threadID,
			threadPath);
		if (startTR == NULL)
			startTR = endRecord;
	} 
}

int startFirstThread(char* outputprefix)
{
	int errL, errG;
	struct threadLocal *firstThreadLocal;
	char threadname[BUFFSZ];
	struct threadResources *firstThreadResources;
	
	//start the first thread
	//first task is read the OPT string
	globalThreadList =
		(struct threadGlobal*)malloc(sizeof (struct threadGlobal));
	if (!globalThreadList) {
		fprintf(stderr,
			"Could not allocate memory for threadGlobal.\n");
		goto failed;
	}

	globalThreadList->globalTree = createPageTree();
	if (!globalThreadList->globalTree) {
		fprintf(stderr,
			"Could not create global tree");
		goto failGlobalTree;
	}

	firstThreadLocal =
		(struct threadLocal*)malloc(sizeof(struct threadLocal));
	if (!firstThreadLocal) {
		fprintf(stderr,
			"Could not allocate memory for threadLocal.\n");
		goto failFirstThreadLocal;
	}
	
	firstThreadLocal->instructionCount = 0;
	firstThreadLocal->localTree = createPageTree();
	if (!firstThreadLocal->localTree) {
		fprintf(stderr,
			"Could not initialise local tree.\n");
		goto failLocalTree;
	}

	firstThreadLocal->optTree = createOPTTree();
	if (!firstThreadLocal->optTree) {
		fprintf(stderr,
			"Could not initialise OPT tree.\n");
		goto failOPTTreeCreate;
	}

	firstThreadLocal->threadNumber = startTR->number;

	globalThreadList->head = firstThreadLocal;
	globalThreadList->tail = firstThreadLocal;
	sprintf(threadname, "%s%i.bin",outputprefix, startTR->number);
	readOPTTree(firstThreadLocal->optTree, threadname);

	//prepare to start the thread
	firstThreadResources =
		(struct threadResources*)malloc(sizeof (struct threadResources));
	if (!firstThreadResources) {
		fprintf(stderr,
			"Could not allocate memory for threadResources.\n");
		goto failResources;
	}
	firstThreadResources->records = startTR;
	firstThreadResources->globals = globalThreadList;
	firstThreadResources->local = firstThreadLocal;
	errL = pthread_mutex_init(&firstThreadLocal->threadLocalLock, NULL);
	errG = pthread_mutex_init(&globalThreadList->threadGlobalLock, NULL);
	if (errL || errG) {
		fprintf(stderr,
			"Mutex initialisation fails with %i and %i.\n",
			errL, errG);
		goto failMutex;
	}

	return 0;

failMutex:
	free(firstThreadResources);
failResources:
failOPTTreeCreate:
	removePageTree(firstThreadLocal->localTree);
failLocalTree:
	free(firstThreadLocal);
failFirstThreadLocal:
	removeOPTTree(firstThreadLocal->optTree);
failGlobalTree:
	free(globalThreadList);	
failed:
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
			printf("Error at column number %lu\n", XML_GetCurrentColumnNumber(p_ctrl));
			printf("Error at line number %lu\n", XML_GetCurrentLineNumber(p_ctrl));
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

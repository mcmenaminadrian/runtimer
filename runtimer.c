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

void mapThread(struct threadRecord *root, int tNum, char *fileName)
{
	if (root == NULL) {
		struct threadRecord *newThread =
			(struct threadRecord*)
				malloc( sizeof (struct threadRecord));
		newThread->number = tNum;
		strcpy(newThread->path, fileName);
		newThread->next = NULL;
		root = newThread;
	} else 
		mapThread(root->next, tNum, fileName);
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
	int threadID;
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
		mapThread(startTR, threadID, threadPath);
	}
	//start the first thread
	//first task is read the OPT string
	globalThreadList =
		(struct threadGlobal*)malloc(sizeof (struct threadGlobal));
	if (!globalThreadList) {
		fprintf(stderr,
			"Could not allocate memory for threadGlobal.\n");
		return;
	}
	struct threadLocal *firstThreadLocal =
		(struct threadLocal*)malloc(sizeof(struct threadLocal));
	if (!firstThreadLocal) {
		fprintf(stderr,
			"Could not allocate memory for threadLocal.\n");
		free(globalThreadList);
		return;
	}
	firstThreadLocal->localTree = createPageTree();
	if (!firstThreadLocal->localTree) {
		fprintf(stderr,
			"Could not initialise local tree.\n");
		free(firstThreadLocal);
		free(globalThreadList);
		return;
	}
	firstThreadLocal->optTree = createOPTTree();
	if (!firstThreadLocal->optTree) {
		fprintf(stderr,
			"Could not initialise OPT tree.\n");
		removePageTree(firstThreadLocal->localTree);
		free(firstThreadLocal);
		free(globalThreadList);
		return;
	}
	firstThreadLocal->threadNumber = startTR->number;
	int err = pthread_mutex_init(&firstThreadLocal->threadLocalLock, NULL);
	if (err) {
		fprintf(stderr, "Mutex initialisation fails with %i.\n", err);
		removeOPTTrree(firstThreadLocal->optTree);
		removePageTree(firstThreadLocal->localTree);
		free(firstThreadLocal);
		free(globalThreadList);
		return;
	}
	globalThreadList->head = firstThreadLocal;
	globalThreadList->tail = firstThreadLocal;
	readOPTTree(firstThreadLocal->optTree, startTR->path); 
	
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
		}
	} while(!done);

	XML_ParserFree(p_ctrl);
	fclose(inXML);
	cleanThreadList(startTR);
	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <expat.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include "threadhandler.h"
#include "pages.h"
#include "insttree.h"

//C code that parses a thread

struct PageToReplace{
	long pageNumber;
	long instructionCount;
};

static void replacePage(long pageNumber, struct ThreadResources *thResources)
{
	//find the page with the longest reuse distance,
	//otherwise find the page with the oldest date
	//either for this thread or all threads
	struct PageChain *currentChain = getPageChain(thResources->localTree);	
	void* instructionTree = createInstructionTree()
	while (currentChain) {
		long instructionNext =
			nextInChain(currentChain->page,
			thResources->local->instructionCount,
			thResources->optTree);
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
	do {
		int pageFound = 0;
		long maximumReuse = maxNode(instructionTree);
		if (maximumReuse) {
			//lock the global tree hard
			pthread_mutex_lock(
				&thResources->globals->threadGlobalLock);
			if (localPageTreePR(maximumReuse,
				thResources->globals->globalTree)) {
				pageFound = 1;
				removeFromPageTree(maximumReuse,
					thResources->globals->globalTree);
			}
			pthread_mutex_unlock(
				&thResources->globals->threadGlobalLock);
		} else {
			break;
		}
	} while (pageFound == 0);

	//if we didn't find the page, kill the oldest page
	if (pageFound == 0) {
		pthread_mutex_lock(&thResources->globals->threadGlobalLock);
		removeOldestFromPageTree(thResources->globals->globalTree);
		pThread_mutex_unlock(&thResources->globals->threadGlobalLock);
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
		thResources->local->instructionCount++;
		countDown--;
	}
	thResources->local->faultCount++;
	return 1;
}

static void XMLCALL
threadXMLProcessor(void* data, const XML_Char *name, const XML_Char **attr)
{
	int i;
	long address;
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
				//have address - is it already present?
				pthread_mutex_lock(&globals->threadGlobalLock);
				if (locatePageTreePR(address >> BITSHIFT,
						globals->globalTree)) {
					updateLRU(address >> BITSHIFT,
						now,
						globals->globalTree);
					pthread_mutex_unlock(
						&globals->threadGlobalLock);
					if (locatePageTreePR(
						address >> BITSHIFT,
						local->localTree)) {
						updateLRU(
							address >> BITSHIFT,
							now, local->localTree);
					} else {
						insertIntoPageTree(
						address >> BITSHIFT,
						now, local->localTree);
					}
					local->instructionCount++;
				} else {
					pthread_mutex_unlock(
					&globals->threadGlobalLock);
					if (faultPage(address >> BITSHIFT,
						thResources)) {
						pthread_mutex_lock(
						&globals->threadGlobalLock);
						//have to add the page
						int howMany = countPageTree(
							globals->globalTree);
						if (howMany >= CORES * COREMEM)
						{
						replacePage(
							address >> BITSHIFT,
							thResources);
						pthread_mutex_unlock(&globals->
							threadGlobalLock);
						} else {
						insertIntoPageTree(
							address >> BITSHIFT,
							now,
							globals->globalTree);
						pthread_mutex_unlock(&globals->
							threadGlobalLock);
						insertIntoPageTree(
							address >>BITSHIFT,
							now, local->localTree);
						}
					}
				}
			}
		}
	}
}

						

void* startThreadHandler(void *resources)
{
	struct ThreadResources *thResources;
	thResources = (struct threadResources*)resources;
	printf("Setting up parser for thread %i\n",
		thResources->local->threadNumber);
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
	return NULL;
}

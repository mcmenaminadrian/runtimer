#include <iostream>
#include <ctime>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <cstdio>
#include <set>
#include "pthread.h"
#include "threadhandler.h"

using namespace std;


void 
buildPageChain(struct PageChain** headChain,
	struct PageChain** activeChain, set<long>* tree)
{
	for (set<long>::iterator it = tree->begin(); it != tree->end(); ++it)
	{
		struct PageChain *nextChain =
			(struct PageChain*) malloc(sizeof(struct PageChain));
		nextChain->next = NULL;
		nextChain->page = *it;
		if (*activeChain == NULL) { //first in chain
			*activeChain = nextChain;
			*headChain = *activeChain;
		} else {
			(*activeChain)->next = nextChain;
			*activeChain = nextChain;
		}
	}
}

extern "C" {

void* createPageTree(void)
{
	return static_cast<void*>(new set<long>());
}

void removePageTree(void* tree)
{
	set<long>* prTree;
	prTree = static_cast<set<long> *>(tree);
	delete prTree;
}

void insertIntoPageTree(long pageNumber, void* tree)
{
	set<long> *prTree;
	prTree = static_cast<set<long> *>(tree);
	prTree->insert(pageNumber);
}

long locatePageTreePR(long pageNumber, void* tree)
{
	set<long>* prTree;
	prTree = static_cast<set<long> *>(tree);
	set<long>::iterator it;
	it = prTree->find(pageNumber);
	if (it != prTree->end()) {
		return *it;
	} else {
		return -1;
	}
}

void removeFromPageTree(long pageNumber, void* tree)
{
	set<long>* prTree = static_cast<set<long> *>(tree);
	prTree->erase(pageNumber);
}

int countPageTree(void* tree)
{
	set<long> *prTree;
	prTree = static_cast<set<long> *>(tree);
	return prTree->size();
}

struct PageChain* getPageChain(void *tree)
{
	set<long> *prTree;
	struct PageChain* activeChain = NULL;
	struct PageChain* headChain = NULL;
	prTree = static_cast<set<long> *>(tree);
	buildPageChain(&headChain, &activeChain, prTree);
	return headChain;
}

void cleanPageChain(struct PageChain* inChain)
{
	if (inChain == NULL){
		return;
	}
	struct PageChain* nextChain = inChain->next;
	delete inChain;
	cleanPageChain(nextChain);
}

}// end extern "C"		

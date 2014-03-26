#include <iostream>
#include <ctime>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <cstdio>
#include <set>
#include "pthread.h"
#include "threadhandler.h"
#include "opttree.h"
#include "pages.h"

using namespace std;


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
		return 0;
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

void
fillInstructionTree(void* global, void* iTree, void* oTree, long instruction);
{
	set<long>* prTree;
	prTree = static_cast<set<long> *>(global);
	set<long>::iterator it;
	for (it = prTree->begin(); it != prTree->end(); it++) {
		insertIntoTree(*it,
			findNextInstruction(instruction, *it, oTree),
			iTree);
	}
}

}// end extern "C"		

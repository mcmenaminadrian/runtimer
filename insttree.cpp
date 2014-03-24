#include <iostream>
#include <cstdio>
#include <map>

using namespace std;

//build a tree ordered by next instruction

extern "C" {

void* createInstructionTree(void)
{
	map<long, long>* instTree = new map<long, long>;
	return static_cast<void *>(instTree);
}

void insertIntoTree(long pageNumber, long instruction, void* tree)
{
	map<long, long>* instTree;
	instTree = static_cast<map<long, long>*>(tree);
	instTree->insert(pair<long, long>(instruction, pageNumber));
}

void freeInstTree(void* tree)
{
	map<long, long>* instTree;
	instTree = static_cast<map<long, long>*>(tree);
	delete instTree;
}

void* createMinTree(void)
{
	map<long, long>* minTree = new map<long, long>();
	return static_cast<void *>(minTree);
}

void killMinTree(void* tree)
{
	map<long, long>* minTree = static_cast<map<long, long>*>(tree);
	delete minTree;
}

void pushToMinTree(void* mTree, void* iTree)
{
	map<long, long>* minTree;
	map<long, long>* instTree;
	minTree = static_cast<map<long, long>*>(mTree);
	instTree = static_cast<map<long, long>*>(iTree);
	map<long, long>::iterator itInst;
	itInst = iTree->begin();
	map<long, long>::iterator itMin = minTree->find(itInst->second);
	if (itMin == minTree->end()) {
		minTree->insert(pair<long, long>(itInst->second,
			itInst->first));
	} else {
		if (itMin->second > itInst->first) {
			itMin->second = itInst->first;
		}
	}
}

long getPageToKill(void* tree)
{
	//find the page with the greatest distance
	long maxDistance = 0;
	long maxPage = 0;
	map<long, long>* minTree = static_cast<map<long, long>*>(tree);
	map<long, long>::iterator it;
	for (it = minTree->begin(); it != minTree->end(); it++) {
		if (it->second > maxDistance) {
			maxDistance = it->second;
			maxPage = it->first;
		}
	}
	return maxPage;
}

};  //end extern "C"		



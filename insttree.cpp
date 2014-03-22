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
	map<long, long>::iterator it;
	it = instTree->find(pageNumber);
	if (it != instTree->end()) {
		instTree->at(pageNumber) = instruction;
	} else {
		instTree->insert(pair<long, long>(pageNumber, instruction));
	}
}

long maxNodePage(void* tree)
{
	map<long, long>* instTree;
	instTree = static_cast<map<long, long>*>(tree);
	map<long, long>::reverse_iterator rit = instTree->rbegin();
	return rit->first;
}

long maxNodeDistance(void* tree)
{
	map<long, long>* instTree;
	instTree = static_cast<map<long, long>*>(tree);
	map<long, long>::reverse_iterator rit = instTree->rbegin();
	return rit->second;
}

long closestPage(void* tree)
{
	map<long, long>* instTree;
	instTree = static_cast<map<long, long>*>(tree);
	return instTree->begin()->first;
}

void freeInstTree(void* tree)
{
	map<long, long>* instTree;
	instTree = static_cast<map<long, long>*> (tree);
	delete instTree;
}
		
};  //end extern "C"		



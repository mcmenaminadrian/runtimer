#include <iostream>
#include <cstdlib>
#include <climits>
#include <fstream>
#include <set>
#include <map>
#include "redblack.hpp"

using namespace std;


extern "C" {

void* createOPTTree(void)
{
	map<long, set<unsigned long> >* optTree =
		new map<long, set<unsigned long> >();
	return static_cast<void *>(optTree);
}

void readOPTTree(void *tree, char *path)
{

	
	int longLength = sizeof(unsigned long);
	unsigned long nextInstructionRead, pageNumberRead;
	char* buffIn = new char[longLength];

	map<long, set<unsigned long> >* optTree =
		static_cast<map<long, set<unsigned long> >*>(tree);
	ifstream inFile(path, ifstream::binary);
	while (inFile && inFile.eof() == false) {
		inFile.read(buffIn, longLength);
		pageNumberRead = *((unsigned long*)buffIn);
		set<unsigned long> pageSet();
		do {
			inFile.read(buffIn, longLength);
			nextInstructionRead = *((unsigned long*)buffIn);
			if (nextInstructionRead != 0) {
				pageSet.insert(nextInstructionRead);
			}
		} while (nextInstructionRead != 0);
		optTree.insert(pair<long, set<unsigned long> >
			(pageNumberRead, pageSet));
	}
	delete[] buffIn;
}

long
findNextInstruction(unsigned long currentInstruction, long pageNumber,
	void* tree)
{
	map<long, set<unsigned long> >* optTree;
	map<long, set<unsigned long> >::iterator it;

	optTree = static_cast<map<long, set<unsigned long> >*>(tree);
	it = optTree->find(pageNumber);
	if (it == optTree->end()) {
		return -1;
	}

	set<unsigned long> setFound = it->second;
	set<unsigned long>::iterator setIT;
	setIT = setFound.upper_bound(pageNumber);
	if (setIT == setFound.end()) {
		return -1;
	}
	return *setIT;	
}


long nextInChain(long pageNumber, long instructionCount, void* tree)
{
	redblacktree<redblacknode<OPTTreeNode> >* optTree;
	OPTTreeNode findOPT(pageNumber);
	redblacknode<OPTTreeNode> findNode(findOPT);
	optTree = static_cast<redblacktree<redblacknode<OPTTreeNode> >*>(tree);
	//find the node with the pageNumber
	redblacknode<OPTTreeNode>* found = optTree->locatenode(&findNode,
		optTree->root);
	if (!found)
		return -1;
	OPTTreeNode v = found->getvalue();
	InstructionChain *vChain = v.getHead(); 	
	return findNextInstruction(instructionCount, vChain);
}

void removeOPTTree(void* tree)
{
	redblacktree<redblacknode<OPTTreeNode> >* optTree;
	optTree = static_cast<map<long, set<unsigned long> >*>(tree);
	delete optTree;
}	
	
} //end extern "C"

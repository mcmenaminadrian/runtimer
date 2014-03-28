#include <iostream>
#include <cstdlib>
#include <climits>
#include <fstream>
#include <set>
#include <map>

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
		set<unsigned long> pageSet;
		optTree->insert(pair<long, set<unsigned long> >
			(pageNumberRead, pageSet));
		map<long, set<unsigned long> >::iterator it;
		it = optTree->find(pageNumber);
		do {
			inFile.read(buffIn, longLength);
			nextInstructionRead = *((unsigned long*)buffIn);
			if (nextInstructionRead != 0) {
				(it->second).insert(nextInstructionRead);
			}
		} while (nextInstructionRead != 0);
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
		return LONG_MAX;
	}

	set<unsigned long>::iterator setIT;
	setIT = (it->second).upper_bound(currentInstruction);
	if (setIT == (it->second).end()) {
		optTree->erase(it);
		return LONG_MAX;
	}
	//return the distance
	return (*setIT - currentInstruction);	
}	

void removeOPTTree(void* tree)
{
	map<long, set<unsigned long> >* optTree;
	optTree = static_cast<map<long, set<unsigned long> >*>(tree);
	delete optTree;
}	
	
} //end extern "C"

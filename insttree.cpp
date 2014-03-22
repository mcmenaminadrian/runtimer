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

//item should only be inserted into tree if it reuse distance greater 
//than current instruction number and less than existing value
void insertIntoTree(const long& pageNumber, const long& instruction,
	const long& curInstruction, void* tree)
{
	if (instruction < curInstruction) {
		return;
	}
	map<long, long>* instTree;
	instTree = static_cast<map<long, long>*>(tree);
	map<long, long>::iterator it;
	it = instTree.find(pageNumber);
	if (it != instTree->end() && it->second > instruction) {
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
	return instTree.begin()->first;
}

void freeInstTree(void* tree)
{
	map<long, long>* instTree;
	instTree = static_cast<map<long, long>*> (tree);
	delete instTree;
}
		
};  //end extern "C"		



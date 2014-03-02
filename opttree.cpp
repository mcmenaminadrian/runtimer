#include <iostream>
#include <cstdlib>
#include <fstream>
#include "redblack.hpp"

using namespace std;

class InstructionChain
{
	private:
	unsigned long instruction;
	InstructionChain* next;

	public:
	InstructionChain(unsigned long inst);
	~InstructionChain();
	const unsigned long getInstruction(void) const;
	InstructionChain* getNext(void) const;
	InstructionChain* setNext(InstructionChain* next);
};

InstructionChain::InstructionChain(unsigned long inst)
{
	instruction = inst;
	next = NULL;
}

void killInstructionChain(InstructionChain* iChain)
{
	if (iChain == NULL)
		return;
	InstructionChain *nextIC = iChain->getNext();
	delete iChain;
	killInstructionChain(nextIC);
}

InstructionChain::~InstructionChain()
{
	killInstructionChain(getNext());
}

const unsigned long InstructionChain::getInstruction(void) const
{
	return instruction;
}

InstructionChain* InstructionChain::getNext(void) const
{
	return next;
}

InstructionChain* InstructionChain::setNext(InstructionChain *nx)
{
	next = nx;
	return next;
}
	

class OPTTreeNode {

	private:
	unsigned long page;
	InstructionChain* head;

	public:
	OPTTreeNode(unsigned long where);
	const long getPage(void) const;
	bool operator==(OPTTreeNode&) const;
	bool operator<(OPTTreeNode&) const;
	InstructionChain* getHead(void) const;
	void setHead(InstructionChain* newhead);
	InstructionChain*
		pushToEnd(InstructionChain* s, InstructionChain* a);
};

OPTTreeNode::OPTTreeNode(unsigned long where)
{
	page = where;
	head = NULL;
}

const long OPTTreeNode::getPage() const
{
	return page;
}

bool OPTTreeNode::operator==(OPTTreeNode& opt) const	
{
	return (page == opt.page);
}

bool OPTTreeNode::operator<(OPTTreeNode& opt) const
{
	return (page < opt.page);
}

InstructionChain* OPTTreeNode::getHead(void) const
{
	return head;
}

void OPTTreeNode::setHead(InstructionChain* ic)
{
	head = ic;
}

InstructionChain*
	OPTTreeNode::pushToEnd(InstructionChain* start, InstructionChain* add)
{
	if (head == NULL) {
		head = add;
		return head;
	}
	if (start->getNext() == NULL) {
		start->setNext(add);
		return start->getNext();
	} else
		return pushToEnd(start->getNext(), add);
}


void cleanOPTTree(redblacknode<OPTTreeNode>* node)
{
	if (node == NULL)
		return;
	cleanOPTTree(node->left);
	cleanOPTTree(node->right);
	delete node->getvalue().getHead();
}

struct pair *pairChainInOrder(struct pair *chain,
	<redblacknode<OPTTreeNode>* node)
{
	if (node == NULL) {
		return chain;
	}
	pairChainInOrder(chain, node);
	if (chain == NULL) {
		chain = (struct pair*)(malloc(sizeof(struct pair)));
		chain->instruction = 

extern "C" {

void* createOPTTree(void)
{
	redblacktree<redblacknode<OPTTreeNode> >* optTree;
	optTree = new redblacktree<redblacknode<OPTTreeNode> >();
	return static_cast<void *>(optTree);
}

void readOPTTree(void *tree, char *path)
{
	int longLength = sizeof(unsigned long);
	unsigned long nextInstructionRead, pageNumberRead;
	char* buffIn = new char[longLength];
	redblacktree<redblacknode<OPTTreeNode> >* optRBTree;
	optRBTree =
		static_cast<redblacktree<redblacknode<OPTTreeNode> >*>(tree);

	ifstream inFile(path, ifstream::binary);
	while (inFile && inFile.eof() == false) {
		inFile.read(buffIn, longLength);
		pageNumberRead = *((unsigned long*)buffIn);	
		OPTTreeNode nextPage(pageNumberRead);
		InstructionChain* addPoint = nextPage.getHead();
		do {
			inFile.read(buffIn, longLength);
			nextInstructionRead = *((unsigned long*)buffIn); 
			if (nextInstructionRead != 0) {
				InstructionChain* nextLink =
				new InstructionChain(nextInstructionRead);
				addPoint =
					nextPage.pushToEnd(addPoint, nextLink);
			}
		} while (nextInstructionRead != 0);
		redblacknode<OPTTreeNode> rbOPTNode(nextPage);
		optRBTree->insertnode(&rbOPTNode, optRBTree->root);
	}
	delete[] buffIn;
}

long findNextInstruction(long currentInstruction, InstructionChain* chain)
{
	if (chain == NULL) {
		return LONG_MAX;
	}
	else {
		if (chain->getInstruction() < currentInstruction) {
			return findNextInstruction(currentInstruction,
				chain->getNext())
		} else {
			return chain->getInstruction();
	}
}


long nextInChain(long pageNumber, long instructionCount, void* tree)
{
	redblacktree<redblackmode<OPTTreeNode> >* optTree;
	redblacknode<OPTTreeNode> findNode(pageNumber);
	optTree = static_cast<redblacktree<redblacknode<OPTTreeNode> >*>(tree);
	//find the node with the pageNumber
	redblacknode<OPTTreeNode>* found = optTree->locateNode(&findNode,
		optTree->root)
	if (!found)
		return -1;
	OPTTreeNode v = found->getValue();
	InstructionChain *vChain = v->getHead(); 	
	return findNextInstruction(instructionCount, InstructionChain* chain);
}

void removeOPTTree(void* tree)
{
	redblacktree<redblacknode<OPTTreeNode> >* optTree;
	optTree = static_cast<redblacktree<redblacknode<OPTTreeNode> >*>(tree);
	cleanOPTTree(optTree->root);
}	
	

} //end extern "C"

				


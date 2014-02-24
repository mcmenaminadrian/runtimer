#include <iostream>
#include <cstdlib>
#include <fstream>
#include "redblack.hpp"

using namespace std;

class InstructionChain
{
	private:
	long instruction;
	InstructionChain* next;

	public:
	InstructionChain(long inst);
	~InstructionChain();
	const long getInstruction(void) const;
	InstructionChain* getNext(void) const;
	InstructionChain* setNext(InstructionChain* next);
};

InstructionChain::InstructionChain(long inst)
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

const long InstructionChain::getInstruction(void) const
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
	

class OPTTree {

	private:
	long page;
	InstructionChain* head;

	public:
	OPTTree(long where);
	const long getPage(void) const;
	bool operator==(OPTTree&) const;
	bool operator<(OPTTree&) const;
	InstructionChain* getHead(void) const;
	void setHead(InstructionChain* newhead);
	InstructionChain*
		pushToEnd(InstructionChain* s, InstructionChain* a);
};

OPTTree::OPTTree(long where)
{
	page = where;
	head = NULL;
}

const long OPTTree::getPage() const
{
	return page;
}

bool OPTTree::operator==(OPTTree& opt) const	
{
	return (page == opt.page);
}

bool OPTTree::operator<(OPTTree& opt) const
{
	return (page < opt.page);
}

InstructionChain* OPTTree::getHead(void) const
{
	return head;
}

void OPTTree::setHead(InstructionChain* ic)
{
	head = ic;
}

InstructionChain*
	OPTTree::pushToEnd(InstructionChain* start, InstructionChain* add)
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


void cleanOPTTree(redblacknode<OPTTree>* node)
{
	if (node == NULL)
		return;
	cleanOPTTree(node->left);
	cleanOPTTree(node->right);
	delete node->getvalue().getHead();
}


extern "C" {

void* createOPTTree(void)
{
	redblacktree<redblacknode<OPTTree> >* optTree;
	optTree = new redblacktree<redblacknode<OPTTree> >();
	return static_cast<void *>(optTree);
}

void readOPTTree(void *tree, char *path)
{
	int longLength = sizeof(long);
	char buff[longLength];

	redblacktree<redblacknode<OPTTree> >* optRBTree;
	optRBTree = static_cast<redblacktree<redblacknode<OPTTree> >*>(tree);

	ifstream inFile(path, ios_base::binary);
	while (inFile.eof() == false) {
		long nextInstructionRead, pageNumberRead;
		inFile.read(buff, longLength);
		pageNumberRead = atol(buff);
		OPTTree nextPage(pageNumberRead);
		InstructionChain* addPoint = nextPage.getHead();
		do {
			inFile.read(buff, longLength);
			nextInstructionRead = atol(buff);
			if (nextInstructionRead > 0) {
				InstructionChain* nextLink =
				new InstructionChain(nextInstructionRead);
				addPoint =
					nextPage.pushToEnd(addPoint, nextLink);
			}
		} while (nextInstructionRead > 0);
		redblacknode<OPTTree> rbOPTNode(nextPage);
		optRBTree->insertnode(&rbOPTNode, optRBTree->root);
	}
}

void removeOPTTree(void* tree)
{
	redblacktree<redblacknode<OPTTree> >* optTree;
	optTree = static_cast<redblacktree<redblacknode<OPTTree> >*>(tree);
	cleanOPTTree(optTree->root);
}	
	

} //end extern "C"

				


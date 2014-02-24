#include <iostream>
#include "redblack.hpp"

using namespace std;

class InstructionChain
{
	private:
	long instruction;
	InstructionChain* next;

	public:
	InstructionChain(long inst);
	const long getInstruction(void) const;
	const InstructionChain* getNext(void) const;
	const InstructionChain* setNext(InstructionChain* next);
};

InstructionChain::InstructionChain(long inst)
{
	instruction = inst;
	next = NULL;
}

const long InstructionChain::getInstruction(void) const
{
	return instruction;
}

const InstructionChain* InstructionChain::getNext(void) const
{
	return next;
}

const InstructionChain* InstructionChain::setNext(InstructionChain *nx)
{
	next = nx;
	return next;
}
	

class OPTTree {

	private:
	long page;
	InstructionChain* head;
	void killChain(InstructionChain* nextItem);

	public:
	OPTTree(long where);
	~OPTTree();
	const long getPage(void) const;
	bool operator==(OPTTree&) const;
	bool operator<(OPTTree&) const;
	const InstructionChain* getHead(void) const;
	void setHead(InstructionChain* newhead);
	InstructionChain* pushToEnd(InstructionChain* nextIC);
};

OPTTree::OPTTree(long where)
{
	page = where;
	head = new InstructionChain();
}

void OPTTree::killChain(InstructionChain* next)
{
	if (next == NULL)
		return;
	InstructionChain* nextHead = head->getNext();
	delete head;
	killChain(nextHead);
}

OPTTree::~OPTTree()
{
	killChain(head);
}

const long OPTTree::getPage() const
{
	return page;
}

bool OPTTree::operator==(OPTTree& opt) const	
{
	return (page == opt.page);
}

bool OPTTree::operator(OPTTree& opt) const
{
	return (page < opt.page);
}

const InstructionChain* OPTTree::getHead(void) const
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
		head = nextIC;
		return head;
	}
	if (start->getNext() == NULL) {
		start->setNext(add);
		return start->getNext();
	} else
		pushToEnd(start->getNext(), add);
}
		


void cleanOPTTree(redblacknode<OPTTree>* node)
{
	if (node == NULL)
		return;
	cleanOPTree(node->left);
	cleanOPTTree(node->right);
	node->getvalue();
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
	optTree = static_cast<redblacktree<redblacknode<OPTTree> >*>(tree);

	ifstream inFile(path, ios::binary|ios::in);
	while (inFile.eof() == false) {
		inFile.read(buff, longLength);
		long pageNumberRead = atol(buff);
		OPTTree nextPage(pageNumberRead);
		InstructionChain* addPoint = head;
		long nextInstructionRead;
		do {
			inFile.read(buff, longLength);
			nextInstructionRead = atol(buff);
			if (nextInstructionRead > 0) {
				InstructionChain* nextLink =
				new InstructionChain(nextInstructionRead);
				addPoint = pushToEnd(addPoint, nextLink);
			}
		} while (nextInstructionRead > 0);
		redblacknode<OPTTree> rbOPTNode =
			new redblacknode<OPTTree>(nextPage);
		optRBTree->insertnode(rbOPTNode, optRBTree->root);
	}
}

void removeOPTTree(void* tree)
{
	redblacktree<redblacknode<OPTTree> >* optTree;
	optTree = static_cast<redblacktree<redblacknode<OPTTree> >*>(tree);
	cleanOPTTree(optTree->root);
}	
	

} //end extern "C"

				


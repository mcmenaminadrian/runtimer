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
	

class OPTTreeNode {

	private:
	long page;
	InstructionChain* head;

	public:
	OPTTreeNode(long where);
	const long getPage(void) const;
	bool operator==(OPTTreeNode&) const;
	bool operator<(OPTTreeNode&) const;
	InstructionChain* getHead(void) const;
	void setHead(InstructionChain* newhead);
	InstructionChain*
		pushToEnd(InstructionChain* s, InstructionChain* a);
};

OPTTreeNode::OPTTreeNode(long where)
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

extern "C" {

void* createOPTTree(void)
{
	redblacktree<redblacknode<OPTTreeNode> >* optTree;
	optTree = new redblacktree<redblacknode<OPTTreeNode> >();
	return static_cast<void *>(optTree);
}

void readOPTTree(void *tree, char *path)
{
	int longLength = sizeof(long);
	char buff[longLength];
	printf("Longlength: %i path:%s\n", longLength, path);
	redblacktree<redblacknode<OPTTreeNode> >* optRBTree;
	optRBTree =
		static_cast<redblacktree<redblacknode<OPTTreeNode> >*>(tree);

	ifstream inFile(path, ifstream::binary);
	while (inFile && inFile.eof() == false) {
		long nextInstructionRead, pageNumberRead;
		inFile.read(buff, longLength);
		pageNumberRead = atol(buff);
		OPTTreeNode nextPage(pageNumberRead);
		InstructionChain* addPoint = nextPage.getHead();
		do {
			inFile.read(buff, longLength);
			nextInstructionRead = atol(buff); printf(" %li ",nextInstructionRead);
			if (nextInstructionRead > 0) {
				InstructionChain* nextLink =
				new InstructionChain(nextInstructionRead);
				addPoint =
					nextPage.pushToEnd(addPoint, nextLink);
			}
		} while (nextInstructionRead > 0);
		redblacknode<OPTTreeNode> rbOPTNode(nextPage);
		optRBTree->insertnode(&rbOPTNode, optRBTree->root);
		printf("Inserted node for page %li\n", pageNumberRead);
	}
}

void removeOPTTree(void* tree)
{
	redblacktree<redblacknode<OPTTreeNode> >* optTree;
	optTree = static_cast<redblacktree<redblacknode<OPTTreeNode> >*>(tree);
	cleanOPTTree(optTree->root);
}	
	

} //end extern "C"

				


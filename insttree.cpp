#include <iostream>
#include "redblack.hpp"

using namespace std;

//build a tree ordered by next instruction

class instructionOrder
{
	private:
	const unsigned long instruction;
	const long pageNumber;

	public:
	InstructionOrder(const unsigned long i, const long p);
	bool operator==(InstructionOrder&) const;
	bool operator<(InstructionOrder&) const;
	const unsigned long getInstruction() const;
	const long getPageNumber() const;
};

InstructionOrder::InstructionOrder(const unsigned long i, const long p)
{
	instruction = i;
	pageNumber = p;
}

bool InstructionOrder::operator==(InstructionOrder& iO) const
{
	return (instruction == io.instruction);
}

bool InstructionOrder::operator<(InstructionOrder& iO) const
{
	return (pageNumber < io.pageNumber);
}

const unsigned long InstructionOrder::getInstruction const
{
	return instruction;
}

const long InstructionOrder::getPageNumber() const
{
	return pageNumber;
}

extern "C" {

void* createInstructionTree(void)
{
	redblacktree<redblacknode<InstructionOrder> >* instTree;
	instTree = new redblacktree<redblacknode<InstructionOrder> >();
	return static_cast<void *> instTree;
}

void insertIntoTree(long pageNumber, unsigned long instruction, void* tree)
{
	redblacktree<redblacknode<InstructionOrder> >* instTree;
	instTree =
		static_cast<redblacktree<redblacknode<InstructionOrder>* >
		(tree);
	InstructionOrder instOrder(instruction, pageNumber);
	redblacknode<InstructionOrder> instOrderNode(instOrder);
	instTree->insertnode(&instOrderNode, instTree->root);
}

long maxNode(void* tree)
{
	redblacktree<redblacknode<InstructionOrder> >* instTree;
	redblacknode<InstructionOrder> *farNode;
	instTree =
		static_cast<redblacktree<redblacknode<InstructionOrder>* >
		(tree);
	farNode = instTree->max();
	//allow repeated calls of maxNode
	if (farNode) {
		InstructionOrder iO = farNode->getValue();
		redblacknode<instructionOrder>
			delNode(iO->getInstruction(), iO->getPageNumber());
		instTree->removenode(delNode);
		return iO->getPageNumber();
	} else {
		return 0;
	}
}

void freeInstTree(void* tree)
{
	redblacktree<redblacknode<InstructionOrder> >* instTree;
	instTree =
		static_cast<redblacktree<redblacknode<InstructionOrder>* >
		(tree);
	delete instTree;
}
		
};  //end extern "C"		



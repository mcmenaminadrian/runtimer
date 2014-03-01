//header for instTree C interface

void* createInstructionTree(void);
void insertIntoTree(long pN, unsigned long inst, void* tree);
long maxNode(void* tree);

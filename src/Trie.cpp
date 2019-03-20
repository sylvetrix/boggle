#include <cstring>

#include "Trie.h"

#define LEAF_BIT (0x1 << 31)
#define MASK_A (0x1 << 25)

using std::ios;

TrieNode::~TrieNode() {
	for (int i = 0; i < 26; i++) {
		delete children[i];
	}
}

Trie::Trie() {
	root = NULL;
	clearTrie();
}

Trie::~Trie() {
	delete root;
}

TrieNode* Trie::getRoot() {
	return root;
}

void Trie::clearTrie() {
	delete root;
	root = createNode();
}

void Trie::insert(const char* key, int len) {
	TrieNode* child = root;

	for (int i = 0; i < len; i++) {
		int index = charToIndex(key[i]);

		if (child->children[index] == NULL) {
			child->children[index] = createNode();
		}
		child = child->children[index];
	}

	child->isLeaf = true;
}

TrieNode* Trie::createNode() {
	TrieNode* newNode = new TrieNode();
	for (int i = 0; i < 26; i++) {
		newNode->children[i] = NULL;
	}
	newNode->isLeaf = false;

	return newNode;
}

uint32_t Trie::nodeToUint32(TrieNode* node, LinkedTrieNode* & tail) {
	uint32_t output = 0;
	if (node == NULL) { return output; }

	if (node->isLeaf) { output |= LEAF_BIT; }
	uint32_t indexMask = MASK_A;
	printf("Node mask: '%d.....", node->isLeaf ? 1 : 0);
	for (int i = 0; i < 26; i++, indexMask >>= 1) {
		if (node->children[i] != NULL) {
			printf("%c", indexToChar(i)); 
			output |= indexMask;
			// add node to end of processing list
			tail->next = new LinkedTrieNode();
			tail->next->node = node->children[i];
			tail = tail->next;
			tail->next = NULL;
		} else {
			printf(".");
		}
	}
	printf("' (0x%08x)\n", output);

	return output;
}

void Trie::serialize(Trie& trie, string fileName) {
	int bufferInc = sizeof(uint32_t);
	int bufferMax = bufferInc * 32;
	int bufferSize = 0;
	char buffer[bufferMax] = {};
	ofstream file;
	file.open(fileName, ios::out | ios::binary | ios::trunc);

	// start at root node
	LinkedTrieNode* head, * tail, * tmp;
	uint32_t mask;
	head = new LinkedTrieNode();
	tail = head;
	head->node = trie.getRoot();

	while (head != NULL) {
		mask = nodeToUint32(head->node, tail);
		// write mask to char buffer
		std::memcpy(&buffer[bufferSize], &mask, bufferInc);
		bufferSize += bufferInc;
		// flush to file if needed
		if (bufferSize >= bufferMax) {
			file.write(buffer, bufferMax);
			bufferSize = 0;
		}

		// advance
		tmp = head;
		head = head->next;
		delete tmp;
	}

	delete tail;

	// flush remaining bytes if needed
	if (bufferSize > 0) {
		file.write(buffer, bufferSize);
	}

	file.close();
}

void Trie::uint32ToNode(uint32_t input, TrieNode* node, LinkedTrieNode* & tail) {
	node->isLeaf = input & LEAF_BIT;

	// return if there are no children
	if ((input & ~LEAF_BIT) == 0) { return; }

	uint32_t indexMask = MASK_A;
	printf("Node mask: '%d.....", node->isLeaf ? 1 : 0);
	for (int i = 0; i < 26; i++, indexMask >>= 1) {
		if (input & indexMask) {
			printf("%c", indexToChar(i)); 
			node->children[i] = new TrieNode();
			// add node to end of processing list
			tail->next = new LinkedTrieNode();
			tail = tail->next;
			tail->next = NULL;
		} else {
			printf(".");
		}
	}
	printf("' (0x%08x)\n", input);
}

bool Trie::deserialize(Trie& trie, string fileName) {
	int bufferInc = sizeof(uint32_t);
	int bufferMax = bufferInc * 32;
	int bufferSize = bufferMax;
	int bufferPos = bufferSize;
	char buffer[bufferMax] = {};
	ifstream file;
	file.open(fileName, ios::in | ios::binary);

	// discard any existing nodes and create a new root
	trie.clearTrie();

	LinkedTrieNode* head, * tail, * tmp;
	TrieNode* newRoot = NULL;
	uint32_t mask;
	head = new LinkedTrieNode();
	tail = head;
	head->node = trie.getRoot();

	while ((bufferPos < bufferSize) || !file.eof()) {
		if (head == NULL) {
			printf("Trie corrupt: file is longer than node list\n");
			//TODO: create trie from dictionary
			trie.clearTrie();
			return false;
		}

		if (bufferPos >= bufferSize) {
			file.read(buffer, bufferMax);
			if (!file) {
				bufferSize = file.gcount();
				file.clear();
			}
			bufferPos = 0;
		}
		
		std::memcpy(&mask, &buffer[bufferPos], bufferInc);
		bufferPos += bufferInc;
		uint32ToNode(mask, head->node, tail);

		// advance
		// calling delete will delete the TrieNode which will cause a segfault
		head = head->next;
	}

	if (head != NULL) {
		printf("Trie corrupt: node list is longer than file\n");
		//TODO: create trie from dictionary
		trie.clearTrie();
		return false;
	}

	return true;
}

bool Trie::trieCompare(Trie& trie) {
	int level = 0, nodesInLevel = 0, nodesInNextLevel = 0;
	TrieNode* currentHead, * currentStaticHead;
	LinkedTrieNode* head, * staticHead, * tail, * staticTail, * tmpHead;
	head = new LinkedTrieNode();
	head->node = root;
	head->next = NULL;
	tail = head;
	staticHead = new LinkedTrieNode();
	staticHead->node = trie.getRoot();
	staticHead->next = NULL;
	staticTail = staticHead;
	nodesInLevel = 1;

	while ((head != NULL) && (staticHead != NULL)) {
		printf("Processing level %d: '", 0);
		currentHead = head->node;
		currentStaticHead = staticHead->node;
		if (currentHead->isLeaf != currentStaticHead->isLeaf) {
			printf("currentHead->isLeaf = %s and currentStaticHead->isLeaf = %s\n",
				currentHead->isLeaf ? "TRUE" : "FALSE",
				currentStaticHead->isLeaf ? "TRUE" : "FALSE");
			return false;
		}

		printf("%d.....", currentHead->isLeaf ? 1 : 0);

		for (int i = 0; i < 26; i++) {
			if ((currentHead->children[i] == NULL) ^ (currentStaticHead->children[i] == NULL)) {
				printf("Only %s has child %c\n", currentHead->children[i] ? "currentHead" : "currentStaticHead", indexToChar(i));
				return false;
			}
			if (currentHead->children[i]) {
				// add child to processing list
				tail->next = new LinkedTrieNode();
				tail->next->node = currentHead->children[i];
				tail = tail->next;
				tail->next = NULL;

				staticTail->next = new LinkedTrieNode();
				staticTail->next->node = currentStaticHead->children[i];
				staticTail = staticTail->next;
				staticTail->next = NULL;

				nodesInNextLevel++;
				printf("%c", indexToChar(i));
			} else {
				printf(".");
			}
		}

		printf("\n");

		// advance processing lists
		tmpHead = head;
		head = head->next;
		delete tmpHead;

		tmpHead = staticHead;
		staticHead = staticHead->next;
		delete tmpHead;

		nodesInLevel--;
		if (nodesInLevel == 0) {
			level++;
			nodesInLevel = nodesInNextLevel;
			nodesInNextLevel = 0;
		}
	}

	return true;
}

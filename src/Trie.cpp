#include <cstring>

#include "Trie.h"

#define LEAF_BIT (1u << 31)
#define MASK_A (1u << 25)
#define BUFFERINC (sizeof(uint32_t))
#define BUFFERMAX (BUFFERINC * 32)

using std::ios;

TrieNode::TrieNode() {
	for (int i = 0; i < 26; i++) {
		children[i] = NULL;
	}
	isLeaf = false;
}

TrieNode::~TrieNode() {
	printf("Destructing TrieNode\n");
	for (int i = 0; i < 26; i++) {
		delete children[i];
		children[i] = NULL;
	}
}

Trie::Trie() {
	root = NULL;
	clearTrie();
}

Trie::~Trie() {
	delete root;
	root = NULL;
}

TrieNode* Trie::getRoot() {
	return root;
}

void Trie::clearTrie() {
	delete root;
	root = new TrieNode();
}

void Trie::insert(const char* key, int len) {
	TrieNode* child = root;

	for (int i = 0; i < len; i++) {
		int index = charToIndex(key[i]);

		if (child->children[index] == NULL) {
			child->children[index] = new TrieNode();
		}
		child = child->children[index];
	}

	child->isLeaf = true;
}


LinkedTrieNode* Trie::nodeToUint32(uint32_t& output, TrieNode* node, LinkedTrieNode* tail) {
	output = 0;
	if (node == NULL) { return tail; }

	if (node->isLeaf) { output |= LEAF_BIT; }
	uint32_t indexMask = MASK_A;
	printf("Node mask: '%d.....", node->isLeaf ? 1 : 0);
	for (int i = 0; i < 26; i++, indexMask >>= 1) {
		if (node->children[i] != NULL) {
			printf("%c", indexToChar(i)); 
			output |= indexMask;
			// add node to end of processing list
			tail->next = new LinkedTrieNode();
			tail = tail->next;
			tail->node = node->children[i];
			tail->next = NULL;
		} else {
			printf(".");
		}
	}
	printf("' (0x%08x)\n", output);

	return tail;
}

bool Trie::serialize(const char* fileName) {
	printf("Serializing to trie file '%s'.\n", fileName);
	int bufferSize = 0;
	char buffer[BUFFERMAX];
	ofstream file;
	file.open(fileName, ios::out | ios::binary | ios::trunc);

	if (!file.is_open()) {
		printf("Unable to open trie file for serialization.\n");
		return false;
	}

	// start at root node
	LinkedTrieNode* head, * tail, * tmp;
	uint32_t mask;
	head = new LinkedTrieNode();
	tail = head;
	head->node = root;

	while (head != NULL) {
		tail = nodeToUint32(mask, head->node, tail);
		// write mask to char buffer
		std::memcpy(&buffer[bufferSize], &mask, BUFFERINC);
		bufferSize += BUFFERINC;
		// flush to file if needed
		if (bufferSize >= BUFFERMAX) {
			file.write(buffer, BUFFERMAX);
			bufferSize = 0;
		}

		// advance
		tmp = head;
		head = head->next;
		delete tmp;
	}

	// flush remaining bytes if needed
	if (bufferSize > 0) {
		file.write(buffer, bufferSize);
	}

	file.close();

	return true;
}

LinkedTrieNode* Trie::uint32ToNode(uint32_t input, TrieNode* node, LinkedTrieNode* tail) {
	node->isLeaf = input & LEAF_BIT;
	printf("Node mask: '%d.....", node->isLeaf ? 1 : 0);

	// return if there are no children
	if ((input & ~LEAF_BIT) == 0) {
		printf("..........................' (0x%08x)\n", input);
		return tail;
	}

	uint32_t indexMask = MASK_A;
	for (int i = 0; i < 26; i++, indexMask >>= 1) {
		if (input & indexMask) {
			printf("%c", indexToChar(i)); 
			node->children[i] = new TrieNode();
			// add node to end of processing list
			tail->next = new LinkedTrieNode();
			tail = tail->next;
			tail->node = node->children[i];
			tail->next = NULL;
		} else {
			printf(".");
		}
	}
	printf("' (0x%08x)\n", input);

	return tail;
}

bool Trie::deserialize(const char* fileName) {
	printf("Deserializing from trie file '%s'.\n", fileName);
	int bufferSize = BUFFERMAX;
	int bufferPos = bufferSize;
	char buffer[BUFFERMAX] = {};
	ifstream file;

	// discard any existing nodes and create a new root
	clearTrie();

	file.open(fileName, ios::in | ios::ate | ios::binary);
	if (!file.is_open()) {
		printf("Unable to open trie file for deserialization.\n");
		return false;
	}
	if (file.tellg() == 0) {
		printf("Trie file is empty.\n");
		return false;
	}
	file.seekg(0);

	LinkedTrieNode* head, * tail, * tmp;
	TrieNode* newRoot = NULL;
	uint32_t mask;
	head = new LinkedTrieNode();
	tail = head;
	head->node = root;

	while ((bufferPos < bufferSize) || !file.eof()) {
		if (head == NULL) {
			printf("Trie corrupt: file is longer than node list.\n");
			clearTrie();
			return false;
		}

		if (bufferPos >= bufferSize) {
			file.read(buffer, BUFFERMAX);
			if (!file) {
				bufferSize = file.gcount();
			}
			bufferPos = 0;
		}
		
		std::memcpy(&mask, &buffer[bufferPos], BUFFERINC);
		bufferPos += BUFFERINC;
		tail = uint32ToNode(mask, head->node, tail);

		// advance
		tmp = head;
		head = head->next;
		delete tmp;
	}

	if (head != NULL) {
		printf("Trie corrupt: node list is longer than file.\n");

		// clean up remaining nodes in processing list
		do {
			tmp = head;
			head = head->next;
			delete tmp;
		} while (tmp != NULL);
		// clear partial trie
		clearTrie();

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
		printf("Processing level %d: '", level);
		currentHead = head->node;
		currentStaticHead = staticHead->node;
		if (currentHead->isLeaf != currentStaticHead->isLeaf) {
			printf("currentHead->isLeaf = %s and currentStaticHead->isLeaf = %s.\n",
				currentHead->isLeaf ? "TRUE" : "FALSE",
				currentStaticHead->isLeaf ? "TRUE" : "FALSE");

			// clean up processing lists
			do {
				tmpHead = head;
				head = head->next;
				delete tmpHead;
			} while (tmpHead != NULL);
			do {
				tmpHead = staticHead;
				staticHead = staticHead->next;
				delete tmpHead;
			} while (tmpHead != NULL);

			return false;
		}

		printf("%d.....", currentHead->isLeaf ? 1 : 0);

		for (int i = 0; i < 26; i++) {
			if ((currentHead->children[i] == NULL) ^ (currentStaticHead->children[i] == NULL)) {
				printf("Only %s has child %c.\n", currentHead->children[i] ? "currentHead" : "currentStaticHead", indexToChar(i));
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

		printf("'\n");

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

TrieInfo Trie::getTrieInfo() {
	TrieInfo info = TrieInfo();
	info.trieSize += sizeof(*this);
	if (root == NULL) { return info; }

	info += getTrieNodeInfo(root);

	return info;
}

TrieInfo Trie::getTrieNodeInfo(TrieNode* node) {
	TrieInfo info = TrieInfo();
	info.trieSize += sizeof(*node);
	if (node->isLeaf) { info.wordCount++; }

	for (int i = 0; i < 26; i++) {
		if (node->children[i] != NULL) {
			info.letterCount++;
			info += getTrieNodeInfo(node->children[i]);
		}
	}

	return info;
}

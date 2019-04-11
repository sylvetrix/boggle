#include <cstring>
#include <string>

#include "Logger.h"
#include "Trie.h"

#define LEAF_BIT (1u << 31)
#define MASK_A (1u << 25)
#define BUFFERINC (sizeof(uint32_t))
#define BUFFERMAX (BUFFERINC * 32)

using std::ios;

TrieNode::TrieNode() {
#if DEBUG
	id = createTrieNodeId();
	LOG_DEBUG("Constructing TrieNode [%lu]", this->id);
#endif
	for (int i = 0; i < 26; i++) {
		children[i] = NULL;
	}
	isLeaf = false;
}

TrieNode::~TrieNode() {
	LOG_DEBUG("Destructing TrieNode [%lu]", this->id);
	for (int i = 0; i < 26; i++) {
		delete children[i];
		children[i] = NULL;
	}
}

Trie::Trie() {
	root = new TrieNode();
}

Trie::~Trie() {
	delete root;
	root = NULL;
}

TrieNode* Trie::getRoot() {
	return root;
}

void Trie::clearTrie() {
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

#if DEBUG
	char logBuffer[59];
	snprintf(logBuffer, 59, "Node mask: 'L...............................' (0x00000000)");
	int cx = 12;	// "Node mask: '"
#endif

	if (node->isLeaf) { output |= LEAF_BIT; }
	uint32_t indexMask = MASK_A;
#if DEBUG
	logBuffer[cx] = node->isLeaf ? '1' : '0';
	cx += 6;	// leaf + '.....'
#endif
	for (int i = 0; i < 26; i++, indexMask >>= 1) {
		if (node->children[i] != NULL) {
#if DEBUG
			logBuffer[cx] = indexToChar(i);
#endif
			output |= indexMask;
			// add node to end of processing list
			tail->next = new LinkedTrieNode();
			tail = tail->next;
			tail->node = node->children[i];
			tail->next = NULL;
		}
#if DEBUG
		cx++;
	}

	cx += 5;	// "' (0x"
	snprintf(logBuffer + cx, 59 - cx, "%08x)", output);
	LOG_DEBUG("%s", logBuffer);
#else
	}
#endif

	return tail;
}

bool Trie::serialize(const char* fileName) {
	LOG_INFO("Serializing to trie file '%s'.", fileName);
	int bufferSize = 0;
	char buffer[BUFFERMAX];
	ofstream file;
	file.open(fileName, ios::out | ios::binary | ios::trunc);

	if (!file.is_open()) {
		LOG_INFO("Unable to open trie file for serialization.");
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
		tmp = NULL;
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

#if DEBUG
	char logBuffer[59];
	snprintf(logBuffer, 59, "Node mask: 'L...............................' (0x00000000)");
	int cx = 12;	// "Node mask: '"
	logBuffer[cx] = node->isLeaf ? '1' : '0';
	cx += 6;	// leaf + '.....'
#endif

	// return if there are no children
	if ((input & ~LEAF_BIT) == 0) {
#if DEBUG
		cx += 31;	// 26 chars + "' (0x"
		snprintf(logBuffer + cx, 59 - cx, "%08x)", input);
		LOG_DEBUG("%s", logBuffer);
#endif
		return tail;
	}

	uint32_t indexMask = MASK_A;
	for (int i = 0; i < 26; i++, indexMask >>= 1) {
		if (input & indexMask) {
#if DEBUG
			logBuffer[cx] = indexToChar(i);
#endif
			node->children[i] = new TrieNode();
			// add node to end of processing list
			tail->next = new LinkedTrieNode();
			tail = tail->next;
			tail->node = node->children[i];
			tail->next = NULL;
		}
#if DEBUG
		cx++;
	}

	cx += 5;	// "' (0x"
	snprintf(logBuffer + cx, 59 - cx, "%08x)", input);
	LOG_DEBUG("%s", logBuffer);
#else
	}
#endif

	return tail;
}

bool Trie::deserialize(const char* fileName) {
	LOG_INFO("Deserializing from trie file '%s'.", fileName);
	int bufferSize = BUFFERMAX;
	int bufferPos = bufferSize;
	char buffer[BUFFERMAX] = {};
	ifstream file;

	// discard any existing nodes and create a new root
	clearTrie();

	file.open(fileName, ios::in | ios::ate | ios::binary);
	if (!file.is_open()) {
		LOG_INFO("Unable to open trie file for deserialization.");
		return false;
	}
	if (file.tellg() == 0) {
		LOG_INFO("Trie file is empty.");
		return false;
	}
	file.seekg(0);

	LinkedTrieNode* head, * tail, * tmp;
	uint32_t mask;
	head = new LinkedTrieNode();
	tail = head;
	head->node = root;

	while ((bufferPos < bufferSize) || !file.eof()) {
		if (head == NULL) {
			LOG_INFO("Trie corrupt: file is longer than node list.");
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
		tmp = NULL;
	}

	if (head != NULL) {
		LOG_INFO("Trie corrupt: node list is longer than file.");

		// clean up remaining nodes in processing list
		do {
			tmp = head;
			head = head->next;
			delete tmp;
			tmp = NULL;
		} while (head != NULL);
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
		currentHead = head->node;
		currentStaticHead = staticHead->node;
		if (currentHead->isLeaf != currentStaticHead->isLeaf) {
			LOG_DEBUG("currentHead->isLeaf = %s and currentStaticHead->isLeaf = %s.",
				currentHead->isLeaf ? "TRUE" : "FALSE",
				currentStaticHead->isLeaf ? "TRUE" : "FALSE");

			// clean up processing lists
			do {
				tmpHead = head;
				head = head->next;
				delete tmpHead;
			} while (head != NULL);
			do {
				tmpHead = staticHead;
				staticHead = staticHead->next;
				delete tmpHead;
			} while (staticHead != NULL);

			return false;
		}

		for (int i = 0; i < 26; i++) {
			if ((currentHead->children[i] == NULL) ^ (currentStaticHead->children[i] == NULL)) {
				LOG_DEBUG("Only %s has child %c.", currentHead->children[i] ? "currentHead" : "currentStaticHead", indexToChar(i));
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
			}
		}

		// advance processing lists
		tmpHead = head;
		head = head->next;
		delete tmpHead;
		tmpHead = NULL;

		tmpHead = staticHead;
		staticHead = staticHead->next;
		delete tmpHead;
		tmpHead = NULL;

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
#if DEBUG
	info.trieSize -= sizeof(node->id);
#endif
	if (node->isLeaf) { info.wordCount++; }

	for (int i = 0; i < 26; i++) {
		if (node->children[i] != NULL) {
			info.letterCount++;
			info += getTrieNodeInfo(node->children[i]);
		}
	}

	return info;
}

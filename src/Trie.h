#ifndef TRIE_H_
#define TRIE_H_

#include <cinttypes>
#include <fstream>
#include <string>

#include "Logger.h"

#define charToIndex(c) ((int)c - (int)'A')
#define indexToChar(i) ((char)i + (char)'A')

using std::ifstream;
using std::ofstream;
using std::string;

struct TrieNode {
#if DEBUG
	uint64_t id;
	static uint64_t createTrieNodeId() { static uint64_t nextId = 0; return nextId++; }
#endif
	TrieNode* children[26];
	bool isLeaf;

	TrieNode();
	~TrieNode();
};

struct LinkedTrieNode {
	TrieNode* node;
	LinkedTrieNode* next;

	LinkedTrieNode() {
		node = NULL;
		next = NULL;
	}
	~LinkedTrieNode() { }
};

struct TrieInfo {
	uint64_t letterCount;
	uint64_t wordCount;
	uint64_t trieSize;

	TrieInfo() : letterCount(0), wordCount(0), trieSize(0) { }

	TrieInfo& operator+=(const TrieInfo& info) {
		letterCount += info.letterCount;
		wordCount += info.wordCount;
		trieSize += info.trieSize;

		return *this;
	}
};

class Trie {
	public:
		Trie();
		~Trie();
		void clearTrie();
		TrieNode* getRoot();
		void insert(const char* key, int len);
		bool trieCompare(Trie& trie);
		bool serialize(const char* fileName);
		bool deserialize(const char* fileName);
		TrieInfo getTrieInfo();

	private:
		TrieNode* root;

		TrieInfo getTrieNodeInfo(TrieNode* node);
		static LinkedTrieNode* nodeToUint32(uint32_t& output, TrieNode* node, LinkedTrieNode* tail);
		static LinkedTrieNode* uint32ToNode(uint32_t input, TrieNode* node, LinkedTrieNode* tail);
};

#endif	// TRIE_H_

#ifndef TRIE_H_
#define TRIE_H_

#include <cinttypes>
#include <fstream>
#include <string>

#define charToIndex(c) ((int)c - (int)'A')
#define indexToChar(i) ((char)i + (char)'A')

using std::ifstream;
using std::ofstream;
using std::string;

struct TrieNode {
	TrieNode* children[26];
	bool isLeaf;

	TrieNode();
	~TrieNode();
};

struct LinkedTrieNode {
	TrieNode* node;
	LinkedTrieNode* next;

	LinkedTrieNode();
	~LinkedTrieNode();
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

	private:
		TrieNode* root;

		static LinkedTrieNode* nodeToUint32(uint32_t& output, TrieNode* node, LinkedTrieNode* tail);
		static LinkedTrieNode* uint32ToNode(uint32_t input, TrieNode* node, LinkedTrieNode* tail);
};

#endif	// TRIE_H_

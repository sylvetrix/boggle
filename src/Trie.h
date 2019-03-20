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
	~TrieNode();
};

struct LinkedTrieNode {
	TrieNode* node;
	LinkedTrieNode* next;
};

class Trie {
	public:
		Trie();
		~Trie();
		void clearTrie();
		TrieNode* getRoot();
		void insert(const char* key, int len);
		bool trieCompare(Trie& trie);
		static TrieNode* createNode();
		static void serialize(Trie& dict, string fileName);
		static bool deserialize(Trie& dict, string fileName);

	private:
		TrieNode* root;

		static uint32_t nodeToUint32(TrieNode* node, LinkedTrieNode* & tail);
		static void uint32ToNode(uint32_t input, TrieNode* node, LinkedTrieNode* & tail);
};

#endif	// TRIE_H_

#include <cinttypes>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include "Boggle.h"
#include "Logger.h"

#define TEST_LOG "BoggleUnitTest.log"
#define TEST_TRIE "TestSerializer.trie"
#define TEST_STATICTRIE "TestSerializerStatic.trie"
#define TEST_LETTERCOUNT 22u
#define TEST_WORDCOUNT 7u
#define TEST_NODECOUNT 23u
#define TEST_TRIESIZEBYTES 4976u
#define BUFFERINC (sizeof(uint32_t))
#define BUFFERSIZE (BUFFERINC * TEST_NODECOUNT)

using std::ios;
using std::ifstream;
using std::ofstream;

// serializer word list
static string words[TEST_WORDCOUNT] = {
	"CHOIR",
	"COINCIDE",
	"CHORUS",
	"COIN",
	"COINED",
	"ANTSY",
	"ANTS" };
// serializer file bytes
static uint32_t fileUints[TEST_NODECOUNT] = {
	0x02800000,	// 0AC
	0x00001000,	// 0N
	0x00040800,	// 0HO
	0x00000040,	// 0T
	0x00000800,	// 0O
	0x00020000,	// 0I
	0x00000080,	// 0S
	0x00020100,	// 0IR
	0x00001000,	// 0N
	0x80000002,	// 1Y
	0x00000100,	// 0R
	0x00000020,	// 0U
	0x80a00000,	// 1CE
	0x80000000,	// 1
	0x80000000,	// 1
	0x00000080,	// 0S
	0x00020000,	// 0I
	0x00400000,	// 0D
	0x80000000,	// 1
	0x00400000,	// 0D
	0x80000000,	// 1
	0x00200000,	// 0E
	0x80000000	// 1
};

// test function forward declarations
bool testSerializer(const char* testFileName, const char* testStaticFileName);
bool testDeserializer(const char* testFileName);
bool testTrieInfo();

int main(int argc, char** argv) {
	Logger::Instance()->openLogFile(TEST_LOG, true);

	LOG_INFO("Testing serializer");
	bool ret = testSerializer(TEST_TRIE, TEST_STATICTRIE);
	LOG_INFO("Serializer test: %s", ret ? "PASS" : "FAIL");

	LOG_INFO("Testing deserializer");
	ret = testDeserializer(TEST_STATICTRIE);
	LOG_INFO("Deserializer test: %s", ret ? "PASS" : "FAIL");

	LOG_INFO("Testing trie info");
	ret = testTrieInfo();
	LOG_INFO("Trie info test: %s", ret ? "PASS" : "FAIL");

	Boggle boggle = Boggle();
	TrieInfo info = boggle.getTrieInfo();
	LOG_INFO("Boggle dictionary word count = %lu", info.wordCount);
	LOG_INFO("Boggle dictionary letter count = %lu", info.letterCount);
	LOG_INFO("Boggle dictionary trie size (bytes) = %lu B", info.trieSize);
	/*boggle.newGame();
	boggle.printBoard(std::cout);*/

	Logger::Instance()->closeLogFile();
}

bool testSerializer(const char* testFileName, const char* testStaticFileName) {
	char buffer[BUFFERSIZE];

	// create test static trie file
	// creating test static file each time makes endianness irrelevant
	ofstream file;
	file.open(testStaticFileName, ios::out | ios::binary | ios::trunc);
	for (int i = 0; i < TEST_NODECOUNT; i++) {
		std::memcpy(&buffer[i*BUFFERINC], &fileUints[i], BUFFERINC);
	}
	file.write(buffer, BUFFERSIZE);
	file.close();

	// create trie
	Trie testTrie = Trie();
	for (int i = 0; i < TEST_WORDCOUNT; i++) {
		testTrie.insert(words[i].c_str(), words[i].length());
	}

	// serialize testTrie to file
	testTrie.serialize(testFileName);

	// compare serialized trie to static trie
	ifstream fileTrie, fileStaticTrie;
	fileTrie.open(testFileName, ios::ate | ios::binary);
	fileStaticTrie.open(testStaticFileName, ios::ate | ios::binary);

	// compare sizes
	if (fileTrie.tellg() != fileStaticTrie.tellg()) {
		fileTrie.close();
		fileStaticTrie.close();
		LOG_DEBUG("testSerializer: file sizes do not match");
		return false;
	}

	// rewind to beginning
	fileTrie.seekg(0);
	fileStaticTrie.seekg(0);

	std::istreambuf_iterator<char> beginTrie(fileTrie);
	std::istreambuf_iterator<char> beginStaticTrie(fileStaticTrie);

	bool ret = std::equal(beginTrie, std::istreambuf_iterator<char>(), beginStaticTrie);
	fileTrie.close();
	fileStaticTrie.close();

	return ret;
}

bool testDeserializer(const char* testStaticFileName) {
	char buffer[BUFFERSIZE];

	// create test static trie file
	// creating test static file each time makes endianness irrelevant
	ofstream file;
	file.open(testStaticFileName, ios::out | ios::binary | ios::trunc);
	for (int i = 0; i < TEST_NODECOUNT; i++) {
		std::memcpy(&buffer[i*BUFFERINC], &fileUints[i], BUFFERINC);
	}
	file.write(buffer, BUFFERSIZE);
	file.close();

	// create static trie
	Trie testStaticTrie = Trie();
	for (int i = 0; i < TEST_WORDCOUNT; i++) {
		testStaticTrie.insert(words[i].c_str(), words[i].length());
	}

	// deserialize trie from file
	Trie testTrie = Trie();
	if (!testTrie.deserialize(testStaticFileName)) { return false; }

	return testTrie.trieCompare(testStaticTrie);
}

bool testTrieInfo() {
	bool ret = true;

	// create static trie
	Trie testStaticTrie = Trie();
	for (int i = 0; i < TEST_WORDCOUNT; i++) {
		testStaticTrie.insert(words[i].c_str(), words[i].length());
	}

	TrieInfo info = testStaticTrie.getTrieInfo();
	if (TEST_LETTERCOUNT != info.letterCount) { ret = false; }
	LOG_INFO("Trie letter count: expected = %u, actual = %lu", TEST_LETTERCOUNT, info.letterCount);
	if (TEST_WORDCOUNT != info.wordCount) { ret = false; }
	LOG_INFO("Trie word count: expected = %u, actual = %lu", TEST_WORDCOUNT, info.wordCount);
	if (TEST_TRIESIZEBYTES != info.trieSize) { ret = false; }
	LOG_INFO("Trie size: expected = %u B, actual = %lu B", TEST_TRIESIZEBYTES, info.trieSize);

	return ret;
}

#include <iostream>

#include "Boggle.h"
#include "Logger.h"

#define MAIN_LOG "BoggleMain.log"

int main(int argc, char** argv) {
	Logger::Instance()->openLogFile(MAIN_LOG, true);

	Boggle boggle = Boggle();
	TrieInfo info = boggle.getTrieInfo();
	LOG_INFO("Boggle dictionary word count = %lu", info.wordCount);
	LOG_INFO("Boggle dictionary letter count = %lu", info.letterCount);
	LOG_INFO("Boggle dictionary trie size (bytes) = %lu B", info.trieSize);

	LOG_INFO("Solving game");
	boggle.solveGame(std::cout);

	Logger::Instance()->closeLogFile();
}

#ifndef BOGGLE_H_
#define BOGGLE_H_

#include <iostream>
#include <list>
#include <string>

#include "Logger.h"
#include "Trie.h"

class Boggle {
	public:
		Boggle();
		~Boggle();
		void newGame();
		void printBoard(std::ostream&);
		void solveGame(std::ostream&);
		TrieInfo getTrieInfo() { return dictionary.getTrieInfo(); }

	private:
		char board[5][5] = { {}, {}, {}, {}, {} };
		string dice[25];
		Trie dictionary;
		bool visited[5][5] = { {}, {}, {}, {}, {} };
		std::list<string> words;

		void clearBoard();
		void clearVisited();
		void findWords();
		bool isSafe(int i, int j);
		void loadBoard();
		void loadDice();
		void loadDict();
		void searchWord(TrieNode* root, int i, int j, string str);
};

#endif	// BOGGLE_H_

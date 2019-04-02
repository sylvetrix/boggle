#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <time.h>
#include <typeinfo>

#include "Boggle.h"

#define DICT_FILE "BoggleWords.dict"
#define TRIE_FILE "BoggleWords.trie"
#define EACH_I (int i = 0; i < 5; i++)
#define EACH_J (int j = 0; j < 5; j++)
#define inRange(i) ((i >= 0) && (i < 5))

using std::list;

Boggle::Boggle() {
	dictionary = Trie();
	words = list<string>();
	clearBoard();
	clearVisited();
	loadDice();
	loadDict();
}

Boggle::~Boggle() {
}

void Boggle::clearBoard() {
	for EACH_I {
		for EACH_J {
			board[i][j] = 0;
		}
	}
}

void Boggle::clearVisited() {
	for EACH_I {
		for EACH_J {
			visited[i][j] = false;
		}
	}
}

void Boggle::findWords() {
	clearVisited();
	words = list<string>();

	TrieNode* child = dictionary.getRoot();
	string str = "";

	for EACH_I {
		for EACH_J {
			int index = charToIndex(board[i][j]);
			if (child->children[index]) {
				str = str + board[i][j];
				searchWord(child->children[index], i, j, str);
				str = "";
			}
		}
	}

	words.sort();
	words.unique();
	list<string>::const_iterator iterator, end;
	int points = 0;
	for (iterator = words.begin(), end = words.end(); iterator != end; iterator++) {
		std::cout << *iterator << std::endl;
		switch ((*iterator).length()) {
			case 4: points += 1; break;
			case 5: points += 2; break;
			case 6: points += 3; break;
			case 7: points += 5; break;
			default: points += 11; break;
		}
	}
	printf("Total points: %d\n", points);
}

bool Boggle::isSafe(int i, int j) {
	return (inRange(i) && inRange(j) && !visited[i][j]);
}

void Boggle::loadDice() {
	dice[0]  = "AAAFRS";
	dice[1]  = "AAEEEE";
	dice[2]  = "AAFIRS";
	dice[3]  = "ADENNN";
	dice[4]  = "AEEEEM";
	dice[5]  = "AEEGMU";
	dice[6]  = "AEGMNN";
	dice[7]  = "AFIRSY";
	dice[8]  = "BJKQXZ";
	dice[9]  = "CCENST";
	dice[10] = "CEIILT";
	dice[11] = "CEILPT";
	dice[12] = "CEIPST";
	dice[13] = "DDHNOT";
	dice[14] = "DHHLNO";
	dice[15] = "DHHLOR";
	dice[16] = "DHLNOR";
	dice[17] = "EIIITT";
	dice[18] = "EMOTTT";
	dice[19] = "ENSSSU";
	dice[20] = "FIPRSY";
	dice[21] = "GORRVW";
	dice[22] = "IKLQUW";
	dice[23] = "NOOTUW";
	dice[24] = "OOOTTU";
}

void Boggle::loadBoard() {
	srand(time(NULL));

	// shuffle dice with Knuth shuffle
	int r;
	string tmpDie;
	for (int i = 24; i >= 1; i--) {
		r = rand() % (i + 1);

		// swap
		if (i == r) continue;
		tmpDie = dice[i];
		dice[i] = dice[r];
		dice[r] = tmpDie;
	}

	// select one side of each die
	for EACH_I {
		for EACH_J {
			board[i][j] = dice[i * 5 + j][rand() % 6];
		}
	}
}

void Boggle::loadDict() {
	bool ret = dictionary.deserialize(TRIE_FILE);
	if (!ret) {
		printf("Trie deserializion failed. Loading trie from dictionary.\n");
		string word;
		ifstream file;
		file.open(DICT_FILE);
		if (file.is_open()) {
			while (getline(file, word)) {
				dictionary.insert(word.c_str(), word.length());
			}
			file.close();

			dictionary.serialize(TRIE_FILE);
		}
	}	
}

void Boggle::newGame() {
	loadBoard();
	clearVisited();
}

void Boggle::printBoard(std::ostream& stream) {
	char tmp;
	for EACH_I {
		stream << "+---+---+---+---+---+\n";
		stream << "|";
		for EACH_J {
			tmp = board[i][j];
			stream << " " << tmp;
			tmp != 'Q' ? stream << " |" : stream << "u|";
		}
		stream << std::endl;
	}
	stream << "+---+---+---+---+---+\n";
}

void Boggle::searchWord(TrieNode* root, int i, int j, string str) {
	if ((root->isLeaf = true) && (str.length() >= 4)) {
		words.push_back(str);
	}

	if (isSafe(i, j)) {
		visited[i][j] = true;

		for (int k = 0; k < 26; k++) {
			if (root->children[k] != NULL) {
				char ch = indexToChar(k);
				// array of possible moves
				int move[8][2] = {
					{i-1, j-1}, {i-1, j  }, {i-1, j+1},
					{i  , j-1}, /* (i,j) */ {i  , j+1},
					{i+1, j-1}, {i+1, j  }, {i+1, j+1}
				};

				for (int m = 0; m < 8; m++) {
					int mi = move[m][0];
					int mj = move[m][1];
					if (isSafe(mi, mj) && (board[mi][mj] == ch)) {
						searchWord(root->children[k], mi, mj, str + ch);
					}
				}
			}
		}

		visited[i][j] = false;
 	}
}

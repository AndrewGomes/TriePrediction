#ifndef __TRIE_PREDICTION_H
#define __TRIE_PREDICTION_H

#define MAX_WORDS_PER_LINE 30
#define MAX_CHARACTERS_PER_WORD 1023

// This directive renames your main() function, which then gives my test cases
// a choice: they can either call your main() function (using this new function
// name), or they can call individual functions from your code and bypass your
// main() function altogether. THIS IS FANCY.
#define main demoted_main

typedef struct TrieNode
{
	// number of times this string occurs in the corpus
	int count;

	// 26 TrieNode pointers, one for each letter of the alphabet
	struct TrieNode *children[26];

	// the co-occurrence subtrie for this string
	struct TrieNode *subtrie;
} TrieNode;


// Functional Prototypes

TrieNode *buildTrie(char *filename);

int processInputFile(TrieNode *root, char *filename);

TrieNode *destroyTrie(TrieNode *root);

TrieNode *getNode(TrieNode *root, char *str);

void getMostFrequentWord(TrieNode *root, char *str);

int containsWord(TrieNode *root, char *str);

int prefixCount(TrieNode *root, char *str);

double difficultyRating(void);

double hoursSpent(void);

#endif

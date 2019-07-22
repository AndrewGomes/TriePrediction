// Author: Andrew Gomes (an289864)
// COP 3502 Dr. Szumlanski
// FALL 2017
// TriePrediction.c
//
// Some functions are written by Dr. Szumlanski and others are taken from tries.c 
// (only with permission from the .pdf)
//
// A few other functions are "suggested" by Dr. Szumlanski and they have the same method
// signature as they appear in the TriePrediction.pdf document.
/////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "TriePrediction.h"

// Suggested function by Dr. Szumlanski
// Creates a Node and initializes it to default settings.
TrieNode *createTrieNode(void) {

	int i = 0;
	TrieNode *root = malloc(sizeof(TrieNode));

	if (root == NULL) 
		return NULL;

	root->count = 0;
	root->subtrie = NULL;

	for (i = 0; i < 26; i++)
		root->children[i] = NULL;

	return root;
}

// Helper function called by printTrie(). (Credit: Dr. S.)
void printTrieHelper(TrieNode *root, char *buffer, int k)
{
	int i;

	if (root == NULL)
		return;

	if (root->count > 0)
		printf("%s (%d)\n", buffer, root->count);

	buffer[k + 1] = '\0';

	for (i = 0; i < 26; i++)
	{
		buffer[k] = 'a' + i;

		printTrieHelper(root->children[i], buffer, k + 1);
	}

	buffer[k] = '\0';
}

// If printing a subtrie, the second parameter should be 1; otherwise, if
// printing the main trie, the second parameter should be 0. (Credit: Dr. S.)
void printTrie(TrieNode *root, int useSubtrieFormatting)
{
	char buffer[1026];

	if (useSubtrieFormatting)
	{
		strcpy(buffer, "- ");
		printTrieHelper(root, buffer, 2);
	}
	else
	{
		strcpy(buffer, "");
		printTrieHelper(root, buffer, 0);
	}
}

// Suggested function by Dr. Szumlanski
// Removes all the extra symbols from a string (also converts into lowercase)
void stripPunctuators(char *str) {

	int i = 0;
	int length;
	int newLength = 0;
	char *newString;

	if (str == NULL)
		return;

	length = strlen(str);
	newString = malloc(sizeof(char) * length);

	if (newString == NULL)
		return;
	
	for (i = 0; i < length; i++) {

		// We only want to insert alphabetic characters. No numbers or other symbols.
		// The characters are then converted into lowercase.
		if (isalpha(str[i])) {

			newString[newLength] = tolower(str[i]);

			// Need to keep track of the new string's length because it will surely be smaller
			// than the original string.
			newLength++;

		}
	}

	// Need to add the terminator character and then use strcpy because a regular
	// equals (str = newString) doesn't work as you think it would.
	newString[newLength] = '\0';
	strcpy(str, newString);

	// Otherwise we'll have a string floating around in memory with no way to access it
	// and valgrind will complain!
	free(newString);
}

// Insert a string into a trie. This function returns the root of the trie.
// Made by Dr. Szumlanski (taken from tries.c)
//
// "Note: You should try to implement this yourself, but there’s a copy of this function in our notes 
// in Webcourses, should you really need it."
//
// Slight modifcation: renamed from insert -> insertString
//
TrieNode *insertString(TrieNode *root, char *str)
{
	int i, index, len = strlen(str);
	TrieNode *wizard;

	if (root == NULL)
		root = createTrieNode();

	// As I mentioned in class, the wizard starts at the root node and spells
	// out the string being inserted as he/she jumps from node to node. (The
	// wizard is acting as a temp pointer.)
	wizard = root;

	for (i = 0; i < len; i++)
	{
		if (!isalpha(str[i]))
		{
			fprintf(stderr, "Error: \"%s\" contains non-alpha characters and will not be inserted.\n", str);
			return NULL;
		}

		index = tolower(str[i]) - 'a';

		// Before the wizard can move forward to the next node, (s)he needs to
		// make sure that node actually exists. If not, create it!
		if (wizard->children[index] == NULL)
			wizard->children[index] = createTrieNode();

		// Now the wizard is able to jump forward.
		wizard = wizard->children[index];
	}

	// When we break out of the for-loop, the wizard should be at the terminal
	// node that represents the string we're trying to insert.
	wizard->count++;
	return root;
}

// Reads the input file and creates the trie from top to bottom.
TrieNode *buildTrie(char *filename) {

	int lastWord = 0;
	TrieNode *root;
	TrieNode *subtrieNode = NULL;
	char *corpus;
	FILE *fp;

	if (filename == NULL)
		return NULL;

	fp = fopen(filename, "rw");
	if (fp != NULL) {

		root = createTrieNode();
		corpus = malloc((sizeof(char) * 1024));

		while (fscanf(fp, "%s", corpus) != EOF) {

			// Determines whether or not the word is the last on the line,
			// according to the .pdf, a line must terminate with a punctuator.
			// We use this to properly let our code below know how to handle re-occurences.
			if (corpus[strlen(corpus) - 1] == '.' || corpus[strlen(corpus) - 1] == '!' || corpus[strlen(corpus) - 1] == '?')
				lastWord = 1;
			else 
				lastWord = 0;

			// Each line needs to be stripped of various non-alpha characters
			// and then inserted into the trie in the proper place.
			stripPunctuators(corpus);
			insertString(root, corpus);

			// Need to properly set-up the subtries for the purpose of re-occuring words,
			// otherwise the word prediction just won't work.
			if (subtrieNode == NULL) {

				subtrieNode = getNode(root, corpus);

			} else {
				
				// The subtrie needs to be created for the particular node.
				// Once the subtrie is created, we can insert it into the trie and then
				// set subTrieNode to be the last node of the string which represents the
				// string as it appears in the trie.
				if (subtrieNode->subtrie == NULL)
					subtrieNode->subtrie = createTrieNode();	

				insertString(subtrieNode->subtrie, corpus);
				subtrieNode = lastWord ? NULL : getNode(root, corpus);

			}
		}

		free(corpus);
		fclose(fp);
		return root;
	}

	// The file doesn't exist.
	return NULL;
}

// Reads the input file named input0X.txt and performs certain actions.
int processInputFile(TrieNode *root, char *filename) {

	FILE *fp;
	TrieNode *foundNode;
	char *input;
	char *predictionString;
	int printMax = 0;
	int i;

	if (root == NULL || filename == NULL)
		return 1;

	fp = fopen(filename, "rw");
	if (fp != NULL) {

		input = malloc((sizeof(char) * 1024));

		// Scan the input file and read in the contents, we need to perform certain actions
		// based on the contents of the input file.
		while (fscanf(fp, "%s", input) != EOF) {
			if (input[0] != NULL) {
				if (input[0] == '@') {
					
					// Word prediction feature, w(0) is the first string.
					// Format: <@ stringToPrint printMax>
					predictionString = malloc((sizeof(char) * 1024));
					fscanf(fp, "%s", predictionString);
					fscanf(fp, "%i", &printMax);
			
					// w(0) is the first string to print (the string to search for)
					// The string here needs to appear eXaCtLy as it appears in the cmd
					printf("%s", predictionString);

					// Now that we've outputted the orginal string to the console (to match the doctor's 
					// output), we need to convert the string to lowercase so we can properly search the trie.
					// We can just re-use our function below to do that
					stripPunctuators(predictionString);

					// Now we need to print out the most frequently appearing words
					// (in order) that follow our predictionString
					for (i = 0; i < printMax; i++) {
					
						foundNode = getNode(root, predictionString);
	
						// printf("\nSearching trie for [%s] -> [%p] -> [%p]\n", predictionString, foundNode, foundNode->subtrie);

						// If the string doesn't even exist in the trie, we have zero information
						// to use with our word predictor.
						if (foundNode == NULL || foundNode->subtrie == NULL)
							break;
						
						// mostFrequentWord copies the return value into the parameter
						// that was passed to it. As instructed, we need to have a space between each word
						// but we're not allowed to have a space at the end.
						getMostFrequentWord(foundNode->subtrie, predictionString);
						printf(" %s", predictionString);
					}

					printf("\n");
					free(predictionString);

				} else if (input[0] == '!') {

					// The ! command simply prints the entire tree.
					printTrie(root, 0);

				} else {

					// We need to print the original string before it's converted to lowercase or else our output
					// won't match the doctor's output files!
					printf("%s\n", input);

					// Search for the specified string to see if it's in the trie. My subtries had a problem
					// for the longest time but I realized I wasn't converting the input string to lowercase
					// (and thus foundNode equalled null) so I've added stripPunctuators() below
					stripPunctuators(input);
					foundNode = getNode(root, input);
					
					// The string isn't in the trie.
					if (foundNode == NULL) {

						printf("(INVALID STRING)\n");

					// String is in the trie, but invalid subtrie.
					} else if (foundNode->subtrie == NULL) {

						printf("(EMPTY)\n");

					// Actually print the thing.
					} else {

						printTrie(foundNode->subtrie, 1);

					}
				}
			}
		}

		free(input);
		fclose(fp);
	}

	return 0;
}

TrieNode *destroyTrie(TrieNode *root) {

	int i = 0;

	if (root == NULL)
		return NULL;

	// Recursively free the subtrie of the current root
	if (root->subtrie != NULL)
		destroyTrie(root->subtrie);

	// Recursively free all the children that the current root has. 
	// The root can't be null at this point.
	for (i = 0; i < 26; i++) {
		if (root->children[i] != NULL)
			destroyTrie(root->children[i]);
	}

	// Root can only be freed after you free all of the child nodes
	// or else you'd encounter a seg fault while running the program.
	// Valgrind does, however, seem to conclude that freeing the root before 
	// the children results in zero memory leaks. Regardless, freeing the root after
	// the children are freed works for both cases.
	free(root);

	return NULL;
}

TrieNode *getNode(TrieNode *root, char *str) {
	
	int i;
	TrieNode *last = NULL;

	if (root == NULL || str == NULL)
		return NULL;

	for (i = 0; i < strlen(str); i++) {
	
		// Traverse down the trie in the proper ordering of the letters 
		last = root->children[tolower(str[i] - 'a')];

		if (last != NULL)
			root = last;
		else
			return NULL;
	}

	// We have have to return a pointer to the last node in the sequence
	// which "represents the string". We're ensuring we are only returning the Node
	// if it actually exists in the trie.
	// For example, if the trie contains "powered" and we search for power, and "power" isn't
	// actually in the trie, it's count will be zero and thus we return NULL instead.
	if (last->count > 0)
		return last;
	
	return NULL;
}

// Custom function: helps to count the most occuring string. Similar to my custom
// function that helps to count prefix occurences.
// Some credits are to the professor and his print trie function (because this function and that
// function both loop through the entire tree -- and this function has to check the entire tree in order to be accurate)
int countMostOccurrences(TrieNode *root, int *maximum, char *maximumString, char *str, int index) {

	int i;
	int count = 0;

	// We found a new highest occuring string. Note that since we loop from 0-25, we will
	// automatically be in alphabetic order. If we did >= instead of >, we'd get the LAST
	// string that occurs the most in the instance of a tie instead of the first one.
	if (*maximum < root->count) {
		*maximum = root->count;
		strcpy(str, maximumString);
	}

	// Traverse the children to see if they have a count greater than our current max.
	for (i = 0; i < 26; i++) {
		maximumString[index] = 'a' + i;
		if (root->children[i] != NULL)
			countMostOccurrences(root->children[i], maximum, maximumString, str, index + 1);	
	}

	maximumString[index] = '\0';

	return count;
}

void getMostFrequentWord(TrieNode *root, char *str) {

	int highest = 0;
	char array[MAX_CHARACTERS_PER_WORD];

	// If the trie is empty, set the string to an empty string.
	if (root == NULL) {
		strcpy(str, "");
		return;
	}

	// This will automatically insert the value into 'str' so we don't have to do
	// any manual assigning right here.
	countMostOccurrences(root, &highest, array, str, 0);
}

int containsWord(TrieNode *root, char *str) {

	// getNode() returns the last character in the string.
	// We should be able to call node->count to get the number of occurences
	// of the string in the trie.
	TrieNode *node = getNode(root, str);	

	return node != NULL ? node->count : 0;
}

// Custom function: helps to count the prefixes
int countChildren(TrieNode *root) {

	int i;
	int count = 0;

	if (root == NULL)
		return 0;
	
	count += root->count;

	// Now traverse the children of the last character in the string.
	for (i = 0; i < 26; i++) {

		// We're searching for the prefix "choco". If "chocohate" is in the trie, it 
		// will be a child of the "choco" string and thus be present/not null and we can count it.
		if (root->children[i] != NULL)
			count += countChildren(root->children[i]);	

	}

	return count;
}

int prefixCount(TrieNode *root, char *str) {

	int i;
	TrieNode *lastNode;

	// Special case: if str is "" (empty string) we just count the entire trie.
	if (strlen(str) < 1)
		return countChildren(root);

	// Advance to the last node/character of the string we're searching for, assuming the string
	// is in the trie, if it isn't we handle that below.
	lastNode = getNode(root, str);

	// The prefix isn't explicitly in the trie. This is the case in which we are searching for the prefix
	// "ch" but the exact string "ch" isn't in the trie. "Chocolate" and "Cherry" are in the trie
	// so we need to find a way to count them!
	if (lastNode == NULL) {
		
		// Traverse down the appropriate child nodes and advance our root node to point to 
		// the last character (in my example that would be "h")
		for (i = 0; i < strlen(str); i++)
			root = root->children[tolower(str[i] - 'a')];

		lastNode = root;
	}

	// ... and begin counting all the child nodes.
	return countChildren(lastNode);
}

double difficultyRating(void) {
	return 4.2;
}

double hoursSpent(void) {
	return 9.0;
}

int demoted_main(int argc, char **argv) {

	TrieNode *root = buildTrie(argv[1]);
	
	if (root == NULL) {
		// Well this isn't good
		return -1;
	}

	processInputFile(root, argv[2]);

	destroyTrie(root);	

	return 0;
}




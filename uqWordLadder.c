/* 
 * CSSE2310 A1
 * UQWordLadder
 * 47435278 - Hamza Khurram
 */

#include <stdlib.h>
#include <stdio.h>
#include <csse2310a1.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define USAGE_ERR "Usage: uqwordladder [--from fromWord] [--to endWord] "\
    "[--limit stepLimit] [--length len] [--dictfile filename]"
#define NON_LETTER_ERR "uqwordladder: Words must not contain non-letters"
#define LENGTH_ERR "uqwordladder: Word lengths must be from 2 to 9" \
    " (inclusive)"
#define WORD_LENGTH_ERR "uqwordladder: Word lengths should be consistent"
#define DIFF_WORD_ERR "uqwordladder: Words must be different"
#define LIMIT_ERR "uqwordladder: Limit on steps must be word length to 60" \
    " (inclusive)"

#define USAGE_ERR_CODE 7
#define NON_LETTER_ERR_CODE 4
#define LENGTH_ERR_CODE 15 
#define WORD_LENGTH_ERR_CODE 6
#define DIFF_WORD_ERR_CODE 10
#define LIMIT_ERR_CODE 5 
#define FILEPATH_ERR_CODE 11

#define INPUT_BUFFER 100
#define DICT_LINE_BUFFER 51
#define DEFAULT_LIMIT 20
#define MAX_LIMIT 60
#define DEFAULT_LENGTH 4
#define MIN_LENGTH 2
#define MAX_LENGTH 9
#define DEFAULT_DICT "/usr/share/dict/words"

#define VALID_MOVE 3
#define GAME_NOT_OVER 2
#define STEP_LIMIT_EXCEEDED 18
#define USER_WON 0
#define USER_GAVE_UP 1

// Initialise functions
int user_move(char* userInput, char** oldWords, char* endWord,
	int stepNumber, char** dictWords, int dictLength);
void check_all_errors(int argc, char** argv, char** validOptions); 
void check_num_options(int argc); 
void check_unexpected_option(int argc, char** argv, char** validOptions);
void check_repeated_option(int argc, char** argv, char** validOptions);
void check_length_limit(int argc, char** argv); 
char* find_arg_value(int argc, char** argv, char* option); 
int get_length(int argc, char** argv); 
void check_length_validity(int argc, char** argv); 
void check_non_letter(char* fromWord, char* endWord);
void check_word_similar(char* fromWord, char* endWord); 
void check_word_validity(int argc, char** argv);
char* convert_capital(char* word); 
void check_limit(int argc, char** argv); 
void check_dictionary(int argc, char** argv); 
char* get_word(int argc, char** argv, char* option, int length); 
int get_limit(int argc, char** argv); 
char* get_dict_word(FILE* file, int length); 
char** get_dictionary(int argc, char** argv, int length, int* dictLength); 
int compare_words(char* firstWord, char* secondWord); 
void print_suggestions(char** oldWords, char* endWord, int numOfMoves, 
	char** dictionary, int dictLength); 
bool check_input_word(char* inputWord, char** oldWords, int numOfMoves,
	char** dictionary, int dictLength);
int is_game_over(char* userInput, char* endWord, int numOfMoves, int limit); 
void free_allocated_memory(char** arrayOfArrays, int length);
void exit_game(int exitStatus, int stepNumber); 

int main(int argc, char** argv) {
    char* validOptions[5] = {"--from", "--to", "--length", "--limit", 
	    "--dictfile"};

    check_all_errors(argc, argv, validOptions);

    // Initialise required variables for game.
    int length = get_length(argc, argv);
    int stepLimit = get_limit(argc, argv);
    int stepNumber = 1;
    int dictLength;
    char** dictWords = get_dictionary(argc, argv, length, &dictLength);
    char* fromWord = get_word(argc, argv, "--from", length);
    char* endWord = get_word(argc, argv, "--to", length);

    fprintf(stdout, "Welcome to UQWordLadder!\nYour goal is to turn "
	    "'%s' into '%s' in at most %d steps\n", fromWord, endWord,
	    stepLimit);

    char buffer[INPUT_BUFFER];
    char* userInput;
    char** oldWords = malloc(sizeof(char*) * stepNumber);
    oldWords[stepNumber - 1] = strdup(fromWord);
    int exitStatus = GAME_NOT_OVER;
    do {
    	fprintf(stdout, "Enter word %d (or ? for help):\n", stepNumber);
    	// Get user input
    	fgets(buffer, sizeof(buffer), stdin);
    	if (feof(stdin) != 0) {
	    exitStatus = USER_GAVE_UP;
	    break;
	}
    	userInput = convert_capital(strdup(buffer));
    	userInput[strlen(userInput) - 1] = '\0';
	// Make a move based on user's actions accordingly.
	int userMove = user_move(userInput, oldWords, endWord, stepNumber,
		dictWords, dictLength);
	if (userMove == VALID_MOVE) {
	    oldWords = realloc(oldWords, sizeof(char*) * (++stepNumber));
	    oldWords[stepNumber - 1] = userInput;
	}
	exitStatus = is_game_over(oldWords[stepNumber - 1], endWord,
		stepNumber, stepLimit);
    } while (exitStatus == GAME_NOT_OVER);

    free_allocated_memory(dictWords, dictLength);
    free_allocated_memory(oldWords, stepNumber);
    exit_game(exitStatus, stepNumber);
    return 0;
}

/* user_move()
 * -----------
 * Makes a move based on the user's input.
 *
 * userInput: a pointer to an array of the user's input.
 * oldWords: an array of previously input words.
 * endWord: the final word needed for a win.
 * stepNumber: the number of steps the user has currently made.
 * dictWords: an array of arrays of the dictionary of valid words.
 * dictLength: the length of the dictionary.
 *
 * Returns: VALID_MOVE if the user inputs a valid word, otherwise returns 0.
 */
int user_move(char* userInput, char** oldWords, char* endWord,
	int stepNumber, char** dictWords, int dictLength) {
    // Print suggestions if user inputs "?"
    if (strcmp(userInput, "?") == 0) {
	    print_suggestions(oldWords, endWord, stepNumber, dictWords, 
		    dictLength);
	    return 0;
    }
    // Check if the input is valid.
    bool valid_input = check_input_word(userInput, oldWords, stepNumber,
	    dictWords, dictLength);
    if (valid_input) {
	    return VALID_MOVE;
    }
    return 0;
}

/* check_all_errors()
 * ------------------
 * Checks for all errors using all other check-error functions.
 *
 * argc:
 * argv:
 * validOptions:
 *
 * Errors: Exits with exit status and prints error message according to the
 * 	error. Specific exit status and message is in corresponding functions
 */
void check_all_errors(int argc, char** argv, char** validOptions) {
    check_num_options(argc);
    check_repeated_option(argc, argv, validOptions);
    check_unexpected_option(argc, argv, validOptions);
    check_length_limit(argc, argv);
    check_length_validity(argc, argv);
    check_word_validity(argc, argv);
    check_limit(argc, argv);
    check_dictionary(argc, argv);
}

/* check_num_options()
 * -------------------
 * Checks if number of options given are in odd numbers and less than 11.
 *
 * argc: the number of command line options
 *
 * Errors: Program exits with exit status of 7 and prints usage error if 
 * 	number of command line options is invalid.
 */
void check_num_options(int argc) {
    if (argc > 11 || (argc % 2) == 0) {
	fprintf(stderr, "%s\n", USAGE_ERR);
	exit(USAGE_ERR_CODE);
    }
}

/* check_unexpected_option()
 * -------------------------
 * Checks for unexpected options among the command line options.
 *
 * argc: the number of command line options
 * argv: an array of arrays of the command line options and their values
 * validOptions: an array of arrays of valid options
 *
 * Errors: Exits with exit status 7 and prints usage error if any options
 * 	are not in validOptions.
 */
void check_unexpected_option(int argc, char** argv, char** validOptions) {
    for (int i = 1; i < argc; i += 2) {
	// numInvalid increases to 5 if option is invalid because it is
	// compared with each valid option.
	int numInvalid = 0;
	for (int j = 0; j < 5; ++j) {
	    if (strcmp(argv[i], validOptions[j]) != 0) {
		numInvalid++;
	    }
	}
	// Print invalid usage error if an unexpected option is input.
	if (numInvalid == 5) {
	    fprintf(stderr, "%s\n", USAGE_ERR);
	    exit(USAGE_ERR_CODE);
	}
    }
}

/* check_repeated_option()
 * -----------------------
 * Checks for repeated command line options.
 *
 * argc: the number of command line options
 * argv: an array of arrays of the command line options and their values
 * validOptions: an array of arrays of valid options
 *
 * Errors: Exits with exit status of 7 and prints usage error if any options
 * 	are repeated.
 */
void check_repeated_option(int argc, char** argv, char** validOptions) {
    // Compare each validOptions[5] with command-line options.
    for (int j = 0; j < 5; ++j) {
	// numValid stores number of times each option is seen.
	// It must be 1 or 0.
	int numValid = 0;
	for (int i = 1; i < argc; i += 2) {
	    if (strcmp(argv[i], validOptions[j]) == 0) {
		numValid++;
	    }
	}
	// Print invalid usage error if any option is repeated.
	if (numValid > 1) {
	    fprintf(stderr, "%s\n", USAGE_ERR);
	    exit(USAGE_ERR_CODE);
	}
    }
}

/* find_arg_value()
 * ----------------
 * Finds the specified option's value from the command line options.
 *
 * argc: the number of command line options
 * argv: an array of arrays of the command line options and their values
 * option: the option to search the value for.
 *
 * Returns: a string of of the specified option's value, but it also returns 
 * 	null if the option cannot be found in argv.
 */
char* find_arg_value(int argc, char** argv, char* option) {
    for (int i = 1; i < argc; i += 2) {
	if (strcmp(argv[i], option) == 0) {
	    return argv[i + 1];
	}
    }
    return NULL;
}

/* check_length_limit()
 * --------------------
 * Checks if the length and limit values given in command line options are 
 * 	valid.
 *
 * argc: the number of command line options
 * argv: an array of arrays of the command line options and their values
 *
 * Errors: Exits with exit code 7 and prints usage error if given values are
 * 	less than or equal to 0.
 */
void check_length_limit(int argc, char** argv) {
    
    char* values[2] = {
	    find_arg_value(argc, argv, "--length"),
	    find_arg_value(argc, argv, "--limit")
    	    };

    for (int i = 0; i < 2; ++i) {
	if (values[i] != NULL) {
	    char* strArg;
	    long numArg = strtol(values[i], &strArg, 36);
	    if (numArg <= 0) {
		fprintf(stderr, "%s\n", USAGE_ERR);
		exit(USAGE_ERR_CODE);
	    }
	}
    }
}

/* check_length_validity()
 * -----------------------
 * Checks if the words given from --from and --to, and the length from 
 * 	--length options are valid.
 *
 * argc: the number of command line options
 * argv: an array of arrays of the command line options and their values
 *
 * Errors: Exits with exit status of 6 and prints word length error if words
 * 	and length are inconsistent. It also exits with exit status of 15 if
 * 	the length is less than 2 or greater than 9.
 */
void check_length_validity(int argc, char** argv) {
    // Get options' values.
    char* fromWord = find_arg_value(argc, argv, "--from");
    char* endWord = find_arg_value(argc, argv, "--to");
    int length = get_length(argc, argv);

    // Check if --from or --to is given and compare and validate lengths.
    char* values[] = {fromWord, endWord};
    for (int i = 0; i < 2; ++i) {
	if (values[i] != NULL) {
	    if (strlen(values[i]) != length) {
		fprintf(stderr, "%s\n", WORD_LENGTH_ERR);
		exit(WORD_LENGTH_ERR_CODE);
	    }
	}
    }

    // Check if length is valid.
    if (length < MIN_LENGTH || length > MAX_LENGTH) {
	fprintf(stderr, "%s\n", LENGTH_ERR);
	exit(LENGTH_ERR_CODE);
    }
}

/* convert_capital()
 * -----------------
 * Capitalises each letter of the specified word.
 *
 * word: the word to capitalise
 *
 * Returns: the capitalised word.
 */
char* convert_capital(char* word) {
    int length = strlen(word);
    for (int i = 0; i < length; ++i) {
	word[i] = toupper(word[i]);
    }
    return word;
}

/* check_non_letter()
 * ------------------
 * Checks if the user-specified fromWord and endWord have non-letters.
 *
 * fromWord: the user-specified starting word.
 * endWord: the user-specified ending word.
 *
 * Errors: Exits with exit status of 4 and prints non-letter error if any
 * 	letter is a non-letter.
 */
void check_non_letter(char* fromWord, char* endWord) {
    char* words[] = {fromWord, endWord};

    // Check if each letter of each word is part of the alphabet.
    for (int i = 0; i < 2; ++i) {
	if (words[i] != NULL) {
	    int wordLength = strlen(words[i]);
	    for (int j = 0; j < wordLength; ++j) {
		// Exit with error if not a letter.
		if (isalpha(words[i][j]) == 0) {
		    fprintf(stderr, "%s\n", NON_LETTER_ERR);
		    exit(NON_LETTER_ERR_CODE);
		}
	    }
	}
    }
}

/* check_word_similar()
 * --------------------
 * Checks if the user-specified fromWord and endWord are the same.
 *
 * fromWord: the user-specified starting word.
 * endWord: the user-specified ending word.
 *
 * Errors: Exits with exit satus of 10 and prints diff_word error if both
 * 	both words are the same.
 */
void check_word_similar(char* fromWord, char* endWord) {
    // If fromWord and endWord are given, check if they are different.
    if (fromWord && endWord) {
	char* capitalWords[] = {convert_capital(fromWord), 
	        convert_capital(endWord)};
	// Exit with error if they are the same.
	if (strcmp(capitalWords[0], capitalWords[1]) == 0) {
	    fprintf(stderr, "%s\n", DIFF_WORD_ERR);
	    exit(DIFF_WORD_ERR_CODE);
	}	
    }
}

/* check_word_validity()
 * ---------------------
 * Checks if the given words are valid.
 *
 * argc: the number of command line options
 * argv: an array of arrays of the command line options and their values
 *
 * Errors: Exits with exit status of 4 according to check_non_letter()
 * 	and exits with exit status of 10 according to check_word_similar().
 */
void check_word_validity(int argc, char** argv) {
    char* fromWord = find_arg_value(argc, argv, "--from");
    char* endWord = find_arg_value(argc, argv, "--to");
    
    check_non_letter(fromWord, endWord);
    check_word_similar(fromWord, endWord);
}

/* check_limit()
 * -------------
 * Checks if the user-specified limit value is valid.
 *
 * argc: the number of command line options
 * argv: an array of arrays of the command line options and their values
 * 
 * Errors: Exits with exit status of 5 and prints limit error if the limit
 * 	value is less than word length or higher than 60.
 */ 
void check_limit(int argc, char** argv) {
    // Check if limit was given.
    char* lim = find_arg_value(argc, argv, "--limit");
    int limit;
    if (lim != NULL) {
	limit = atoi(lim);
    } else {
	return;
    }
    
    // Get the word length.
    int length = get_length(argc, argv);

    // Check if limit is within allowed range.
    if (limit < length || limit > MAX_LIMIT) {
	fprintf(stderr, "%s\n", LIMIT_ERR);
	exit(LIMIT_ERR_CODE);
    }
}

/* check_dictionary()
 * ------------------
 * Checks if the user-defined dictionary is a valid file.
 *
 * argc: the number of command line options
 * argv: an array of arrays of the command line options and their values
 * 
 * Errors: Exits with exit status of 11 and prints filepath error if the
 * 	user-specified file does not exist or it cannot be opened.
 */
void check_dictionary(int argc, char** argv) {
    // Check if user has specified filepath.
    char* filePath = find_arg_value(argc, argv, "--dictfile");
    if (filePath == NULL) {
	return;
    }
    // Exit with error if file cannot be opened.
    FILE* dictFile = fopen(filePath, "r");
    if (dictFile == NULL) {
	fprintf(stderr, "uqwordladder: File named \"%s\" cannot be "
		"opened\n", filePath);
	exit(FILEPATH_ERR_CODE);
    }
    fclose(dictFile);
}

/* get_length()
 * -------------
 * Gets the length value from the user-specified words or length option.
 *
 * argc: the number of command line options
 * argv: an array of arrays of the command line options and their values
 *
 * Returns: the length of either word, or the user-specified length.
 */
int get_length(int argc, char** argv) {
    // Get the user-specified option values if given.
    char* fromWord = find_arg_value(argc, argv, "--from");
    char* endWord = find_arg_value(argc, argv, "--to");
    char* len = find_arg_value(argc, argv, "--length");

    // Check if length or word is given and use their length.
    // Otherwise, default length to 4.
    char* values[2] = {fromWord, endWord};
    int length = DEFAULT_LENGTH;
    for (int i = 0; i < 2; i++) {
	if (values[i] != NULL) {
	    length = strlen(values[i]);
	}
    }
    if (len != NULL) {
	length = atoi(len);
    }
    return length;
}

/* get_word()
 * ----------
 * Get the user-specified word if given, otherwise get a random word from
 * 	get_uqwordladder_word().
 *
 * argc: the number of command line options
 * argv: an array of arrays of the command line options and their values
 * option: it can be either the "--from" or "--to" option to get the word for.
 * length: a number between 2 and 9 to specify the length of the word needed
 * 	from get_uqwordladder_word().
 *
 * Returns: a pointer to the array of the word for the corresponding option
 * 	and length.
 */
char* get_word(int argc, char** argv, char* option, int length) {
    // Get the user-specified word for the corresponding option.
    char* word = find_arg_value(argc, argv, option);
    // Get a random word if no word is provided.
    if (word == NULL) {
	word = strdup(get_uqwordladder_word(length));
    } else {
	word = convert_capital(word);
    }
    return word;
}

/* get_limit()
 * ---------------
 * Gets the user-specified limit, otherwise defaults to 20.
 *
 * argc: the number of command line options
 * argv: an array of arrays of the command line options and their values
 *
 * Returns: the limit (either user-specified or default)
 */
int get_limit(int argc, char** argv) {
    // Get the user-specified limit.
    char* lim = find_arg_value(argc, argv, "--limit");
    int limit;
    // Convert it into an integer if given, otherwise default to 20.
    if (lim == NULL) {
	limit = DEFAULT_LIMIT;
    } else {
	limit = atoi(lim);
    }
    return limit;
}

/* get_dict_word()
 * ---------------
 * Gets a word from the specified dictionary file.
 *
 * file: the file to read words from.
 * length: the required length of words to look for.
 *
 * Returns: a pointer to a valid word from the dictionary with the required
 * 	length.
 */
char* get_dict_word(FILE* file, int length) {
    int buffer = 51;
    char word[buffer];
    int wordLength;

    if (fgets(word, buffer, file) == NULL) {
	return NULL;
    }
    
    // Remove '\n' from end of word.
    wordLength = strlen(word);
    word[wordLength-- - 1] = '\0';

    // Check if word has any non-letters and is of required length.
    for (int i = 0; i < length; i++) {
	if (isalpha(word[i]) == 0 || wordLength != length) {
	    return NULL;
	}
    }
    return convert_capital(strdup(word));
}

/* get_dictionary()
 * ----------------
 * Gets an array of the valid words from dictionary with the required length.
 *
 * argc: the number of command line options
 * argv: an array of arrays of the command line options and their values
 * length: the required length of each word.
 * dictLength: a pointer to a number from main() that specifies the number of
 * 	words in dictionary which helps to check valid user input in game.
 *
 * Returns: an array of arrays with all valid words from dictionary with
 * 	required length.
 */
char** get_dictionary(int argc, char** argv, int length, int* dictLength) {
    // Check if user has specified file, otherwise open default dictionary.
    char* filePath = find_arg_value(argc, argv, "--dictfile");
    if (filePath == NULL) {
	filePath = DEFAULT_DICT;
    }

    // Open the file and make an array with dynamically allocated memory.
    FILE* file = fopen(filePath, "r");
    int numOfWords = 1;
    char** words = malloc(numOfWords * sizeof(char*));
    char* word;

    while (feof(file) == 0) {
	    // Get a word from the dictionary.
	    word = get_dict_word(file, length);
	    if (word == NULL) {
	        continue;
	    }
        // Put each character into the array of arrays.
        words[numOfWords - 1] = word;

        // Allocate more memory for the next word.
        words = realloc(words, sizeof(char*) * ++numOfWords);
        words[numOfWords - 1] = malloc(sizeof(char) * (length + 1));
    }

    // Remove last array in words[] because it is empty or contains invalid
    // word.
    free(words[(numOfWords--) - 1]);
    fclose(file);
    *dictLength = numOfWords;
    return words;
}

/* compare_words()
 * ---------------
 * Compares two words of same length to get the number of the differences.
 *
 * firstWord: a pointer to the array of the first word to compare.
 * secondWord: a pointer to the array of the second word to compare.
 *
 * Returns: the number of character differences between two words of same
 * 	length.
 */
int compare_words(char* firstWord, char* secondWord) {
    int wordLength = strlen(firstWord);
    int numOfDiff = 0;
    for (int i = 0; i < wordLength; i++) {
	if (firstWord[i] != secondWord[i]) {
	    numOfDiff++;
	}
    }
    return numOfDiff;
}

/* print_suggestions()
 * -------------------
 * Prints the valid suggestions from dictionary based on the current word and
 * 	and excludes previous words.
 *
 * oldWords: an array of arrays of previously entered words.
 * endWord: the final word which is needed to win.
 * numOfMoves: the number of moves the user has made in the game (which helps
 * 	iterate through oldWords).
 * dictionary: an array of arrays of valid words of the required length.
 * dictLength: the number of words in the dictionary (which helps iterate
 * 	through the dictionary).
 *
 * Returns: void, however it prints any valid suggestions.
 */
void print_suggestions(char** oldWords, char* endWord, int numOfMoves, 
	char** dictionary, int dictLength) {

    int isSuggestions = 0;

    // Print endWord if it is one move away from latest input word.
    int numOfDiff = compare_words(oldWords[numOfMoves - 1], endWord);
    if (numOfDiff == 1) {
	isSuggestions = 1;
	fprintf(stdout, "Suggestions:-----------\n");
	fprintf(stdout, " %s\n", endWord);
    }
    // Print all words from dictionary with one letter difference by 
    // comparing current word with all words in dictionary.
    for (int i = 0; i < dictLength; i++) {
	numOfDiff = compare_words(oldWords[numOfMoves - 1], dictionary[i]);
	if (numOfDiff == 1) {
	    if (strcmp(dictionary[i], endWord) == 0) {
		continue;
	    }
	    // Exclude any previously entered words.
	    int inOldWord = 0;
	    for (int j = 0; j < numOfMoves; j++) {
		if (strcmp(oldWords[j], dictionary[i]) == 0) {
		    inOldWord = 1;
		    break;
		}
	    }
	    if (inOldWord == 1) {
		continue;
	    }
	    if (isSuggestions == 0) {
		isSuggestions = 1;
		fprintf(stdout, "Suggestions:-----------\n");
	    }

	    fprintf(stdout, " %s\n", dictionary[i]);
	}
    }
    if (isSuggestions == 1) {
	fprintf(stdout, "-----End of Suggestions\n");
    } else {
	fprintf(stdout, "No suggestions found.\n");
    }
}

/* check_input_word()
 * ------------------
 * Checks if the input word is valid.
 *
 * inputWord: an array of the user's input word.
 * oldWords: an array of arrays of previously entered words.
 * numOfMoves: the number of moves the user has made in the game (which helps
 * 	iterate through oldWords).
 * dictionary: an array of arrays of valid words of the required length.
 * dictLength: the number of words in the dictionary (which helps iterate
 * 	through the dictionary).
 *
 * Returns: true if word is correct length, one character different to
 * 	previous word, and in the dictionary. Otherwise it returns false.
 */
bool check_input_word(char* inputWord, char** oldWords, int numOfMoves,
	char** dictionary, int dictLength) {
    int length = strlen(oldWords[0]);S
    // Check length
    if (strlen(inputWord) != length) {
	fprintf(stdout, "Word should be %d characters long - try again.\n", 
		length);
	return false;
    }
    // Check if there are non-letters
    for (int i = 0; i < length; i++) {
	if (isalpha(inputWord[i]) == 0) {
	    fprintf(stdout, "Word should contain only letters - "
		    "try again.\n");
	    return false;
	}
    }

    // Check if there is only one different character
    if (compare_words(inputWord, oldWords[numOfMoves - 1]) != 1) {
	fprintf(stdout, "Word should have only one letter different - "
		"try again.\n");
	return false;
    }

    // Check if input word and any previous words are the same.
    for (int i = 0; i < numOfMoves; i++) {
	if (strcmp(inputWord, oldWords[i]) == 0) {
	    fprintf(stdout, "Previous word can't be repeated - "
		    "try again.\n");
	    return false;
	}
    }

    // Check if word is in dictionary
    for (int i = 0; i < dictLength; i++) {
        if (strcmp(inputWord, dictionary[i]) == 0) {
            return true;
        }
    }
    fprintf(stdout, "Word not found in dictionary - try again.\n");
    return false;
}

/* is_game_over()
 * --------------
 * Checks if the game is won or lost.
 *
 * userInput: an array of the valid word input from the user.
 * endWord: an array of the final word needed to win.
 * numOfMoves: the number of moves the user has made in the game 
 * limit: the maximum number of moves the user can make in the game.
 *
 * Returns: 18 if number of moves is greater than the limit, 0 if the input
 * 	word matches end word, or 2 if not game over.
 */
int is_game_over(char* userInput, char* endWord, int numOfMoves, int limit) {
    int numDiff = compare_words(userInput, endWord);
    if (numOfMoves > limit && numDiff != 0) {
	return STEP_LIMIT_EXCEEDED;
    }
    if (numDiff == 0) {
	return USER_WON;
    }
    return GAME_NOT_OVER;
}

/* free_allocated_memory()
 * -----------------------
 * Frees memory of the specified array of arrays, including the arrays inside
 *
 * arrayOfArrays: the array to deallocate memory for.
 * length: the number of arrays inside array.
 *
 * Returns: nothing, however it frees the memory allocated for a 
 * 	two-dimentisonal array.
 */
void free_allocated_memory(char** arrayOfArrays, int length) {
    for (int i = 0; i < length; i++) {
	free(arrayOfArrays[i]);
    }
    free(arrayOfArrays);
}

/* exit_game()
 * -----------
 * Exits the game with exit status and message according to a win or loss.
 *
 * exitStatus: an integer returned from is_game_over() to get the end status
 * 	of the game.
 *
 * Returns: Exits with exit status of 18 if stepNumber has exceeded limit,
 * 	exits with 0 if user has won, or exits with 1 if user gave up.
 */
void exit_game(int exitStatus, int stepNumber) {
    switch (exitStatus) {
	case STEP_LIMIT_EXCEEDED:
	    fprintf(stdout, "Game over - no more attempts remaining.\n");
	    exit(STEP_LIMIT_EXCEEDED);
	case USER_WON:
	    fprintf(stdout, "You solved the ladder in %d steps.\n", 
		    stepNumber - 1);
	    exit(USER_WON);
	case USER_GAVE_UP:
	    fprintf(stdout, "Game over - you gave up.\n");
	    exit(USER_GAVE_UP);
    }
}

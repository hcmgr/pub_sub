//shared.h//
//------------//
//shared.c contains some static methods that both the client and 
//server make use of
//------------//


#ifndef SHARED_FUNCTIONS
#define SHARED_FUNCTIONS

#include <stdbool.h>

/* has_space_colon_newline
 * ----------------------------
 * Checks whether the given string contains spaces, colons or newlines
 * 
 * str - given string to check
 *
 * Returns:
 *        true iff the string contains a space, colon or newline, false 
 *        otherwise
 * */
bool has_space_colon_newline(char* str);

/**
 * add_new_line
 * ------------
 * Adds a new line character to the end of the given string
 *
 * str - given string
 *
 * Returns: string with newline char appended
 * */
char* add_new_line(char* str);

/* string_array_length
 * -------------------
 * Returns the length of the given array of strings
 *
 * arr - array of strings
 * 
 * Returns:
 *      length of the array
 * */
int string_array_length(char** arr);

/* string_to_int 
 * ---------------
 * Attempts to convert the given string to a non-negative integer
 *
 * str - string to be converted
 *
 * Returns:
 *      - -1 if the string cannot be converted to a non-negative integer OR;
 *      - the integer if it CAN BE converted
 * */
int string_to_int(char* str);

/* split_line_max
 * --------------
 * Compiles the given array of strings into a another array of strings 
 * of length == maxToks
 * 
 *  For example, if: 
 *      toks = {"pub", "bingus", "sneeds", "big", "govamint"}
 *      maxToks = 3
 *
 *  Then:
 *      output = {"pub", "bingus", "sneeds big govamint"}
 *
 * toks - array of strings (tokenised form of a string)
 * maxToks - number of tokens 
 *
 * Returns the array of string tokens
 *
 * */
char** split_line_max(char** toks, int maxToks);

#endif //SHARED_FUNCTIONS

#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED
#include<string>

/** Reads a single character from stdin
* @param allowedInput a string containing all characters that are allowed as input
* @return a character read from stdin
*/
char charInput(std::string allowedInput);

#endif // INPUT_H_INCLUDED
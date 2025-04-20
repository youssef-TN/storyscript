// Provides details for token types and descriptions.
#include "../include/token.h"
#include <string>

namespace storyscript
{ // StoryScript code

	// Text names for each TokenType.
	// IMPORTANT: Order must match TokenType enum exactly.
	const char *TokenTypeStrings[] = {
		// Keywords
		"ROOM", "ITEM", "VAR", "FUNCTION", "IF", "ELSE", "WHILE", "FOR", "RETURN", "WHEN", "ENTERED", "SAY", "GOTO", "TRUE", "FALSE", "NOT", "AND", "OR",

		// Names and values
		"IDENTIFIER", "STRING", "NUMBER",

		// Operators
		"PLUS", "MINUS", "MULTIPLY", "DIVIDE", "MODULO", "ASSIGN", "EQ", "NEQ", "LT", "GT", "LTE", "GTE",

		// Structure symbols
		"LPAREN", "RPAREN", "LBRACE", "RBRACE", "COLON", "COMMA", "SEMICOLON", "DOT",

		// Special
		"EOF_TOKEN", "UNKNOWN", "COMMENT"};

	// Get a description string for a Token.
	std::string Token::toString() const
	{
		// Format: [TYPE, 'text', line: #, col: #]
		return std::string("[") +
			   TokenTypeStrings[static_cast<int>(type)] + // Type name
			   ", '" + lexeme +							  // Token text
			   "', line: " + std::to_string(line) +		  // Line
			   ", col: " + std::to_string(column) + "]";  // Column
	}

} // namespace storyscript

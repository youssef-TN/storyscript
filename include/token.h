// Defines tokens and source location tracking.
#ifndef STORYSCRIPT_TOKEN_H 
#define STORYSCRIPT_TOKEN_H

#include <string> 
#include <vector> 

namespace storyscript
{ // Language specific components

	// Types of tokens the lexer can find.
	enum class TokenType
	{
		// Keywords
		ROOM, ITEM, VAR, FUNCTION, IF, ELSE, WHILE, FOR, RETURN, WHEN, ENTERED, SAY, GOTO, TRUE, FALSE, NOT, AND, OR,

		// Names and values
		IDENTIFIER, STRING, NUMBER,

		// Operators
		PLUS, MINUS, MULTIPLY, DIVIDE, MODULO, ASSIGN, EQ, NEQ, LT, GT, LTE, GTE,

		// Structure symbols
		LPAREN, RPAREN, LBRACE, RBRACE, COLON, COMMA, SEMICOLON, DOT,

		// Special
		EOF_TOKEN, UNKNOWN, COMMENT
	};

	// Text names for token types (defined in token.cpp).
	extern const char *TokenTypeStrings[];

	// Holds information about one token.
	struct Token
	{
		TokenType type;		// Kind of token
		std::string lexeme; // The text from the code
		int line;			// Line number (1-based)
		int column;			// Column number (1-based)

		// Create a Token.
		Token(TokenType type, const std::string &lexeme, int line, int column)
			: type(type), lexeme(lexeme), line(line), column(column) {}

		// Get a text description of the token.
		std::string toString() const;
	};

	// Tracks position in the source file.
	class SourceLocation
	{
	public:
		std::string filename; // File name
		int line;			  // Line number
		int column;			  // Column number

		// Create a SourceLocation.
		SourceLocation(const std::string &filename, int line, int column)
			: filename(filename), line(line), column(column) {}

		// Format as "file:line:column".
		std::string toString() const
		{
			return filename + ":" + std::to_string(line) + ":" + std::to_string(column);
		}
	};

} // namespace storyscript

#endif // STORYSCRIPT_TOKEN_H

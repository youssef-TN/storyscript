// Implements the Lexer that turns code into tokens.
#include "../include/lexer.h"
#include <iostream>
#include <cctype>

namespace storyscript
{ // StoryScript code

	/// --- Keywords setup ---

	// Map of keyword text to TokenType.
	std::unordered_map<std::string, TokenType> Lexer::keywords = Lexer::initKeywords();

	// Fills the keyword map.
	std::unordered_map<std::string, TokenType> Lexer::initKeywords()
	{
		std::unordered_map<std::string, TokenType> map;
		// Add all keywords
		map["room"] = TokenType::ROOM;
		map["item"] = TokenType::ITEM;
		map["var"] = TokenType::VAR;
		map["function"] = TokenType::FUNCTION;
		map["if"] = TokenType::IF;
		map["else"] = TokenType::ELSE;
		map["while"] = TokenType::WHILE;
		map["for"] = TokenType::FOR;
		map["return"] = TokenType::RETURN;
		map["when"] = TokenType::WHEN;
		map["entered"] = TokenType::ENTERED;
		map["say"] = TokenType::SAY;
		map["goto"] = TokenType::GOTO;
		map["true"] = TokenType::TRUE;
		map["false"] = TokenType::FALSE;
		map["not"] = TokenType::NOT;
		map["and"] = TokenType::AND;
		map["or"] = TokenType::OR;
		return map;
	}

	/// --- Lexer Core ---

	Lexer::Lexer(const std::string &source, const std::string &filename)
		: source(source), filename(filename) {}

	// Get and consume the next token.
	Token Lexer::nextToken()
	{
		skipWhitespace(); // Skip spaces and comments
		start = position; // Mark token start
		if (isAtEnd())
			return makeToken(TokenType::EOF_TOKEN);
		char c = advance(); // Get next char

		if (isalpha(c) || c == '_')
			return identifier(); // Handle names/keywords
		if (isdigit(c))
			return number(); // Handle numbers

		switch (c)
		{ // Handle symbols and operators
		case '(':
			return makeToken(TokenType::LPAREN);
		case ')':
			return makeToken(TokenType::RPAREN);
		case '{':
			return makeToken(TokenType::LBRACE);
		case '}':
			return makeToken(TokenType::RBRACE);
		case ':':
			return makeToken(TokenType::COLON);
		case ',':
			return makeToken(TokenType::COMMA);
		case ';':
			return makeToken(TokenType::SEMICOLON);
		case '.':
			return makeToken(TokenType::DOT);
		case '+':
			return makeToken(TokenType::PLUS);
		case '-':
			return makeToken(TokenType::MINUS);
		case '*':
			return makeToken(TokenType::MULTIPLY);
		case '/': // Division or comment
			if (match('/'))
			{ // Line comment
				while (peek() != '\n' && !isAtEnd())
					advance();		// Skip comment text
				return nextToken(); // Get next token after comment
			}
			else
				return makeToken(TokenType::DIVIDE); // It's division
		// Two-char operators
		case '=':
			return makeToken(match('=') ? TokenType::EQ : TokenType::ASSIGN);
		case '!':
			return makeToken(match('=') ? TokenType::NEQ : TokenType::NOT);
		case '<':
			return makeToken(match('=') ? TokenType::LTE : TokenType::LT);
		case '>':
			return makeToken(match('=') ? TokenType::GTE : TokenType::GT);
		case '"':
			return string(); // Handle strings
		default:
			return errorToken("Unexpected character."); // Unknown char
		}
	}

	/// --- Look Ahead & Batch ---

	// Look at next token without consuming.
	Token Lexer::peekToken()
	{
		// Save state, get token, restore state.
		size_t savedPosition = position;
		size_t savedStart = start;
		int savedLine = line;
		int savedColumn = column;
		Token token = nextToken();
		position = savedPosition;
		start = savedStart;
		line = savedLine;
		column = savedColumn;
		return token;
	}

	// Get all tokens from the source.
	std::vector<Token> Lexer::tokenize()
	{
		std::vector<Token> tokens;
		Token token(TokenType::UNKNOWN, "", 0, 0);
		do
		{
			token = nextToken();
			tokens.push_back(token);
		} while (token.type != TokenType::EOF_TOKEN);
		return tokens;
	}

	/// --- Position ---

	// Get current position.
	SourceLocation Lexer::getCurrentLocation() const
	{
		return SourceLocation(filename, line, column);
	}

	/// --- Errors ---

	// Report error with position.
	void Lexer::error(const std::string &message)
	{
		std::cerr << getCurrentLocation().toString() << ": Error: " << message << std::endl;
	}

	/// --- Character Utilities ---

	// Move to next char and return it.
	char Lexer::advance()
	{
		position++;
		column++;
		return source[position - 1];
	}

	// Look at current char.
	char Lexer::peek() const
	{
		return isAtEnd() ? '\0' : source[position];
	}

	// Look at next char.
	char Lexer::peekNext() const
	{
		return (position + 1 >= source.length()) ? '\0' : source[position + 1];
	}

	// Check if at end of source.
	bool Lexer::isAtEnd() const
	{
		return position >= source.length();
	}

	// If current char matches expected, consume it.
	bool Lexer::match(char expected)
	{
		if (isAtEnd() || source[position] != expected)
			return false;
		position++;
		column++;
		return true;
	}

	/// --- Token Creation ---

	// Create a normal token.
	Token Lexer::makeToken(TokenType type) const
	{
		std::string lexeme = source.substr(start, position - start);
		return Token(type, lexeme, line, column - lexeme.length());
	}

	// Create an error token.
	Token Lexer::errorToken(const std::string &message) const
	{
		return Token(TokenType::UNKNOWN, message, line, column);
	}

	/// --- Token Handlers ---

	// Handle identifiers and keywords.
	Token Lexer::identifier()
	{
		while (isalnum(peek()) || peek() == '_')
			advance(); // Consume rest of name
		std::string text = source.substr(start, position - start);
		TokenType type = TokenType::IDENTIFIER;
		auto keyword = keywords.find(text); // Check if keyword
		if (keyword != keywords.end())
			type = keyword->second;
		return makeToken(type);
	}

	// Handle numbers.
	Token Lexer::number()
	{
		while (isdigit(peek()))
			advance(); // Whole part
		if (peek() == '.' && isdigit(peekNext()))
		{ // Fractional part
			advance();
			while (isdigit(peek()))
				advance();
		}
		return makeToken(TokenType::NUMBER);
	}

	// Handle strings.
	Token Lexer::string()
	{
		while (peek() != '"' && !isAtEnd())
		{ // Consume until closing quote
			if (peek() == '\n')
			{
				line++;
				column = 1;
			} // Track newlines
			advance();
		}
		if (isAtEnd())
			return errorToken("Unterminated string.");
		advance(); // Consume closing quote
		return makeToken(TokenType::STRING);
	}

	/// --- Whitespace & Comments ---

	// Skip whitespace and comments.
	void Lexer::skipWhitespace()
	{
		while (true)
		{
			switch (peek())
			{
			case ' ':
			case '\r':
			case '\t':
				advance();
				break; // Skip simple whitespace
			case '\n':
				line++;
				column = 1;
				advance();
				break; // Skip newline, update line/col
			default:
				return; // Stop on non-whitespace/non-comment
			}
		}
	}

	/// --- Identifier Type ---

	// Determine if identifier is a keyword or not.
	TokenType Lexer::identifierType() {
		std::string text = source.substr(start, position - start);
		auto it = keywords.find(text);
		if (it != keywords.end()) return it->second;
		return TokenType::IDENTIFIER;
	}
	

} // namespace storyscript

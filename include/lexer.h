// Defines the Lexer that turns code text into tokens.
#ifndef STORYSCRIPT_LEXER_H 
#define STORYSCRIPT_LEXER_H

#include <string>
#include <unordered_map>
#include <vector>
#include "token.h"

namespace storyscript { // StoryScript code

// Reads code and produces tokens.
class Lexer {
public:
    // Set up lexer with code and file name.
    explicit Lexer(const std::string& source, const std::string& filename = "script.story");

    //// --- Token Methods ---

    // Get and use the next token.
    Token nextToken();

    // Look at the next token without using it.
    Token peekToken();

    // Get all tokens.
    std::vector<Token> tokenize();

    /// --- Position ---

    // Get current line/column.
    SourceLocation getCurrentLocation() const;

    /// --- Errors ---

    // Report error at current spot.
    void error(const std::string& message);

private:
    /// --- Internal State ---
    std::string source;        // Code text
    std::string filename;      // File name
    size_t position = 0;       // Current character index
    size_t start = 0;          // Token start index
    int line = 1;              // Current line
    int column = 1;            // Current column

    // Map of keywords.
    static std::unordered_map<std::string, TokenType> keywords;

    /// --- Character Handling ---

    // Use and return next char.
    char advance();
    // Look at current char.
    char peek() const;
    // Look at next char.
    char peekNext() const;
    // Check if at end.
    bool isAtEnd() const;
    // Use char if it matches expected.
    bool match(char expected);

    /// --- Token Creation ---

    // Make a regular token.
    Token makeToken(TokenType type) const;
    // Make an error token.
    Token errorToken(const std::string& message) const;

    /// --- Token Recognition ---

    // Handle names/keywords.
    Token identifier();
    // Handle numbers.
    Token number();
    // Handle strings.
    Token string();

    /// --- Utilities ---

    // Skip spaces, etc.
    void skipWhitespace();
    // Determine if identifier is a keyword.
    TokenType identifierType();

    // Setup keywords map.
    static std::unordered_map<std::string, TokenType> initKeywords();
};

} // namespace storyscript

#endif // STORYSCRIPT_LEX_

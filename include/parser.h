// Header guard to prevent multiple inclusions
#ifndef STORYSCRIPT_PARSER_H
#define STORYSCRIPT_PARSER_H

#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include "token.h"
#include "lexer.h"
#include "ast.h"

namespace storyscript {

class Parser {
public:
    // Initialize parser with a lexer instance
    explicit Parser(Lexer& lexer);
    
    // Main entry point - generates AST from token stream
    std::unique_ptr<Program> parse();
    
    // Error handling interface
    void error(const std::string& message);
    bool hadError() const { return errorOccurred; }
    
private:
    // Core components
    Lexer& lexer;         // Reference to token generator
    Token current;        // Current token being processed
    Token previous;       // Previously consumed token
    bool errorOccurred = false;  // Error state flag

    // Token stream management
    void advance();       // Move to next token
    bool check(TokenType type) const;  // Check current token type
    bool match(TokenType type);        // Conditional token consumption
    bool match(const std::vector<TokenType>& types); // Multi-type match
    Token consume(TokenType type, const std::string& message); // Enforced consumption

    // Error recovery
    void synchronize();   // Resynchronize after parse errors

    // Grammar rule implementations
    std::unique_ptr<Program> parseProgram();      // Root production
    std::unique_ptr<Room> parseRoom();            // Room declaration
    std::unique_ptr<Item> parseItem();            // Item definition
    std::unique_ptr<FunctionStmt> parseFunction();// Function declaration
    std::unique_ptr<Statement> parseStatement();  // General statement
    std::unique_ptr<Statement> parseVarDeclaration(); // Variable declaration
    std::unique_ptr<BlockStmt> parseBlock();      // Code block { ... }
    std::unique_ptr<Statement> parseIfStatement();// Conditional
    std::unique_ptr<Statement> parseWhileStatement(); // Loop
    std::unique_ptr<Statement> parseReturnStatement();// Function return
    std::unique_ptr<Statement> parseSayStatement();   // Dialogue output
    std::unique_ptr<Statement> parseGotoStatement();  // Navigation
    std::unique_ptr<Statement> parseExpressionStatement(); // Expression as statement

    // Expression parsing (precedence climbing)
    std::unique_ptr<Expression> parseExpression();    // Entry point
    std::unique_ptr<Expression> parseAssignment();    // = operator
    std::unique_ptr<Expression> parseLogicalOr();     // ||
    std::unique_ptr<Expression> parseLogicalAnd();    // &&
    std::unique_ptr<Expression> parseEquality();      // == !=
    std::unique_ptr<Expression> parseComparison();    // < > <= >=
    std::unique_ptr<Expression> parseTerm();          // + -
    std::unique_ptr<Expression> parseFactor();        // * / %
    std::unique_ptr<Expression> parseUnary();         // ! -
    std::unique_ptr<Expression> parseCall();          // Function calls
    std::unique_ptr<Expression> parsePrimary();       // Atomic expressions

    // Function call helper
    std::unique_ptr<Expression> finishCall(std::unique_ptr<Expression> callee);
};

} // namespace storyscript

#endif // STORYSCRIPT_PARSER_H
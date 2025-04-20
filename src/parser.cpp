// Builds the code structure (AST) from tokens.
#include "../include/parser.h" // Parser definition
#include <iostream> // Error reporting
#include <vector> // Lists of nodes
#include <memory> // AST object management

namespace storyscript { // StoryScript parser

// Setup with lexer and get first token.
Parser::Parser(Lexer& lexer) : lexer(lexer) {
    advance();
}

// Move to the next token.
void Parser::advance() {
    previous = current;
    current = lexer.nextToken();
}

// Check current token type without consuming.
bool Parser::check(TokenType type) const {
    return current.type == type;
}

// Consume current token if type matches.
bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

// Consume current token if type matches any in list.
bool Parser::match(const std::vector<TokenType>& types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

// Consume current token if type matches, otherwise report error.
Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) {
        advance();
        return previous;
    }

    error(message);
    return current; // Return incorrect token on error
}

// Report a parsing error.
void Parser::error(const std::string& message) {
    errorOccurred = true;
    std::cerr << "Error at " << current.line << ":" << current.column
             << " - " << message << std::endl;
}

// Attempt to recover from an error by skipping tokens.
void Parser::synchronize() {
    advance(); // Skip the bad token

    // Skip until a likely statement/declaration start or end of file.
    while (!check(TokenType::EOF_TOKEN)) {
        if (previous.type == TokenType::SEMICOLON) return; // End of statement

        switch (current.type) {
            // Start of a known structure
            case TokenType::ROOM:
            case TokenType::ITEM:
            case TokenType::FUNCTION:
            case TokenType::VAR:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::RETURN:
            case TokenType::SAY:
            case TokenType::GOTO:
                return; // Found a sync point
            default:
                break; // Keep skipping
        }

        advance(); // Skip current token
    }
}

// Start building the AST.
std::unique_ptr<Program> Parser::parse() {
    return parseProgram();
}

// Parse the whole program structure.
std::unique_ptr<Program> Parser::parseProgram() {
    SourceLocation location{lexer.getCurrentLocation()};
    auto program = std::make_unique<Program>(location);

    // Parse top-level items until end of file.
    while (!check(TokenType::EOF_TOKEN)) {
        try {
            if (match(TokenType::ROOM)) program->addRoom(parseRoom());
            else if (match(TokenType::FUNCTION)) program->addFunction(parseFunction());
            else program->addStatement(parseStatement());
        } catch (...) {
            synchronize(); // Recover on error
        }
    }

    return program;
}

// Parse a 'room' definition.
std::unique_ptr<Room> Parser::parseRoom() {
    SourceLocation location{lexer.getCurrentLocation()};
    Token name = consume(TokenType::IDENTIFIER, "Expected room name.");

    consume(TokenType::LBRACE, "Expected '{' after room name.");

    // Room contents
    std::vector<std::pair<std::string, std::unique_ptr<Expression>>> properties;
    std::vector<std::unique_ptr<Item>> items;
    std::vector<std::pair<std::string, std::unique_ptr<BlockStmt>>> events;

    // Parse body until closing brace.
    while (!check(TokenType::RBRACE) && !check(TokenType::EOF_TOKEN)) {
        if (match(TokenType::ITEM)) {
            items.push_back(parseItem());
        } else if (match(TokenType::WHEN)) { // Event handler
            Token eventType = consume(TokenType::IDENTIFIER, "Expected event type after 'when'.");
            std::unique_ptr<BlockStmt> body = parseBlock();
            events.emplace_back(eventType.lexeme, std::move(body));
        } else { // Property
            Token propertyName = consume(TokenType::IDENTIFIER, "Expected property name.");
            consume(TokenType::COLON, "Expected ':' after property name.");
            auto value = parseExpression();
            consume(TokenType::SEMICOLON, "Expected ';' after property value.");
            properties.emplace_back(propertyName.lexeme, std::move(value));
        }
    }

    consume(TokenType::RBRACE, "Expected '}' after room body.");

    return std::make_unique<Room>(location, name, std::move(properties),
                                  std::move(items), std::move(events));
}

// Parse an 'item' definition.
std::unique_ptr<Item> Parser::parseItem() {
    SourceLocation location{lexer.getCurrentLocation()};
    Token name = consume(TokenType::IDENTIFIER, "Expected item name.");

    consume(TokenType::LBRACE, "Expected '{' after item name.");

    // Item properties
    std::vector<std::pair<std::string, std::unique_ptr<Expression>>> properties;

    // Parse body until closing brace.
    while (!check(TokenType::RBRACE) && !check(TokenType::EOF_TOKEN)) {
        // Property
        Token propertyName = consume(TokenType::IDENTIFIER, "Expected property name.");
        consume(TokenType::COLON, "Expected ':' after property name.");
        auto value = parseExpression();
        consume(TokenType::SEMICOLON, "Expected ';' after property value.");
        properties.emplace_back(propertyName.lexeme, std::move(value));
    }

    consume(TokenType::RBRACE, "Expected '}' after item body.");

    return std::make_unique<Item>(location, name, std::move(properties));
}

// Parse a 'function' definition.
std::unique_ptr<FunctionStmt> Parser::parseFunction() {
    SourceLocation location{lexer.getCurrentLocation()};
    Token name = consume(TokenType::IDENTIFIER, "Expected function name.");

    consume(TokenType::LPAREN, "Expected '(' after function name.");

    // Function parameters
    std::vector<Token> parameters;
    if (!check(TokenType::RPAREN)) { // If parameters exist
        do {
            parameters.push_back(consume(TokenType::IDENTIFIER, "Expected parameter name."));
        } while (match(TokenType::COMMA)); // More parameters?
    }

    consume(TokenType::RPAREN, "Expected ')' after parameters.");
    consume(TokenType::LBRACE, "Expected '{' before function body.");

    auto body = parseBlock(); // Function code block

    return std::make_unique<FunctionStmt>(location, name, std::move(parameters), std::move(body));
}

// Parse any statement type.
std::unique_ptr<Statement> Parser::parseStatement() {
    if (match(TokenType::IF)) return parseIfStatement();
    else if (match(TokenType::WHILE)) return parseWhileStatement();
    else if (match(TokenType::VAR)) return parseVarDeclaration();
    else if (match(TokenType::LBRACE)) return parseBlock();
    else if (match(TokenType::RETURN)) return parseReturnStatement();
    else if (match(TokenType::SAY)) return parseSayStatement();
    else if (match(TokenType::GOTO)) return parseGotoStatement();
    else return parseExpressionStatement(); // Default is expression statement
}

// Parse a variable declaration.
std::unique_ptr<Statement> Parser::parseVarDeclaration() {
    SourceLocation location{lexer.getCurrentLocation()};
    Token name = consume(TokenType::IDENTIFIER, "Expected variable name.");

    std::unique_ptr<Expression> initializer = nullptr; // Optional initializer
    if (match(TokenType::ASSIGN)) {
        initializer = parseExpression();
    }

    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration.");
    return std::make_unique<VarStmt>(location, name, std::move(initializer));
}

// Parse a code block { ... }.
std::unique_ptr<BlockStmt> Parser::parseBlock() {
    SourceLocation location{lexer.getCurrentLocation()};
    std::vector<std::unique_ptr<Statement>> statements;

    // Parse statements until closing brace.
    while (!check(TokenType::RBRACE) && !check(TokenType::EOF_TOKEN)) {
        statements.push_back(parseStatement());
    }

    consume(TokenType::RBRACE, "Expected '}' after block.");
    return std::make_unique<BlockStmt>(location, std::move(statements));
}

// Parse an if statement.
std::unique_ptr<Statement> Parser::parseIfStatement() {
    SourceLocation location{lexer.getCurrentLocation()};
    consume(TokenType::LPAREN, "Expected '(' after 'if'.");
    auto condition = parseExpression(); // If condition
    consume(TokenType::RPAREN, "Expected ')' after if condition.");

    auto thenBranch = parseStatement(); // Code if true
    std::unique_ptr<Statement> elseBranch = nullptr; // Optional else

    if (match(TokenType::ELSE)) {
        elseBranch = parseStatement(); // Code if false
    }

    return std::make_unique<IfStmt>(location, std::move(condition),
                                   std::move(thenBranch), std::move(elseBranch));
}

// Parse a while loop.
std::unique_ptr<Statement> Parser::parseWhileStatement() {
    SourceLocation location{lexer.getCurrentLocation()};
    consume(TokenType::LPAREN, "Expected '(' after 'while'.");
    auto condition = parseExpression(); // Loop condition
    consume(TokenType::RPAREN, "Expected ')' after while condition.");

    auto body = parseStatement(); // Loop body

    return std::make_unique<WhileStmt>(location, std::move(condition), std::move(body));
}

// Parse a return statement.
std::unique_ptr<Statement> Parser::parseReturnStatement() {
    SourceLocation location{lexer.getCurrentLocation()};
    Token keyword = previous; // The 'return' token

    std::unique_ptr<Expression> value = nullptr; // Optional return value
    if (!check(TokenType::SEMICOLON)) {
        value = parseExpression();
    }

    consume(TokenType::SEMICOLON, "Expected ';' after return value.");
    return std::make_unique<ReturnStmt>(location, keyword, std::move(value));
}

// Parse a 'say' statement.
std::unique_ptr<Statement> Parser::parseSayStatement() {
    SourceLocation location{lexer.getCurrentLocation()};
    auto message = parseExpression(); // What to say
    consume(TokenType::SEMICOLON, "Expected ';' after message.");
    return std::make_unique<SayStmt>(location, std::move(message));
}

// Parse a 'goto' statement.
std::unique_ptr<Statement> Parser::parseGotoStatement() {
    SourceLocation location{lexer.getCurrentLocation()};
    consume(TokenType::LPAREN, "Expected '(' after 'goto'.");
    auto destination = parseExpression(); // Where to go
    consume(TokenType::RPAREN, "Expected ')' after goto destination.");
    consume(TokenType::SEMICOLON, "Expected ';' after goto statement.");
    return std::make_unique<GotoStmt>(location, std::move(destination));
}

// Parse an expression statement (expr followed by ';').
std::unique_ptr<Statement> Parser::parseExpressionStatement() {
    SourceLocation location{lexer.getCurrentLocation()};
    auto expr = parseExpression(); // The expression
    consume(TokenType::SEMICOLON, "Expected ';' after expression.");
    return std::make_unique<ExpressionStmt>(location, std::move(expr));
}

// Parse any expression (starts precedence chain).
std::unique_ptr<Expression> Parser::parseExpression() {
    return parseAssignment();
}

// Parse assignment expressions.
std::unique_ptr<Expression> Parser::parseAssignment() {
    auto expr = parseLogicalOr(); // Left side
    if (match(TokenType::ASSIGN)) { // If '='
        Token equals = previous;
        auto value = parseAssignment(); // Right side (recursive)
        if (auto* varExpr = dynamic_cast<VariableExpr*>(expr.get())) {
            return std::make_unique<BinaryExpr>(
                expr->location,
                std::move(expr),
                equals,
                std::move(value)
            ); // Assignment AST node
        }
        error("Invalid assignment target."); // Must be variable
    }
    return expr; // Not assignment
}

// Parse logical OR ( 'or' ).
std::unique_ptr<Expression> Parser::parseLogicalOr() {
    auto expr = parseLogicalAnd(); // Left side
    while (match(TokenType::OR)) { // While 'or' exists
        Token op = previous;
        auto right = parseLogicalAnd(); // Right side
        expr = std::make_unique<BinaryExpr>(expr->location, std::move(expr), op, std::move(right));
    }
    return expr;
}

// Parse logical AND ( 'and' ).
std::unique_ptr<Expression> Parser::parseLogicalAnd() {
    auto expr = parseEquality(); // Left side
    while (match(TokenType::AND)) { // While 'and' exists
        Token op = previous;
        auto right = parseEquality(); // Right side
        expr = std::make_unique<BinaryExpr>(expr->location, std::move(expr), op, std::move(right));
    }
    return expr;
}

// Parse equality ( '==' | '!=' ).
std::unique_ptr<Expression> Parser::parseEquality() {
    auto expr = parseComparison(); // Left side
    while (match({TokenType::EQ, TokenType::NEQ})) { // While '==' or '!=' exists
        Token op = previous;
        auto right = parseComparison(); // Right side
        expr = std::make_unique<BinaryExpr>(expr->location, std::move(expr), op, std::move(right));
    }
    return expr;
}

// Parse comparisons ( '>' | '>=' | '<' | '<=' ).
std::unique_ptr<Expression> Parser::parseComparison() {
    auto expr = parseTerm(); // Left side
    while (match({TokenType::GT, TokenType::GTE, TokenType::LT, TokenType::LTE})) { // While comparison op exists
        Token op = previous;
        auto right = parseTerm(); // Right side
        expr = std::make_unique<BinaryExpr>(expr->location, std::move(expr), op, std::move(right));
    }
    return expr;
}

// Parse addition/subtraction ( '+' | '-' ).
std::unique_ptr<Expression> Parser::parseTerm() {
    auto expr = parseFactor(); // Left side
    while (match({TokenType::PLUS, TokenType::MINUS})) { // While '+' or '-' exists
        Token op = previous;
        auto right = parseFactor(); // Right side
        expr = std::make_unique<BinaryExpr>(expr->location, std::move(expr), op, std::move(right));
    }
    return expr;
}

// Parse multiplication/division/modulo ( '*' | '/' | '%' ).
std::unique_ptr<Expression> Parser::parseFactor() {
    auto expr = parseUnary(); // Left side
    while (match({TokenType::MULTIPLY, TokenType::DIVIDE, TokenType::MODULO})) { // While '*', '/', or '%' exists
        Token op = previous;
        auto right = parseUnary(); // Right side
        expr = std::make_unique<BinaryExpr>(expr->location, std::move(expr), op, std::move(right));
    }
    return expr;
}

// Parse unary ops ( '-' | '!' ) or call/primary.
std::unique_ptr<Expression> Parser::parseUnary() {
    if (match({TokenType::MINUS, TokenType::NOT})) { // If unary op
        Token op = previous;
        auto right = parseUnary(); // Operand (recursive)
        return std::make_unique<UnaryExpr>(right->location, op, std::move(right));
    }
    return parseCall(); // Not unary
}

// Parse function calls or property access.
std::unique_ptr<Expression> Parser::parseCall() {
    auto expr = parsePrimary(); // Base (function/object)
    while (true) {
        if (match(TokenType::LPAREN)) { // If '(' -> Call
            expr = finishCall(std::move(expr));
        } else if (match(TokenType::DOT)) { // If '.' -> Property access
            Token name = consume(TokenType::IDENTIFIER, "Expected property name after '.'.");
            SourceLocation location{lexer.getCurrentLocation()};
            // Simplified: represents property access as variable lookup
            expr = std::make_unique<VariableExpr>(location, name);
        } else {
            break; // No more calls/accesses
        }
    }
    return expr;
}

// Helper to finish parsing call arguments.
std::unique_ptr<Expression> Parser::finishCall(std::unique_ptr<Expression> callee) {
    std::vector<std::unique_ptr<Expression>> arguments;
    if (!check(TokenType::RPAREN)) { // If arguments exist
        do {
            arguments.push_back(parseExpression());
        } while (match(TokenType::COMMA)); // More arguments?
    }
    Token paren = consume(TokenType::RPAREN, "Expected ')' after arguments.");
    return std::make_unique<CallExpr>(
        callee->location,
        std::move(callee),
        paren,
        std::move(arguments)
    );
}

// Parse basic elements: literals, identifiers, grouped expressions.
std::unique_ptr<Expression> Parser::parsePrimary() {
    SourceLocation location{lexer.getCurrentLocation()};

    if (match(TokenType::FALSE)) return std::make_unique<LiteralExpr>(location, false);
    if (match(TokenType::TRUE)) return std::make_unique<LiteralExpr>(location, true);
    if (match(TokenType::NUMBER)) { // Number literal
        double value = std::stod(previous.lexeme);
        return std::make_unique<LiteralExpr>(location, value);
    }
    if (match(TokenType::STRING)) { // String literal
        std::string value = previous.lexeme.substr(1, previous.lexeme.length() - 2); // Strip quotes
        return std::make_unique<LiteralExpr>(location, value);
    }

    if (match(TokenType::IDENTIFIER)) return std::make_unique<VariableExpr>(location, previous); // Identifier (variable)

    if (match(TokenType::LPAREN)) { // Grouped expression ( expr )
        auto expr = parseExpression();
        consume(TokenType::RPAREN, "Expected ')' after expression.");
        return expr; // Return inner expression
    }

    error("Expected expression."); // Nothing matched
    return nullptr; // Should be handled by error recovery
}

} // namespace storyscript
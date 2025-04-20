// Defines the Abstract Syntax Tree (AST) node types for StoryScript.
#ifndef STORYSCRIPT_AST_H
#define STORYSCRIPT_AST_H

#include <memory>
#include <string>
#include <vector>
#include <variant>
#include "token.h"

namespace storyscript
{ // StoryScript language structure

    // Forward declarations
    class Expression;
    class Statement;
    class Room;
    class Item;

    // Holds literal values (number, string, or boolean).
    using Value = std::variant<double, std::string, bool>;

    /// --- Base Node ---

    // Base class for all AST nodes. Tracks source location.
    class ASTNode
    {
    public:
        virtual ~ASTNode() = default; // Allow deleting derived objects correctly
        SourceLocation location;      // Where this code came from in the source

        explicit ASTNode(const SourceLocation &location)
            : location(location) {}
    };

    /// --- Expressions ---

    // Base for nodes that produce a value.
    class Expression : public ASTNode
    {
    public:
        explicit Expression(const SourceLocation &location)
            : ASTNode(location) {}
        virtual ~Expression() = default;
    };

    // A constant value (like 123, "hello", true).
    class LiteralExpr : public Expression
    {
    public:
        Value value; // The actual value

        LiteralExpr(const SourceLocation &location, const Value &value)
            : Expression(location), value(value) {}
    };

    // A variable name reference.
    class VariableExpr : public Expression
    {
    public:
        Token name; // The variable's name token

        VariableExpr(const SourceLocation &location, const Token &name)
            : Expression(location), name(name) {}
    };

    // An operation with two parts (like a + b, x == y).
    class BinaryExpr : public Expression
    {
    public:
        std::unique_ptr<Expression> left;  // First part
        Token op;                          // The operator symbol
        std::unique_ptr<Expression> right; // Second part

        BinaryExpr(const SourceLocation &location,
                   std::unique_ptr<Expression> left,
                   const Token &op,
                   std::unique_ptr<Expression> right)
            : Expression(location), left(std::move(left)),
              op(op), right(std::move(right)) {}
    };

    // An operation with one part (like -x, !condition).
    class UnaryExpr : public Expression
    {
    public:
        Token op;                          // The operator symbol
        std::unique_ptr<Expression> right; // The part the operator applies to

        UnaryExpr(const SourceLocation &location,
                  const Token &op,
                  std::unique_ptr<Expression> right)
            : Expression(location), op(op), right(std::move(right)) {}
    };

    // A function or method call (like func(arg1, arg2)).
    class CallExpr : public Expression
    {
    public:
        std::unique_ptr<Expression> callee;                 // The function/method itself
        Token paren;                                        // The closing parenthesis token (for location)
        std::vector<std::unique_ptr<Expression>> arguments; // The values passed in

        CallExpr(const SourceLocation &location,
                 std::unique_ptr<Expression> callee,
                 const Token &paren,
                 std::vector<std::unique_ptr<Expression>> arguments)
            : Expression(location), callee(std::move(callee)),
              paren(paren), arguments(std::move(arguments)) {}
    };

    /// --- Statements ---

    // Base for nodes that perform an action but don't necessarily produce a value.
    class Statement : public ASTNode
    {
    public:
        explicit Statement(const SourceLocation &location)
            : ASTNode(location) {}
        virtual ~Statement() = default;
    };

    // An expression used as a statement (e.g., a function call like 'say("hello");').
    class ExpressionStmt : public Statement
    {
    public:
        std::unique_ptr<Expression> expression; // The expression being used

        ExpressionStmt(const SourceLocation &location,
                       std::unique_ptr<Expression> expression)
            : Statement(location), expression(std::move(expression)) {}
    };

    // Variable declaration (e.g., 'var x = 5;').
    class VarStmt : public Statement
    {
    public:
        Token name;                              // The variable's name
        std::unique_ptr<Expression> initializer; // The starting value (optional)

        VarStmt(const SourceLocation &location,
                const Token &name,
                std::unique_ptr<Expression> initializer)
            : Statement(location), name(name),
              initializer(std::move(initializer)) {}
    };

    // A block of code enclosed in {}.
    class BlockStmt : public Statement
    {
    public:
        std::vector<std::unique_ptr<Statement>> statements; // List of statements inside

        BlockStmt(const SourceLocation &location,
                  std::vector<std::unique_ptr<Statement>> statements)
            : Statement(location), statements(std::move(statements)) {}
    };

    // An if/else condition.
    class IfStmt : public Statement
    {
    public:
        std::unique_ptr<Expression> condition; // True/false test
        std::unique_ptr<Statement> thenBranch; // Code if true
        std::unique_ptr<Statement> elseBranch; // Code if false (optional)

        IfStmt(const SourceLocation &location,
               std::unique_ptr<Expression> condition,
               std::unique_ptr<Statement> thenBranch,
               std::unique_ptr<Statement> elseBranch)
            : Statement(location), condition(std::move(condition)),
              thenBranch(std::move(thenBranch)),
              elseBranch(std::move(elseBranch)) {}
    };

    // A while loop.
    class WhileStmt : public Statement
    {
    public:
        std::unique_ptr<Expression> condition; // Loop continues while true
        std::unique_ptr<Statement> body;       // Code to repeat

        WhileStmt(const SourceLocation &location,
                  std::unique_ptr<Expression> condition,
                  std::unique_ptr<Statement> body)
            : Statement(location), condition(std::move(condition)),
              body(std::move(body)) {}
    };

    // A function definition.
    class FunctionStmt : public Statement
    {
    public:
        Token name;                      // Function name
        std::vector<Token> params;       // Parameter names
        std::unique_ptr<BlockStmt> body; // Function code

        FunctionStmt(const SourceLocation &location,
                     Token name,
                     std::vector<Token> params,
                     std::unique_ptr<BlockStmt> body)
            : Statement(location), name(name), params(std::move(params)),
              body(std::move(body)) {}
    };

    // A return statement.
    class ReturnStmt : public Statement
    {
    public:
        Token keyword;                     // The 'return' token
        std::unique_ptr<Expression> value; // The value to return (optional)

        ReturnStmt(const SourceLocation &location,
                   const Token &keyword,
                   std::unique_ptr<Expression> value)
            : Statement(location), keyword(keyword),
              value(std::move(value)) {}
    };

    // The 'say' command for dialogue/output.
    class SayStmt : public Statement
    {
    public:
        std::unique_ptr<Expression> message; // The text to display

        SayStmt(const SourceLocation &location,
                std::unique_ptr<Expression> message)
            : Statement(location), message(std::move(message)) {}
    };

    // The 'goto' command for navigation.
    class GotoStmt : public Statement
    {
    public:
        std::unique_ptr<Expression> destination; // The target room

        GotoStmt(const SourceLocation &location,
                 std::unique_ptr<Expression> destination)
            : Statement(location), destination(std::move(destination)) {}
    };

    /// --- Storyscript Structures ---

    // A room definition.
    class Room : public ASTNode
    {
    public:
        Token name; // Room name
        // Properties (like "description" = "...")
        std::vector<std::pair<std::string, std::unique_ptr<Expression>>> properties;
        std::vector<std::unique_ptr<Item>> items; // Items inside the room
        // Event handlers (like "when entered { ... }")
        std::vector<std::pair<std::string, std::unique_ptr<BlockStmt>>> events;

        Room(const SourceLocation &location,
             const Token &name,
             std::vector<std::pair<std::string, std::unique_ptr<Expression>>> properties,
             std::vector<std::unique_ptr<Item>> items,
             std::vector<std::pair<std::string, std::unique_ptr<BlockStmt>>> events)
            : ASTNode(location), name(name), properties(std::move(properties)),
              items(std::move(items)), events(std::move(events)) {}
    };

    // An item definition.
    class Item : public ASTNode
    {
    public:
        Token name; // Item name
        // Properties (like "weight" = 10)
        std::vector<std::pair<std::string, std::unique_ptr<Expression>>> properties;

        Item(const SourceLocation &location,
             const Token &name,
             std::vector<std::pair<std::string, std::unique_ptr<Expression>>> properties)
            : ASTNode(location), name(name), properties(std::move(properties)) {}
    };

    /// --- Program Root ---

    // The top-level container for the whole script.
    class Program : public ASTNode
    {
    public:
        std::vector<std::unique_ptr<Room>> rooms;             // All rooms
        std::vector<std::unique_ptr<Statement>> statements;   // Global statements
        std::vector<std::unique_ptr<FunctionStmt>> functions; // Global functions

        Program(const SourceLocation &location)
            : ASTNode(location) {}

        // Methods to add nodes to the program
        void addRoom(std::unique_ptr<Room> room)
        {
            rooms.push_back(std::move(room));
        }

        void addStatement(std::unique_ptr<Statement> stmt)
        {
            statements.push_back(std::move(stmt));
        }

        void addFunction(std::unique_ptr<FunctionStmt> func)
        {
            functions.push_back(std::move(func));
        }
    };

} // namespace storyscript

#endif // STORYSCRIPT_AST_H
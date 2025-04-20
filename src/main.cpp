#include <iostream>
#include <fstream>
#include <sstream>
#include "../include/lexer.h"
#include "../include/parser.h"

using namespace storyscript;

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Could not open file: " << path << std::endl;
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void printTokens(const std::vector<Token>& tokens) {
    for (const auto& token : tokens) {
        std::cout << token.toString() << std::endl;
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <script.story>" << std::endl;
        return 1;
    }
    
    std::string source = readFile(argv[1]);
    if (source.empty()) {
        return 1;
    }
    
    // Create lexer and tokenize the source
    Lexer lexer(source, argv[1]);
    std::vector<Token> tokens = lexer.tokenize();
    
    std::cout << "===== Tokens =====" << std::endl;
    printTokens(tokens);
    
    // Reset lexer and create parser
    Lexer lexer2(source, argv[1]);
    Parser parser(lexer2);
    
    std::cout << "\n===== Parsing =====" << std::endl;
    auto program = parser.parse();
    
    if (parser.hadError()) {
        std::cout << "Parsing failed with errors." << std::endl;
        return 1;
    } else {
        std::cout << "Parsing completed successfully!" << std::endl;
        // We could add AST pretty-printing here in a full implementation
    }
    
    return 0;
}
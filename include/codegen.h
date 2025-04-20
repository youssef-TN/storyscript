// IRGenerator.h
// Translates the code structure (AST) into LLVM Intermediate Representation (IR).

#ifndef IR_GENERATOR_H
#define IR_GENERATOR_H

// LLVM and standard library tools for IR generation
#include <map>					 // Variable storage
#include <string>				 // Text handling
#include "AST.h"				 // Code structure definitions
#include "llvm/IR/LLVMContext.h" // LLVM core
#include "llvm/IR/Module.h"		 // Generated code container
#include "llvm/IR/IRBuilder.h"	 // Instruction builder
#include "llvm/IR/Verifier.h"	 // IR validator

// Generates LLVM IR from the AST.
class IRGenerator
{
private:
	// LLVM components
	llvm::LLVMContext Context;
	llvm::Module *TheModule;
	llvm::IRBuilder<> Builder;

	// Maps variable names to their IR values
	std::map<std::string, llvm::Value *> NamedValues;

	/// --- Helper functions ---
	
	// Get LLVM type for a language type name.
	llvm::Type *getLLVMType(const std::string &TypeName);

	// Add declarations for runtime functions.
	void generateRuntimeDeclarations();

	/// --- IR generation for specific code elements ---
	
	// Generate IR for a variable declaration.
	llvm::Value *generateVariableDeclIR(VariableDeclAST *VarDecl);

	// Generate IR for a function declaration.
	llvm::Value *generateFunctionDeclIR(FunctionDeclAST *FuncDecl);

	// Generate IR for a 'Room' definition.
	llvm::Value *generateRoomDeclIR(RoomDeclAST *RoomDecl);

	// Generate IR for an 'if' statement.
	llvm::Value *generateIfStmtIR(IfStmtAST *IfStmt);

	// Generate IR for any AST node.
	llvm::Value *generateNodeIR(AST *Node);

	// Add more methods for other AST node types
	// (e.g., loops, operations, calls).

public:
	// Constructor: set up with a module name.
	IRGenerator(const std::string &ModuleName);

	// Destructor.
	~IRGenerator();

	// Start generating IR from the root AST node.
	void generateIR(AST *Root);

	// Get the generated LLVM module.
	llvm::Module *getModule() { return TheModule; }

	/// --- Output methods ---
	
	// Print IR to console.
	void dumpIR();

	// Save IR as text to a file.
	bool writeIRToFile(const std::string &Filename);

	// Save IR as binary (bitcode) to a file.
	bool writeBitcodeToFile(const std::string &Filename);
};

#endif // IR_GENERATOR_H

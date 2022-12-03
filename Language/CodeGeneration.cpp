#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "CodeGeneration.hpp"
#include "Parser.hpp"

std::unique_ptr<llvm::LLVMContext> CodeGeneration::TheContext;
std::unique_ptr<llvm::IRBuilder<>> CodeGeneration::Builder;
std::unique_ptr<llvm::Module> CodeGeneration::TheModule;
std::map<std::string, llvm::Value *> CodeGeneration::NamedValues;
std::unique_ptr<llvm::legacy::FunctionPassManager> CodeGeneration::TheFPM;
bool CodeGeneration::isPureNumber = false;
int CodeGeneration::lastPureInt = 0;
llvm::Value* CodeGeneration::lastLLVMInOp = nullptr;
std::unique_ptr<llvm::JITCompiler> CodeGeneration::TheJIT;

llvm::Value* CodeGeneration::LogErrorV(std::string str)
{
	Parser::LogError(str);
	return nullptr;
}

llvm::Function* CodeGeneration::LogErrorFLLVM(std::string str)
{
	Parser::LogError(str);
	return nullptr;
}

void CodeGeneration::Initialize()
{
 	// Open a new context and module.
 	TheContext = std::make_unique<llvm::LLVMContext>();
 	TheModule = std::make_unique<llvm::Module>("Nucleus", *TheContext);

 	 // Create a new builder for the module.
 	Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);

 	TheFPM = std::make_unique<llvm::legacy::FunctionPassManager>(TheModule.get());

 	// ==========================
 	// ENABLE BASIC OPTIMIZATIONS
 	// ==========================

 	// Do simple optimizations and bit-twiddling.
 	TheFPM->add(llvm::createInstructionCombiningPass());

 	// Reassociate the expressions.
 	TheFPM->add(llvm::createReassociatePass());

 	// Delete Common SubExpressions.
 	TheFPM->add(llvm::createGVNPass());

 	// Simplify the control flow graph (deleting unreacable blocks, etc.)
 	TheFPM->add(llvm::createCFGSimplificationPass());

 	TheFPM->doInitialization();
}
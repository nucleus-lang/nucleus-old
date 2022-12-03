#ifndef CODE_GEN_H
#define CODE_GEN_H

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "JITCompiler.h"
#include <map>

struct CodeGeneration
{
	static std::unique_ptr<llvm::LLVMContext> TheContext;
	static std::unique_ptr<llvm::IRBuilder<>> Builder;
	static std::unique_ptr<llvm::Module> TheModule;
	static std::map<std::string, llvm::Value *> NamedValues;
	static std::unique_ptr<llvm::legacy::FunctionPassManager> TheFPM;
	static std::unique_ptr<llvm::JITCompiler> TheJIT;

	static bool isPureNumber;
	static int lastPureInt;
	static llvm::Value* lastLLVMInOp;

	static llvm::Value* LogErrorV(std::string str);
	static llvm::Function* LogErrorFLLVM(std::string str);

	static void Initialize();
};

#endif
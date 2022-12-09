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
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Utils.h"
#include "JITCompiler.hpp"
#include <map>

struct CodeGeneration
{
	static std::unique_ptr<llvm::LLVMContext> TheContext;
	static std::unique_ptr<llvm::IRBuilder<>> Builder;
	static std::unique_ptr<llvm::Module> TheModule;
	static std::map<std::string, llvm::AllocaInst*> NamedValues;
	static std::unique_ptr<llvm::legacy::FunctionPassManager> TheFPM;
	static std::unique_ptr<llvm::orc::JITCompiler> TheJIT;
	static llvm::ExitOnError ExitOnErr;

	static bool isPureNumber;
	static int lastPureInt;
	static llvm::Value* lastLLVMInOp;

	static llvm::Function* GetFunction(std::string name);

	static llvm::Value* LogErrorV(std::string str);
	static llvm::Function* LogErrorFLLVM(std::string str);

	static llvm::AllocaInst* CreateEntryAllocation(llvm::Function* TheFunction, const std::string& VarName);

	static void StartJIT();
	static void Initialize();
};

#endif
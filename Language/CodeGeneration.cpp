#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "CodeGeneration.hpp"
#include "Parser.hpp"
#include "AbstractSyntaxTree.hpp"

std::unique_ptr<llvm::LLVMContext> CodeGeneration::TheContext;
std::unique_ptr<llvm::IRBuilder<>> CodeGeneration::Builder;
std::unique_ptr<llvm::Module> CodeGeneration::TheModule;
std::map<std::string, llvm::AllocaInst*> CodeGeneration::NamedValues;
std::unique_ptr<llvm::legacy::FunctionPassManager> CodeGeneration::TheFPM;
bool CodeGeneration::isPureNumber = false;
int CodeGeneration::lastPureInt = 0;
llvm::Value* CodeGeneration::lastLLVMInOp = nullptr;
std::unique_ptr<llvm::orc::JITCompiler> CodeGeneration::TheJIT;
llvm::ExitOnError CodeGeneration::ExitOnErr;

std::unique_ptr<llvm::DIBuilder> CodeGeneration::DBuilder;

llvm::DICompileUnit* CodeGeneration::DebugInfo::TheCU;
llvm::DIType* CodeGeneration::DebugInfo::DblTy, *CodeGeneration::DebugInfo::IntTy, *CodeGeneration::DebugInfo::FloTy;

std::vector<llvm::DIScope*> CodeGeneration::DebugInfo::LexicalBlocks;

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

llvm::Function* CodeGeneration::GetFunction(std::string name)
{
	//std::cout << "Looking for Function '" << name << "'...\n";

	if(auto* F = TheModule->getFunction(name))
	{
		//std::cout << "'" << name << "' Function Found!\n";
		return F;
	}

	//std::cout << "Existing function called '" << name << "' not found, finding Prototype...\n";

	auto FI = AST::FunctionProtos.find(name);
	if(FI != AST::FunctionProtos.end())
	{
		//std::cout << "Generating CodeGen...\n";
		return FI->second->codegen();
	}

	//std::cout << "Function " << name << " not found!\n";

	return nullptr;
}

llvm::AllocaInst* CodeGeneration::CreateEntryAllocation(llvm::Function* TheFunction, const std::string& VarName)
{
	  llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                 TheFunction->getEntryBlock().begin());
  return TmpB.CreateAlloca(TheFunction->getFunctionType()->getReturnType(), 0,
                           VarName.c_str());
}

void CodeGeneration::StartJIT()
{
	TheJIT = ExitOnErr(llvm::orc::JITCompiler::Create());
}

void CodeGeneration::Initialize()
{
 	// Open a new context and module.
 	TheContext = std::make_unique<llvm::LLVMContext>();

 	TheModule = std::make_unique<llvm::Module>("Nucleus", *TheContext);
 	TheModule->setDataLayout(TheJIT->getDataLayout());

 	 // Create a new builder for the module.
 	Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);

 	TheFPM = std::make_unique<llvm::legacy::FunctionPassManager>(TheModule.get());

#if 1

 	llvm::PassManagerBuilder Builder;

 	Builder.OptLevel = 3;
 	Builder.SizeLevel = 1;

 	Builder.Inliner = llvm::createFunctionInliningPass(Builder.OptLevel, Builder.SizeLevel, false);

 	//TheModule->adjustPassManager(Builder);

 	//TheFPM->add(llvm::createStripDeadDebugInfoPass());

 	// ==========================
 	// ENABLE BASIC OPTIMIZATIONS
 	// ==========================

 	//TheFPM->add(llvm::createConstantPropagationPass());

 	for(int i = 0; i < 3; i++)
 	{
 		Builder.populateFunctionPassManager(*TheFPM);

 		TheFPM->add(llvm::createIndVarSimplifyPass());
		TheFPM->add(llvm::createDeadCodeEliminationPass());
		TheFPM->add(llvm::createLoopSimplifyPass());

		// Do simple optimizations and bit-twiddling.
 		TheFPM->add(llvm::createInstructionCombiningPass());

 		// Reassociate the expressions.
 		//TheFPM->add(llvm::createReassociatePass());

 		// Delete Common SubExpressions.
 		TheFPM->add(llvm::createGVNPass());

 		// Simplify the control flow graph (deleting unreacable blocks, etc.)
 		TheFPM->add(llvm::createCFGSimplificationPass());
 	}

#endif

 	TheFPM->doInitialization();

 	TheModule->addModuleFlag(llvm::Module::Warning, "Debug Info Version", llvm::DEBUG_METADATA_VERSION);

 	if(llvm::Triple(llvm::sys::getProcessTriple()).isOSDarwin())
 		TheModule->addModuleFlag(llvm::Module::Warning, "Dwarf Version", 2);

 	DBuilder = std::make_unique<llvm::DIBuilder>(*TheModule);

 	CodeGeneration::DebugInfo::TheCU = DBuilder->createCompileUnit(

 		llvm::dwarf::DW_LANG_C, DBuilder->createFile("main.nk", "."),
 		"Nucleus Compiler", false, "", 0

 		);
}

void CodeGeneration::CompileToObjectCode()
{
	////std::cout << "Compiling...\n";

#if 0
	llvm::InitializeAllTargetInfos();
  	llvm::InitializeAllTargets();
  	llvm::InitializeAllTargetMCs();
  	llvm::InitializeAllAsmParsers();
  	llvm::InitializeAllAsmPrinters();

  	auto TargetTriple = llvm::sys::getDefaultTargetTriple();

  	TheModule->setTargetTriple(TargetTriple);

  	std::string Error;
  	auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

  	if(!Target)
  	{
  		llvm::errs() << Error;
  		exit(1);
  	}

  	auto CPU = "generic";
  	auto Features = "";

  	llvm::TargetOptions opt;

  	auto RM = llvm::Optional<llvm::Reloc::Model>();
  	auto TargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

  	TheModule->setDataLayout(TargetMachine->createDataLayout());
  	TheModule->setTargetTriple(TargetTriple);

  	auto FileName = "output.o";

  	std::error_code EC;
  	llvm::raw_fd_ostream dest(FileName, EC, llvm::sys::fs::OF_None);

  	if(EC)
  	{
  		llvm::errs() << "Could not open file: " << EC.message();
  		exit(1);
  	}

  	llvm::legacy::PassManager pass;
  	auto FileType = llvm::CGFT_ObjectFile;

  	if(TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType))
  	{
  		llvm::errs() << "TargetMachine can't emit a file of this type.";
  		exit(1);
  	}

  	pass.run(*TheModule);
  	dest.flush();

  	//std::cout << FileName << " succesfully compiled!\n";

 #endif

	DBuilder->finalize();

	TheModule->print(llvm::errs(), nullptr);
}

llvm::DIType* CodeGeneration::DebugInfo::getDoubleTy()
{
	if(DblTy)
		return DblTy;

	DblTy = DBuilder->createBasicType("double", 64, llvm::dwarf::DW_ATE_float);
	return DblTy;
}

llvm::DIType* CodeGeneration::DebugInfo::getIntegerTy()
{
	if(IntTy)
		return IntTy;

	IntTy = DBuilder->createBasicType("int", 32, llvm::dwarf::DW_ATE_float);
	return IntTy;
}

llvm::DIType* CodeGeneration::DebugInfo::getFloatTy()
{
	if(FloTy)
		return FloTy;

	FloTy = DBuilder->createBasicType("float", 64, llvm::dwarf::DW_ATE_float);
	return FloTy;
}
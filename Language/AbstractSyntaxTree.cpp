#include "AbstractSyntaxTree.hpp"
#include "Parser.hpp"

std::map<std::string, std::unique_ptr<AST::FunctionPrototype>> AST::FunctionProtos;

llvm::Value* AST::Number::codegen()
{
	//std::cout << "CodeGen Number...\n";

	if(isDouble)
	{
		CodeGeneration::isPureNumber = false;
		return llvm::ConstantFP::get(*CodeGeneration::TheContext, llvm::APFloat(doubleValue));
	}
	else if(isFloat)
	{
		CodeGeneration::isPureNumber = false;
		return llvm::ConstantFP::get(*CodeGeneration::TheContext, llvm::APFloat(floatValue));
	}
	else if(isInt)
	{
		//return llvm::ConstantFP::get(*CodeGeneration::TheContext, llvm::APFloat((double)intValue));

		llvm::Type *i32_type = llvm::IntegerType::getInt32Ty(*CodeGeneration::TheContext);
		CodeGeneration::isPureNumber = true;
		return llvm::ConstantInt::get(i32_type, intValue, true);
	}

	return llvm::ConstantFP::get(*CodeGeneration::TheContext, llvm::APFloat(doubleValue));
}

llvm::Value* AST::Integer::codegen()
{
	//std::cout << "CodeGen Integer...\n";
	CodeGeneration::isPureNumber = false;
	return llvm::ConstantInt::get(*CodeGeneration::TheContext, llvm::APInt(32, value, true));
}

llvm::Value* AST::Float::codegen()
{
	//std::cout << "CodeGen Float...\n";
	CodeGeneration::isPureNumber = false;
	return llvm::ConstantFP::get(*CodeGeneration::TheContext, llvm::APFloat(value));
}

llvm::Value* AST::Double::codegen()
{
	//std::cout << "CodeGen Double...\n";
	CodeGeneration::isPureNumber = false;
	return llvm::ConstantFP::get(*CodeGeneration::TheContext, llvm::APFloat(value));
}

llvm::Value* AST::Variable::codegen()
{
	//std::cout << "CodeGen Variable...\n";

	llvm::Value* V = CodeGeneration::NamedValues[name];

	bool isInt = V->getType()->isIntegerTy();

	if(isInt)
		CodeGeneration::isPureNumber = true;
	else
		CodeGeneration::isPureNumber = false;

	if(!V)
		CodeGeneration::LogErrorV("Unknown variable name.\n");

	return V;
}

llvm::Value* AST::Binary::codegen()
{
	//std::cout << "CodeGen Binary...\n";

	int pureIntCount = 0;

	llvm::Value* L = lhs->codegen();

	if(CodeGeneration::isPureNumber)
		pureIntCount++;

	llvm::Value* R = rhs->codegen();

	if(CodeGeneration::isPureNumber)
		pureIntCount++;

	llvm::Value* opLLVM = nullptr;

	if(!L || !R)
	{
		//std::cout << "Warning: One of the Values is nullptr.\n";
		return nullptr;
	}

	if(op == '+')
	{
		if(pureIntCount == 2)
		{
			opLLVM = CodeGeneration::Builder->CreateAdd(L, R, "addtmp");
		}
		else
		{
			opLLVM = CodeGeneration::Builder->CreateFAdd(L, R, "addtmp");
		}
	}
	else if(op == '-')
	{
		if(pureIntCount == 2)
		{
			opLLVM = CodeGeneration::Builder->CreateSub(L, R, "subtmp");
		}
		else
		{
			opLLVM = CodeGeneration::Builder->CreateFSub(L, R, "subtmp");
		}
	}
	else if(op == '*')
	{
		if(pureIntCount == 2)
		{
			opLLVM = CodeGeneration::Builder->CreateMul(L, R, "multmp");
		}
		else
		{
			opLLVM = CodeGeneration::Builder->CreateFMul(L, R, "multmp");
		}
	}
	else if(op == '<')
	{
		bool isFloat = L->getType()->isFloatTy();
		bool isDouble = L->getType()->isDoubleTy();
		bool isInt = L->getType()->isIntegerTy();

		if(pureIntCount == 2)
		{
			//std::cout << "Comparing two pure integers.\n";
			L = CodeGeneration::Builder->CreateICmpULT(L, R, "cmptmp");
		}
		else
		{
			//std::cout << "Comparing floats or doubles.\n";
			L = CodeGeneration::Builder->CreateFCmpULT(L, R, "cmptmp");
		}

		if(isDouble)
		{
			//std::cout << "Compare type: Double\n";
			opLLVM = CodeGeneration::Builder->CreateUIToFP(L, llvm::Type::getDoubleTy(*CodeGeneration::TheContext), "booltmp");
		}
		else if(isFloat)
		{
			//std::cout << "Compare type: Float\n";
			opLLVM = CodeGeneration::Builder->CreateUIToFP(L, llvm::Type::getFloatTy(*CodeGeneration::TheContext), "booltmp");
		}
		else if(isInt)
		{
			//std::cout << "Compare type: Integer\n";
			opLLVM = CodeGeneration::Builder->CreateIntCast(L, llvm::Type::getInt32Ty(*CodeGeneration::TheContext), true, "booltmp");
		}
	}
		//case '<':
		//	L = CodeGeneration::Builder->CreateFCmpULT(L, R, "cmptmp");
		//	return CodeGeneration::Builder->CreateUIToFP(L, llvm::Type::getDoubleTy(*CodeGeneration::TheContext), "booltmp");

		//default:

	if(opLLVM == nullptr)
	{
		llvm::Function* F = CodeGeneration::GetFunction(std::string("binary") + op);

		if(F == nullptr)
			return CodeGeneration::LogErrorV("Invalid binary operator (" + std::to_string(op) + ").\n");

		llvm::Value* Ops[2] = { L, R };

		opLLVM = CodeGeneration::Builder->CreateCall(F, Ops, "binop");
	}

	CodeGeneration::lastLLVMInOp = opLLVM;

	return opLLVM;
}

llvm::Value* AST::Call::codegen()
{
	//std::cout << "CodeGen Call...\n";

	llvm::Function* CalleeF = CodeGeneration::TheModule->getFunction(callee);
	if(!CalleeF)
		return CodeGeneration::LogErrorV("Unknown function " + callee + " referenced.\n");

	if(CalleeF->arg_size() != arguments.size())
		return CodeGeneration::LogErrorV("Incorrect # arguments passed.\n");

	std::vector<llvm::Value*> ArgsV;
	for(unsigned i = 0, e = arguments.size(); i != e; ++i)
	{
		ArgsV.push_back(arguments[i]->codegen());
		if(!ArgsV.back())
			return nullptr;
	}

	return CodeGeneration::Builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

llvm::Function* AST::FunctionPrototype::codegen()
{
	//std::cout << "CodeGen Prototype...\n";

	std::vector<llvm::Type*> llvmArgs;

	for(auto const& i: arguments)
	{
		if(dynamic_cast<Double*>(i.first.get()) != nullptr)
			llvmArgs.push_back(llvm::Type::getDoubleTy(*CodeGeneration::TheContext));
		else if(dynamic_cast<Integer*>(i.first.get()) != nullptr)
			llvmArgs.push_back(llvm::Type::getInt32Ty(*CodeGeneration::TheContext));
		else if(dynamic_cast<Float*>(i.first.get()) != nullptr)
			llvmArgs.push_back(llvm::Type::getFloatTy(*CodeGeneration::TheContext));
	}

	llvm::FunctionType* FT = nullptr;

	if(dynamic_cast<Double*>(type.get()) != nullptr)
		FT = llvm::FunctionType::get(llvm::Type::getDoubleTy(*CodeGeneration::TheContext), llvmArgs, false);
	else if(dynamic_cast<Integer*>(type.get()) != nullptr)
		FT = llvm::FunctionType::get(llvm::Type::getInt32Ty(*CodeGeneration::TheContext), llvmArgs, false);
	else if(dynamic_cast<Float*>(type.get()) != nullptr)
		FT = llvm::FunctionType::get(llvm::Type::getFloatTy(*CodeGeneration::TheContext), llvmArgs, false);
	else
		return CodeGeneration::LogErrorFLLVM("Unknown function type.");

	if(FT == nullptr)
		return CodeGeneration::LogErrorFLLVM("FT is nullptr.");

	llvm::Function* F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, name, CodeGeneration::TheModule.get());

	unsigned Idx = 0;
	for (auto &Arg : F->args())
  		Arg.setName(arguments[Idx++].second->name);

	return F;
}

llvm::Function* AST::Function::codegen()
{
	//std::cout << "CodeGen Function...\n";

	//std::cout << "Getting Function...\n";
	auto &P = *prototype;
	FunctionProtos[prototype->Name()] = std::move(prototype);
	llvm::Function* TheFunction = CodeGeneration::GetFunction(P.Name());
	//std::cout << "Function Found!\n";

	//std::cout << "Assigning Name to Function...\n";
	name = P.Name();
	type = std::move(P.type);
	//std::cout << "Name Assigned!\n";

	if(!TheFunction)
		return nullptr;

	if(P.IsBinaryOperator())
		Parser::BinaryOpPrecedence[P.GetOperatorName()] = P.GetBinaryPrecedence();

	if(!TheFunction->empty())
		return (llvm::Function*)CodeGeneration::LogErrorV("Function cannot be redefined.\n");

	// Basic block start
	llvm::BasicBlock* BB = llvm::BasicBlock::Create(*CodeGeneration::TheContext, "entry", TheFunction);
	CodeGeneration::Builder->SetInsertPoint(BB);

	CodeGeneration::NamedValues.clear();

	for(auto& A : TheFunction->args())
	{
		auto getNameOfA = std::string(A.getName());
		CodeGeneration::NamedValues[getNameOfA] = &A;
	}

	if(llvm::Value* RetVal = body->codegen())
	{
		CodeGeneration::Builder->CreateRet(RetVal);

		llvm::verifyFunction(*TheFunction);

		CodeGeneration::TheFPM->run(*TheFunction);

		return TheFunction;
	}

	TheFunction->eraseFromParent();
	return nullptr;
}

llvm::Value* AST::If::codegen()
{
	llvm::Value* ConditionV = Condition->codegen();
	if(!ConditionV)
		return nullptr;

	bool isInt = ConditionV->getType()->isIntegerTy();
	bool isDouble = ConditionV->getType()->isDoubleTy();
	bool isFloat = ConditionV->getType()->isFloatTy();

	if(isInt)
	{
		ConditionV = CodeGeneration::Builder->CreateICmpNE(
			ConditionV, 
			llvm::ConstantInt::get(*CodeGeneration::TheContext, llvm::APInt(32, 0, true)), 
			"ifcond");
	}
	else if(isDouble || isFloat)
	{
		//std::cout << "Condition is double or float!\n";

		ConditionV = CodeGeneration::Builder->CreateFCmpONE(
			ConditionV, 
			llvm::ConstantFP::get(*CodeGeneration::TheContext, llvm::APFloat(0.0)), 
			"ifcond");
	}
	else
		return CodeGeneration::LogErrorV("Unknown condition type in 'if' statement");

	llvm::Function *TheFunction = CodeGeneration::Builder->GetInsertBlock()->getParent();

	llvm::BasicBlock* ThenBB = llvm::BasicBlock::Create(*CodeGeneration::TheContext, "then", TheFunction);
	llvm::BasicBlock* ElseBB = llvm::BasicBlock::Create(*CodeGeneration::TheContext, "else");
	llvm::BasicBlock* MergeBB = llvm::BasicBlock::Create(*CodeGeneration::TheContext, "ifcont");

	CodeGeneration::Builder->CreateCondBr(ConditionV, ThenBB, ElseBB);

	CodeGeneration::Builder->SetInsertPoint(ThenBB);

	llvm::Value* ThenV = Then->codegen();

	if(!ThenV)
		return nullptr;

	CodeGeneration::Builder->CreateBr(MergeBB);

	ThenBB = CodeGeneration::Builder->GetInsertBlock();

	TheFunction->getBasicBlockList().push_back(ElseBB);
	CodeGeneration::Builder->SetInsertPoint(ElseBB);

	llvm::Value* ElseV = Else->codegen();
	if(!ElseV)
		return nullptr;

	CodeGeneration::Builder->CreateBr(MergeBB);

	ElseBB = CodeGeneration::Builder->GetInsertBlock();

	TheFunction->getBasicBlockList().push_back(MergeBB);
	CodeGeneration::Builder->SetInsertPoint(MergeBB);

	llvm::PHINode* PN = nullptr;
	PN = CodeGeneration::Builder->CreatePHI(TheFunction->getFunctionType()->getReturnType(), 2, "iftmp");

	PN->addIncoming(ThenV, ThenBB);
	PN->addIncoming(ElseV, ElseBB);
	return PN;
}

llvm::Value* AST::For::codegen()
{
	llvm::Value* StartVal = Start->codegen();
	if(!StartVal)
		return nullptr;

	llvm::Function* TheFunction = CodeGeneration::Builder->GetInsertBlock()->getParent();

	llvm::BasicBlock* PreHeaderBB = CodeGeneration::Builder->GetInsertBlock();
	llvm::BasicBlock* LoopBB = llvm::BasicBlock::Create(*CodeGeneration::TheContext, "loop", TheFunction);

	CodeGeneration::Builder->CreateBr(LoopBB);

	CodeGeneration::Builder->SetInsertPoint(LoopBB);

	llvm::PHINode* PHIVariable = nullptr;

	if(dynamic_cast<Double*>(varType.get()) != nullptr)
	{
		//std::cout << "Adding ConstantFP (double) PHI Variable...\n";
		PHIVariable = CodeGeneration::Builder->CreatePHI(llvm::Type::getDoubleTy(*CodeGeneration::TheContext), 2, varName.c_str());
		//std::cout << "ConstantFP (double) PHI Variable added!\n";
	}
	else if(dynamic_cast<Integer*>(varType.get()) != nullptr)
	{
		PHIVariable = CodeGeneration::Builder->CreatePHI(llvm::Type::getInt32Ty(*CodeGeneration::TheContext), 2, varName.c_str());
	}
	else if(dynamic_cast<Float*>(varType.get()) != nullptr)
	{
		PHIVariable = CodeGeneration::Builder->CreatePHI(llvm::Type::getFloatTy(*CodeGeneration::TheContext), 2, varName.c_str());
	}
	else
		return CodeGeneration::LogErrorV("Unknown function type.");

	PHIVariable->addIncoming(StartVal, PreHeaderBB);

	llvm::Value* oldValue = CodeGeneration::NamedValues[varName];
	CodeGeneration::NamedValues[varName] = PHIVariable;

	if(!Body->codegen())
		return nullptr;

	llvm::Value* StepVal = nullptr, *NextVar = nullptr;

	if(Step)
	{
		StepVal = Step->codegen();

		if(!StepVal)
			return nullptr;

		if(dynamic_cast<Double*>(varType.get()) != nullptr || dynamic_cast<Float*>(varType.get()) != nullptr)
			NextVar = CodeGeneration::Builder->CreateFAdd(PHIVariable, StepVal, "nextvar");		
		else if(dynamic_cast<Integer*>(varType.get()) != nullptr)
			NextVar = CodeGeneration::Builder->CreateAdd(PHIVariable, StepVal, "nextvar");
		else
			return CodeGeneration::LogErrorV("Unknown function type.");
	}
	else
	{
		if(dynamic_cast<Double*>(varType.get()) != nullptr || dynamic_cast<Float*>(varType.get()) != nullptr)
		{
			//std::cout << "Adding ConstantFP next var condition...\n";

			StepVal = llvm::ConstantFP::get(*CodeGeneration::TheContext, llvm::APFloat(1.0));
			NextVar = CodeGeneration::Builder->CreateFAdd(PHIVariable, StepVal, "nextvar");

			//std::cout << "ConstantFP next var added!\n";
		}
		else if(dynamic_cast<Integer*>(varType.get()) != nullptr)
		{
			StepVal = llvm::ConstantInt::get(*CodeGeneration::TheContext, llvm::APInt(32, 1, true));
			NextVar = CodeGeneration::Builder->CreateAdd(PHIVariable, StepVal, "nextvar");
		}
		else
			return CodeGeneration::LogErrorV("Unknown function type.");
	}

	llvm::Value* EndCond = End->codegen();
	if(!EndCond)
		return nullptr;

	if(dynamic_cast<Double*>(varType.get()) != nullptr || dynamic_cast<Float*>(varType.get()) != nullptr)
	{
		//std::cout << "Adding ConstantFP loop condition...\n";

		EndCond = CodeGeneration::Builder->CreateFCmpONE(
			EndCond, llvm::ConstantFP::get(*CodeGeneration::TheContext, llvm::APFloat(0.0)), "loopcond");

		//std::cout << "ConstantFP loop condition added!\n";
	}
	else if(dynamic_cast<Integer*>(varType.get()) != nullptr)
	{
		//std::cout << "Adding ConstantInt loop condition...\n";

		EndCond = CodeGeneration::Builder->CreateICmpNE(
			EndCond, llvm::ConstantInt::get(*CodeGeneration::TheContext, llvm::APInt(32, 0, true)), "loopcond");

		//std::cout << "ConstantInt loop condition added!\n";
	}
	else
		return CodeGeneration::LogErrorV("Unknown function type.");

	llvm::BasicBlock* LoopEndBB = CodeGeneration::Builder->GetInsertBlock();

	llvm::BasicBlock* AfterBB = llvm::BasicBlock::Create(*CodeGeneration::TheContext, "afterloop", TheFunction);

	CodeGeneration::Builder->CreateCondBr(EndCond, LoopBB, AfterBB);

	CodeGeneration::Builder->SetInsertPoint(AfterBB);

	PHIVariable->addIncoming(NextVar, LoopEndBB);

	if(oldValue)
		CodeGeneration::NamedValues[varName] = oldValue;
	else
		CodeGeneration::NamedValues.erase(varName);

	if(dynamic_cast<Double*>(varType.get()) != nullptr)
		return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*CodeGeneration::TheContext));
	else if(dynamic_cast<Integer*>(varType.get()) != nullptr)
		return llvm::Constant::getNullValue(llvm::Type::getInt32Ty(*CodeGeneration::TheContext));
	else if(dynamic_cast<Float*>(varType.get()) != nullptr)
		return llvm::Constant::getNullValue(llvm::Type::getFloatTy(*CodeGeneration::TheContext));
	else
		return CodeGeneration::LogErrorV("Unknown function type.");
}

llvm::Value* AST::Unary::codegen()
{
	llvm::Value* OperandV = Operand->codegen();

	if(!OperandV)
		return nullptr;

	llvm::Function* F = CodeGeneration::GetFunction(std::string("unary") + OpCode);

	if(!F)
		return CodeGeneration::LogErrorV(std::string("Unknown unary operator: ") + OpCode);

	return CodeGeneration::Builder->CreateCall(F, OperandV, "unop");
}
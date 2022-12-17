#include "AbstractSyntaxTree.hpp"
#include "Parser.hpp"

std::map<std::string, std::unique_ptr<AST::FunctionPrototype>> AST::FunctionProtos;

llvm::Value* AST::Number::codegen()
{
	//std::cout << "CodeGen Number...\n";

	if(isDouble)
	{
		CodeGeneration::isPureNumber = false;
		//AST::EmitLocation(this);
		return llvm::ConstantFP::get(*CodeGeneration::TheContext, llvm::APFloat(doubleValue));
	}
	else if(isFloat)
	{
		CodeGeneration::isPureNumber = false;
		//AST::EmitLocation(this);
		return llvm::ConstantFP::get(*CodeGeneration::TheContext, llvm::APFloat(floatValue));
	}
	else if(isInt)
	{
		//return llvm::ConstantFP::get(*CodeGeneration::TheContext, llvm::APFloat((double)intValue));

		llvm::Type *getType = nullptr;

		if(bit == 1)
			getType = llvm::IntegerType::getInt1Ty(*CodeGeneration::TheContext);
		else if(bit == 8)
			getType = llvm::IntegerType::getInt8Ty(*CodeGeneration::TheContext);
		else if(bit == 16)
			getType = llvm::IntegerType::getInt16Ty(*CodeGeneration::TheContext);
		else if(bit == 32)
			getType = llvm::IntegerType::getInt32Ty(*CodeGeneration::TheContext);
		else if(bit == 64)
			getType = llvm::IntegerType::getInt64Ty(*CodeGeneration::TheContext);
		else if(bit == 128)
			getType = llvm::IntegerType::getInt128Ty(*CodeGeneration::TheContext);
		else
			getType = llvm::IntegerType::getInt32Ty(*CodeGeneration::TheContext);


		CodeGeneration::isPureNumber = true;
		//AST::EmitLocation(this);
		return llvm::ConstantInt::get(getType, intValue, true);
	}

	//AST::EmitLocation(this);
	return llvm::ConstantFP::get(*CodeGeneration::TheContext, llvm::APFloat(doubleValue));
}

llvm::Value* AST::Integer::codegen()
{
	//std::cout << "CodeGen Integer...\n";
	CodeGeneration::isPureNumber = false;
	//AST::EmitLocation(this);
	return llvm::ConstantInt::get(*CodeGeneration::TheContext, llvm::APInt(bit, value, true));
}

llvm::Value* AST::Float::codegen()
{
	//std::cout << "CodeGen Float...\n";
	CodeGeneration::isPureNumber = false;
	//AST::EmitLocation(this);
	return llvm::ConstantFP::get(*CodeGeneration::TheContext, llvm::APFloat(value));
}

llvm::Value* AST::Double::codegen()
{
	//std::cout << "CodeGen Double...\n";
	CodeGeneration::isPureNumber = false;
	//AST::EmitLocation(this);
	return llvm::ConstantFP::get(*CodeGeneration::TheContext, llvm::APFloat(value));
}

llvm::Value* AST::Variable::codegen()
{
	//std::cout << "CodeGen Variable...\n";

	llvm::AllocaInst* V = CodeGeneration::NamedValues[name];

	if(!V)
		CodeGeneration::LogErrorV("Unknown variable name: " + name + "\n");

	bool isInt = V->getAllocatedType()->isIntegerTy();
	
	if(isArrayElement)
	{
		llvm::Value* index = arrayIndex->codegen();

		llvm::Type* vType = V->getAllocatedType();

		llvm::Value* indexList[2] = {llvm::ConstantInt::get(index->getType(), 0), index};

		std::string idx = name + "idx";

		return CodeGeneration::Builder->CreateLoad(V->getAllocatedType()->getArrayElementType(), CodeGeneration::Builder->CreateInBoundsGEP(vType, V, llvm::ArrayRef<llvm::Value*>(indexList, 2), idx.c_str()), name.c_str());
	}

	if(isInt)
		CodeGeneration::isPureNumber = true;
	else
		CodeGeneration::isPureNumber = false;

	//AST::EmitLocation(this);
	return CodeGeneration::Builder->CreateLoad(V->getAllocatedType(), V, name.c_str());
}

llvm::Value* AST::Binary::codegen()
{
	//std::cout << "CodeGen Binary...\n";

	//AST::EmitLocation(this);

	if(op == '=')
	{
		AST::Variable* LHSE = dynamic_cast<AST::Variable*>(lhs.get());

		if(!LHSE)
			return CodeGeneration::LogErrorV("Destionation of '=' must be a variable.");

		llvm::Value* RVal = rhs->codegen();
		if(!RVal)
			return nullptr;

		llvm::Value* Variable = CodeGeneration::NamedValues[LHSE->name];

		if(!Variable)
			return CodeGeneration::LogErrorV("Unknown variable name.");

		CodeGeneration::Builder->CreateStore(RVal, Variable);
		return RVal;
	}

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
			L = CodeGeneration::Builder->CreateICmpULT(L, CodeGeneration::Builder->CreateIntCast(R, L->getType(), true, "castmp"), "cmptmp");
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
			AST::Integer* LInt = (AST::Integer*)lhs.get();

			//std::cout << "CodeGen Condition Integer...\n";

			if(!LInt)
				return CodeGeneration::LogErrorV("Invalid Variable Type. Expected Integer.");

			//std::cout << "Compare type: Integer\n";
			opLLVM = CodeGeneration::Builder->CreateIntCast(L, GetASTIntegerType(LInt), true, "booltmp");
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

	//AST::EmitLocation(this);

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
		{
			Integer* getI = (Integer*)i.first.get();
			llvmArgs.push_back(GetASTIntegerType(getI));
		}
		else if(dynamic_cast<Float*>(i.first.get()) != nullptr)
			llvmArgs.push_back(llvm::Type::getFloatTy(*CodeGeneration::TheContext));
		else
			return CodeGeneration::LogErrorFLLVM("One of the argument types is unknown.");
	}

	llvm::FunctionType* FT = nullptr;

	if(type == nullptr)
		return CodeGeneration::LogErrorFLLVM("type is nullptr.");

	if(dynamic_cast<Double*>(type.get()) != nullptr)
		FT = llvm::FunctionType::get(llvm::Type::getDoubleTy(*CodeGeneration::TheContext), llvmArgs, false);
	else if(dynamic_cast<Integer*>(type.get()) != nullptr)
	{
		AST::Integer* getI = (AST::Integer*)type.get();

		llvm::Type* getASTIntType = GetASTIntegerType(getI);

		FT = llvm::FunctionType::get(getASTIntType, llvmArgs, false);
	}
	else if(dynamic_cast<Float*>(type.get()) != nullptr)
		FT = llvm::FunctionType::get(llvm::Type::getFloatTy(*CodeGeneration::TheContext), llvmArgs, false);
	else
		return CodeGeneration::LogErrorFLLVM("Unknown function type.");

	if(FT == nullptr)
		return CodeGeneration::LogErrorFLLVM("FT is nullptr.");

	llvm::Function* F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, name, CodeGeneration::TheModule.get());

	//std::cout << "Setting Arguments...\n";

	unsigned Idx = 0;
	for (auto &Arg : F->args())
	{
		if(arguments[Idx].second == nullptr)
			return CodeGeneration::LogErrorFLLVM("One of the Arguments is nullptr.");

  		Arg.setName(arguments[Idx++].second->name);
	}

	//std::cout << "Done!\n";

	return F;
}

llvm::Function* AST::Function::codegen()
{
	//std::cout << "CodeGen Function...\n";

	if(prototype == nullptr)
		return (llvm::Function*)CodeGeneration::LogErrorV("Function prototype is nullptr.\n");

	//std::cout << "Getting Function...\n";
	auto &P = *prototype;
	//std::cout << prototype->Name() << "\n";
	FunctionProtos[prototype->Name()] = std::move(prototype);
	//std::cout << FunctionProtos[P.Name()]->name << " Added!" << "\n";
	llvm::Function* TheFunction = CodeGeneration::GetFunction(P.Name());
	//std::cout << "Function Found!\n";

	//std::cout << "Assigning Name and Type to Function...\n";
	name = P.Name();
	type = std::move(P.type);
	//std::cout << "Name and Type Assigned!\n";

	if(!TheFunction)
	{
		//std::cout << "Function " << P.Name() << " not found :c\n";
		return nullptr;
	}

	//std::cout << "Checking for Binary Operators...\n";

	if(P.IsBinaryOperator())
		Parser::BinaryOpPrecedence[P.GetOperatorName()] = P.GetBinaryPrecedence();

	//std::cout << "Binary Operators Checked!\n";

	if(!TheFunction->empty())
		return (llvm::Function*)CodeGeneration::LogErrorV("Function cannot be redefined.\n");

	// Basic block start
	llvm::BasicBlock* BB = llvm::BasicBlock::Create(*CodeGeneration::TheContext, "entry", TheFunction);
	CodeGeneration::Builder->SetInsertPoint(BB);

	llvm::DIFile* Unit = CodeGeneration::DBuilder->createFile(CodeGeneration::DebugInfo::TheCU->getFilename(),
															  CodeGeneration::DebugInfo::TheCU->getDirectory());

	llvm::DIScope* FContext = Unit;

	unsigned LineNo = 0, ScopeLine = 0;

	llvm::DISubprogram* SP = CodeGeneration::DBuilder->createFunction(

		FContext, P.Name(), llvm::StringRef(), Unit, LineNo,

		CreateFunctionType(TheFunction->arg_size(), type.get()),

		ScopeLine,
		llvm::DINode::FlagPrototyped,

		llvm::DISubprogram::SPFlagDefinition

		);

	TheFunction->setSubprogram(SP);

	//std::cout << "Subprogram Successfully Created!\n";

	CodeGeneration::DebugInfo::LexicalBlocks.push_back(SP);
	//AST::EmitLocation(nullptr);

	CodeGeneration::NamedValues.clear();

	unsigned ArgIdx = 0;

	for(auto& A : TheFunction->args())
	{
		llvm::AllocaInst* Alloca = CodeGeneration::CreateEntryAllocation(TheFunction, std::string(A.getName()));

		llvm::DILocalVariable* D = CodeGeneration::DBuilder->createParameterVariable(
			SP, A.getName(), ArgIdx, Unit, LineNo, AST::GetFunctionDIType(P.arguments[ArgIdx].first.get()), true
			);

		CodeGeneration::DBuilder->insertDeclare(Alloca, D, CodeGeneration::DBuilder->createExpression(),
			llvm::DILocation::get(SP->getContext(), LineNo, 0, SP), CodeGeneration::Builder->GetInsertBlock());

		CodeGeneration::Builder->CreateStore(&A, Alloca);

		auto getNameOfA = std::string(A.getName());
		CodeGeneration::NamedValues[getNameOfA] = Alloca;

		ArgIdx++;
	}

	//AST::EmitLocation(body.get());

	//std::cout << "Emit Location (body) successfull!\n";

	if(llvm::Value* RetVal = body->codegen())
	{
		//std::cout << "Body generated!\n";

		CodeGeneration::Builder->CreateRet(RetVal);

		CodeGeneration::DebugInfo::LexicalBlocks.pop_back();

		llvm::verifyFunction(*TheFunction);

		CodeGeneration::TheFPM->run(*TheFunction);

		return TheFunction;
	}

	TheFunction->eraseFromParent();

	if(P.IsBinaryOperator())
		Parser::BinaryOpPrecedence.erase(prototype->GetOperatorName());

	CodeGeneration::DebugInfo::LexicalBlocks.pop_back();

	return nullptr;
}

llvm::Value* AST::If::codegen()
{
	//AST::EmitLocation(this);

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
	llvm::Function* TheFunction = CodeGeneration::Builder->GetInsertBlock()->getParent();

	llvm::AllocaInst* Alloca = CodeGeneration::CreateEntryAllocation(TheFunction, varName);

	//AST::EmitLocation(this);

	llvm::Value* StartVal = Start->codegen();
	if(!StartVal)
		return nullptr;

	CodeGeneration::Builder->CreateStore(StartVal, Alloca);

	llvm::BasicBlock* LoopBB = llvm::BasicBlock::Create(*CodeGeneration::TheContext, "loop", TheFunction);

	CodeGeneration::Builder->CreateBr(LoopBB);

	CodeGeneration::Builder->SetInsertPoint(LoopBB);

	llvm::AllocaInst* oldValue = CodeGeneration::NamedValues[varName];
	CodeGeneration::NamedValues[varName] = Alloca;

	if(!Body->codegen())
		return nullptr;

	llvm::Value* StepVal = nullptr, *NextVar = nullptr, 
	*CurVar = CodeGeneration::Builder->CreateLoad(Alloca->getAllocatedType(), Alloca, varName.c_str());

	//std::cout << "CodeGen For Loop Step...\n";

	if(Step)
	{
		StepVal = Step->codegen();

		if(!StepVal)
			return nullptr;

		if(dynamic_cast<Double*>(varType.get()) != nullptr || dynamic_cast<Float*>(varType.get()) != nullptr)
			NextVar = CodeGeneration::Builder->CreateFAdd(CurVar, StepVal, "nextvar");		
		else if(dynamic_cast<Integer*>(varType.get()) != nullptr)
			NextVar = CodeGeneration::Builder->CreateAdd(CurVar, StepVal, "nextvar");
		else
			return CodeGeneration::LogErrorV("Unknown function type.");
	}
	else
	{
		if(dynamic_cast<Double*>(varType.get()) != nullptr || dynamic_cast<Float*>(varType.get()) != nullptr)
		{
			//std::cout << "Adding ConstantFP next var condition...\n";

			StepVal = llvm::ConstantFP::get(*CodeGeneration::TheContext, llvm::APFloat(1.0));
			NextVar = CodeGeneration::Builder->CreateFAdd(CurVar, StepVal, "nextvar");

			//std::cout << "ConstantFP next var added!\n";
		}
		else if(dynamic_cast<Integer*>(varType.get()) != nullptr)
		{
			Integer* getI = (Integer*)varType.get();

			StepVal = llvm::ConstantInt::get(*CodeGeneration::TheContext, llvm::APInt(getI->bit, 1, true));
			NextVar = CodeGeneration::Builder->CreateAdd(CurVar, StepVal, "nextvar");
		}
		else
			return CodeGeneration::LogErrorV("Unknown function type.");
	}

	//std::cout << "For Loop Step generated!\n";

	llvm::Value* EndCond = End->codegen();
	if(!EndCond)
		return nullptr;

	CodeGeneration::Builder->CreateStore(NextVar, Alloca);

	bool isDouble = TheFunction->getFunctionType()->getReturnType()->isDoubleTy();
	bool isInt = TheFunction->getFunctionType()->getReturnType()->isIntegerTy();
	bool isFloat = TheFunction->getFunctionType()->getReturnType()->isFloatTy();

	if(isDouble || isFloat)
	{
		//std::cout << "Adding ConstantFP loop condition...\n";

		EndCond = CodeGeneration::Builder->CreateFCmpONE(
			EndCond, llvm::ConstantFP::get(*CodeGeneration::TheContext, llvm::APFloat(0.0)), "loopcond");

		//std::cout << "ConstantFP loop condition added!\n";
	}
	else if(isInt)
	{
		//std::cout << "Adding ConstantInt loop condition...\n";

		Integer* getI = (Integer*)varType.get();

		llvm::IntegerType* ty = static_cast<llvm::IntegerType*>(TheFunction->getFunctionType()->getReturnType());

		if(ty == nullptr)
			return CodeGeneration::LogErrorV("Function Type returned nullptr.");

		EndCond = CodeGeneration::Builder->CreateICmpNE(
			EndCond, llvm::ConstantInt::get(*CodeGeneration::TheContext, llvm::APInt(ty->getBitWidth(), 0, true)), "loopcond");

		//std::cout << "ConstantInt loop condition added!\n";
	}
	else
		return CodeGeneration::LogErrorV("Unknown function type.");

	llvm::BasicBlock* AfterBB = llvm::BasicBlock::Create(*CodeGeneration::TheContext, "afterloop", TheFunction);

	CodeGeneration::Builder->CreateCondBr(EndCond, LoopBB, AfterBB);

	CodeGeneration::Builder->SetInsertPoint(AfterBB);

	if(oldValue)
		CodeGeneration::NamedValues[varName] = oldValue;
	else
		CodeGeneration::NamedValues.erase(varName);

	if(Next != nullptr)
	{
		llvm::Value* NextVar = Next->codegen();

		if(!NextVar)
			return nullptr;

		return NextVar;
	}
	else
	{
		if(dynamic_cast<Double*>(varType.get()) != nullptr)
			return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*CodeGeneration::TheContext));
		else if(dynamic_cast<Integer*>(varType.get()) != nullptr)
		{
			Integer* getI = (Integer*)varType.get();
			return llvm::Constant::getNullValue(GetASTIntegerType(getI));
		}
		else if(dynamic_cast<Float*>(varType.get()) != nullptr)
			return llvm::Constant::getNullValue(llvm::Type::getFloatTy(*CodeGeneration::TheContext));
		else
			return CodeGeneration::LogErrorV("Unknown function type.");
	}
}

llvm::Value* AST::Unary::codegen()
{
	llvm::Value* OperandV = Operand->codegen();

	if(!OperandV)
		return nullptr;

	llvm::Function* F = CodeGeneration::GetFunction(std::string("unary") + OpCode);

	if(!F)
		return CodeGeneration::LogErrorV(std::string("Unknown unary operator: ") + OpCode);

	//AST::EmitLocation(this);
	return CodeGeneration::Builder->CreateCall(F, OperandV, "unop");
}

llvm::Value* AST::Var::codegen()
{
	//std::cout << "CodeGen Var..." << std::endl;

	std::vector<llvm::AllocaInst*> OldBindings;

	llvm::Function* TheFunction = CodeGeneration::Builder->GetInsertBlock()->getParent();

	for(unsigned i = 0, e = VarNames.size(); i != e; ++i)
	{
		const std::string& VarName = VarNames[i].name;
		Expression* Init = VarNames[i].body.get();

		llvm::Value* InitVal = nullptr;

		if(Init)
		{
			//std::cout << "CodeGen Init..." << std::endl;

			if(dynamic_cast<ArrayInitContent*>(Init) != nullptr)
			{
				if(dynamic_cast<Array*>(VarNames[i].type.get()))
				{
					Array* getA = (Array*)VarNames[i].type.get();
					InitVal = getA->codegen();
				}
			}
			else
			{
				InitVal = Init->codegen();
			}

			if(!InitVal)
				return nullptr;

			//std::cout << "CodeGen Init Successfull!" << std::endl;
		}
		else
		{
			if(dynamic_cast<Double*>(VarNames[i].type.get()))
				InitVal = llvm::ConstantFP::get(*CodeGeneration::TheContext, llvm::APFloat(0.0));
			else if(dynamic_cast<Float*>(VarNames[i].type.get()))
				InitVal = llvm::ConstantFP::get(*CodeGeneration::TheContext, llvm::APFloat(0.0f));
			else if(dynamic_cast<Integer*>(VarNames[i].type.get()))
			{
				Integer* getI = (Integer*)VarNames[i].type.get();

				InitVal = llvm::ConstantInt::get(*CodeGeneration::TheContext, llvm::APInt(getI->bit, 0, true));
			}
			else if(dynamic_cast<Array*>(VarNames[i].type.get()))
			{
				Array* getA = (Array*)VarNames[i].type.get();

				InitVal = getA->codegen();
			}
		}

		llvm::AllocaInst* Alloca = nullptr;

		if(dynamic_cast<Array*>(VarNames[i].type.get()))
			Alloca = CodeGeneration::Builder->CreateAlloca(InitVal->getType(), 0, VarName.c_str());
		else
			Alloca = CodeGeneration::CreateEntryAllocation(TheFunction, VarName);

		CodeGeneration::Builder->CreateStore(InitVal, Alloca);

		OldBindings.push_back(CodeGeneration::NamedValues[VarName]);

		CodeGeneration::NamedValues[VarName] = Alloca;
	}

	//AST::EmitLocation(this);

	//std::cout << "CodeGen Body..." << std::endl;

	llvm::Value* BodyVal = Body->codegen();
	if(!BodyVal)
		return nullptr;

	//std::cout << "CodeGen Body Successfull!" << std::endl;

	for(unsigned i = 0, e = VarNames.size(); i != e; ++i)
	{
		CodeGeneration::NamedValues[VarNames[i].name] = OldBindings[i];
	}

	return BodyVal;
}

llvm::Value* AST::Array::codegen()
{
	llvm::Function* TheFunction = CodeGeneration::Builder->GetInsertBlock()->getParent();

	llvm::Type* getT = nullptr;

	if(dynamic_cast<Double*>(type.get()) != nullptr)
		getT = llvm::Type::getDoubleTy(*CodeGeneration::TheContext);
	else if(dynamic_cast<Integer*>(type.get()) != nullptr)
	{
		getT = GetASTIntegerType((Integer*)type.get());
	}
	else if(dynamic_cast<Float*>(type.get()) != nullptr)
		getT = llvm::Type::getFloatTy(*CodeGeneration::TheContext);
	else
		return CodeGeneration::LogErrorV("Unknown function type.");

	llvm::ArrayType* arrayType = llvm::ArrayType::get(getT, size);

	std::vector<llvm::Constant*> values;

	if(variables.size() == 0 || variables.size() != size)
	{
		for(unsigned int i = 0; i < size; i++)
		{
			if(dynamic_cast<Double*>(type.get()) != nullptr)
				values.push_back(llvm::ConstantFP::get(*CodeGeneration::TheContext, llvm::APFloat(0.0)));
			else if(dynamic_cast<Float*>(type.get()) != nullptr)
				values.push_back(llvm::ConstantFP::get(*CodeGeneration::TheContext, llvm::APFloat(0.0f)));
			else if(dynamic_cast<Integer*>(type.get()) != nullptr)
			{
				Integer* getASTI = (Integer*)type.get();

				values.push_back(llvm::ConstantInt::get(*CodeGeneration::TheContext, llvm::APInt(getASTI->bit, 0, true)));
			}
		}
	}
	else
	{
		for(unsigned int i = 0; i < size; i++)
		{
			llvm::Value* getV = variables[i]->codegen();

			if(static_cast<llvm::ConstantFP*>(getV) != nullptr)
				values.push_back((llvm::ConstantFP*)getV);
			else if(static_cast<llvm::ConstantInt*>(getV) != nullptr)
				values.push_back((llvm::ConstantInt*)getV);
			else
				return CodeGeneration::LogErrorV("Variable initialized in array is not constant.");
		}
	}

	return llvm::ConstantArray::get(arrayType, values);
}

llvm::Value* AST::ArrayInitContent::codegen()
{
	return CodeGeneration::LogErrorV("This codegen() shouldn't be executed! This expression is only used to store array information!");
}

void AST::EmitLocation(AST::Expression* a)
{
	if (!a)
    	return CodeGeneration::Builder->SetCurrentDebugLocation(llvm::DebugLoc());

	llvm::DIScope* Scope = nullptr;

	if(CodeGeneration::DebugInfo::LexicalBlocks.empty())
		Scope = CodeGeneration::DebugInfo::TheCU;
	else
		Scope = CodeGeneration::DebugInfo::LexicalBlocks.back();

	CodeGeneration::Builder->SetCurrentDebugLocation(

		llvm::DILocation::get(Scope->getContext(), a->GetLine(), a->GetColumn(), Scope)

		);
}

llvm::DIType* AST::GetFunctionDIType(AST::Expression* t)
{
	bool isDouble = dynamic_cast<AST::Double*>(t) != nullptr;
	bool isInteger = dynamic_cast<AST::Integer*>(t) != nullptr;
	bool isFloat = dynamic_cast<AST::Float*>(t) != nullptr;

	if(isDouble) { return CodeGeneration::DebugInfo::getDoubleTy(); }
	if(isInteger) { return CodeGeneration::DebugInfo::getIntegerTy(); }
	if(isFloat) { return CodeGeneration::DebugInfo::getFloatTy(); }

	return nullptr;
}

llvm::DISubroutineType* AST::CreateFunctionType(unsigned NumArgs, AST::Expression* t) 
{
	llvm::SmallVector<llvm::Metadata *, 8> EltTys;
	llvm::DIType *Typ = AST::GetFunctionDIType(t);
	
	// Add the result type.
	EltTys.push_back(Typ);
	
	for (unsigned i = 0, e = NumArgs; i != e; ++i)
	  EltTys.push_back(Typ);
	
	return CodeGeneration::DBuilder->createSubroutineType(CodeGeneration::DBuilder->getOrCreateTypeArray(EltTys));
}

llvm::Type* AST::GetASTIntegerType(AST::Integer* getI)
{
	if(getI->bit == 1)
		return llvm::Type::getInt1Ty(*CodeGeneration::TheContext);
	else if(getI->bit == 8)
		return llvm::Type::getInt8Ty(*CodeGeneration::TheContext);
	else if(getI->bit == 16)
		return llvm::Type::getInt16Ty(*CodeGeneration::TheContext);
	else if(getI->bit == 32)
		return llvm::Type::getInt32Ty(*CodeGeneration::TheContext);
	else if(getI->bit == 64)
		return llvm::Type::getInt64Ty(*CodeGeneration::TheContext);
	else if(getI->bit == 128)
		return llvm::Type::getInt128Ty(*CodeGeneration::TheContext);

	return llvm::Type::getInt32Ty(*CodeGeneration::TheContext);
}
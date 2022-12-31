#ifndef ABSTRACT_SYNTAX_TREE_H
#define ABSTRACT_SYNTAX_TREE_H

#include "Lexer.hpp"
#include <string>
#include "CodeGeneration.hpp"

struct AST
{

	static llvm::raw_ostream &Indent(llvm::raw_ostream &O, int size) {
  		return O << std::string(size, ' ');
	}

	struct Expression
	{
		bool isPointer = false;

		SourceLocation Loc;

		Expression(SourceLocation Loc = Lexer::CurrentLocation) : Loc(Loc) {}

		virtual ~Expression() = default;
		virtual llvm::Value* codegen() = 0;

		int GetLine() const { return Loc.Line; }
		int GetColumn() const { return Loc.Column; }

		virtual llvm::raw_ostream& Dump(llvm::raw_ostream& out, int index)
		{
			return out << ":" << GetLine() << ":" << GetColumn() << '\n';
		}
	};

	struct Number : public Expression
	{
		bool isDouble = false, isInt = false, isFloat = false;
		int intValue = 0;
		double doubleValue = 0;
		float floatValue = 0;
		unsigned bit = 32;
		std::string valueAsString;
		Number(std::string val) 
		{
			//std::cout << "Number Input: " << val << "\n";
			if(val.find(".") != std::string::npos)
			{
				if(val.back() == 'f')
				{
					isFloat = true;
					floatValue = std::stof(val);
				}
				else
				{
					isDouble = true;
					doubleValue = std::stod(val);
				}
			}
			else
			{
				isInt = true;
				intValue = std::stoi(val);
			}

			valueAsString = val;
		}

		llvm::raw_ostream& Dump(llvm::raw_ostream& out, int index) override
		{
			return Expression::Dump(out << valueAsString, index);
		}

		llvm::Value* codegen() override;
	};

	struct Integer : public Expression
	{
		int value;
		unsigned int bit;
		Integer(int val) : value(val) {}

		llvm::raw_ostream& Dump(llvm::raw_ostream& out, int index) override
		{
			return Expression::Dump(out << value, index);
		}

		llvm::Value* codegen() override;
	};

	struct Double : public Expression
	{
		double value;
		Double(double val) : value(val) {}

		llvm::raw_ostream& Dump(llvm::raw_ostream& out, int index) override
		{
			return Expression::Dump(out << value, index);
		}

		llvm::Value* codegen() override;
	};

	struct Float : public Expression
	{
		float value;
		Float(float val) : value(val) {}

		llvm::raw_ostream& Dump(llvm::raw_ostream& out, int index) override
		{
			return Expression::Dump(out << value, index);
		}

		llvm::Value* codegen() override;
	};

	struct Variable : public Expression
	{
		std::string name;

		bool isArrayElement = false;
		bool isArrayPointer = false;
		bool isTDArrayElement = false;
		bool isTDArrayPointer = false;

		std::unique_ptr<Expression> TDArrayIndex;
		std::unique_ptr<Expression> arrayIndex;

		Variable(SourceLocation Loc, const std::string& n) : Expression(Loc), name(n) {}

		llvm::raw_ostream& Dump(llvm::raw_ostream& out, int index) override
		{
			return Expression::Dump(out << name, index);
		}

		llvm::Value* codegen() override;
	};

	struct Binary : public Expression
	{
		char op;
		std::unique_ptr<Expression> lhs, rhs;

		Binary(SourceLocation Loc, char oper, std::unique_ptr<Expression> left,
                std::unique_ptr<Expression> right)
    	: Expression(Loc), op(oper), lhs(std::move(left)), rhs(std::move(right)) {}

    	llvm::raw_ostream& Dump(llvm::raw_ostream& out, int index) override
		{
			Expression::Dump(out << "binary" << op, index);
			lhs->Dump(Indent(out, index) << "Left:", index + 1);
			rhs->Dump(Indent(out, index) << "Right:", index + 1);
			return out;
		}

    	llvm::Value* codegen() override;
	};

	struct Call : public Expression
	{
		std::string callee;
		std::vector<std::unique_ptr<Expression>> arguments;

		Call(SourceLocation Loc, const std::string& c, std::vector<std::unique_ptr<Expression>> args)
		: Expression(Loc), callee(c), arguments(std::move(args)) {}

		llvm::raw_ostream& Dump(llvm::raw_ostream& out, int index) override
		{
			Expression::Dump(out << "Call " << callee, index);

			for(const auto& Arg : arguments)
			{
				Arg->Dump(Indent(out, index + 1), index + 1);
			}

			return out;
		}

		llvm::Value* codegen() override;
	};

	struct FunctionPrototype
	{
		std::unique_ptr<Expression> type;
		std::string name;
		std::vector<std::pair<std::unique_ptr<AST::Expression>, std::unique_ptr<AST::Variable>>> arguments;
		bool IsOperator;
		unsigned Precedence;
		int Line;

		FunctionPrototype(SourceLocation Loc, std::unique_ptr<Expression> t, const std::string& n, std::vector<std::pair<std::unique_ptr<AST::Expression>, std::unique_ptr<AST::Variable>>> args, bool IsOperator = false, unsigned Prec = 0)
		: type(std::move(t)), name(n), arguments(std::move(args)), IsOperator(IsOperator), Precedence(Prec), Line(Loc.Line) {}

		const std::string& Name() const { return name; }

		bool IsUnaryOperator() const { return IsOperator && arguments.size() == 1; }
		bool IsBinaryOperator() const  { return IsOperator && arguments.size() == 2; }

		char GetOperatorName() const
		{
			assert(IsUnaryOperator() || IsBinaryOperator());
			return name.back();
		}

		unsigned GetBinaryPrecedence() const { return Precedence; }

		int GetLine() const { return Line; }

		llvm::Function* codegen();
	};

	static std::map<std::string, std::unique_ptr<AST::FunctionPrototype>> FunctionProtos;

	struct Function
	{
		std::unique_ptr<Expression> type;
		std::string name;
		std::unique_ptr<FunctionPrototype> prototype;
		std::unique_ptr<Expression> body;

		Function(std::unique_ptr<FunctionPrototype> proto, std::unique_ptr<Expression> b)
		: prototype(std::move(proto)), body(std::move(b)) {}

		llvm::raw_ostream& Dump(llvm::raw_ostream& out, int index)
		{
			Indent(out, index) << "Function\n";
			++index;
			Indent(out, index) << "Body:";

			return body ? body->Dump(out, index) : out << "null\n";
		}

		llvm::Function* codegen();
	};

	struct If : public Expression
	{
		std::unique_ptr<Expression> Condition, Then, Else;

		If(SourceLocation Loc, std::unique_ptr<Expression> cond, std::unique_ptr<Expression> t, std::unique_ptr<Expression> e) :
			Expression(Loc), Condition(std::move(cond)), Then(std::move(t)), Else(std::move(e)) {}

		llvm::raw_ostream& Dump(llvm::raw_ostream& out, int index) override
		{
			Expression::Dump(out << "if", index);

			Condition->Dump(Indent(out, index) << "Condition:", index + 1);
			Then->Dump(Indent(out, index) << "Then:", index + 1);
			Else->Dump(Indent(out, index) << "Else:", index + 1);

			return out;
		}

		llvm::Value* codegen() override;
	};

	struct For : public Expression
	{
		std::unique_ptr<Expression> varType;
		std::string varName;

		std::unique_ptr<Expression> Start, End, Step, Body, Next;

		For(std::unique_ptr<Expression> Type,
			const std::string &VarName, std::unique_ptr<Expression> Start,
    	         std::unique_ptr<Expression> End, std::unique_ptr<Expression> Step,
    	         std::unique_ptr<Expression> Body, std::unique_ptr<Expression> Next) : 
		varType(std::move(Type)), varName(VarName), Start(std::move(Start)), End(std::move(End)),
    	  Step(std::move(Step)), Body(std::move(Body)), Next(std::move(Next)) {}

    	llvm::raw_ostream& Dump(llvm::raw_ostream& out, int index) override
    	{
    		Expression::Dump(out << "for", index);

    		Start->Dump(Indent(out, index) << "Condition:", index + 1);
    		End->Dump(Indent(out, index) << "End:", index + 1);
    		Step->Dump(Indent(out, index) << "Step:", index + 1);
    		Body->Dump(Indent(out, index) << "Body:", index + 1);

    		return out;
    	}

		llvm::Value *codegen() override;
	};

	struct Unary : public Expression
	{
		char OpCode;
		std::unique_ptr<Expression> Operand;

		Unary(char op, std::unique_ptr<Expression> oper)
		: OpCode(op), Operand(std::move(oper)) {}

		llvm::raw_ostream& Dump(llvm::raw_ostream& out, int index) override
		{
			Expression::Dump(out << "unary" << OpCode, index);
			Operand->Dump(out, index + 1);

			return out;
		}

		llvm::Value* codegen() override;
	};

	struct VarStruct
	{
		std::string name;
		std::unique_ptr<Expression> type;
		std::unique_ptr<Expression> body;
	};

	struct Var : public Expression
	{
		std::vector<VarStruct> VarNames;
		std::unique_ptr<Expression> Body;

		Var(std::vector<VarStruct> vn, std::unique_ptr<Expression> b) :
			Expression(Loc), VarNames(std::move(vn)), Body(std::move(b)) {}

		llvm::raw_ostream& Dump(llvm::raw_ostream& out, int index) override
		{
			Expression::Dump(out << "var", index);

			for(const auto& NamedVar : VarNames)
			{
				NamedVar.body->Dump(Indent(out, index) << NamedVar.name << ":", index + 1);
			}

			Body->Dump(Indent(out, index) << "Body:", index + 1);

			return out;
		}

		llvm::Value* codegen() override;
	};

	struct Array : public Expression
	{
		std::vector<std::unique_ptr<Expression>> variables;
		std::unique_ptr<Expression> type;
		unsigned int size;
		bool dynamicInitialization = false;

		Array(std::unique_ptr<Expression> ty,
			unsigned int s
			) :
			size(s), type(std::move(ty)) {}

		llvm::raw_ostream& Dump(llvm::raw_ostream& out, int index) override
		{
			Expression::Dump(out << "array", index);

			//for(const auto& NamedVar : variables)
			//{
			//	NamedVar.body->Dump(Indent(out, index) << NamedVar.name << ":", index + 1);
			//}

			return out;
		}

		llvm::Value* codegen() override;
	};

	struct ArrayInitContent : public Expression
	{
		std::vector<std::unique_ptr<Expression>> variables;

		ArrayInitContent(std::vector<std::unique_ptr<Expression>> v) :
		variables(std::move(v)) {}

		llvm::raw_ostream& Dump(llvm::raw_ostream& out, int index) override
		{
			Expression::Dump(out << "arraycontent", index);

			//for(const auto& NamedVar : variables)
			//{
			//	NamedVar.body->Dump(Indent(out, index) << NamedVar.name << ":", index + 1);
			//}

			return out;
		}

		llvm::Value* codegen() override;
	};

	struct NestedArray : public Expression
	{
		std::vector<std::unique_ptr<Expression>> arrays;
		std::unique_ptr<Expression> type;
		bool dynamicInitialization = false;
		unsigned int size = 0;

		NestedArray(std::unique_ptr<Expression> t, unsigned int s) : type(std::move(t)), size(s) {}

		llvm::raw_ostream& Dump(llvm::raw_ostream& out, int index) override
		{
			Expression::Dump(out << "nestedarray", index);

			//for(const auto& NamedVar : variables)
			//{
			//	NamedVar.body->Dump(Indent(out, index) << NamedVar.name << ":", index + 1);
			//}

			return out;
		}

		llvm::Value* codegen() override;
	};

	struct NestedArrayContent : public Expression
	{
		std::vector<std::unique_ptr<Expression>> variables;

		NestedArrayContent(std::vector<std::unique_ptr<Expression>> v) :
		variables(std::move(v)) {}

		llvm::raw_ostream& Dump(llvm::raw_ostream& out, int index) override
		{
			//Expression::Dump(out << "nestedarraycontent", index);

			//for(const auto& NamedVar : variables)
			//{
			//	NamedVar.body->Dump(Indent(out, index) << NamedVar.name << ":", index + 1);
			//}

			return out;
		}

		llvm::Value* codegen() override;
	};

	struct StructTy : public Expression
	{
		llvm::StructType* existingStruct = nullptr;

		StructTy(llvm::StructType* s) : existingStruct(s) {}

		llvm::raw_ostream& Dump(llvm::raw_ostream& out, int index) override
		{
			//Expression::Dump(out << "nestedarraycontent", index);

			//for(const auto& NamedVar : variables)
			//{
			//	NamedVar.body->Dump(Indent(out, index) << NamedVar.name << ":", index + 1);
			//}

			return out;
		}

		llvm::Value* codegen() override;
	};

	struct StructEx
	{
		std::string Name;
		std::vector<std::unique_ptr<Expression>> variables;

		StructEx(std::string n, std::vector<std::unique_ptr<Expression>> v) :
		Name(n), variables(std::move(v)) {}

		//llvm::raw_ostream& Dump(llvm::raw_ostream& out, int index) override
		//{
		//	Expression::Dump(out << "struct", index);

		//	//for(const auto& NamedVar : variables)
		//	//{
		//	//	NamedVar.body->Dump(Indent(out, index) << NamedVar.name << ":", index + 1);
		//	//}

		//	return out;
		//}

		llvm::Type* codegen();
	};

	static llvm::DIType* GetFunctionDIType(AST::Expression* t);
	static void EmitLocation(AST::Expression* AST);
	static llvm::DISubroutineType* CreateFunctionType(unsigned NumArgs, AST::Expression* t);

	static llvm::Type* GetASTIntegerType(AST::Integer* i);
};

#endif
#ifndef ABSTRACT_SYNTAX_TREE_H
#define ABSTRACT_SYNTAX_TREE_H

#include "Lexer.hpp"
#include <string>
#include "CodeGeneration.hpp"

struct AST
{
	struct Expression
	{
		virtual ~Expression() = default;
		virtual llvm::Value* codegen() = 0;
	};

	struct Number : public Expression
	{
		bool isDouble = false, isInt = false, isFloat = false;
		int intValue = 0;
		double doubleValue = 0;
		float floatValue = 0;
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
		}
		llvm::Value* codegen() override;
	};

	struct Integer : public Expression
	{
		int value;
		Integer(int val) : value(val) {}
		llvm::Value* codegen() override;
	};

	struct Double : public Expression
	{
		double value;
		Double(double val) : value(val) {}
		llvm::Value* codegen() override;
	};

	struct Float : public Expression
	{
		float value;
		Float(float val) : value(val) {}
		llvm::Value* codegen() override;
	};

	struct Variable : public Expression
	{
		std::string name;
		Variable(const std::string& n) : name(n) {}
		llvm::Value* codegen() override;
	};

	struct Binary : public Expression
	{
		char op;
		std::unique_ptr<Expression> lhs, rhs;

		Binary(char oper, std::unique_ptr<Expression> left,
                std::unique_ptr<Expression> right)
    	: op(oper), lhs(std::move(left)), rhs(std::move(right)) {}

    	llvm::Value* codegen() override;
	};

	struct Call : public Expression
	{
		std::string callee;
		std::vector<std::unique_ptr<Expression>> arguments;

		Call(const std::string& c, std::vector<std::unique_ptr<Expression>> args)
		: callee(c), arguments(std::move(args)) {}

		llvm::Value* codegen() override;
	};

	struct FunctionPrototype
	{
		std::unique_ptr<Expression> type;
		std::string name;
		std::vector<std::pair<std::unique_ptr<AST::Expression>, std::unique_ptr<AST::Variable>>> arguments;
		bool IsOperator;
		unsigned Precedence;

		FunctionPrototype(std::unique_ptr<Expression> t, const std::string& n, std::vector<std::pair<std::unique_ptr<AST::Expression>, std::unique_ptr<AST::Variable>>> args, bool IsOperator = false, unsigned Prec = 0)
		: type(std::move(t)), name(n), arguments(std::move(args)), IsOperator(IsOperator), Precedence(Prec) {}

		const std::string& Name() const { return name; }

		bool IsUnaryOperator() const { return IsOperator && arguments.size() == 1; }
		bool IsBinaryOperator() const  { return IsOperator && arguments.size() == 2; }

		char GetOperatorName() const
		{
			assert(IsUnaryOperator() || IsBinaryOperator());
			return name.back();
		}

		unsigned GetBinaryPrecedence() const { return Precedence; }

		llvm::Function* codegen();
	};

	static std::map<std::string, std::unique_ptr<FunctionPrototype>> FunctionProtos;

	struct Function
	{
		std::unique_ptr<Expression> type;
		std::string name;
		std::unique_ptr<FunctionPrototype> prototype;
		std::unique_ptr<Expression> body;

		Function(std::unique_ptr<FunctionPrototype> proto, std::unique_ptr<Expression> b)
		: prototype(std::move(proto)), body(std::move(b)) {}

		llvm::Function* codegen();
	};

	struct If : public Expression
	{
		std::unique_ptr<Expression> Condition, Then, Else;

		If(std::unique_ptr<Expression> cond, std::unique_ptr<Expression> t, std::unique_ptr<Expression> e) :
			Condition(std::move(cond)), Then(std::move(t)), Else(std::move(e)) {}

		llvm::Value* codegen() override;
	};

	struct For : public Expression
	{
		std::unique_ptr<Expression> varType;
		std::string varName;

		std::unique_ptr<Expression> Start, End, Step, Body;

		For(std::unique_ptr<Expression> Type,
			const std::string &VarName, std::unique_ptr<Expression> Start,
    	         std::unique_ptr<Expression> End, std::unique_ptr<Expression> Step,
    	         std::unique_ptr<Expression> Body) : 
		varType(std::move(Type)), varName(VarName), Start(std::move(Start)), End(std::move(End)),
    	  Step(std::move(Step)), Body(std::move(Body)) {}

		llvm::Value *codegen() override;
	};

	struct Unary : public Expression
	{
		char OpCode;
		std::unique_ptr<Expression> Operand;

		Unary(char op, std::unique_ptr<Expression> oper)
		: OpCode(op), Operand(std::move(oper)) {}

		llvm::Value* codegen() override;
	};
};

#endif
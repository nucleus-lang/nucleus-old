#ifndef ABSTRACT_SYNTAX_TREE_HPP
#define ABSTRACT_SYNTAX_TREE_HPP

#include <memory>
#include <vector>
#include <string>

struct AST
{
	struct Expression
	{
		virtual ~Expression() = default;
	};

	struct Type
	{
		virtual ~Type() = default;
	};

	struct Variable : public Expression
	{
		std::unique_ptr<Type> getType;
		std::string name;
		Variable(std::unique_ptr<Type> t, const std::string& n) : getType(std::move(t)), name(n) {}
	};

	struct Integer : public Type
	{
		int value;
		Integer(int v) : value(v) {}
	};

	struct Double : public Type
	{
		double value;
		Double(double v) : value(v) {}
	};

	struct Binary : public Expression
	{
		char op;
		std::unique_ptr<Expression> lhs, rhs;

		Binary(char oper, std::unique_ptr<Expression> left,
                std::unique_ptr<Expression> right)
    	: op(oper), lhs(std::move(left)), rhs(std::move(right)) {}
	};

	struct Call : public Expression
	{
		std::string callee;
		std::vector<std::unique_ptr<Expression>> arguments;

		Call(const std::string& c, std::vector<std::unique_ptr<Expression>> args)
		: callee(c), arguments(std::move(args)) {}
	};

	struct Prototype
	{
		std::string name;
		std::vector<Variable> arguments;

		Prototype(const std::string& n, std::vector<Variable> args)
		: name(n), arguments(std::move(args)) {}

		const std::string& Name() const { return name; }
	};

	struct Function
	{
		std::unique_ptr<Prototype> prototype;
		std::unique_ptr<Expression> body;

		Function(std::unique_ptr<Prototype> proto, std::unique_ptr<Expression> b)
		: prototype(std::move(proto)), body(std::move(b)) {}
	};
};

#endif
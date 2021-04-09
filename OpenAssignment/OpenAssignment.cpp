#include <iostream>
#include <stack>
#include <string>
#include <utility>
#include <vector>


class OperandToken;
class Operation;

class ParseError : public std::runtime_error
{
public:
	explicit ParseError(const std::string& _Message)
		: runtime_error(_Message)
	{
	}
};

enum class OperandType
{
	None, Number, String
};

class Token
{
public:
	virtual ~Token() = default;
	virtual std::string ToString() = 0;
};

class OperandToken : public Token
{
public:
	virtual ~OperandToken() = default;
};

class NumericToken : public OperandToken
{
public:
	double value;

	explicit NumericToken(double value)
		: value(value)
	{
	}


	std::string ToString() override
	{
		return std::to_string(value);
	}

	virtual double Value()
	{
		return value;
	}
};

class StringToken : public OperandToken
{
public:
	std::string value;
};

class OperatorOrFunction
{
public:
	int precedence;

	virtual ~OperatorOrFunction() = default;
	
	explicit OperatorOrFunction(int precedence)
		: precedence(precedence)
	{
	}


	bool Precedes(const OperatorOrFunction& other) const
	{
		return other.precedence <= precedence;
	}
};

class Operator : public OperatorOrFunction
{
protected:
	Operator(std::string operator_, int precedence)
		: operator_(std::move(operator_)),
		OperatorOrFunction(precedence)
	{
	}
public:

	std::string operator_;

	virtual std::size_t NumOperands() = 0;
};

class OperatorOrFunctionCallToken : public Token
{
public:
	OperatorOrFunction* value;

	explicit OperatorOrFunctionCallToken(OperatorOrFunction* value)
		: value(value)
	{
	}
};

class OperatorToken : public OperatorOrFunctionCallToken
{
public:

	explicit OperatorToken(Operator* value)
		: OperatorOrFunctionCallToken(value)
	{
	}

	std::string ToString() override
	{
		return Value()->operator_;
	}

	Operator* Value()
	{
		return dynamic_cast<Operator*>(value);
	}
};

class Operation
{
};

class SingleOperandOperation : public Operation
{
public:
	virtual std::unique_ptr<OperandToken> Eval(OperandToken* a) = 0;
};

class DualOperandOperation : public Operation
{
public:
	virtual std::unique_ptr<OperandToken> Eval(OperandToken* a, OperandToken* b) = 0;
};

class DualNumericsOperation : public DualOperandOperation
{
	virtual std::unique_ptr<NumericToken> EvalNumeric(NumericToken*, NumericToken*) = 0;

public:

	std::unique_ptr<OperandToken> Eval(OperandToken* a, OperandToken* b) override
	{
		NumericToken* _a, *_b;
		if (((_a = dynamic_cast<NumericToken*>(a))) && ((_b = dynamic_cast<NumericToken*>(b))))
		{
			return EvalNumeric(_a, _b);
		}
		return nullptr;
	}
};

class SingleNumericOperation : public SingleOperandOperation
{
	virtual std::unique_ptr<NumericToken> EvalNumeric(NumericToken*) = 0;

public:

	std::unique_ptr<OperandToken> Eval(OperandToken* a) override
	{
		if (auto* token = dynamic_cast<NumericToken*>(a))
		{
			return EvalNumeric(token);
		}
		return nullptr;
	}
};

class SingleOperandOperator : public Operator
{
public:
	std::vector<SingleOperandOperation*> operations;

	std::size_t NumOperands() override { return 1; }

	SingleOperandOperator(const std::string& operator_, int precedence,
		std::vector<SingleOperandOperation*> operations={})
		: Operator(operator_, precedence), operations(std::move(operations))
	{
	}

	std::unique_ptr<OperandToken> Eval(OperandToken* token)
	{
		for (auto& operation : operations)
		{
			if (auto result = operation->Eval(token)) return result;
		}
		return nullptr;
	}
};

class DualOperandOperator : public Operator
{
public:
	std::vector<DualOperandOperation*> operations;

	std::size_t NumOperands() override { return 2; }

	DualOperandOperator(const std::string& operator_, int precedence,
		std::vector<DualOperandOperation*> operations={})
		: Operator(operator_, precedence), operations(std::move(operations))
	{
	}

	std::unique_ptr<OperandToken> Eval(OperandToken* a, OperandToken* b)
	{
		for (auto& operation : operations)
		{
			if (auto result = operation->Eval(a, b))
				return result;
		}
		return nullptr;
	}
};

inline std::unique_ptr<NumericToken> Numeric(double x)
{
	return std::unique_ptr<NumericToken>(new NumericToken(x));
}

class LogicalOrOperation : public DualNumericsOperation
{
	std::unique_ptr<NumericToken> EvalNumeric(NumericToken* a, NumericToken* b) override
	{
		return Numeric(a->Value() || b->Value());
	}
};

class LogicalAndOperation : public DualNumericsOperation
{
	std::unique_ptr<NumericToken> EvalNumeric(NumericToken* a, NumericToken* b) override
	{
		return Numeric(a->Value() && b->Value());
	}
};

bool DoubleEquals(double a, double b)
{
	const auto EPSILON = 0.0001;
	const auto diff = a - b;
	return diff < EPSILON && -diff < EPSILON;
}

class EqualsOperation : public DualNumericsOperation
{
	std::unique_ptr<NumericToken> EvalNumeric(NumericToken* a, NumericToken* b) override
	{
		return Numeric(DoubleEquals(a->Value(), b->Value()));
	}
};

class NotEqualsOperation : public DualNumericsOperation
{
	std::unique_ptr<NumericToken> EvalNumeric(NumericToken* a, NumericToken* b) override
	{
		return Numeric(!DoubleEquals(a->Value(), b->Value()));
	}
};

class GTOperation : public DualNumericsOperation
{
	std::unique_ptr<NumericToken> EvalNumeric(NumericToken* a, NumericToken* b) override
	{
		return Numeric(a->Value() > b->Value());
	}
};

class GTEOperation : public DualNumericsOperation
{
	std::unique_ptr<NumericToken> EvalNumeric(NumericToken* a, NumericToken* b) override
	{
		return Numeric(a->Value() >= b->Value());
	}
};


class LTOperation : public DualNumericsOperation
{
	std::unique_ptr<NumericToken> EvalNumeric(NumericToken* a, NumericToken* b) override
	{
		return Numeric(a->Value() < b->Value());
	}
};

class LTEOperation : public DualNumericsOperation
{
	std::unique_ptr<NumericToken> EvalNumeric(NumericToken* a, NumericToken* b) override
	{
		return Numeric(a->Value() <= b->Value());
	}
};

class BitwiseAndOperation : public DualNumericsOperation
{
	std::unique_ptr<NumericToken> EvalNumeric(NumericToken* a, NumericToken* b) override
	{
		return Numeric(static_cast<int>(a->Value()) & static_cast<int>(b->Value()));
	}
};

class BitwiseOrOperation : public DualNumericsOperation
{
	std::unique_ptr<NumericToken> EvalNumeric(NumericToken* a, NumericToken* b) override
	{
		return Numeric(static_cast<int>(a->Value()) | static_cast<int>(b->Value()));
	}
};

class LeftShiftOperation : public DualNumericsOperation
{
	std::unique_ptr<NumericToken> EvalNumeric(NumericToken* a, NumericToken* b) override
	{
		return Numeric(static_cast<int>(a->Value()) << static_cast<int>(b->Value()));
	}
};

class RightShiftOperation : public DualNumericsOperation
{
	std::unique_ptr<NumericToken> EvalNumeric(NumericToken* a, NumericToken* b) override
	{
		return Numeric(static_cast<int>(a->Value()) >> static_cast<int>(b->Value()));
	}
};

class MultiplyOperation : public DualNumericsOperation
{
	std::unique_ptr<NumericToken> EvalNumeric(NumericToken* a, NumericToken* b) override
	{
		return Numeric(a->Value() * b->Value());
	}
};

class AddOperation : public DualNumericsOperation
{
	std::unique_ptr<NumericToken> EvalNumeric(NumericToken* a, NumericToken* b) override
	{
		return Numeric(a->Value() + b->Value());
	}
};

class SubtractOperation : public DualNumericsOperation
{
	std::unique_ptr<NumericToken> EvalNumeric(NumericToken* a, NumericToken* b) override
	{
		return Numeric(a->Value() - b->Value());
	}
};

class DivideOperation : public DualNumericsOperation
{
	std::unique_ptr<NumericToken> EvalNumeric(NumericToken* a, NumericToken* b) override
	{
		if (b->Value() == 0)
		{
			throw ParseError("Division by zero");
		}
		return Numeric(a->Value() / b->Value());
	}
};

class ModuloOperation : public DualNumericsOperation
{
	std::unique_ptr<NumericToken> EvalNumeric(NumericToken* a, NumericToken* b) override
	{
		if (b->Value() == 0)
		{
			throw ParseError("Modulo by zero");
		}
		return Numeric(static_cast<int>(a->Value()) % static_cast<int>(b->Value()));
	}
};

class PowOperation : public DualNumericsOperation
{
	std::unique_ptr<NumericToken> EvalNumeric(NumericToken* a, NumericToken* b) override
	{
		return Numeric(std::pow(a->Value(), b->Value()));
	}
};

class NegateOperation : public SingleNumericOperation
{
	std::unique_ptr<NumericToken> EvalNumeric(NumericToken* t)
	{
		return Numeric(-t->value);
	}
};

class LogicalNotOperation : public SingleNumericOperation
{
	std::unique_ptr<NumericToken> EvalNumeric(NumericToken* t)
	{
		return Numeric(!t->value);
	}
};


std::vector<Operator*> s_operators =
{
	new DualOperandOperator("=", 2),
	new DualOperandOperator("||", 5, {new LogicalOrOperation()}),
	new DualOperandOperator("&&", 7, {new LogicalAndOperation()}),
	new DualOperandOperator("==", 13, {new EqualsOperation()}),
	new DualOperandOperator("!=", 15, {new NotEqualsOperation()}),
	new DualOperandOperator(">", 15, {new GTOperation()}),
	new DualOperandOperator("<", 15, {new LTOperation()}),
	new DualOperandOperator(">=", 15, {new GTEOperation()}),
	new DualOperandOperator("<=", 15, {new LTEOperation()}),
	new DualOperandOperator("|", 16, {new BitwiseOrOperation()}),
	new DualOperandOperator("&", 16, {new BitwiseAndOperation()}),
	new DualOperandOperator("<<", 18, {new LeftShiftOperation()}),
	new DualOperandOperator(">>", 18, {new RightShiftOperation()}),
	new DualOperandOperator("+", 19, {new AddOperation()}),
	new DualOperandOperator("-", 19, {new SubtractOperation()}),
	new DualOperandOperator("*", 21, {new MultiplyOperation()}),
	new DualOperandOperator("/", 21, {new DivideOperation()}),
	new DualOperandOperator("%", 21, {new ModuloOperation()}),
	new DualOperandOperator("^", 23, {new PowOperation()}),
	new SingleOperandOperator("-", 25, {new NegateOperation()}),
	new SingleOperandOperator("!", 27, {new LogicalNotOperation()}),
	new SingleOperandOperator("(", 80),
	new SingleOperandOperator(")", 80),
};

class Function : public OperatorOrFunction
{
public:
	std::string name;
	std::size_t numParams;

	explicit Function(const std::string& name, std::size_t numParams)
		: OperatorOrFunction(23), name(name), numParams(numParams)
	{
	}

	virtual double Execute(const std::vector<OperandToken*>& params) = 0;
	virtual bool ValidateParams(const std::vector<OperandToken*>& params) = 0;
};

class FunctionCallToken : public OperatorOrFunctionCallToken
{
public:

	explicit FunctionCallToken(Function* function) : OperatorOrFunctionCallToken(function)
	{
	}

	Function* Value()
	{
		return dynamic_cast<Function*>(value);
	}

	std::string ToString() override
	{
		return Value()->name;
	}
};

class SqrtFunction : public Function
{
public:

	SqrtFunction() : Function("sqrt", 1){}

	double Execute(const std::vector<OperandToken*>& params) override
	{
		auto* param = dynamic_cast<NumericToken*>(params.at(0));
		return std::sqrt(param->Value());
	}


	bool ValidateParams(const std::vector<OperandToken*>& params) override
	{
		return dynamic_cast<NumericToken*>(params.at(0));
	}
};

std::vector<Function*> s_functions =
{
	new SqrtFunction()
};

class StringIterator
{
public:
	std::string string;
	unsigned int pos = 0;

	char Peek()
	{
		return string.at(pos);
	}

	char Seek()
	{
		return string.at(pos++);
	}

	bool End() const
	{
		return pos == string.size();
	}

	explicit StringIterator(std::string string)
		: string(std::move(string))
	{
	}

	void Advance()
	{
		++pos;
	}

};

char Peek(std::string::const_iterator& iter)
{
	
	return *(iter + 1);
}

std::string GetCurOperandString(StringIterator& iterator)
{
	std::string result;
	while (!iterator.End())
	{
		const auto ch = iterator.Peek();
		if (isspace(static_cast<unsigned char>(ch)) || ispunct(static_cast<unsigned char>(ch)) && ch != '_')
			break;
		iterator.Advance();
		result += ch;
	}
	return result;
}

std::string GetCurOperatorString(StringIterator& iterator)
{
	std::string result;
	while (!iterator.End())
	{
		const auto ch = iterator.Peek();
		if (isalnum(ch) || isspace(ch))
			break;
		iterator.Advance();
		result += ch;
	}
	return result;
}

std::unique_ptr<OperatorToken> ParseOperator(StringIterator& iterator)
{
	const auto opStr = GetCurOperatorString(iterator);
	if (opStr.empty())
	{
		return nullptr;
	}
	for (auto& op : s_operators)
	{
		if (op->operator_ == opStr)
		{
			return std::unique_ptr<OperatorToken>(new OperatorToken(op));
		}
	}
	throw ParseError("Unsupported operator " + opStr);
}

std::unique_ptr<OperandToken> ParseNumericConstant(const std::string& opStr)
{
	try
	{
		auto num = std::stod(opStr);
		return std::unique_ptr<NumericToken>(new NumericToken(num));
	}
	catch (std::invalid_argument&)
	{
	}
	return nullptr;
}

std::unique_ptr<FunctionCallToken> ParseFunctionCall(const std::string& opStr)
{
	for (auto* iter : s_functions)
	{
		if (iter->name == opStr)
		{
			return std::unique_ptr<FunctionCallToken>(new FunctionCallToken(iter));
		}
	}
	return nullptr;
}

bool IsOpenBracket(OperatorOrFunctionCallToken* token)
{
	if (auto* op = dynamic_cast<Operator*>(token->value))
		return op->operator_ == "(";
	return false;
}

bool IsClosedBracket(OperatorOrFunctionCallToken* token)
{
	if (auto* op = dynamic_cast<Operator*>(token->value))
		return op->operator_ == ")";
	return false;
}

std::vector<std::unique_ptr<Token>> ParseExpression(StringIterator& iterator)
{
	std::vector<std::unique_ptr<Token>> result;
	std::stack<std::unique_ptr<OperatorOrFunctionCallToken>> operatorsOrFuncs;
	while (!iterator.End())
	{
		if (isspace(iterator.Peek()))
		{
			iterator.Advance();
			continue;
		}
		
		if (auto operator_ = ParseOperator(iterator))
		{
			if (IsClosedBracket(operator_.get()))
			{
				while (!operatorsOrFuncs.empty() && !IsOpenBracket(operatorsOrFuncs.top().get()))
				{
					result.push_back(std::move(operatorsOrFuncs.top()));
					operatorsOrFuncs.pop();
				}
				if (operatorsOrFuncs.empty())
					throw ParseError("Mismatched brackets");
				operatorsOrFuncs.pop();
			}
			else
			{
				if (!IsOpenBracket(operator_.get()))
				{
					while (!operatorsOrFuncs.empty() && operatorsOrFuncs.top()->value->Precedes(*operator_->value) && !IsOpenBracket(operator_.get()))
					{
						result.push_back(std::move(operatorsOrFuncs.top()));
						operatorsOrFuncs.pop();
					}
				}
				operatorsOrFuncs.push(std::move(operator_));
			}
		}
		else
		{
			const auto opStr = GetCurOperandString(iterator);
			if (!opStr.empty())
			{
				if (auto operand = ParseNumericConstant(opStr))
				{
					result.push_back(std::move(operand));
				}
				else if (auto function = ParseFunctionCall(opStr))
				{
					operatorsOrFuncs.push(std::move(function));
				}
			}
		}
	}
	while (!operatorsOrFuncs.empty())
	{
		result.push_back(std::move(operatorsOrFuncs.top()));
		operatorsOrFuncs.pop();
	}
	return result;
}

std::unique_ptr<OperandToken> EvaluateExpression(std::vector<std::unique_ptr<Token>>& tokens)
{
	std::vector<std::unique_ptr<OperandToken>> tempTokens;
	std::stack<OperandToken*> stack;
	for (auto& token : tokens)
	{
		if (auto* operand = dynamic_cast<OperandToken*>(token.get()))
		{
			stack.push(operand);
		}
		else if (auto* operator_ = dynamic_cast<OperatorToken*>(token.get()))
		{
			std::unique_ptr<OperandToken> result;
			if (stack.size() < operator_->Value()->NumOperands())
				throw ParseError("Invalid number of operands for operator " + std::to_string(operator_->Value()->NumOperands()));
			if (auto* op = dynamic_cast<DualOperandOperator*>(operator_->Value()))
			{
				auto* rhsToken = stack.top();
				stack.pop();
				auto* lhsToken = stack.top();
				stack.pop();
				result = op->Eval(lhsToken, rhsToken);
			}
			else if (auto* op = dynamic_cast<SingleOperandOperator*>(operator_->Value()))
			{
				auto* token = stack.top();
				stack.pop();
				result = op->Eval(token);
			}
			if (!result)
				throw ParseError("Invalid operands for operator " + operator_->Value()->operator_);
			tempTokens.push_back(std::move(result));
			stack.push(tempTokens.back().get());
		}
		else if (auto* function = dynamic_cast<FunctionCallToken*>(token.get()))
		{
			if (stack.size() < function->Value()->numParams)
				throw ParseError("Invalid number of arguments for function " + function->ToString());
			std::vector<OperandToken*> params;
			for (auto i = 0u; i < function->Value()->numParams; ++i)
			{
				params.push_back(stack.top());
				stack.pop();
			}
			if (!function->Value()->ValidateParams(params))
				throw ParseError("Wrong parameter types for function " + function->ToString());
			const auto result = function->Value()->Execute(params);
			tempTokens.push_back(Numeric(result));
			stack.push(tempTokens.back().get());
		}
	}
	if (stack.size() != 1)
	{
		throw ParseError("There was an error evaluating");
	}
	for (auto& temp : tempTokens)
	{
		if (stack.top() == temp.get())
			return std::move(temp);
	}
	return nullptr;
}

int main()
{
	StringIterator iterator("5 + sqrt 9");
	auto compiled = ParseExpression(iterator);
	auto evaluated = EvaluateExpression(compiled);
	std::cout << evaluated->ToString() << std::endl;
}

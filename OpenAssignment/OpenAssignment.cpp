#include <fstream>
#include <functional>
#include <iostream>
#include <map>
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

class Variable
{
public:
	std::string name;

	explicit Variable(std::string name)
		: name(std::move(name))
	{
		
	}

	virtual ~Variable() = default;
};

class NumericVariable : public Variable
{
public:
	double data = 0;

	explicit NumericVariable(std::string name, double data)
		: Variable(std::move(name)), data(data)
	{
	}
};

class StringVariable : public Variable
{
public:
	std::string data;

	explicit StringVariable(std::string name, std::string data)
		: Variable(std::move(name)), data(std::move(data))
	{
	}
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

/*class VariableToken : public OperandToken
{
public:
	Variable* variable;

	explicit VariableToken(Variable* variable)
		: variable(variable)
	{
	}

	std::string ToString() override
	{
		return variable->name;
	}
};*/

class NumericToken : public OperandToken
{
public:

	std::string ToString() override
	{
		return std::to_string(Value());
	}

	virtual double Value() = 0;
};

class VariableToken 
{
public:
	virtual Variable* GetVariable() = 0;
};

class NumericVariableToken : public NumericToken, public VariableToken
{
public:
	NumericVariable* variable;

	explicit NumericVariableToken(NumericVariable* variable)
		: variable(variable)
	{
	}

	std::string ToString() override
	{
		return std::to_string(variable->data);
	}


	Variable* GetVariable() override
	{
		return variable;
	}


	double Value() override
	{
		return variable->data;
	}
};

class StringToken : public OperandToken
{
public:
	std::string ToString() override
	{
		return Value();
	}
	virtual std::string& Value() = 0;
};

class StringVariableToken : public StringToken, public VariableToken
{
public:
	StringVariable* variable;

	explicit StringVariableToken(StringVariable* variable)
		: variable(variable)
	{
	}

	std::string ToString() override
	{
		return variable->data;
	}


	Variable* GetVariable() override
	{
		return variable;
	}


	std::string& Value() override
	{
		return variable->data;
	}
};


class NumericConstantToken : public NumericToken
{

public:
	double value;

	explicit NumericConstantToken(double value)
		: value(value)
	{
	}

	double Value() override
	{
		return value;
	}
};

class StringConstantToken : public StringToken
{
public:
	std::string value;

	explicit StringConstantToken(std::string value)
		: value(std::move(value))
	{
	}

	std::string& Value() override
	{
		return value;
	}
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

	bool Precedes(OperatorOrFunction* other) const;
};

class Operator : public OperatorOrFunction
{

public:
	Operator(std::string operator_, int precedence, int numOperands)
		: operator_(std::move(operator_)),
		OperatorOrFunction(precedence), numOperands(0)
	{
	}
	std::size_t numOperands;
	std::string operator_;
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

class DualStringOperation : public DualOperandOperation
{
	virtual std::unique_ptr<StringToken> EvalString(StringToken*, StringToken*) = 0;

public:

	std::unique_ptr<OperandToken> Eval(OperandToken* a, OperandToken* b) override
	{
		StringToken* _a, * _b;
		if (((_a = dynamic_cast<StringToken*>(a))) && ((_b = dynamic_cast<StringToken*>(b))))
		{
			return EvalString(_a, _b);
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

	SingleOperandOperator(const std::string& operator_, int precedence,
		std::vector<SingleOperandOperation*> operations={})
		: Operator(operator_, precedence, 1), operations(std::move(operations))
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

	DualOperandOperator(const std::string& operator_, int precedence,
		std::vector<DualOperandOperation*> operations={})
		: Operator(operator_, precedence, 2), operations(std::move(operations))
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

class ScriptLine
{
public:
	std::vector<std::unique_ptr<Token>> tokens;

	explicit ScriptLine(std::vector<std::unique_ptr<Token>> tokens)
		: tokens(std::move(tokens))
	{
	}
};

class ScriptModule
{
public:

	explicit ScriptModule(std::vector<std::string> scriptCompileLines)
		: scriptCompileLines(std::move(scriptCompileLines))
	{
	}

	ScriptModule() = default;
	
	std::vector<std::string> scriptCompileLines;
	std::vector<ScriptLine> scriptRunLines;
	std::vector<std::string>::iterator* curCompileLineIter = nullptr;
	std::vector<ScriptLine>::iterator* curRunLineIter = nullptr;
	std::map<std::string, std::unique_ptr<Variable>> scriptVariables;
	std::stack<int> nestStack;
	void Compile();
	void Execute();
};

ScriptModule s_scriptModule;

class AssignVariableOperation : public DualOperandOperation
{
public:
	std::unique_ptr<OperandToken> Eval(OperandToken* a, OperandToken* b) override
	{
		std::string varName;
		if (auto* varNameToken = dynamic_cast<StringConstantToken*>(a))
		{
			varName = varNameToken->ToString();
		}
		else if (auto* varToken = dynamic_cast<VariableToken*>(a))
		{
			varName = varToken->GetVariable()->name;
		}
		if (!varName.empty())
		{
			if (auto* numericToken = dynamic_cast<NumericToken*>(b))
			{
				s_scriptModule.scriptVariables[varName] = std::make_unique<NumericVariable>(varName, numericToken->Value());
				return std::make_unique<NumericVariableToken>(dynamic_cast<NumericVariable*>(s_scriptModule.scriptVariables[varName].get()));
			}
			if (auto* stringToken = dynamic_cast<StringToken*>(b))
			{
				s_scriptModule.scriptVariables[varName] = std::make_unique<StringVariable>(varName, stringToken->Value());
				return std::make_unique<StringVariableToken>(dynamic_cast<StringVariable*>(s_scriptModule.scriptVariables[varName].get()));
			}
		}
		return nullptr;
	}
};

inline std::unique_ptr<NumericToken> Numeric(double x)
{
	return std::make_unique<NumericConstantToken>(x);
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
		return Numeric(static_cast<int64_t>(a->Value()) << static_cast<int>(b->Value()));
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

class StringAddOperation : public DualStringOperation
{
	std::unique_ptr<StringToken> EvalString(StringToken* a, StringToken* b) override
	{
		return std::make_unique<StringConstantToken>(a->Value() + b->Value());
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
		return Numeric(-t->Value());
	}
};

class LogicalNotOperation : public SingleNumericOperation
{
	std::unique_ptr<NumericToken> EvalNumeric(NumericToken* t)
	{
		return Numeric(!t->Value());
	}
};


std::vector<Operator*> s_operators =
{
	new DualOperandOperator("=", 2, {new AssignVariableOperation()}),
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
	new DualOperandOperator("+", 19, {new AddOperation(), new StringAddOperation()}),
	new DualOperandOperator("-", 19, {new SubtractOperation()}),
	new DualOperandOperator("*", 21, {new MultiplyOperation()}),
	new DualOperandOperator("/", 21, {new DivideOperation()}),
	new DualOperandOperator("%", 21, {new ModuloOperation()}),
	new DualOperandOperator("^", 23, {new PowOperation()}),
	new SingleOperandOperator("-", 25, {new NegateOperation()}),
	new SingleOperandOperator("!", 27, {new LogicalNotOperation()}),
	new Operator("(", 80, 0),
	new Operator(")", 80, 0),
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
	virtual void Parse()
	{
		
	}
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

class PrintFunction : public Function
{
public:

	PrintFunction() : Function("print", 1) {}

	double Execute(const std::vector<OperandToken*>& params) override
	{
		std::string toPrint;
		if (auto* strToken = dynamic_cast<StringToken*>(params.at(0)))
		{
			toPrint = strToken->Value();
		}
		else if (auto* numericToken = dynamic_cast<NumericToken*>(params.at(0)))
		{
			toPrint = std::to_string(numericToken->Value());
		}
		std::cout << toPrint << std::endl;
		return 1;
	}

	bool ValidateParams(const std::vector<OperandToken*>& params) override
	{
		return true;
	}
};

class IfFunction : public Function
{
public:

	IfFunction() : Function("if", 1){}

	double Execute(const std::vector<OperandToken*>& params)
	{
		
	};
	
	bool ValidateParams(const std::vector<OperandToken*>& params)
	{
		return true;
	};
};

std::vector<Function*> s_functions =
{
	new SqrtFunction(),
	new PrintFunction(),
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

	std::string GetStringInQuotationMarks()
	{
		Advance();
		auto fPos = string.find_first_of('"', this->pos);
		if (fPos == std::string::npos)
			throw ParseError("Mismatched quotation marks (\")");
		auto ret = string.substr(pos, fPos - pos);
		pos = fPos + 1;
		return ret;
	}

	std::string GetCurOperandString()
	{
		std::string result;
		while (!End())
		{
			const auto ch = Peek();
			if (isspace(static_cast<unsigned char>(ch)) || ispunct(static_cast<unsigned char>(ch)) && ch != '_')
				break;
			Advance();
			result += ch;
		}
		return result;
	}

	std::string GetCurOperatorString()
	{
		std::string result;
		while (!End())
		{
			const auto ch = Peek();
			if (isalnum(ch) || isspace(ch) || ch == '"')
				break;
			Advance();
			result += ch;
		}
		return result;
	}

	std::unique_ptr<OperatorToken> ParseOperator()
	{
		const auto opStr = GetCurOperatorString();
		if (opStr.empty())
		{
			return nullptr;
		}
		for (auto& op : s_operators)
		{
			if (op->operator_ == opStr)
			{
				return std::make_unique<OperatorToken>(op);
			}
		}
		throw ParseError("Unsupported operator " + opStr);
	}
	
};

std::unique_ptr<OperandToken> ParseNumericConstant(const std::string& opStr)
{
	try
	{
		const auto num = std::stod(opStr);
		return Numeric(num);
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
			return std::make_unique<FunctionCallToken>(iter);
		}
	}
	return nullptr;
}

std::unique_ptr<OperandToken> ParseVariableToken(const std::string& opStr)
{
	if (const auto iter = s_scriptModule.scriptVariables.find(opStr); iter != s_scriptModule.scriptVariables.end())
	{
		if (auto* numVar = dynamic_cast<NumericVariable*>(iter->second.get()))
			return std::make_unique<NumericVariableToken>(numVar);
		if (auto* strVar = dynamic_cast<StringVariable*>(iter->second.get()))
			return std::make_unique<StringVariableToken>(strVar);
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

std::vector<std::unique_ptr<Token>> ParseExpression(StringIterator& iterator, ScriptModule& scriptModule)
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
		if (auto operator_ = iterator.ParseOperator())
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
					while (!operatorsOrFuncs.empty() && operatorsOrFuncs.top()->value->Precedes(operator_->value) && !IsOpenBracket(operatorsOrFuncs.top().get()))
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
			if (iterator.Peek() == '"')
			{
				auto str = iterator.GetStringInQuotationMarks();
				result.push_back(std::make_unique<StringConstantToken>(str));
			}
			else
			{
				const auto opStr = iterator.GetCurOperandString();
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
					else
					{
						result.push_back(std::make_unique<StringConstantToken>(opStr));
					}
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
			std::unique_ptr<OperandToken> varToken;
			if (auto* strToken = dynamic_cast<StringConstantToken*>(operand); strToken && ((varToken = ParseVariableToken(strToken->Value()))))
			{
				// variable
				tempTokens.push_back(std::move(varToken));
				stack.push(tempTokens.back().get());
			}
			else
			{
				stack.push(operand);
			}
		}
		else if (auto* operator_ = dynamic_cast<OperatorToken*>(token.get()))
		{
			std::unique_ptr<OperandToken> result;
			if (stack.size() < operator_->Value()->numOperands)
				throw ParseError("Invalid number of operands for operator " + std::to_string(operator_->Value()->numOperands));
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

bool OperatorOrFunction::Precedes(OperatorOrFunction* other) const
{
	if (auto* op = dynamic_cast<Operator*>(other))
	{
		if (op->numOperands == 1)
			return other->precedence < precedence;
	}
	return other->precedence <= precedence;
}

void ScriptModule::Compile()
{
	for (auto iter = scriptCompileLines.begin(); iter != scriptCompileLines.end(); ++iter)
	{
		auto& line = *iter;
		if (line.empty())
			continue;
		StringIterator iterator(line);
		curCompileLineIter = &iter;
		scriptRunLines.emplace_back(ParseExpression(iterator, *this));
	}
}

void ScriptModule::Execute()
{
	auto lineNum = 0u;
	try
	{
		for (auto& line : scriptRunLines)
		{
			++lineNum;
			EvaluateExpression(line.tokens);
		}
	}
	catch (const ParseError& e)
	{
		std::cout << "Syntax error on line " << lineNum << std::endl;
		std::cout << e.what() << std::endl;
	}
	
}

void ParseFile(const std::string& fileName)
{
	std::ifstream is(fileName);
	std::vector<std::string> scriptLines;
	do
	{
		scriptLines.emplace_back();
	} while (std::getline(is, scriptLines.back()));
	s_scriptModule = ScriptModule(scriptLines);
	s_scriptModule.Compile();
	s_scriptModule.Execute();
}

int main(int argc, char* argv[])
{
	if (argc == 2)
	{
		ParseFile(argv[1]);
	}
	else
	{
		
	}
}

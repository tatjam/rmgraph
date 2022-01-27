#pragma once
#include <string>
#include <unordered_map>
#include <vector>

struct MathContext;

using numer_t = double;

struct MathToken
{
	enum Type
	{
		VARIABLE,  			// x, y, a, b, c...
		NUMBER, 			// 10, 15, 1.4, pi, e...
		OPERATOR,			// + - * / ^
		FUNCTION,			// sqrt, cos... Affects the next group of tokens, so use parens!
		LPAREN,				// (
		RPAREN,				// )
	};

	Type type;
	std::string value;

	int get_precedence();
	std::optional<numer_t> get_value(MathContext* context);
	numer_t operate(numer_t x, numer_t y);
	numer_t function(numer_t x);
};



class MathExpression
{

public:
	// If set, contains the error code
	std::string error;
	std::vector<MathToken> tokens;
	// If it returns a value, it was possible to evaluate the expression down to a numeric
	// value, otherwise it was not!
	std::optional<numer_t> evaluate(MathContext* in_context);
};

// A context associates variables with expressions
struct MathContext
{
	// Fixed values are an expression which can be simplified to free values only (or to a numerical value)
	std::unordered_map<std::string, MathExpression> fixed_values;
	// Free values may be set externally to finally obtain a numerical result,
	// for example, parameters or the x and y variables
	std::unordered_map<std::string, numer_t> free_values;
};

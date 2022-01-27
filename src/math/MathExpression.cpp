#include "MathExpression.h"
#include <stack>
#include <queue>
#include <cmath>


std::optional<numer_t> MathExpression::evaluate(MathContext* in_context)
{
	error = "";
	// Shunting-yard algorithm from wikipedia

	// Holds tokens which affect other tokens (operators)
	std::stack<MathToken> operator_stack;
	std::queue<MathToken> output_queue;

	for(size_t i = 0; i < tokens.size(); i++)
	{
		if(tokens[i].type == MathToken::NUMBER || tokens[i].type == MathToken::VARIABLE)
		{
			output_queue.push(tokens[i]);
		}
		else if(tokens[i].type == MathToken::FUNCTION)
		{
			operator_stack.push(tokens[i]);
		}
		else if(tokens[i].type == MathToken::OPERATOR)
		{
			while(true)
			{
				if(operator_stack.empty())
				{
					break;
				}

				auto top = operator_stack.top();
				if(top.type != MathToken::LPAREN && top.get_precedence() > tokens[i].get_precedence())
				{
					output_queue.push(top);
					operator_stack.pop();
				}
				else
				{
					break;
				}
			}

			operator_stack.push(tokens[i]);
		}
		else if(tokens[i].type == MathToken::LPAREN)
		{
			operator_stack.push(tokens[i]);
		}
		else if(tokens[i].type == MathToken::RPAREN)
		{
			while(true)
			{
				if(operator_stack.empty())
				{
					error = "Right paren called without left paren";
					return {};
				}

				auto top = operator_stack.top();
				if(top.type != MathToken::LPAREN)
				{
					output_queue.push(top);
					operator_stack.pop();
				}
				else
				{
					break;
				}
			}

			if(operator_stack.empty())
			{
				error = "Right paren called without left paren";
				return {};
			}

			operator_stack.pop();

			if(!operator_stack.empty())
			{
				auto top = operator_stack.top();
				if (top.type == MathToken::FUNCTION)
				{
					output_queue.push(top);
					operator_stack.pop();
				}
			}

		}
	}

	while(!operator_stack.empty())
	{
		auto top = operator_stack.top();
		if(top.type == MathToken::LPAREN)
		{
			error = "Unmatched left paren";
			return {};
		}

		output_queue.push(top);
		operator_stack.pop();
	}

	// Now everything is in reverse polish notation (ie, value value operator)
	// let see if it can be reduced to a number
	std::stack<numer_t> rpn_stack;
	while(!output_queue.empty())
	{
		auto front = output_queue.front();
		output_queue.pop();

		if(front.type == MathToken::VARIABLE || front.type == MathToken::NUMBER)
		{
			auto value = front.get_value(in_context);
			if(!value.has_value())
			{
				error = "Could not resolve value";
				return {};
			}
			else
			{
				rpn_stack.push(value.value());
			}
		}
		else if(front.type == MathToken::OPERATOR)
		{
			numer_t v1 = rpn_stack.top();
			rpn_stack.pop();
			numer_t v2 = rpn_stack.top();
			rpn_stack.pop();
			rpn_stack.push(front.operate(v1, v2));
		}
		else if(front.type == MathToken::FUNCTION)
		{
			numer_t v1 = rpn_stack.top();
			rpn_stack.pop();
			rpn_stack.push(front.function(v1));
		}
		else
		{
			error = "A bad token made its way into RPN stack?";
			return {};
		}
	}

	if(!rpn_stack.empty())
	{
		numer_t val = rpn_stack.top();
		return val;
	}
	else
	{
		return {};
	}
}

int MathToken::get_precedence()
{
	if(type != OPERATOR)
	{
		fprintf(stderr, "get_precedence called on non-operator\n");
		exit(-1);
	}

	if(value == "+")
	{
		return 1;
	}
	else if(value == "-")
	{
		return 1;
	}
	else if(value == "*")
	{
		return 2;
	}
	else if(value == "/")
	{
		return 2;
	}
	else if(value == "^")
	{
		return 3;
	}
	else
	{
		fprintf(stderr, "get_precedence called on invalid operator\n");
		exit(-1);
	}
}

std::optional<numer_t> MathToken::get_value(MathContext* context)
{
	if(type == NUMBER)
	{
		if (value == "pi")
		{
			return M_PI;
		}
		else if (value == "e")
		{
			return M_E;
		}
		else
		{
			char *ending;
			numer_t val = strtod(value.c_str(), &ending);
			if (*ending != 0)
			{
				return {};
			}
			else
			{
				return val;
			}
		}
	}
	else
	{
		auto as_free_value = context->free_values.find(value);
		if(as_free_value != context->free_values.end())
		{
			return as_free_value->second;
		}

		auto as_fixed_value = context->fixed_values.find(value);
		if(as_fixed_value != context->fixed_values.end())
		{
			return as_fixed_value->second.evaluate(context);
		}
	}
}

numer_t MathToken::operate(numer_t x, numer_t y)
{
	if(value == "+")
	{
		return x + y;
	}
	else if(value == "-")
	{
		return x - y;
	}
	else if(value == "*")
	{
		return x * y;
	}
	else if(value == "/")
	{
		return x / y;
	}
	else if(value == "^")
	{
		return std::pow(x, y);
	}
}

numer_t MathToken::function(numer_t x)
{
	if(value == "sin")
	{
		return sin(x);
	}
	else if(value == "cos")
	{
		return cos(x);
	}
	else if(value == "tan")
	{
		return tan(x);
	}
	else if(value == "log")
	{
		return log(x);
	}
	// TODO: More functions

}

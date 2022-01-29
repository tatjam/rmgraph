#include "MathExpression.h"
#include <stack>
#include <queue>
#include <cmath>
#include "util.h"


std::optional<numer_t> MathExpression::evaluate(MathContext* in_context)
{
	error = "";
	// Shunting-yard algorithm from wikipedia

	// Holds tokens which affect other tokens (operators)
	std::optional<std::queue<MathToken>> output_queue_opt = get_rpn();
	if(!output_queue_opt.has_value())
	{
		return {};
	}

	auto output_queue = output_queue_opt.value();

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
			int arg_count = front.get_argument_count();
			std::vector<numer_t> values;
			for(int i = 0; i < arg_count; i++)
			{
				if(rpn_stack.empty())
				{
					error = "Invalid number of arguments to function";
					return {};
				}
				values.push_back(rpn_stack.top());
				rpn_stack.pop();
			}
			rpn_stack.push(front.function(values));
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

struct SimpleElement
{
	std::vector<size_t> tokens;
	int width;
};

void MathExpression::draw(int sx, int sy, framebuffer::FB* fb)
{
	// We do very simplistic math rendering, just handling fractions, square roots, and exponentiations
	auto rpn = get_rpn();
	if(!rpn.has_value() || true)
	{
		// Simplistic rendering
		draw_simple(sx, sy, fb);
		return;
	}

}


void MathExpression::draw_simple(int sx, int sy, framebuffer::FB* fb)
{
	int x = sx;
	int y = sy;
	bool cleared = false;
	// We simply put every token in a line, old calculator style
	for(size_t ti = 0 ; ti < tokens.size(); ti++)
	{
		auto& tok = tokens[ti];
		// Check differences to clear the area
		if(ti < old_tokens.size())
		{
			if (old_tokens[ti].type != tokens[ti].type || old_tokens[ti].value != tokens[ti].value && !cleared)
			{
				fb->draw_rect(x, sy, last_maxx, 70, WHITE, true);
				cleared = true;
			}
		}
		std::string text = "";
		// TODO: Proper width detection for characters using stb!
		int width = 34;
		int off = 0;
		int offx = 0;
		if(tok.type == MathToken::VARIABLE || tok.type == MathToken::OPERATOR || tok.type == MathToken::FUNCTION
		|| tok.type == MathToken::NUMBER)
		{
			text = tok.value;
			if(tok.value == "*")
			{
				text = "";
				off = -13;
			}
		}
		else if(tok.type == MathToken::LPAREN)
		{
			text = "(";
			off = -15;
		}
		else if(tok.type == MathToken::RPAREN)
		{
			text = ")";
			offx = -10;
		}
		else if(tok.type == MathToken::COMMA)
		{
			text = ",";
			off = -15;
		}
		else if(tok.type == MathToken::DUMMY)
		{
			text = "[]";
		}

		fb->draw_text(x + offx, y, text, 64);
		tok.render_x = x + offx;
		tok.render_y = y;
		int twidth = count_utf8(text) * width + off;
		tok.render_w = twidth;
		tok.render_h = 64;
		tok.render_offset = off;
		x += count_utf8(text) * width + off + offx;
	}

	last_maxx = x;
}

std::optional<std::queue<MathToken>> MathExpression::get_rpn()
{
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
		else if(tokens[i].type == MathToken::COMMA)
		{
			continue;
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

	return output_queue;

}

void MathExpression::draw_cursor(size_t working_pos, framebuffer::FB *fb)
{
	// Clear the prev cursor if present
	if(prev_cursorx1 - prev_cursorx0 > 0)
	{
		fb->draw_line(prev_cursorx0, prev_cursory, prev_cursorx1, prev_cursory, 2, WHITE);
	}

	int x = 0, y = 0, w = 0;
	if(tokens.size() == 0)
	{

	}
	else if(working_pos < tokens.size())
	{
		auto tok = tokens[working_pos];
		x = tok.render_x;
		y = tok.render_y + tok.render_h + 4;
		w = tok.render_w - tok.render_offset;
	}
	else if(working_pos >= tokens.size())
	{
		// Draw on the last pos
		auto last_tok = tokens.back();
		w = 20;
		x = last_tok.render_x + last_tok.render_w + 4;
		y = last_tok.render_y + last_tok.render_h + 4;
	}

	if(w != 0)
	{
		fb->draw_line(x, y, x + w, y, 2, BLACK);
	}
	prev_cursorx0 = x;
	prev_cursorx1 = x + w;
	prev_cursory = y;

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
		if (value == "\xCF\x80")
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

numer_t MathToken::operate(numer_t x, numer_t y) const
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

numer_t MathToken::function(std::vector<numer_t> args)
{
	// Single argument functions
	if(get_argument_count() == 1)
	{
		auto x = args[0];
		if(value == "sin")
			return sin(x);
		else if(value == "cos")
			return cos(x);
		else if(value == "tan")
			return tan(x);
		else if(value == "sinh")
			return sinh(x);
		else if(value == "cosh")
			return cosh(x);
		else if(value == "tanh")
			return tanh(x);
		else if(value == "asin")
			return asin(x);
		else if(value == "acos")
			return acos(x);
		else if(value == "atan")
			return atan(x);
		else if(value == "asinh")
			return asinh(x);
		else if(value == "acosh")
			return acosh(x);
		else if(value == "atanh")
			return atanh(x);
		else if(value == "log")
			return log(x);
		else if(value == "abs")
			return abs(x);
		else if(value == "\xE2\x88\x9A")
			return sqrt(x);
	}
	else if(get_argument_count() == 2)
	{
		auto x = args[0];
		auto y = args[1];
		if(value == "max")
		{
			return std::max(x, y);
		}
		else if(value == "min")
		{
			return std::min(x, y);
		}
		else if(value == "logb")
		{
			return log(x) / log(y);
		}
	}
	// TODO: More functions

}

int MathToken::get_argument_count() const
{
	if(value == "sin" || value == "cos" || value == "tan" || value == "log" || value == "abs" ||
	value == "sinh" || value == "cosh" || value == "tanh" || value == "asin" || value == "acos" ||
	value == "atan" || value == "asinh" || value == "acosh" || value == "atanh")
	{
		return 1;
	}
	else if(value == "max" || value == "min" || value == "logb")
	{
		return 2;
	}

	return 0;
}

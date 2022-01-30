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
		draw_advanced(sx, sy + 400, fb);
		return;
	}
	else
	{
		draw_advanced(sx, sy, fb);
		return;
	}

}

void MathExpression::get_render(size_t pos, int& width, int& off, int& offx, std::string& text)
{
	int base_width = 34;
	width = 0;
	off = 0;
	offx = 0;
	text = "";

	auto tok = tokens[pos];
	if(tok.type == MathToken::VARIABLE || tok.type == MathToken::OPERATOR || tok.type == MathToken::FUNCTION
	   || tok.type == MathToken::NUMBER)
	{
		text = tok.value;
		if(tok.value == "*")
		{
			text = "\xC2\xB7";
			// We don't display the * if to its right is a variable or function or parenthesis and to its left a number
			// (This is safe to do thanks to placeholders)
			if(tokens[pos - 1].type == MathToken::NUMBER &&
			   (tokens[pos + 1].type == MathToken::VARIABLE || tokens[pos + 1].type == MathToken::LPAREN ||
				tokens[pos + 1].type == MathToken::FUNCTION))
			{
				text = "";
				off = -10;
			}
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

	width = count_utf8(text) * base_width;

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
		int width = 0;
		int off = 0;
		int offx = 0;

		get_render(ti, width, off, offx, text);

		fb->draw_text(x + offx, y, text, 64);
		tok.render_x = x + offx;
		tok.render_y = y;
		int twidth = width + off;
		tok.render_w = twidth;
		tok.render_h = 64;
		tok.render_offset = off;
		x += twidth + offx;
	}

	last_maxx = x;
}

std::vector<size_t> MathExpression::find_minimum_depth_operators(std::string op_value, int lpar, int rpar)
{
	// We find the divisions higher up in the hierarchy
	std::vector<size_t> lowest_found;

	int pdepth = 0;
	int min_pdepth = INT_MAX;
	for(size_t i = lpar; i < rpar; i++)
	{
		if(tokens[i].type == MathToken::LPAREN)
		{
			pdepth++;
		}
		else if(tokens[i].type == MathToken::RPAREN)
		{
			pdepth--;
		}
		else if(tokens[i].type == MathToken::OPERATOR && tokens[i].value == op_value)
		{
			if(pdepth < min_pdepth)
			{
				lowest_found.clear();
				min_pdepth = pdepth;
			}

			// Now we can push it
			if(pdepth == min_pdepth)
			{
				lowest_found.push_back(i);
			}
		}
	}

	return lowest_found;

}

void MathExpression::dimension_div(size_t pos)
{
	int nw, dw, nh, dh;
	find_div_num_denum_sizes(pos, true, nw, dw, nh, dh);
	tokens[pos].render_w = max(nw, dw) + 10;
	tokens[pos].render_h = nh + dh + 10;
}

std::pair<int, int> MathExpression::find_left_right_pair(size_t pos)
{
	int num_open_par, den_close_par;

	int pardepth = 0;
	// Find numerator lpar
	int i = pos - 1;
	do
	{
		if(tokens[i].type == MathToken::LPAREN)
		{
			pardepth--;
		}
		else if(tokens[i].type == MathToken::RPAREN)
		{
			pardepth++;
		}
		i--;
	} while(pardepth != 0);

	num_open_par = i + 1;

	// Find denominator rpar
	i = pos + 1;
	do
	{
		if(tokens[i].type == MathToken::RPAREN)
		{
			pardepth--;
		}
		else if(tokens[i].type == MathToken::LPAREN)
		{
			pardepth++;
		}
		i++;
	} while(pardepth != 0);

	den_close_par = i - 1;

	return std::make_pair(num_open_par, den_close_par);
}


void MathExpression::draw_advanced(int sx, int sy, framebuffer::FB *fb)
{
	// TODO: Smart cleaning
	fb->draw_rect(sx, sy - 150, 1404 - sx, 500, WHITE, true);

	// Start at the top
	auto lowest_divs = find_minimum_depth_operators("/", 0, tokens.size());

	for(size_t div : lowest_divs)
	{
		dimension_div(div);
	}

	draw_advanced_expr(sx, sy, 0, tokens.size(), fb, false);

}

void MathExpression::draw_div(int sx, int sy, size_t pos, framebuffer::FB* fb)
{
	// first, we draw the line
	int div_width = tokens[pos].render_w;

	fb->draw_line(sx, sy + 32, sx + div_width - 6, sy + 32, 3, BLACK);
	// Now we recursively draw num and denum, centered
	int nw, dw, nh, dh;
	find_div_num_denum_sizes(pos, false, nw, dw, nh, dh);
	auto[left, right] = find_left_right_pair(pos);
	int top_free = div_width - nw;
	int bottom_free = div_width - dw;
	// Stuff grows upwards, so the numerator is never a problem
	draw_advanced_expr(sx + top_free / 2, sy - 32, left, pos, fb, true);
	// The denominator we need to consider height
	draw_advanced_expr(sx + bottom_free / 2, sy + dh, pos + 1, right + 1, fb, true);

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

void MathExpression::find_div_num_denum_sizes(size_t pos, bool dimension_child, int& nw, int& dw, int& nh, int& dh)
{
	// We must find all other divs in the parenthesis and dimension them

	auto[open_par_num, close_par_den] = find_left_right_pair(pos);
	std::vector<size_t> num_divs = find_minimum_depth_operators("/", open_par_num, pos - 1);
	std::vector<size_t> den_divs = find_minimum_depth_operators("/", pos + 1, close_par_den);

	std::vector<std::pair<int, int>> num_divs_par;
	std::vector<std::pair<int, int>> den_divs_par;
	// Dimension the sub divs and obtain their parenthesis
	for(size_t num_div : num_divs)
	{
		auto par = find_left_right_pair(num_div);
		num_divs_par.push_back(par);
		if(dimension_child)
		{
			dimension_div(num_div);
		}
	}
	for(size_t den_div : den_divs)
	{
		auto par = find_left_right_pair(den_div);
		den_divs_par.push_back(par);
		if(dimension_child)
		{
			dimension_div(den_div);
		}
	}

	// Now we can dimension ourselves, count symbols in parenthesis skipping them
	int num_size = 0;
	int den_size = 0;

	// Heights are very easy
	int num_height = 0;
	int den_height = 0;

	auto dimension = [this](int& target_size, int& target_height, int start, int end,
							std::vector<std::pair<int, int>> pars, std::vector<size_t> divs)
	{
		for(int i = start; i < end; i++)
		{
			bool inside_pars = false;
			// Make sure we are outside numerator divisions
			for(std::pair<int, int> num_par : pars)
			{
				if(num_par.first <= i && num_par.second >= i)
				{
					inside_pars = true;
					break;
				}
			}

			if(inside_pars)
			{
				// If we find a division that's top level, we must consider it too!
				if(tokens[i].type == MathToken::OPERATOR && tokens[i].value == "/" &&
				   std::find(divs.begin(), divs.end(), i) != divs.end())
				{
					// Get the division size that was computed previously
					target_size += tokens[i].render_w;
					target_height = std::max(target_height, tokens[i].render_h);
				}
			}
			else
			{
				int width, off, offx;
				std::string text;
				get_render(i, width, off, offx, text);
				target_size += width + off + offx;
				target_height = std::max(target_height, 32);
			}
		}
	};

	dimension(num_size, num_height, open_par_num + 1, pos - 1, num_divs_par, num_divs);
	dimension(den_size, den_height, pos + 2, close_par_den, den_divs_par, den_divs);

	nw = num_size;
	dw = den_size;
	nh = num_height;
	dh = den_height;
}

void MathExpression::draw_advanced_expr(int sx, int sy, int start, int end, framebuffer::FB *fb, bool in_pars)
{
	auto lowest_divs = find_minimum_depth_operators("/", start, end);
	std::vector<std::pair<int, int>> lowest_divs_par;

	for(size_t div : lowest_divs)
	{
		lowest_divs_par.push_back(find_left_right_pair(div));
	}

	// Now we draw symbols like in simple mode but being careful of divisions
	// which we must draw recursively
	int x = sx;
	int y = sy;
	// We skip the pars
	if(in_pars)
	{
		start++;
		end--;
	}
	for(size_t ti = start; ti < end; ti++)
	{
		bool in_div = false;
		for(size_t j = 0; j < lowest_divs_par.size(); j++)
		{
			if(lowest_divs_par[j].first <= ti && lowest_divs_par[j].second >= ti)
			{
				in_div = true;
				break;
			}
		}

		if(in_div)
		{
			// If we find the division symbol, draw the fraction recursively
			if(tokens[ti].type == MathToken::OPERATOR && tokens[ti].value == "/" &&
			   std::find(lowest_divs.begin(), lowest_divs.end(), ti) != lowest_divs.end())
			{
				draw_div(x, y, ti, fb);
				x += tokens[ti].render_w;
			}
		}
		else
		{
			auto &tok = tokens[ti];
			std::string text = "";
			int width, off, offx;

			get_render(ti, width, off, offx, text);

			fb->draw_text(x + offx, y, text, 64);
			//tok.render_x = x + offx;
			//tok.render_y = y;
			int twidth = width + off;
			//tok.render_w = twidth;
			//tok.render_h = 64;
			//tok.render_offset = off;
			x += twidth + offx;
		}
	}

	last_maxx = x;

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

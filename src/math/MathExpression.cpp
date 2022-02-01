#include "MathExpression.h"
#include <stack>
#include <queue>
#include <cmath>
#include "util.h"

#define DIV_EXTRA_SPACE 20

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
	// Remove all interactions first
	for(MathToken& tok : tokens)
	{
		tok.render_x = 0; tok.render_y = 0; tok.render_w = 0; tok.render_h = 0;
	}
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

void MathExpression::get_render(size_t pos, int& width, int& off, int& offx, std::string& text, bool simp_mult)
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
		if(tok.value == "*" && simp_mult)
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

		get_render(ti, width, off, offx, text, false);

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
	// This finds operators that are not within the parenthesis of other operators
	std::vector<size_t> lowest_found;
	for(size_t i = lpar; i < rpar; i++)
	{
		if(tokens[i].type == MathToken::OPERATOR && tokens[i].value == op_value)
		{
			lowest_found.push_back(i);
		}
	}

	std::unordered_map<size_t, std::pair<int, int>> pars;
	// Now find all parenthesis
	for(size_t op : lowest_found)
	{
		pars[op] = find_left_right_pair(op);
	}

	// Now remove all that are within parenthesis
	for(auto it = lowest_found.begin(); it != lowest_found.end();)
	{
		size_t pos = *it;
		bool in_pars = false;
		for(const auto& pair : pars)
		{
			if((pair.second.first < pos && pair.first > pos) ||
				pair.first < pos && pair.second.second > pos)
			{
				in_pars = true;
				break;
			}
		}
		if(in_pars)
		{
			it = lowest_found.erase(it);
		}
		else
		{
			it++;
		}
	}

	return lowest_found;

}

void MathExpression::dimension_div(size_t pos)
{
	int nw, dw, nh, dh;
	find_div_num_denum_sizes(pos, true, nw, dw, nh, dh);
	tokens[pos].render_w = max(nw, dw) + DIV_EXTRA_SPACE * 2;
	tokens[pos].render_h = nh + dh + 10;
	tokens[pos].old_height = tokens[pos].render_h;
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
	} while(pardepth != 0 && i >= 0);

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
	} while(pardepth != 0 && i < tokens.size());

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
	int div_width = tokens[pos].render_w - DIV_EXTRA_SPACE;

	fb->draw_line(sx, sy + 32, sx + div_width, sy + 32, 3, BLACK);
	// Now we recursively draw num and denum, centered
	int nw, dw, nh, dh;
	find_div_num_denum_sizes(pos, false, nw, dw, nh, dh);
	auto[left, right] = find_left_right_pair(pos);
	int top_free = div_width - nw;
	int bottom_free = div_width - dw;
	draw_advanced_expr(sx + top_free / 2, sy - nh - 10, left, pos, fb, true);
	// The denominator we need to consider height
	draw_advanced_expr(sx + bottom_free / 2, sy + dh + 10, pos + 1, right + 1, fb, true);

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
				get_render(i, width, off, offx, text, true);
				target_size += width + off + offx;
				target_height = std::max(target_height, 28);
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
	std::unordered_map<size_t, std::pair<int, int>> div_to_pars;
	// Simple exponents are those formed without any division in the exponent, which we can draw with ease,
	// and we also make sure they don't contain any further exponents
	std::vector<std::pair<int, int>> simple_exponents;
	std::vector<int> ignore;
	std::vector<int> exit_par;
	// Find simple exponents
	for(size_t i = start; i < end; i++)
	{
		if(tokens[i].type == MathToken::OPERATOR && tokens[i].value == "^")
		{
			auto[left, right] = find_left_right_pair(i);
			// Explore the exponent, see if it's a simple one
			bool is_simple = true;
			for(size_t j = i + 1; j < right; j++)
			{
				if(tokens[j].type == MathToken::OPERATOR && (tokens[j].value == "/" || tokens[j].value == "^"))
				{
					is_simple = false;
					break;
				}
			}
			if(!is_simple)
			{
				break;
			}

			simple_exponents.push_back(std::make_pair(i + 1, right));
			ignore.push_back(i + 1);
			ignore.push_back(right);
			ignore.push_back(i);
			// Now, see if we can remove parenthesis of the base, this is only done if it doesn't contain a fraction
			// and it doesn't already have parens
			bool ignore_left = true;
			std::vector<std::pair<int, int>> div_parens;

			for (size_t j = left; j < i; j++)
			{
				if (tokens[j].type == MathToken::OPERATOR && tokens[j].value == "/")
				{
					div_parens.push_back(find_left_right_pair(j));
				}
			}

			// If there are already user parens, then we don't need to draw our parens
			if(tokens[i - 2].type == MathToken::RPAREN)
			{
				for (size_t j = 0; j < div_parens.size(); j++)
				{
					if (div_parens[j].second == i - 2)
					{
						ignore_left = false;
						break;
					}
				}
			}

			if(ignore_left)
			{
				ignore.push_back(left);
				ignore.push_back(i - 1);
			}
			exit_par.push_back(right);
		}
	}

	for(size_t div : lowest_divs)
	{
		auto pars = find_left_right_pair(div);
		lowest_divs_par.push_back(pars);
		div_to_pars[div] = pars;
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
				// Disable interaction with the fraction, but enable it for the right parenthesis
				auto pars = div_to_pars[ti];
				tokens[pars.second].render_x = x - 15;
				tokens[pars.second].render_y = y;
				if(tokens.size() <= pars.second + 1)
				{
					// Specially big and easy to click
					tokens[pars.second].render_w = 45;
				}
				else
				{
					tokens[pars.second].render_w = 30;
				}
				tokens[pars.second].render_h = 27 * 2;

				tokens[ti].render_w = tokens[ti].render_h = tokens[ti].render_x = tokens[ti].render_y = 0;
			}
		}
		else
		{
			bool in_ignore = std::find(ignore.begin(), ignore.end(), ti) != ignore.end();
			// Check if we are inside a simple exponent
			bool in_exponent = false;
			for(size_t j = 0; j < simple_exponents.size(); j++)
			{
				if(simple_exponents[j].first < ti && simple_exponents[j].second > ti)
				{
					in_exponent = true;
					break;
				}
			}
			if(std::find(exit_par.begin(), exit_par.end(), ti) != exit_par.end())
			{
				// We just give it dimensions to be able to exit the exponent
				tokens[ti].render_x = x;
				tokens[ti].render_y = y;
				tokens[ti].render_w = 30;
				tokens[ti].render_h = 64;
				x += 15;
			}
			else if(!in_ignore && (tokens[ti].type == MathToken::LPAREN || tokens[ti].type == MathToken::RPAREN))
			{
				bool left = tokens[ti].type == MathToken::RPAREN;
				int ptr = ti;
				int par_depth = 0;
				// The code is fairly simple because we only need to find the max
				int max_v_size = 32;
				while(true)
				{
					if(tokens[ptr].type == (left ? MathToken::LPAREN : MathToken::RPAREN))
					{
						par_depth--;
					}
					else if(tokens[ptr].type == (left ? MathToken::RPAREN : MathToken::LPAREN))
					{
						par_depth++;
					}
					// We only care about divs because they are the only ones which grow vertically
					// and have been previously dimensioned
					else if(tokens[ptr].type == MathToken::OPERATOR && tokens[ptr].value == "/")
					{
						max_v_size = max(max_v_size, tokens[ptr].old_height - 10);
					}

					if(par_depth == 0 || ptr < 0 || ptr >= tokens.size())
					{
						break;
					}
					ptr += left ? -1 : 1;
				}

				auto &tok = tokens[ti];
				int px = x + (left ? 0 : 12);
				int yoff = max_v_size == 32 ? 0 : max_v_size / 3;
				int cx = x + (left ? 6 : 6);
				int cy = y + max_v_size;
				// We have to draw a bezier manually to prevent the parenthesis from growing very thick
				fb->draw_bezier(px, y + yoff + 4, cx, cy, cx,cy,
								px, y + max_v_size * 2 - yoff - 4, 4, BLACK);
				tok.render_x = x;
				tok.render_y = y;
				tok.render_w = 32;
				tok.render_h = max_v_size * 2;
				tok.render_offset = 0;
				x += 20;
			}
			else if(!in_ignore)
			{

				auto &tok = tokens[ti];
				std::string text = "";
				int width, off, offx, ry = y, height = 27 * 2;

				get_render(ti, width, off, offx, text, true);
				if(in_exponent)
				{
					height *= 0.7;
					width *= 0.7;
					ry -= 5;
					fb->draw_text(x + offx, ry, text, 64 * 0.7);
				}
				else
				{
					fb->draw_text(x + offx, y, text, 64);
				}

				tok.render_x = x + offx;
				tok.render_y = ry;
				int twidth = width + off;
				tok.render_w = twidth;
				tok.render_h = height;
				tok.render_offset = off;
				x += twidth + offx;
			}
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

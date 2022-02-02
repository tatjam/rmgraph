#include "MathKeyboard.h"
#include "Buttons.h"
#include "util/Dimensions.h"


void MathKeyboard::on_mouse_down(input::SynMotionEvent &ev)
{
	if(was_down)
	{
		return;
	}
	was_down = true;
	if(ev.y > Dimensions::keyboard_start)
	{
		active_buttons->handle(ev.x, ev.y);
	}
	else if(ev.y < Dimensions::top_end)
	{
		// Top bar click
		top_buttons.handle(ev.x, ev.y);
	}
	else
	{
		// TODO: Proper bounds for expression click
		if(!working)
		{
			return;
		}

		for(size_t i = 0; i < working->tokens.size(); i++)
		{
			const auto& tok = working->tokens[i];
			if(ev.x > tok.render_x && ev.x < tok.render_x + tok.render_w && ev.y > tok.render_y &&
				ev.y < tok.render_y + tok.render_h)
			{
				working_pos = i;
				dirty = true;
				draw_expression = true;
				update_graph = true;
				return;
			}
		}
	}
}

void MathKeyboard::render()
{
	top_buttons.draw(fb);
	active_buttons->draw(fb);

	if(draw_expression && working)
	{
		working->draw(40, 500, fb);
		working->draw_cursor(working_pos, fb);
		draw_expression = false;
	}
}

MathKeyboard::MathKeyboard(int x, int y, int w, int h) : Widget(x, y, w, h),
	basic_buttons(&dirty), top_buttons(&dirty)
{
	basic_buttons.on_click += [this](Button& b){this->on_click(&b);};
	top_buttons.on_click += [this](Button& b){this->on_click(&b);};

	working_pos = -1;
	active_buttons = &basic_buttons;
	is_done = false;

	int start = Dimensions::keyboard_start;
	int top_end = Dimensions::top_end;


	basic_buttons.buttons.emplace_back(156 * 0, start + 156 * 0, 156, 156, "+");
	basic_buttons.buttons.emplace_back(156 * 1, start +  156 * 0, 156, 156, "-");
	basic_buttons.buttons.emplace_back(156 * 2, start +  156 * 0, 156, 156, "^");
	basic_buttons.buttons.emplace_back(156 * 3, start +  156 * 0, 156, 156, "(");
	basic_buttons.buttons.emplace_back(156 * 4, start +  156 * 0, 156, 156, ")");
	basic_buttons.buttons.emplace_back(156 * 5, start +  156 * 0, 156, 156, "a_");
	basic_buttons.buttons.emplace_back(156 * 6, start +  156 * 0, 156, 156, "\xCF\x80", "pi");
	basic_buttons.buttons.emplace_back(156 * 7, start +  156 * 0, 156, 156, "e");
	basic_buttons.buttons.emplace_back(156 * 8, start +  156 * 0, 156, 156, "x");

	basic_buttons.buttons.emplace_back(156 * 0, start +  156 * 1, 156, 156, "*");
	basic_buttons.buttons.emplace_back(156 * 1, start +  156 * 1, 156, 156, "/");
	basic_buttons.buttons.emplace_back(156 * 2, start +  156 * 1, 156, 156, "a\xC2\xB2", "a^2");
	basic_buttons.buttons.emplace_back(156 * 3, start +  156 * 1, 156, 156, "\xE2\x88\x9A", "sqrt");
	basic_buttons.buttons.emplace_back(156 * 4, start +  156 * 1, 156, 156, "log");
	basic_buttons.buttons.emplace_back(156 * 5, start +  156 * 1, 156, 156, "1");
	basic_buttons.buttons.emplace_back(156 * 6, start +  156 * 1, 156, 156, "2");
	basic_buttons.buttons.emplace_back(156 * 7, start +  156 * 1, 156, 156, "3");
	basic_buttons.buttons.emplace_back(156 * 8, start +  156 * 1, 156, 156, "y");

	basic_buttons.buttons.emplace_back(156 * 0, start +  156 * 2, 156, 156, "sin");
	basic_buttons.buttons.emplace_back(156 * 1, start +  156 * 2, 156, 156, "cos");
	basic_buttons.buttons.emplace_back(156 * 2, start +  156 * 2, 156, 156, "tan");
	basic_buttons.buttons.emplace_back(156 * 3, start +  156 * 2, 156, 156, ">");
	basic_buttons.buttons.emplace_back(156 * 4, start +  156 * 2, 156, 156, "<");
	basic_buttons.buttons.emplace_back(156 * 5, start +  156 * 2, 156, 156, "4");
	basic_buttons.buttons.emplace_back(156 * 6, start +  156 * 2, 156, 156, "5");
	basic_buttons.buttons.emplace_back(156 * 7, start +  156 * 2, 156, 156, "6");
	basic_buttons.buttons.emplace_back(156 * 8, start +  156 * 2, 156, 156, "a");

	basic_buttons.buttons.emplace_back(156 * 0, start +  156 * 3, 156, 156, "sinh");
	basic_buttons.buttons.emplace_back(156 * 1, start +  156 * 3, 156, 156, "cosh");
	basic_buttons.buttons.emplace_back(156 * 2, start +  156 * 3, 156, 156, "tanh");
	basic_buttons.buttons.emplace_back(156 * 3, start +  156 * 3, 156, 156, "\xE2\x89\xA5"); // >=
	basic_buttons.buttons.emplace_back(156 * 4, start +  156 * 3, 156, 156, "\xE2\x89\xA4"); // <=
	basic_buttons.buttons.emplace_back(156 * 5, start +  156 * 3, 156, 156, "7");
	basic_buttons.buttons.emplace_back(156 * 6, start +  156 * 3, 156, 156, "8");
	basic_buttons.buttons.emplace_back(156 * 7, start +  156 * 3, 156, 156, "9");
	basic_buttons.buttons.emplace_back(156 * 8, start +  156 * 3, 156, 156, "b");

	basic_buttons.buttons.emplace_back(156 * 0, start +  156 * 4, 156 * 2, 156, "Func");
	basic_buttons.buttons.emplace_back(156 * 2, start +  156 * 4, 156, 156, "=");
	basic_buttons.buttons.emplace_back(156 * 3, start +  156 * 4, 156, 156, "=P");
	basic_buttons.buttons.emplace_back(156 * 4, start +  156 * 4, 156, 156, ",");
	basic_buttons.buttons.emplace_back(156 * 5, start +  156 * 4, 156, 156, ".");
	basic_buttons.buttons.emplace_back(156 * 6, start +  156 * 4, 156, 156, "0");
	basic_buttons.buttons.emplace_back(156 * 7, start +  156 * 4, 156 * 2, 156, "Symb");

	top_buttons.buttons.emplace_back(0, 0, 250, top_end, "Undo");
	top_buttons.buttons.emplace_back(250, 0, 250, top_end, "Redo");
	top_buttons.buttons.emplace_back(500, 0, 250, top_end, "Erase");
	top_buttons.buttons.emplace_back(1404 - 250, 0, 250, top_end, "Done");

	cur_screen = BASIC;
	draw_expression = true;
}

void MathKeyboard::on_mouse_up(input::SynMotionEvent &ev)
{
	was_down = false;
}

static void insert_or_push(std::vector<MathToken>& target, int pos, MathToken tok)
{
	if(pos + 1 >= target.size())
	{
		target.push_back(tok);
	}
	else
	{
		target.insert(target.begin() + pos + 1, tok);
	}
}

static void insert_or_push(std::vector<MathToken>& target, int pos, std::vector<MathToken>& tok)
{
	if(pos + 1 >= target.size())
	{
		for(int i = 0; i < tok.size(); i++)
		{
			target.push_back(tok[i]);
		}
	}
	else
	{
		target.insert(target.begin() + pos + 1, tok.begin(), tok.end());
	}
}

static void replace_dummy(std::vector<MathToken>& target, int pos, std::vector<MathToken>& tok)
{
	target[pos] = tok[0];
	target.insert(target.begin() + pos + 1, tok.begin() + 1, tok.end());
}

// working_pos is to the left of where the next written character will be inserted
// unless it's over a dummy in which case the dummy is replaced by the inserted character
void MathKeyboard::on_click(Button* b)
{
	if(handle_control_buttons(b))
	{
		return;
	}

	if(working == nullptr)
	{
		return;
	}

	working->old_tokens = working->tokens;

	MathToken placeholder, lparen, rparen, comma, mult;
	placeholder.type = MathToken::DUMMY;
	lparen.type = MathToken::LPAREN;
	rparen.type = MathToken::RPAREN;
	comma.type = MathToken::COMMA;
	mult.type = MathToken::OPERATOR; mult.value = "*";


	// All these tokens are inserted as a whole group
	std::vector<MathToken> tokl;
	tokl.emplace_back();

	bool build_number = false;
	int insert_placeholder = 0;
	bool needs_prev_placeholder = false;
	bool insert_parens_around_prev = false;
	bool parens_around_prev_include_mult = true;
	bool insert_parens_around_placeholder = false;

	// WARNING: If you create a multi-token, don't use tok after pushing to tokl!
	// (it will be revalidated after the if block)
	if(b->id == "+" || b->id == "-" || b->id == "*")
	{
		// Simple operators, we insert them and a placeholder
		tokl[0].type = MathToken::OPERATOR;
		tokl[0].value = b->id;
		insert_placeholder = 1;
		needs_prev_placeholder = true;
	}
	else if(b->id == "=" && !has_equals)
	{
		tokl[0].type = MathToken::OPERATOR;
		tokl[0].value = "=",
		insert_placeholder = true;
		needs_prev_placeholder = true;
		has_equals = true;
	}
	else if(b->id == "/" || b->id == "^")
	{
		tokl[0].type = MathToken::OPERATOR;
		tokl[0].value = b->id;
		insert_placeholder = 1;
		insert_parens_around_placeholder = true;
		insert_parens_around_prev = true;
		if(b->id == "^")
		{
			parens_around_prev_include_mult = false;
		}
		needs_prev_placeholder = true;
	}
	// Single argument functions
	else if(b->id == "sin" || b->id == "cos" || b->id == "tan" || b->id == "sinh" ||
		b->id == "cosh" || b->id == "tanh" || b->id == "sqrt")
	{
		tokl[0].type = MathToken::FUNCTION;
		tokl[0].value = b->id;
		insert_placeholder = 1;
		insert_parens_around_placeholder = true;
	}
	// Two argument functions
	else if(b->id == "max")
	{
		tokl[0].type = MathToken::FUNCTION;
		tokl[0].value = b->id;
		insert_placeholder = 2;
		insert_parens_around_placeholder = true;
	}
	else if(b->id == "a^2")
	{
		// Insert ^2 block
		tokl[0].type = MathToken::OPERATOR;
		tokl[0].value = "^";
		needs_prev_placeholder = true;
		insert_parens_around_prev = true;
		parens_around_prev_include_mult = false;
		tokl.emplace_back();
		tokl[1].type = MathToken::LPAREN;
		tokl.emplace_back();
		tokl[2].type = MathToken::NUMBER;
		tokl[2].value = "2";
		tokl.emplace_back();
		tokl[3].type = MathToken::RPAREN;
	}
	else if(b->id == "(")
	{
		tokl[0].type = MathToken::LPAREN;
	}
	else if(b->id == ")")
	{
		tokl[0].type = MathToken::RPAREN;
	}
	else if(is_buildable_number(b->id))
	{
		tokl[0].type = MathToken::NUMBER;
		tokl[0].value = b->id;
		build_number = true;
	}
	// Special numbers
	else if(b->id == "pi")
	{
		tokl[0].type == MathToken::NUMBER;
		// We use UTF-8 in the calculator
		tokl[0].value = b->as_char;
	}
	else
	{
		tokl[0].type == MathToken::VARIABLE;
		tokl[0].value = b->as_char;
	}

	auto& tok = tokl[0];

	auto& tokens = working->tokens;

	dirty = 1;
	draw_expression = true;

	if(build_number)
	{
		bool dont_add = false;
		if(!tokens.empty() && tokens[working_pos].type == MathToken::OPERATOR)
		{
			// We try to replace the number to the right if it's easy to do, otherwise do nothing
			// This is safe to do thanks to placeholders
			if(tokens[working_pos + 1].type == MathToken::NUMBER)
			{
				tokens[working_pos + 1].value = "";
				working_pos++;
			}
			else if(tokens[working_pos + 1].type == MathToken::LPAREN &&
				tokens.size() > working_pos + 1 && tokens[working_pos + 2].type == MathToken::NUMBER)
			{
				tokens[working_pos + 2].value = "";
				working_pos += 2;
			}
			else
			{
				dont_add = true;
			}
		}
		if(!dont_add)
		{
			// If there's another buildable number previously, add to it
			if (!tokens.empty() && tokens[working_pos].type == MathToken::NUMBER &&
				is_buildable_number(tokens[working_pos].value))
			{
				// Check that it doesn't have two decimal points
				if (tok.value == ".")
				{
					for (size_t i = 0; i < tokens[working_pos].value.size(); i++)
					{
						if (tokens[working_pos].value[i] == '.')
						{
							// We don't do anything in this case
							return;
						}
					}
				}

				tokens[working_pos].value += tok.value;
				// We don't advance working pos!
			}
			else
			{
				// Add the number or replace dummy
				if (!tokens.empty() && tokens[working_pos].type == MathToken::DUMMY)
				{
					// We replace the dummy which is already properly parenthesized, etc...
					tokens[working_pos] = tok;
				}
				else
				{
					// We are adding next to something, insert next to working_pos and advance
					insert_or_push(tokens, working_pos, tok);
					working_pos++;
				}
			}
		}
	}
	else if(!tokens.empty() && tokens[working_pos].type == MathToken::OPERATOR &&
		tok.type == MathToken::OPERATOR)
	{
		// Replace the operator or do nothing while trying to add a group or special operators
		if(tokl.size() == 1 && tok.value != "/" && tok.value != "^")
		{
			tokens[working_pos].value = tok.value;
		}

	}
	else
	{
		// Now do the smart insertion
		if (!tokens.empty() && tokens[working_pos].type == MathToken::DUMMY
			&& tok.type != MathToken::OPERATOR)
		{
			// We replace the dummy which is already properly parenthesized, etc...
			replace_dummy(tokens, working_pos, tokl);
		}
		// If we add ( to the right of a number / variable, add a multiplication sign too
		else if(!tokens.empty() && tok.type == MathToken::LPAREN &&
			(tokens[working_pos].type == MathToken::NUMBER || tokens[working_pos].type == MathToken::VARIABLE))
		{
			// Special case, if to the right is an operator, we replace it by the multiplication
			if(tokens.size() > working_pos + 1 && tokens[working_pos + 1].type == MathToken::OPERATOR)
			{
				tokens[working_pos + 1].value = "*";
			}
			else
			{
				insert_or_push(tokens, working_pos, mult);
			}

			working_pos++;
			insert_or_push(tokens, working_pos, tok);
			working_pos++;

			// Now, we may need placeholders, or just to close the parenthesis to the right
		}
		else
		{
			// We are adding next to something, insert to the right of working_pos
			insert_or_push(tokens, working_pos, tokl);
			working_pos+=tokl.size();
		}

		int first_placeholder = -1;
		if (needs_prev_placeholder)
		{
			// A prev placeholder is added if the operator is not preceded by anything, or its preceded
			// by an opening parenthesis or operator
			if (working_pos == tokl.size() - 1)
			{
				// We add to the start! Careful
				tokens.insert(tokens.begin(), placeholder);
				first_placeholder = 0;
				working_pos++;
			}
			else if (tokens[working_pos - tokl.size()].type == MathToken::LPAREN
				|| tokens[working_pos - tokl.size()].type == MathToken::OPERATOR)
			{

				insert_or_push(tokens, working_pos - 1, placeholder);
				working_pos++;
				first_placeholder = working_pos - tokl.size() + 1;
			}
		}

		if (insert_parens_around_prev)
		{
			// This is actually quite complex, we parenthesize until we find:
			// a (+ or -) or we reach the end of the document
			// We respect parenthesis so if we find a + or - inside parenthesis it doesn't matter
			int ptr = working_pos - tokl.size() + 1;
			int paren_depth = 0;
			int finish = -1;
			while(ptr != 0)
			{
				if(tokens[ptr].type == MathToken::RPAREN)
				{
					paren_depth++;
				}
				else if(tokens[ptr].type == MathToken::LPAREN)
				{
					paren_depth--;
				}
				else if(tokens[ptr].type == MathToken::OPERATOR &&
					(tokens[ptr].value == "+" || tokens[ptr].value == "-" ||
					(tokens[ptr].value == "*" && !parens_around_prev_include_mult) ||
					tokens[ptr].get_precedence() == INT_MAX)
					&& paren_depth == 0 || paren_depth < 0)
				{
					finish = ptr;
					break;
				}

				ptr--;
			}

			insert_or_push(tokens, finish, lparen);
			working_pos++;
			insert_or_push(tokens, working_pos - tokl.size(), rparen);
			working_pos++;
		}

		if (insert_parens_around_placeholder)
		{
			insert_or_push(tokens, working_pos, lparen);
			working_pos++;
		}
		for (int i = 0; i < insert_placeholder; i++)
		{
			// Insert place holders following the element with commas
			insert_or_push(tokens, working_pos, placeholder);
			working_pos++;
			if(i == 0 && first_placeholder == -1)
			{
				first_placeholder = working_pos;
			}
			if (i != insert_placeholder - 1)
			{
				insert_or_push(tokens, working_pos, comma);
				working_pos++;
			}
		}
		if (insert_parens_around_placeholder)
		{
			insert_or_push(tokens, working_pos, rparen);
			working_pos++;
		}

		// Go back to the first dummy
		if(first_placeholder != -1)
		{
			working_pos = first_placeholder;
		}

		if(working_pos >= tokens.size())
		{
			working_pos = tokens.size();
		}

	}
}

bool MathKeyboard::is_buildable_number(std::string str)
{
	for(size_t i = 0; i < str.size(); i++)
	{
		if(!isdigit(str[i]) && str[i] != '.')
		{
			return false;
		}
	}

	return true;
}

bool MathKeyboard::handle_control_buttons(Button *b)
{
	if(b->as_char == "Done")
	{
		is_done = true;
		return true;
	}
	else if(b->as_char == "Undo")
	{
		return true;
	}
	else if(b->as_char == "Redo")
	{
		return true;
	}
	else if(b->as_char == "Erase")
	{
		return true;
	}

	return false;
}

void MathKeyboard::on_enter_kb()
{
	dirty = 1;
	draw_expression = true;
	top_buttons.draw_buttons = true;
	active_buttons->draw_buttons = true;
	if(working)
	{
		if(working->tokens.empty())
		{
			working_pos = -1;
		}
		else
		{
			working_pos = working->tokens.size() - 1;
		}
	}
	else
	{
		working_pos = -1;
	}
	update_has_equals();
}

void MathKeyboard::update_has_equals()
{
	has_equals = false;
	if(working)
	{
		for (MathToken &tok: working->tokens)
		{
			if (tok.type == MathToken::OPERATOR && tok.value == "=")
			{
				has_equals = true;
			}
		}
	}
}


void Button::draw(framebuffer::FB* fb, bool clicked, bool cleared)
{
	if(clicked)
	{
		fb->draw_rect(x, y, w, h, BLACK, true);
	}
	else if(cleared)
	{
		fb->draw_rect(x, y, w, h, WHITE, true);
	}
	else
	{
		fb->draw_rect(x, y, w, h, BLACK, false);
	}

	int twidth = count_utf8(as_char) * 34;
	int theight = 64;

	int free_w = w - twidth;
	int free_h = h - theight;

	fb->draw_text(x + free_w * 0.5, y + free_h * 0.5, as_char, 64);

}


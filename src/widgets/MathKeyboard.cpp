#include "MathKeyboard.h"


void MathKeyboard::on_mouse_down(input::SynMotionEvent &ev)
{
	if(was_down)
	{
		return;
	}
	was_down = true;
	if(ev.y > keyboard_start)
	{
		handle_buttons(active_buttons, ev.x, ev.y);
	}
	else if(ev.y < top_end)
	{
		// Top bar click
		handle_buttons(&top_buttons, ev.x, ev.y);
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
				return;
			}
		}
	}
}

void MathKeyboard::render()
{
	if(draw_buttons)
	{
		for(Button& b : *active_buttons)
		{
			b.draw(fb);
		}

		draw_buttons = false;
	}

	if(draw_topbar)
	{
		fb->draw_rect(0, 0, 1404, top_end, WHITE, true);
		for(Button& b : top_buttons)
		{
			b.draw(fb);
		}
		draw_topbar = false;
	}

	if(draw_clicked)
	{
		for(Button* b : clicked)
		{
			b->draw(fb, true);
		}
		clicked.clear();
		draw_clicked = false;
	}

	if(draw_to_clear)
	{
		for(Button* b : to_clear)
		{
			b->draw(fb, false, true);
		}
		to_clear.clear();
		draw_to_clear = false;
	}

	if(draw_cleared)
	{
		for(Button* b : cleared)
		{
			b->draw(fb);
		}
		cleared.clear();
		draw_cleared = false;
	}

	if(draw_expression && working)
	{
		working->draw(40, 900, fb);
		working->draw_cursor(working_pos, fb);
		draw_expression = false;
	}
}

MathKeyboard::MathKeyboard(int x, int y, int w, int h) : Widget(x, y, w, h)
{
	working_pos = -1;
	active_buttons = &basic_buttons;
	draw_cleared = false;
	draw_to_clear = false;
	draw_clicked = false;
	draw_topbar = true;

	keyboard_start = 936 + 155;
	top_end = 80;
	int start = keyboard_start;


	basic_buttons.emplace_back(156 * 0, start + 156 * 0, 156, 156, "+");
	basic_buttons.emplace_back(156 * 1, start +  156 * 0, 156, 156, "-");
	basic_buttons.emplace_back(156 * 2, start +  156 * 0, 156, 156, "^");
	basic_buttons.emplace_back(156 * 3, start +  156 * 0, 156, 156, "(");
	basic_buttons.emplace_back(156 * 4, start +  156 * 0, 156, 156, ")");
	basic_buttons.emplace_back(156 * 5, start +  156 * 0, 156, 156, "a_");
	basic_buttons.emplace_back(156 * 6, start +  156 * 0, 156, 156, "\xCF\x80"); // pi
	basic_buttons.emplace_back(156 * 7, start +  156 * 0, 156, 156, "e");
	basic_buttons.emplace_back(156 * 8, start +  156 * 0, 156, 156, "x");

	basic_buttons.emplace_back(156 * 0, start +  156 * 1, 156, 156, "*");
	basic_buttons.emplace_back(156 * 1, start +  156 * 1, 156, 156, "/");
	basic_buttons.emplace_back(156 * 2, start +  156 * 1, 156, 156, "a\xC2\xB2"); //a^2
	basic_buttons.emplace_back(156 * 3, start +  156 * 1, 156, 156, "\xE2\x88\x9A"); // sqrt
	basic_buttons.emplace_back(156 * 4, start +  156 * 1, 156, 156, "log");
	basic_buttons.emplace_back(156 * 5, start +  156 * 1, 156, 156, "1");
	basic_buttons.emplace_back(156 * 6, start +  156 * 1, 156, 156, "2");
	basic_buttons.emplace_back(156 * 7, start +  156 * 1, 156, 156, "3");
	basic_buttons.emplace_back(156 * 8, start +  156 * 1, 156, 156, "y");

	basic_buttons.emplace_back(156 * 0, start +  156 * 2, 156, 156, "sin");
	basic_buttons.emplace_back(156 * 1, start +  156 * 2, 156, 156, "cos");
	basic_buttons.emplace_back(156 * 2, start +  156 * 2, 156, 156, "tan");
	basic_buttons.emplace_back(156 * 3, start +  156 * 2, 156, 156, ">");
	basic_buttons.emplace_back(156 * 4, start +  156 * 2, 156, 156, "<");
	basic_buttons.emplace_back(156 * 5, start +  156 * 2, 156, 156, "4");
	basic_buttons.emplace_back(156 * 6, start +  156 * 2, 156, 156, "5");
	basic_buttons.emplace_back(156 * 7, start +  156 * 2, 156, 156, "6");
	basic_buttons.emplace_back(156 * 8, start +  156 * 2, 156, 156, "a");

	basic_buttons.emplace_back(156 * 0, start +  156 * 3, 156, 156, "sinh");
	basic_buttons.emplace_back(156 * 1, start +  156 * 3, 156, 156, "cosh");
	basic_buttons.emplace_back(156 * 2, start +  156 * 3, 156, 156, "tanh");
	basic_buttons.emplace_back(156 * 3, start +  156 * 3, 156, 156, "\xE2\x89\xA5"); // >=
	basic_buttons.emplace_back(156 * 4, start +  156 * 3, 156, 156, "\xE2\x89\xA4"); // <=
	basic_buttons.emplace_back(156 * 5, start +  156 * 3, 156, 156, "7");
	basic_buttons.emplace_back(156 * 6, start +  156 * 3, 156, 156, "8");
	basic_buttons.emplace_back(156 * 7, start +  156 * 3, 156, 156, "9");
	basic_buttons.emplace_back(156 * 8, start +  156 * 3, 156, 156, "b");

	basic_buttons.emplace_back(156 * 0, start +  156 * 4, 156 * 2, 156, "Func");
	basic_buttons.emplace_back(156 * 2, start +  156 * 4, 156, 156, "=");
	basic_buttons.emplace_back(156 * 3, start +  156 * 4, 156, 156, "=P");
	basic_buttons.emplace_back(156 * 4, start +  156 * 4, 156, 156, ",");
	basic_buttons.emplace_back(156 * 5, start +  156 * 4, 156, 156, ".");
	basic_buttons.emplace_back(156 * 6, start +  156 * 4, 156, 156, "0");
	basic_buttons.emplace_back(156 * 7, start +  156 * 4, 156 * 2, 156, "Symb");

	top_buttons.emplace_back(0, 0, 250, top_end, "Undo");
	top_buttons.emplace_back(250, 0, 250, top_end, "Redo");
	top_buttons.emplace_back(500, 0, 250, top_end, "Erase");
	top_buttons.emplace_back(1404 - 250, 0, 250, top_end, "Done");

	cur_screen = BASIC;
	draw_buttons = true;
	draw_expression = true;
}

void MathKeyboard::draw_basic_screen()
{

}

void MathKeyboard::clear_clicked(Button *b)
{
	std::remove(clicked.begin(), clicked.end(), b);
	to_clear.push_back(b);
	ui::set_timeout([this, b](){this->redraw_cleared(b);}, 500);
	dirty = true;
	draw_to_clear = true;
}

void MathKeyboard::redraw_cleared(Button *b)
{
	std::remove(to_clear.begin(), to_clear.end(), b);
	cleared.push_back(b);
	dirty = true;
	draw_cleared = true;
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

	printf("before: working_pos = %i\n", working_pos);
	working->old_tokens = working->tokens;

	MathToken placeholder, lparen, rparen, comma;
	placeholder.type = MathToken::DUMMY;
	lparen.type = MathToken::LPAREN;
	rparen.type = MathToken::RPAREN;
	comma.type = MathToken::COMMA;


	// All these tokens are inserted as a whole group
	std::vector<MathToken> tokl;
	tokl.push_back(MathToken());
	// For convenience of code
	auto& tok = tokl[0];

	bool build_number = false;
	int insert_placeholder = 0;
	bool needs_prev_placeholder = false;
	bool insert_parens_around_prev = false;
	bool insert_parens_around_placeholder = false;

	if(b->as_char == "+" || b->as_char == "-" || b->as_char == "*" || b->as_char == "^")
	{
		// Simple operators, we insert them and a placeholder
		tok.type = MathToken::OPERATOR;
		tok.value = b->as_char;
		insert_placeholder = 1;
		needs_prev_placeholder = true;
	}
	else if(b->as_char == "/")
	{
		tok.type = MathToken::OPERATOR;
		tok.value = b->as_char;
		insert_placeholder = 1;
		insert_parens_around_placeholder = true;
		insert_parens_around_prev = true;
		needs_prev_placeholder = true;
	}
	// Single argument functions
	else if(b->as_char == "sin")
	{
		tok.type = MathToken::FUNCTION;
		tok.value = b->as_char;
		insert_placeholder = 1;
		insert_parens_around_placeholder = true;
	}
	// Two argument functions
	else if(b->as_char == "max")
	{
		tok.type = MathToken::FUNCTION;
		tok.value = b->as_char;
		insert_placeholder = 2;
		insert_parens_around_placeholder = true;
	}
	else if(b->as_char == "a\xC2\xB2")
	{
		// Insert ^2 block
		tok.type = MathToken::OPERATOR;
		tok.value = "^";
		needs_prev_placeholder = true;
		tokl.push_back(MathToken());
		tokl[1].type = MathToken::NUMBER;
		tokl[1].value = "2";
	}
	else if(is_buildable_number(b->as_char))
	{
		tok.type = MathToken::NUMBER;
		tok.value = b->as_char;
		build_number = true;
	}
	else
	{
		tok.type == MathToken::VARIABLE;
		tok.value = b->as_char;
	}

	auto& tokens = working->tokens;

	dirty = 1;
	draw_expression = true;

	if(build_number)
	{
		// If there's another buildable number previously, add to it
		if(!tokens.empty() && tokens[working_pos].type == MathToken::NUMBER &&
			is_buildable_number(tokens[working_pos].value))
		{
			// Check that it doesn't have two decimal points
			if(tok.value == ".")
			{
				for(size_t i = 0; i < tokens[working_pos].value.size(); i++)
				{
					if(tokens[working_pos].value[i] == '.')
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
	else
	{
		// Now do the smart insertion
		if (!tokens.empty() && tokens[working_pos].type == MathToken::DUMMY)
		{
			// We replace the dummy which is already properly parenthesized, etc...
			replace_dummy(tokens, working_pos, tokl);
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
				first_placeholder = working_pos;
				working_pos++;
			}
		}

		if (insert_parens_around_prev)
		{
			// This is actually quite complex, we parenthesize until we find:
			// a (+ or -) or we reach the end of the document
			// We respect parenthesis so if we find a + or - inside parenthesis it doesn't matter
			int ptr = working_pos;
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
					(tokens[ptr].value == "+" || tokens[ptr].value == "-") && paren_depth == 0)
				{
					finish = ptr;
				}

				ptr--;
			}

			insert_or_push(tokens, finish, lparen);
			working_pos++;
			insert_or_push(tokens, working_pos - 1, rparen);
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


	}

	printf("after: working_pos = %i\n", working_pos);
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

bool MathKeyboard::handle_buttons(std::vector<Button> *buttons, int cx, int cy)
{
	for (Button &b: *buttons)
	{
		if (cx >= b.x && cy >= b.y && b.x + b.w > cx && b.y + b.h > cy)
		{
			Button *bptr = &b;
			clicked.push_back(bptr);
			ui::set_timeout([this, bptr]()
							{ this->clear_clicked(bptr); }, 1000);
			on_click(bptr);
			dirty = true;
			draw_clicked = true;
			return true;
		}
	}

	return false;
}

bool MathKeyboard::handle_control_buttons(Button *b)
{
	if(b->as_char == "Done")
	{
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

#include "MathKeyboard.h"


void MathKeyboard::on_mouse_down(input::SynMotionEvent &ev)
{

}

void MathKeyboard::render()
{
	if(draw_buttons)
	{
		if(cur_screen == Screen::BASIC)
		{
			draw_basic_screen();
		}

		draw_buttons = false;
	}
}

MathKeyboard::MathKeyboard(int x, int y, int w, int h) : Widget(x, y, w, h)
{
	int start = 936 + 155;

	basic_buttons.emplace_back(156 * 0, start + 156 * 0, 156, 156, "+");
	basic_buttons.emplace_back(156 * 1, start +  156 * 0, 156, 156, "-");
	basic_buttons.emplace_back(156 * 2, start +  156 * 0, 156, 156, "^");
	basic_buttons.emplace_back(156 * 3, start +  156 * 0, 156, 156, "(");
	basic_buttons.emplace_back(156 * 4, start +  156 * 0, 156, 156, ")");
	basic_buttons.emplace_back(156 * 5, start +  156 * 0, 156, 156, "a_b");
	basic_buttons.emplace_back(156 * 6, start +  156 * 0, 156, 156, "pi");
	basic_buttons.emplace_back(156 * 7, start +  156 * 0, 156, 156, "e");
	basic_buttons.emplace_back(156 * 8, start +  156 * 0, 156, 156, "x");

	basic_buttons.emplace_back(156 * 0, start +  156 * 1, 156, 156, "*");
	basic_buttons.emplace_back(156 * 1, start +  156 * 1, 156, 156, "/");
	basic_buttons.emplace_back(156 * 2, start +  156 * 1, 156, 156, "a^2");
	basic_buttons.emplace_back(156 * 3, start +  156 * 1, 156, 156, "sqrt");
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
	basic_buttons.emplace_back(156 * 3, start +  156 * 3, 156, 156, ">=");
	basic_buttons.emplace_back(156 * 4, start +  156 * 3, 156, 156, "<=");
	basic_buttons.emplace_back(156 * 5, start +  156 * 3, 156, 156, "7");
	basic_buttons.emplace_back(156 * 6, start +  156 * 3, 156, 156, "8");
	basic_buttons.emplace_back(156 * 7, start +  156 * 3, 156, 156, "9");
	basic_buttons.emplace_back(156 * 8, start +  156 * 3, 156, 156, "b");

	basic_buttons.emplace_back(156 * 0, start +  156 * 4, 156 * 2, 156, "Func");
	basic_buttons.emplace_back(156 * 2, start +  156 * 4, 156 * 2, 156, "=");
	basic_buttons.emplace_back(156 * 4, start +  156 * 4, 156, 156, "=P");
	basic_buttons.emplace_back(156 * 5, start +  156 * 4, 156, 156, ".");
	basic_buttons.emplace_back(156 * 6, start +  156 * 4, 156, 156, "0");
	basic_buttons.emplace_back(156 * 7, start +  156 * 4, 156 * 2, 156, "Symb");

	cur_screen = BASIC;
	draw_buttons = true;
}

void MathKeyboard::draw_basic_screen()
{
	for(Button& b : basic_buttons)
	{
		b.draw(fb);
	}

}

void Button::draw(framebuffer::FB* fb)
{
	fb->draw_rect(x, y, w, h, BLACK, false);
	if(as_char.empty())
	{

	}
	else
	{
		int twidth = as_char.size() * 34;
		int theight = 64;

		int free_w = w - twidth;
		int free_h = h - theight;

		fb->draw_text(x + free_w * 0.5, y + free_h * 0.5, as_char, 64);
	}

}

#include "Buttons.h"

void Buttons::draw(framebuffer::FB *fb)
{
	if(draw_buttons)
	{
		for(Button& b : buttons)
		{
			b.draw(fb);
		}

		draw_buttons = false;
	}

	if(draw_clicked && !stop_anim)
	{
		for(Button* b : clicked)
		{
			b->draw(fb, true);
		}
		clicked.clear();
		draw_clicked = false;
	}

	if(draw_to_clear && !stop_anim)
	{
		for(Button* b : to_clear)
		{
			b->draw(fb, false, true);
		}
		to_clear.clear();
		draw_to_clear = false;
	}

	if(draw_cleared && !stop_anim)
	{
		for(Button* b : cleared)
		{
			b->draw(fb);
		}
		cleared.clear();
		draw_cleared = false;
	}
}

void Buttons::clear_clicked(Button *b)
{
	std::remove(clicked.begin(), clicked.end(), b);
	to_clear.push_back(b);
	ui::set_timeout([this, b]()
					{ this->redraw_cleared(b); }, 500);
	if(!stop_anim)
	{
		*dirty_set = true;
		draw_to_clear = true;
	}
}

void Buttons::redraw_cleared(Button *b)
{
	std::remove(to_clear.begin(), to_clear.end(), b);
	cleared.push_back(b);
	if(!stop_anim)
	{
		*dirty_set = true;
		draw_cleared = true;
	}
}

Buttons::Buttons(int* dirty_set)
{
	draw_cleared = false;
	draw_to_clear = false;
	draw_clicked = false;
	draw_buttons = true;
	stop_anim = false;
	this->dirty_set = dirty_set;

}

bool Buttons::handle(int cx, int cy)
{
	for (Button &b: buttons)
	{
		if (cx >= b.x && cy >= b.y && b.x + b.w > cx && b.y + b.h > cy)
		{
			Button *bptr = &b;
			clicked.push_back(bptr);
			ui::set_timeout([this, bptr]()
							{ this->clear_clicked(bptr); }, 1000);
			on_click(&b);
			*dirty_set = true;
			draw_clicked = true;
			return true;
		}
	}

	return false;

}

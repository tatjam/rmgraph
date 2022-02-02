#pragma once
#include "rmkit.h"

struct Button
{
	// if empty, we are a custom symbol
	std::string as_char;
	std::string id;
	int x, y;
	int w, h;

	void draw(framebuffer::FB* fb, bool clicked = false, bool cleared = false);

	Button(int x, int y, int w, int h, std::string as_char)
	{
		this->x = x; this->y = y; this->w = w; this->h = h; this->as_char = as_char;
		this->id = as_char;
	}

	Button(int x, int y, int w, int h, std::string as_char, std::string id)
	{
		this->x = x; this->y = y; this->w = w; this->h = h; this->as_char = as_char;
		this->id = id;
	}
};

// Implements many buttons and correctly draws them to avoid ghosting due to rm2fb
class Buttons
{
public:

	std::vector<Button> buttons;
	std::vector<Button*> clicked;
	std::vector<Button*> to_clear;
	std::vector<Button*> cleared;

	int* dirty_set;
	PLS_DEFINE_SIGNAL(CLICKED_EVENT, Button);
	CLICKED_EVENT on_click;

	bool draw_buttons;
	bool draw_clicked;
	bool draw_to_clear;
	bool draw_cleared;

	void clear_clicked(Button* b);
	void redraw_cleared(Button* b);
	bool handle(int cx, int cy);

	void draw(framebuffer::FB* fb);
	bool stop_anim;

	explicit Buttons(int* dirty_set);
};

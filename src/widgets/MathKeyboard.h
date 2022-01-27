#pragma once
#include "rmkit.h"

struct Button
{
	// if empty, we are a custom symbol
	std::string as_char;
	enum Symbol
	{
		DOT,
		FRAC,
		POWER,
		SQRT,
		PI
	};
	Symbol symbol;

	int x, y;
	int w, h;
	float click_time;

	void draw(framebuffer::FB* fb);

	Button(int x, int y, int w, int h, std::string as_char)
	{
		this->x = x; this->y = y; this->w = w; this->h = h; this->as_char = as_char;
	}
	Button(int x, int y, int w, int h, Symbol as_smb)
	{
		this->as_char = "";
		this->x = x; this->y = y; this->w = w; this->h = h; this->symbol = as_smb;
	}
};

class MathKeyboard : public ui::Widget
{
private:
	enum Screen
	{
		BASIC,
		SYMBOLS,
		FUNCTIONS,
		PARAM_SET
	};

	Screen cur_screen;

	bool draw_buttons;

	void draw_basic_screen();

	std::vector<Button> basic_buttons;

public:



	void on_mouse_down(input::SynMotionEvent& ev) override;
	void render() override;

	MathKeyboard(int x, int y, int w, int h);

};

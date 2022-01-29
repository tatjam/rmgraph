#pragma once
#include "rmkit.h"
#include "math/MathExpression.h"

struct Button
{
	// if empty, we are a custom symbol
	std::string as_char;
	int x, y;
	int w, h;
	float click_time;

	void draw(framebuffer::FB* fb, bool clicked = false, bool cleared = false);

	Button(int x, int y, int w, int h, std::string as_char)
	{
		this->x = x; this->y = y; this->w = w; this->h = h; this->as_char = as_char;
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
	bool draw_screen;
	bool draw_clicked;
	bool draw_to_clear;
	bool draw_cleared;
	bool draw_expression;
	bool draw_topbar;

	void draw_basic_screen();
	void clear_clicked(Button* b);
	void redraw_cleared(Button* b);
	std::vector<Button*> clicked;
	std::vector<Button*> to_clear;
	std::vector<Button*> cleared;

	std::vector<Button> top_buttons;
	std::vector<Button> basic_buttons;
	std::vector<Button>* active_buttons;

	void on_click(Button* b);
	bool is_buildable_number(std::string str);
	bool handle_buttons(std::vector<Button>* buttons, int cx, int cy);

	bool handle_control_buttons(Button* b);

	int keyboard_start;
	int top_end;

public:
	MathExpression* working;
	int working_pos;

	bool was_down = false;


	void on_mouse_down(input::SynMotionEvent& ev) override;
	void on_mouse_up(input::SynMotionEvent& ev) override;
	void render() override;

	MathKeyboard(int x, int y, int w, int h);

};

#pragma once
#include "rmkit.h"
#include "math/MathExpression.h"
#include "Buttons.h"


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

	bool draw_screen;
	bool draw_expression;

	Buttons top_buttons;
	Buttons basic_buttons;
	Buttons* active_buttons;

	void on_click(Button* b);
	bool is_buildable_number(std::string str);

	bool handle_control_buttons(Button* b);

public:
	bool has_equals;
	bool is_done;
	bool update_graph;
	MathExpression* working;
	int working_pos;

	bool was_down = false;


	void on_mouse_down(input::SynMotionEvent& ev) override;
	void on_mouse_up(input::SynMotionEvent& ev) override;
	void render() override;
	void on_enter_kb();
	void update_has_equals();

	MathKeyboard(int x, int y, int w, int h);

};

#pragma once
#include "quadtree/Quadtree.h"
#include <string>
#include <queue>
#include "rmkit.h"
#include "util/Vec.h"
#include "math/MathExpression.h"
#include "Buttons.h"

struct DrawnLine
{
	int x0, y0, x1, y1;
};

struct DrawnStroke
{
	std::vector<DrawnLine> lines;
};

// A Page contains user drawn stuff and equations (math objects)
// The MathObjects are not neccesarily in the same page as their display
struct Page
{
	std::vector<DrawnStroke> drawn;
	std::vector<std::pair<MathExpression, Vec2i>> exprs;
};

// Allows writing formulas and comments at the bottom of the screen with a graph at the top
// The graph can be resized and we can turn pages with gestures (both upper and lower)
// All other GUI elements are handled separately
class Notebook : public ui::Widget
{
private:

	enum State
	{
		DRAWING,		// Allow free-hand draw and scrolling of the graph
		CHOOSE_ADD,		// Show add screen to choose what to add
		ADDING,			// User clicks to add the equation there and enter drag mode
		DRAGGING		// User can drag the equations
	};

	State state;

	std::vector<DrawnLine> drawing;
	bool last_expr_dirty = false;
	bool clear_last_rectangle = false;
	Vec2i expr_size;
	int yoff;
	std::vector<Vec2i> prev_drag_pos;

	void draw_graph();
	void draw_bottom();

	void on_pen_move();


	Vec2i transform_point(Vec2f p);
	bool in_bounds_x(const Vec2i p) const
	{
		return p.x >= 0 && p.x < 1404;
	}
	bool in_bounds_y(const Vec2i p) const
	{
		return p.y >= 80 && p.y < separator;
	}
	bool in_bounds(const Vec2i p) const
	{
		return in_bounds_x(p) && in_bounds_y(p);
	}

	void draw_axis(int start_p, int end_p, int offset, float start_v, float end_v, float step,
				   bool vertical, int color, int size, bool drag);
	void draw_cross(int color, int size, bool drag);

	Buttons top_bar;
	Buttons choose_add;
	void on_click(Button* b);

	bool plot_f_of_x(MathContext* ctx, MathExpression& expr);
	bool plot_f_of_y(MathContext* ctx, MathExpression& expr);

public:
	bool eraser;
	bool was_down;

	MathExpression* to_edit;

	bool top_dirty;
	bool refresh_top;
	bool bottom_dirty;
	bool redraw_bottom;
	bool in_draw;

	bool in_drag = false;
	bool first_white_cross = false;
	int drag_finish = 0;

	framebuffer::VirtualFB vfb;

	std::vector<Page> bottom_pages;

	// Pen is simpler
	Vec2i cur_pen, last_pen;

	int top_page = 0;
	int bottom_page = 0;
	// How big is the graph?
	const int separator = 800;

	Vec2f center, zoom;
	std::vector<Vec2f> old_center;

	void on_mouse_up(input::SynMotionEvent& ev) override;
	void on_mouse_down(input::SynMotionEvent& ev) override;
	void on_mouse_move(input::SynMotionEvent& ev) override;
	void on_mouse_hover(input::SynMotionEvent& ev) override;

	void render() override;
	void on_exit_kb();
	void on_enter_kb();

	Notebook(int x, int y, int w, int h);

};


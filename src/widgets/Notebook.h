#pragma once
#include "quadtree/Quadtree.h"
#include <string>
#include <queue>
#include "rmkit.h"
#include "util/Vec.h"

struct DrawnLine
{
	int x0, y0, x1, y1;
};

struct MathObject
{
	enum MathType
	{
		EQUALITY, // An equality gives LHS the value in RHS
		EQUATION, // An equation relates LHS (x or y) to the value in RHS, computed for every value of (x or y)
		PARAMETRIC // A parametric denotes a pair of coordinates on a timer variable "t". May be a point too
	};
	// For parametric, each of the coordinates
	std::string lhs;
	std::string rhs;

	bool dirty;
	std::vector<int> in_pages;
};

// A Page contains user drawn stuff and equations (math objects)
// The MathObjects are not neccesarily in the same page as their display
struct Page
{
	std::vector<DrawnLine> dirty;
	std::vector<MathObject> objects;
};

// Allows writing formulas and comments at the bottom of the screen with a graph at the top
// The graph can be resized and we can turn pages with gestures (both upper and lower)
// All other GUI elements are handled separately
class Notebook : public ui::Widget
{
private:

	void draw_graph();
	void draw_bottom();

	void on_pen_move();

	bool bottom_dirty;
	bool top_dirty;

	Vec2i transform_point(Vec2f p);
	bool in_bounds_x(const Vec2i p) const
	{
		return p.x >= 0 && p.x < 1404;
	}
	bool in_bounds_y(const Vec2i p) const
	{
		return p.y >= 0 && p.y < separator;
	}
	bool in_bounds(const Vec2i p) const
	{
		return in_bounds_x(p) && in_bounds_y(p);
	}

	void draw_axis(int start_p, int end_p, int offset, float start_v, float end_v, float step,
				   bool vertical, int color, int size, bool drag);
	void draw_cross(int color, int size, bool drag);

public:

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

	virtual void on_mouse_up(input::SynMotionEvent& ev) override;
	virtual void on_mouse_down(input::SynMotionEvent& ev) override;
	virtual void on_mouse_move(input::SynMotionEvent& ev) override;
	virtual void on_mouse_hover(input::SynMotionEvent& ev) override;

	virtual void render() override;

	Notebook(int x, int y, int w, int h);

};


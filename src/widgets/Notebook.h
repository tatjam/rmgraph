#pragma once
#include "quadtree/Quadtree.h"
#include <string>
#include <queue>
#include "rmkit.h"
#include "util/Vec.h"
#include "math/MathExpression.h"
#include "Buttons.h"
#include "Plot.h"

struct DrawnLine
{
	int x0, y0, x1, y1;
};

struct DrawnStroke
{
	std::vector<DrawnLine> lines;
};

class Notebook;

// Used for the quadtree, storing strokes is enough as they are usually short
struct StrokePointer
{
public:

	Notebook* in_notebook;
	size_t in_page;
	size_t stroke_index;
	static quadtree::Box<int> get_box(const StrokePointer& a);
	static bool equals(const StrokePointer& a, const StrokePointer& b);

	StrokePointer(Notebook* nb, size_t pag, size_t idx) : in_notebook(nb), in_page(pag), stroke_index(idx) {}
};

// A Page contains user drawn stuff and equations (math objects)
// The MathObjects are not neccesarily in the same page as their display
struct Page
{
	std::vector<DrawnStroke> drawn;
	std::vector<std::pair<MathExpression, Vec2i>> exprs;
	// Plot may be resized in each page
	int plot_x, plot_y, plot_ex, plot_ey;
	static constexpr quadtree::Box<int> page_box = quadtree::Box<int>(0, 0, 1404, 1872);
	using QuadTree = quadtree::Quadtree<StrokePointer, decltype(StrokePointer::get_box)*, decltype(StrokePointer::equals)*, int>;
	QuadTree quadtree = QuadTree(page_box, &StrokePointer::get_box, &StrokePointer::equals);

	// Sanifies all index of line pointers to avoid trouble
	void remove_stroke(size_t ptr);
	Page();
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

	void draw_bottom();

	void on_pen_move(Vec2i cur_pen);



	Buttons top_bar;
	Buttons choose_add;
	void on_click(Button* b);

public:
	Plot plot;
	bool eraser;
	bool was_down;

	MathExpression* to_edit;

	bool top_dirty;
	bool bottom_dirty;
	bool redraw_bottom;
	bool in_draw;


	framebuffer::VirtualFB vfb;

	std::vector<Page*> bottom_pages;

	Vec2i last_pen;

	int top_page = 0;
	int bottom_page = 0;

	void on_mouse_up(input::SynMotionEvent& ev) override;
	void on_mouse_down(input::SynMotionEvent& ev) override;
	void on_mouse_move(input::SynMotionEvent& ev) override;
	void on_mouse_hover(input::SynMotionEvent& ev) override;

	void render() override;
	void on_exit_kb();
	void on_enter_kb();

	Notebook(int x, int y, int w, int h);

};


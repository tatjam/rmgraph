#pragma once
#include "rmkit.h"

class Page;

class Plot
{
private:
	framebuffer::FB* fb;
	int start_x, start_y, end_x, end_y;
	int width, height;
public:

	Vec2i last_input;
	bool in_drag = false;
	bool first_white_cross = false;
	int drag_finish = 0;
	bool refresh = false;
	Vec2f center, zoom;
	std::vector<Vec2f> old_center;

	// Returns if mouse was inside plot
	bool on_input_move(int cx, int cy, bool& set_dirty);
	void on_input_up(int cx, int cy, bool& set_dirty);

	bool plot_f_of_x(MathContext* ctx, MathExpression& expr, int res = 4);
	bool plot_f_of_y(MathContext* ctx, MathExpression& expr, int res = 4);
	// Resolution specifies the size of the cells in pixels
	void plot_marching_squares(MathContext* ctx, MathExpression& expr, int width, int resolution, bool fill);

	void draw(int start_x, int start_y, int end_x, int end_y, framebuffer::FB* fb,
			  Page& page);

	void draw_axis(int start_p, int end_p, int offset, float start_v, float end_v, float step,
				   bool vertical, int color, int size, bool drag);
	void draw_cross(int color, int size, bool drag);

	// From plot space to screen space
	Vec2i transform_point(Vec2f p);
	// From screen space to plot space
	Vec2f inverse_transform_point(Vec2i p);
	bool in_bounds_x(const Vec2i p) const
	{
		return p.x >= start_x && p.x < end_x;
	}
	bool in_bounds_y(const Vec2i p) const
	{
		return p.y >= start_y && p.y < end_y;
	}
	bool in_bounds(const Vec2i p) const
	{
		return in_bounds_x(p) && in_bounds_y(p);
	}

	Plot();

};

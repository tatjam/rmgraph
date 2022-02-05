#include "Plot.h"

bool Plot::plot_f_of_x(MathContext *ctx, MathExpression &expr, int res)
{
	bool first = true;
	ctx->free_values["x"] = 0.0f;
	if(expr.is_explicit() && expr.get_value_of_y(ctx))
	{
		Vec2i prev;
		for(int xpx = start_x; xpx < end_x; xpx++)
		{
			Vec2f plot_space = inverse_transform_point(Vec2i(xpx, 0.0f));
			float xf = plot_space.x;
			ctx->free_values["x"] = xf;
			float y = (float)expr.get_value_of_y(ctx).value();
			Vec2i point = transform_point(Vec2f(xf, -y));
			printf("%i, %i\n", point.x, point.y);
			if(in_bounds(point) && in_bounds(prev) && !first)
			{
				fb->draw_line(point.x, point.y, prev.x, prev.y, 1, BLACK);
				printf("IN_BOUNDS\n");
			}
			prev = point;
			first = false;
		}
		ctx->free_values.erase("x");
		return true;
	}

	ctx->free_values.erase("x");
	return false;
}

bool Plot::plot_f_of_y(MathContext *ctx, MathExpression &expr, int res)
{
	bool first = true;
	ctx->free_values["y"] = 0.0f;

	if(expr.is_explicit() && expr.get_value_of_x(ctx))
	{
		Vec2i prev;
		for(int ypx = start_y; ypx < end_y; ypx++)
		{
			Vec2f plot_space = inverse_transform_point(Vec2i(0, ypx));
			float yf = plot_space.y;
			ctx->free_values["y"] = yf;
			float x = (float)expr.get_value_of_x(ctx).value();
			Vec2i point = transform_point(Vec2f(x, yf));
			if(in_bounds(point) && in_bounds(prev) && !first)
			{
				fb->draw_line(point.x, point.y, prev.x, prev.y, 1, BLACK);
			}
			prev = point;
			first = false;
		}
		ctx->free_values.erase("y");
		return true;
	}

	ctx->free_values.erase("y");
	return false;
}

void Plot::plot_marching_squares(MathContext *ctx, MathExpression &expr, int width, int resolution, bool fill)
{
	float left = (-1404.0f * 0.5f + center.x) / zoom.x;
	float right = (1404.0f * 0.5f + center.x) / zoom.x;
	float top = (-end_y * 0.5f + center.y + start_y) / zoom.y;
	float bottom = (end_y * 0.5f + center.y + start_y) / zoom.y;

	int x_cells = 1404 / resolution;
	int y_cells = (end_y - start_y) / resolution;
	// Graphical steps
	float xs = resolution;
	float ys = resolution;
	// Function steps
	float xfs = ((right - left) * resolution) / 1404.0f;
	float yfs = ((bottom - top) * resolution) / (float)(end_y - start_y);
	// x, y points to the top-left corners of the cells, graphical
	float x = 0.0f, y = Dimensions::top_end;
	// x, y points of the top-left corners of the cells, function
	float xf = left, yf = top;
	auto [lhs, rhs] = expr.get_sides();

	for(int cx = 0; cx < x_cells; cx++)
	{
		for(int cy = 0; cy < y_cells; cy++)
		{
			// Evaluate all four corners
			ctx->free_values["x"] = xf;
			ctx->free_values["y"] = -yf;
			float tl = lhs.evaluate(ctx).value() - rhs.evaluate(ctx).value();
			ctx->free_values["x"] = xf + xfs;
			float tr = lhs.evaluate(ctx).value() - rhs.evaluate(ctx).value();
			ctx->free_values["y"] = -yf - yfs;
			float br = lhs.evaluate(ctx).value() - rhs.evaluate(ctx).value();
			ctx->free_values["x"] = xf;
			float bl = lhs.evaluate(ctx).value() - rhs.evaluate(ctx).value();

			int ms_case = 0;
			if(bl >= 0.0f) ms_case += 1;
			if(br >= 0.0f) ms_case += 2;
			if(tr >= 0.0f) ms_case += 4;
			if(tl >= 0.0f) ms_case += 8;
#ifdef USE_SIMPLE_MSQUARES
			// Simple algorithm, faster but ugly as hell
			float top_progress = 0.5f;
			float left_progress = 0.5f;
			float bottom_progress = 0.5f;
			float right_progress = 0.5f;
#else
			// Algorithm with linear interpolation
			// Progress from left to right in the top
			auto get_interp = [](float val1, float val2)
			{
				// We image the line that joins x = 0, y = val1 and x = 1, y = val2
				// and find the intersection with y = 0 (that's prg)
				// 0 = m*prg + c => prg = -c / m
				float m = val2 - val1;
				float c = val1;
				float prg = -c / m;
				// Bounds!
				prg = max(prg, 0.0f);
				prg = min(prg, 1.0f);
				return prg;
			};

			float top_progress = get_interp(tl, tr);
			float left_progress = get_interp(tl, bl);
			float bottom_progress = get_interp(bl, br);
			float right_progress = get_interp(tr, br);
#endif

			if(ms_case == 1 || ms_case == 14)
				fb->draw_line(x, y + ys * left_progress, x + xs * bottom_progress, y + ys, width, BLACK);
			else if(ms_case == 2 || ms_case == 13)
				fb->draw_line(x + xs * bottom_progress, y + ys, x + xs, y + ys * right_progress, width, BLACK);
			else if(ms_case == 3 || ms_case == 12)
				fb->draw_line(x, y + ys * left_progress, x + xs, y + ys * right_progress, width, BLACK);
			else if(ms_case == 4 || ms_case == 11)
				fb->draw_line(x + xs * top_progress, y, x + xs, y + ys * right_progress, width, BLACK);
			else if(ms_case == 5)
			{
				fb->draw_line(x + xs * bottom_progress, y + ys, x + xs, y + ys * right_progress, width, BLACK);
				fb->draw_line(x, y + ys * left_progress, x + xs * top_progress, y, width, BLACK);
			}
			else if(ms_case == 6 || ms_case == 9)
				fb->draw_line(x + xs * top_progress, y, x + xs * bottom_progress, y + ys, width, BLACK);
			else if(ms_case == 7 || ms_case == 8)
				fb->draw_line(x, y + ys * left_progress, x + xs * top_progress, y, width, BLACK);
			else if(ms_case == 10)
			{
				fb->draw_line(x + xs * top_progress, y, x + xs, y + ys * right_progress, width, BLACK);
				fb->draw_line(x, y + ys * left_progress, x + xs * bottom_progress, y + ys, width, BLACK);
			}


			y += ys;
			yf += yfs;
		}
		y = Dimensions::top_end;
		yf = top;
		x += xs;
		xf += xfs;
	}


}
Vec2i Plot::transform_point(Vec2f p)
{
	p.x *= zoom.x;
	p.y *= zoom.y;
	p -= center;
	p.x += start_x + (float)width / 2.0f;
	p.y += start_y + (float)height / 2.0f;
	return Vec2i(round(p.x), round(p.y));
}

Vec2f Plot::inverse_transform_point(Vec2i p)
{
	p.x += center.x - start_x - width / 2;
	p.y += center.y - start_y - height / 2;
	Vec2f out = Vec2f(p.x, p.y);
	out.x /= zoom.x;
	out.y /= zoom.y;
	return out;
}

void Plot::draw_axis(int start_p, int end_p, int offset, float start_v, float end_v, float step,
						 bool vertical, int color, int size, bool drag)
{

	if(vertical)
	{
		fb->draw_line(offset, start_p, offset, end_p, size, color);
	}
	else
	{
		fb->draw_line(start_p, offset, end_p, offset, size, color);
	}
}

void Plot::draw_cross(int color, int size, bool drag)
{
	Vec2i center = transform_point(Vec2f(0.0f, 0.0f));
	if(in_bounds_x(center))
	{
		draw_axis(start_y, end_y, center.x, 0.0f, 0.0f, 1.0f,
				  true, color, size, drag);
	}

	if(in_bounds_y(center))
	{
		draw_axis(start_x, end_x, center.y, 0.0f, 0.0f, 1.0f,
				  false, color, size, drag);
	}
}

void Plot::draw(int sx, int sy, int ex, int ey, framebuffer::FB* f, Page& page)
{
	start_x = sx; start_y = sy; end_x = ex; end_y = ey; fb = f;
	width = end_x - start_x; height = end_y - start_y;

	if(drag_finish == 0 || refresh)
	{
		// Flash black and white
		fb->draw_rect(start_x, start_y, end_x, end_y, BLACK, true);
		this_thread::sleep_for(1.5s);
		fb->draw_rect(start_x, start_y, end_x, end_y, WHITE, true);
		drag_finish = -1;
		refresh = false;
	}

	if(in_drag)
	{
		fb->waveform_mode = WAVEFORM_MODE_A2;
		Vec2f safe_center = center;
		for(Vec2f old : old_center)
		{
			center = old;
			draw_cross(WHITE, first_white_cross ? 2 : 1, true);
			first_white_cross = false;
		}
		old_center.clear();
		center = safe_center;
		draw_cross(BLACK, 1, true);
	}
	else
	{
		draw_cross(BLACK, 2, false);
		// Draw implicit functions of x
		for(auto& pair : page.exprs)
		{
			MathContext ctx;
			MathExpression& expr = pair.first;
			// Try all possibilities
			bool was_explicit = false;
			was_explicit |= plot_f_of_x(&ctx, expr);
			if(!was_explicit)
			{
				was_explicit |= plot_f_of_y(&ctx, expr);
			}
			// Draw implicit equation using marching squares
			if(!was_explicit)
			{
				plot_marching_squares(&ctx, expr, 2, 16, false);
			}
		}
	}

}

Plot::Plot()
{
	center = Vec2f(0.0f, 0.0f);
	zoom = Vec2f(40.0f, 40.0f);
	drag_finish = -1;
}

bool Plot::on_input_move(int x, int y, bool& set_dirty)
{
	set_dirty = false;
	bool was_in_plot = false;
	if (y < end_y && last_input.y < end_y && last_input.y > start_y && y > start_y
		&& x < end_x && last_input.x < end_x && last_input.x > start_x && x > start_x)
	{
		in_drag = true;
		first_white_cross = true;
		set_dirty = true;

		Vec2i offset = Vec2i(x, y) - last_input;
		if (old_center.size() == 0 || center != old_center.back())
		{
			old_center.push_back(center);
		}
		center.x += offset.x;
		center.y += offset.y;
		was_in_plot = true;
	}

	last_input = Vec2i(x, y);

	return was_in_plot;

}

void Plot::on_input_up(int cx, int cy, bool& set_dirty)
{
	last_input = OUTSIDE;
	old_center.clear();
	if (in_drag)
	{
		drag_finish = 0;
		set_dirty = true;
	}
	in_drag = false;
}


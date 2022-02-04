#include "Notebook.h"
#include "util/Dimensions.h"

void Notebook::on_mouse_up(input::SynMotionEvent& ev)
{
	was_down = false;

	if(state == DRAWING)
	{
		if (!input::is_touch_event(ev))
		{
			cur_pen = OUTSIDE;
			last_pen = OUTSIDE;
			old_center.clear();
			if (in_drag)
			{
				drag_finish = 0;
				dirty = true;
				top_dirty = true;
			}
			in_drag = false;
			in_draw = false;
		}
	}
	else
	{
		cur_pen = OUTSIDE;
		last_pen = OUTSIDE;
		in_drag = false;
		in_draw = false;
	}

	if(state == DRAGGING)
	{
		// Finish the drag
		state = DRAWING;
		redraw_bottom = true;
		bottom_dirty = true;
		clear_last_rectangle = true;
		last_expr_dirty = true;
		prev_drag_pos.push_back(bottom_pages[bottom_page].exprs.back().second);
		dirty = 1;
	}

}

void Notebook::on_mouse_down(input::SynMotionEvent& ev)
{
	if (!was_down)
	{
		if (state == DRAWING)
		{
			top_bar.handle(ev.x, ev.y);
		}
		else if (state == CHOOSE_ADD)
		{
			choose_add.handle(ev.x, ev.y);
		}
		else if(state == ADDING)
		{
			bottom_dirty = true;
			last_expr_dirty = false;
			prev_drag_pos.push_back(bottom_pages[bottom_page].exprs.back().second);
			bottom_pages[bottom_page].exprs.back().second = Vec2i(ev.x, ev.y);
			dirty = 1;
			state = DRAGGING;
		}

		was_down = true;
	}

}

void Notebook::on_mouse_move(input::SynMotionEvent& ev)
{
	if(state == DRAWING)
	{
		if (is_touch_event(ev))
		{
			return;
		}
		cur_pen = Vec2(ev.x, ev.y);
		on_pen_move();
		last_pen = cur_pen;
	}
	else if(state == DRAGGING)
	{
		prev_drag_pos.push_back(bottom_pages[bottom_page].exprs.back().second);
		bottom_pages[bottom_page].exprs.back().second = Vec2i(ev.x, ev.y);
		dirty = 1;
	}
}

void Notebook::on_mouse_hover(input::SynMotionEvent& ev)
{

}

void Notebook::on_pen_move()
{
	if(state == DRAWING)
	{
		if (last_pen != OUTSIDE)
		{
			if (cur_pen.y < separator && last_pen.y < separator &&
				last_pen.y > Dimensions::top_end && cur_pen.y > Dimensions::top_end)
			{
				in_drag = true;
				first_white_cross = true;
				dirty = 1;
				top_dirty = true;

				Vec2i offset = cur_pen - last_pen;
				if (old_center.size() == 0 || center != old_center.back())
				{
					old_center.push_back(center);
				}
				center.x += offset.x;
				center.y += offset.y;
			}
			else
			{
				if (!in_draw)
				{
					bottom_pages[bottom_page].drawn.push_back(DrawnStroke());
					in_draw = true;
				}
				// Draw a line
				DrawnLine line;
				line.x0 = last_pen.x;
				line.y0 = last_pen.y;
				line.x1 = cur_pen.x;
				line.y1 = cur_pen.y;

				drawing.push_back(line);
				bottom_dirty = true;
				dirty = 1;
			}
		}
	}
}

void Notebook::render()
{
	top_bar.draw(fb);

	if(bottom_dirty)
	{
		draw_bottom();
		bottom_dirty = false;
	}


	if(top_dirty)
	{
		draw_graph();
		top_dirty = false;
	}

	if(state == CHOOSE_ADD)
	{
		// Draw the choose add if needed
		choose_add.draw(fb);

	}
	else if(state == ADDING)
	{
		fb->draw_text(30, separator - 64, "Click somewhere to add the equation and drag to adjust", 32);
	}
	else if(state == DRAGGING || clear_last_rectangle)
	{
		Vec2i cur_drag_pos = bottom_pages[bottom_page].exprs.back().second;
		// Draw a rectangle the size of the equation, and clear the previous one
		fb->waveform_mode = WAVEFORM_MODE_A2;
		for(Vec2i pos : prev_drag_pos)
		{
			fb->draw_rect(pos.x, pos.y + yoff, expr_size.x, expr_size.y, WHITE, false);
		}
		prev_drag_pos.clear();
		if(!clear_last_rectangle)
		{
			fb->draw_rect(cur_drag_pos.x, cur_drag_pos.y + yoff, expr_size.x, expr_size.y, BLACK, false);
		}
		clear_last_rectangle = false;
	}
}

Notebook::Notebook(int x, int y, int w, int h) : Widget(x, y, w, h), vfb(w, h),
	top_bar(&dirty), choose_add(&dirty)
{
	was_down = false;
	eraser = false;

	Page page;
	bottom_pages.push_back(page);

	cur_pen = OUTSIDE;
	last_pen = OUTSIDE;
	center = Vec2f(0.0f, 0.0f);
	zoom = Vec2f(40.0f, 40.0f);

	top_dirty = true;
	drag_finish = -1;
	in_draw = false;
	to_edit = nullptr;

	int top_end = Dimensions::top_end;

	top_bar.buttons.emplace_back(0, 0, 250, top_end, "File");
	top_bar.buttons.emplace_back(250, 0, 250, top_end, "Undo");
	top_bar.buttons.emplace_back(500, 0, 250, top_end, "Redo");
	top_bar.buttons.emplace_back(1404 - 350, 0, 350, top_end, "Add");
	top_bar.buttons.emplace_back(1404 - 600, 0, 250, top_end, "Eraser");

	choose_add.buttons.emplace_back(1404 - 350, top_end * 1, 350, top_end, "Equation");
	choose_add.buttons.emplace_back(1404 - 350, top_end * 2, 350, top_end, "Definition");
	choose_add.buttons.emplace_back(1404 - 350, top_end * 3, 350, top_end, "Parameter");
	choose_add.buttons.emplace_back(1404 - 350, top_end * 4, 350, top_end, "Point");
	choose_add.buttons.emplace_back(1404 - 350, top_end * 5, 350, top_end, "Cancel");

	top_bar.on_click += [this](Button& b){this->on_click(&b);};
	choose_add.on_click += [this](Button& b){this->on_click(&b);};
}

void Notebook::draw_bottom()
{
	// Draw old lines and all equations if needed
	if(redraw_bottom)
	{
		auto& drawn = bottom_pages[bottom_page].drawn;
		for(const auto& stroke : drawn)
		{
			for(const DrawnLine& l : stroke.lines)
			{
				fb->draw_line(l.x0, l.y0, l.x1, l.y1, 3, BLACK);
			}
		}

		auto& exprs = bottom_pages[bottom_page].exprs;
		for(auto& expr : exprs)
		{
			if(expr.second.x >= 0)
			{
				expr.first.draw(expr.second.x, expr.second.y, fb, false);
			}
		}
		redraw_bottom = false;
	}

	if(!drawing.empty())
	{
		// Draw newly drawn lines
		for(DrawnLine& l : drawing)
		{
			fb->draw_line(l.x0, l.y0, l.x1, l.y1, 3, BLACK);
		}
		// Insert the drawn lines to the last stroke (a new one will be created!)
		auto &drawn = bottom_pages[bottom_page].drawn;
		auto &lines = drawn[drawn.size() - 1].lines;
		lines.insert(lines.end(), drawing.begin(), drawing.end());
		drawing.clear();
	}

	if(last_expr_dirty)
	{
		auto expr = bottom_pages[bottom_page].exprs.back();
		expr.first.draw(expr.second.x, expr.second.y, fb, false);
		last_expr_dirty = false;
	}

}



void Notebook::draw_graph()
{
	if(drag_finish == 0 || refresh_top)
	{
		// Flash black and white
		fb->draw_rect(0, 80, 1404, separator, BLACK, true);
		this_thread::sleep_for(1.5s);
		fb->draw_rect(0, 80, 1404, separator, WHITE, true);
		drag_finish = -1;
		refresh_top = false;
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
		for(auto& pair : bottom_pages[bottom_page].exprs)
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

Vec2i Notebook::transform_point(Vec2f p)
{
	p.x *= zoom.x;
	p.y *= zoom.y;
	p -= center;
	p.x += 1404.0f / 2.0f;
	p.y += separator / 2.0f;
	return Vec2i(round(p.x), round(p.y));
}

void Notebook::draw_axis(int start_p, int end_p, int offset, float start_v, float end_v, float step,
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

void Notebook::draw_cross(int color, int size, bool drag)
{
	Vec2i center = transform_point(Vec2f(0.0f, 0.0f));
	if(in_bounds_x(center))
	{
		draw_axis(80, separator, center.x, 0.0f, 0.0f, 1.0f,
				  true, color, size, drag);
	}

	if(in_bounds_y(center))
	{
		draw_axis(0, 1404, center.y, 0.0f, 0.0f, 1.0f,
				  false, color, size, drag);
	}
}

void Notebook::on_exit_kb()
{
	choose_add.stop_anim = false;
	top_bar.stop_anim = false;

	if(to_edit->tokens.size() != 0)
	{
		state = ADDING;
		auto[w, h] = to_edit->get_dimensions(yoff);
		expr_size.x = w; expr_size.y = h;
	}
	else
	{
		state = DRAWING;
	}
	to_edit = nullptr;
	dirty = true;
	bottom_dirty = true;
	top_dirty = true;
	refresh_top = true;
	in_draw = false;
	redraw_bottom = true;
	to_edit = nullptr;
	top_bar.draw_buttons = true;
}

void Notebook::on_click(Button* b)
{
	if(state == DRAWING)
	{
		if (b->id == "Add")
		{
			state = CHOOSE_ADD;
			choose_add.draw_buttons = true;
			dirty = 1;
		}
	}
	if(state == CHOOSE_ADD)
	{
		if(b->id == "Equation")
		{
			on_enter_kb();
			bottom_pages[bottom_page].exprs.emplace_back(MathExpression(), Vec2i(-1, -1));
			to_edit = &bottom_pages[bottom_page].exprs.back().first;
		}
		else if (b->id == "Cancel")
		{
			state = DRAWING;
			top_dirty = true;
			refresh_top = true;
			dirty = 1;
		}
	}
}

void Notebook::on_enter_kb()
{
	top_bar.stop_anim = true;
	choose_add.stop_anim = true;
}

bool Notebook::plot_f_of_x(MathContext *ctx, MathExpression &expr)
{
	bool first = true;
	Vec2i prev;
	float left = (-1404.0f * 0.5f + center.x) / zoom.x;
	float right = (1404.0f * 0.5f + center.x) / zoom.x;
	ctx->free_values["x"] = 0.0f;

	if(expr.is_explicit() && expr.get_value_of_y(ctx))
	{
		for (float x = left; x < right; x += 0.05f)
		{
			ctx->free_values["x"] = x;
			float y = (float)expr.get_value_of_y(ctx).value();
			Vec2i point = transform_point(Vec2f(x, -y));
			if (in_bounds(point) && in_bounds(prev) && !first)
			{
				fb->draw_line(point.x, point.y, prev.x, prev.y, 1, BLACK);
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

bool Notebook::plot_f_of_y(MathContext *ctx, MathExpression &expr)
{
	bool first = true;
	Vec2i prev;
	float top = (-separator * 0.5f + center.y + Dimensions::top_end) / zoom.y;
	float bottom = (separator * 0.5f + center.y) / zoom.y;
	ctx->free_values["y"] = 0.0f;

	if(expr.is_explicit() && expr.get_value_of_x(ctx))
	{
		for (float y = top; y < bottom; y += 0.05f)
		{
			ctx->free_values["y"] = y;
			float x = (float)expr.get_value_of_x(ctx).value();
			Vec2i point = transform_point(Vec2f(x, -y));
			if (in_bounds(point) && in_bounds(prev) && !first)
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

void Notebook::plot_marching_squares(MathContext *ctx, MathExpression &expr, int width, int resolution, bool fill)
{
	float left = (-1404.0f * 0.5f + center.x) / zoom.x;
	float right = (1404.0f * 0.5f + center.x) / zoom.x;
	float top = (-separator * 0.5f + center.y + Dimensions::top_end) / zoom.y;
	float bottom = (separator * 0.5f + center.y) / zoom.y;

	int x_cells = 1404 / resolution;
	int y_cells = (separator - Dimensions::top_end) / resolution;
	// Graphical steps
	float xs = resolution;
	float ys = resolution;
	// Function steps
	float xfs = ((right - left) * resolution) / 1404.0f;
	float yfs = ((bottom - top) * resolution) / (float)(separator - Dimensions::top_end);
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

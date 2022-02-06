#include "Notebook.h"
#include "util/Dimensions.h"

void Notebook::on_mouse_up(input::SynMotionEvent& ev)
{
	was_down = false;

	if(state == DRAWING)
	{
		bool set_dirty;
		plot.on_input_up(ev.x, ev.y, set_dirty);
		if(set_dirty)
		{
			dirty = 1;
			top_dirty = true;
		}
		in_draw = false;
		last_pen = OUTSIDE;
	}
	else
	{
		last_pen = OUTSIDE;
		plot.in_drag = false;
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
		prev_drag_pos.push_back(bottom_pages[bottom_page]->exprs.back().second);
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
			prev_drag_pos.push_back(bottom_pages[bottom_page]->exprs.back().second);
			bottom_pages[bottom_page]->exprs.back().second = Vec2i(ev.x, ev.y);
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
		bool set_dirty;
		bool was_in_plot = plot.on_input_move(ev.x, ev.y, set_dirty);
		if(set_dirty)
		{
			dirty = 1;
			top_dirty = true;
		}
		Vec2i cur_pen = Vec2(ev.x, ev.y);
		if(!was_in_plot && !is_touch_event(ev))
		{
			on_pen_move(cur_pen);
		}
		last_pen = cur_pen;
	}
	else if(state == DRAGGING)
	{
		prev_drag_pos.push_back(bottom_pages[bottom_page]->exprs.back().second);
		bottom_pages[bottom_page]->exprs.back().second = Vec2i(ev.x, ev.y);
		dirty = 1;
	}
}

void Notebook::on_mouse_hover(input::SynMotionEvent& ev)
{

}

void Notebook::on_pen_move(Vec2i cur_pen)
{
	if(state == DRAWING)
	{
		if (last_pen != OUTSIDE)
		{
			if (!in_draw && !eraser)
			{
				bottom_pages[bottom_page]->drawn.push_back(DrawnStroke());
				// Add it to the quadtree too
				StrokePointer ptr(this, bottom_page, bottom_pages[bottom_page]->drawn.size() - 1);
				bottom_pages[bottom_page]->quadtree.add(ptr);
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
		Page& p = *bottom_pages[bottom_page];
		plot.draw(p.plot_x, p.plot_y, p.plot_ex, p.plot_ey, fb, p);
		top_dirty = false;
	}

	if(state == CHOOSE_ADD)
	{
		// Draw the choose add if needed
		choose_add.draw(fb);

	}
	else if(state == ADDING)
	{
		int separator = bottom_pages[bottom_page]->plot_ey;
		if(separator < 100)
		{
			separator = 100;
		}
		fb->draw_text(30, separator - 64, "Click somewhere to add the equation and drag to adjust", 32);
	}
	else if(state == DRAGGING || clear_last_rectangle)
	{
		Vec2i cur_drag_pos = bottom_pages[bottom_page]->exprs.back().second;
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

	Page* page = new Page();
	bottom_pages.push_back(page);

	last_pen = OUTSIDE;
	top_dirty = true;
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

constexpr int INSIDE = 0; // 0000
constexpr int LEFT = 1;   // 0001
constexpr int RIGHT = 2;  // 0010
constexpr int BOTTOM = 4; // 0100
constexpr int TOP = 8;    // 1000


// Implementation of the Cohen-Sutherland algorithm, see
// https://en.wikipedia.org/wiki/Cohen-Sutherland_algorithm
static int compute_out_code(int x, int y, int xmin, int ymin, int xmax, int ymax)
{
	int code = INSIDE;
	if(x < xmin)
		code |= LEFT;
	else if(x > xmax)
		code |= RIGHT;
	if(y < ymin)
		code |= TOP;
	else if(y > ymax)
		code |= BOTTOM;

	return code;
}

// Returns true if line was completely inside
bool clip_line(DrawnLine& l, int min_x, int min_y, int max_x, int max_y)
{
	int code0 = compute_out_code(l.x0, l.y0, min_x, min_y, max_x, max_y);
	int code1 = compute_out_code(l.x1, l.y1, min_x, min_y, max_x, max_y);

	while(true)
	{
		if(!(code0 | code1))
		{
			// Both points are inside
			return true;
		}
		else if(code0 & code1)
		{
			// Both points share an outside so both are outside
			return false;
		}
		else
		{
			int x, y;

			// Test againt each side and adjust
			int codeout = code1 > code0 ? code1 : code0;
			if(codeout & TOP)
			{
				x = l.x0 + (l.x1 - l.x0) * (max_y - l.y0) / (l.y1 - l.y0);
				y = max_y;
			}
			else if(codeout & BOTTOM)
			{
				x = l.x0 + (l.x1 - l.x0) * (min_y - l.y0) / (l.y1 - l.y0);
				y = min_y;
			}
			else if(codeout & RIGHT)
			{
				y = l.y0 + (l.y1 - l.y0) * (max_x - l.x0) / (l.x1 - l.x0);
				x = max_x;
			}
			else if(codeout & LEFT)
			{
				y = l.y0 + (l.y1 - l.y0) * (min_x - l.x0) / (l.x1 - l.x0);
				x = min_x;
			}

			if(codeout == code0)
			{
				l.x0 = x;
				l.y0 = y;
				code0 = compute_out_code(l.x0, l.y0, min_x, min_y, max_x, max_y);
			}
			else
			{
				l.x1 = x;
				l.y1 = y;
				code1 = compute_out_code(l.x1, l.y1, min_x, min_y, max_x, max_y);
			}
		}
	}
}

void Notebook::draw_bottom()
{
	// Draw old lines and all equations if needed
	if(redraw_bottom)
	{
		auto& drawn = bottom_pages[bottom_page]->drawn;
		for(const auto& stroke : drawn)
		{
			for(const DrawnLine& l : stroke.lines)
			{
				fb->draw_line(l.x0, l.y0, l.x1, l.y1, 3, BLACK);
			}
		}

		auto& exprs = bottom_pages[bottom_page]->exprs;
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
		if(eraser)
		{
			constexpr int EWIDTH = 64;
			// Draw white and try to remove vectors that we erased
			for(DrawnLine& l : drawing)
			{
				fb->draw_rect(l.x0 - EWIDTH / 2, l.y0 - EWIDTH / 2, EWIDTH, EWIDTH, WHITE);
				auto &drawn = bottom_pages[bottom_page]->drawn;
				auto &quadtree = bottom_pages[bottom_page]->quadtree;
				quadtree::Box<int> box(l.x0 - EWIDTH / 2, l.y0 - EWIDTH / 2,
									   EWIDTH, EWIDTH);
				// Use the quadtree to find which lines to adjust or erase
				std::vector<StrokePointer> strokes = quadtree.query(box);
				for(const auto& stroke_ptr : strokes)
				{
					DrawnStroke& stroke = bottom_pages[stroke_ptr.in_page]->drawn[stroke_ptr.stroke_index];
					for(auto it = stroke.lines.begin(); it != stroke.lines.end();)
					{
						DrawnLine& line = *it;
						bool erase = clip_line(line, l.x0 - EWIDTH / 2, l.y0 - EWIDTH / 2,
											   l.x0 + EWIDTH / 2, l.y0 + EWIDTH / 2);
						if(erase)
						{
							it = stroke.lines.erase(it);
						}
						else
						{
							it++;
						}
					}

				}
			}
			drawing.clear();
		}
		else
		{
			// Draw newly drawn lines
			for (DrawnLine &l: drawing)
			{
				fb->draw_line(l.x0, l.y0, l.x1, l.y1, 3, BLACK);
			}
			// Insert the drawn lines to the last stroke (a new one will be created!)
			auto &drawn = bottom_pages[bottom_page]->drawn;
			auto &lines = drawn[drawn.size() - 1].lines;
			lines.insert(lines.end(), drawing.begin(), drawing.end());
			drawing.clear();
		}
	}

	if(last_expr_dirty)
	{
		auto expr = bottom_pages[bottom_page]->exprs.back();
		expr.first.draw(expr.second.x, expr.second.y, fb, false);
		last_expr_dirty = false;
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
	plot.refresh = true;
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
		else if(b->id == "Eraser")
		{
			eraser = true;
			b->id = "Pen";
			b->as_char = "Pen";
			dirty = 1;
		}
		else if(b->id == "Pen")
		{
			eraser = false;
			b->id = "Eraser";
			b->as_char = "Eraser";
		}
	}
	if(state == CHOOSE_ADD)
	{
		if(b->id == "Equation")
		{
			on_enter_kb();
			bottom_pages[bottom_page]->exprs.emplace_back(MathExpression(), Vec2i(-1, -1));
			to_edit = &bottom_pages[bottom_page]->exprs.back().first;
		}
		else if (b->id == "Cancel")
		{
			state = DRAWING;
			top_dirty = true;
			plot.refresh = true;
			dirty = 1;
		}
	}
}

void Notebook::on_enter_kb()
{
	top_bar.stop_anim = true;
	choose_add.stop_anim = true;
}

Page::Page()
{
	plot_x = 0;
	plot_y = Dimensions::top_end;
	plot_ex = 1404;
	plot_ey = 800;

}

void Page::remove_stroke(size_t ptr)
{
	StrokePointer* to_remove = nullptr;
	for(StrokePointer& line : quadtree.query(page_box))
	{
		if(line.stroke_index == ptr)
		{
			to_remove = &line;
		}
		else if(line.stroke_index > ptr)
		{
			line.stroke_index--;
		}
	}

	if(to_remove != nullptr)
	{
		quadtree.remove(*to_remove);
	}
}

quadtree::Box<int> StrokePointer::get_box(const StrokePointer &a)
{
	quadtree::Box<int> out;
	const Page& in_page = *a.in_notebook->bottom_pages[a.in_page];
	const DrawnStroke& in_stroke = in_page.drawn[a.stroke_index];

	int min_x = INT_MAX, min_y = INT_MAX, max_x = INT_MIN, max_y = INT_MIN;
	for(const DrawnLine& l : in_stroke.lines)
	{
		min_x = min(l.x0, min_x); min_x = min(l.x1, min_x);
		min_y = min(l.y0, min_y); min_y = min(l.y1, min_y);
		max_x = max(l.x0, max_x); max_x = max(l.x1, max_x);
		max_y = max(l.y0, max_y); max_y = max(l.y1, max_y);
	}

	out.top = min_y;
	out.left = min_x;
	out.width = max_x - min_x;
	out.height = max_y - min_y;
	return out;
}

bool StrokePointer::equals(const StrokePointer &a, const StrokePointer &b)
{
	return a.stroke_index == b.stroke_index && a.in_page == b.in_page;
}

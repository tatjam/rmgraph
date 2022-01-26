#include "Notebook.h"

void Notebook::on_mouse_up(input::SynMotionEvent& ev)
{
	if(is_touch_event(ev))
	{
	}
	else
	{
		cur_pen = OUTSIDE;
		last_pen = OUTSIDE;
		old_center.clear();
		if(in_drag)
		{
			drag_finish = 0;
			dirty = true;
			top_dirty = true;
		}
		in_drag = false;
	}

}

void Notebook::on_mouse_down(input::SynMotionEvent& ev)
{

}

void Notebook::on_mouse_move(input::SynMotionEvent& ev)
{
	if(is_touch_event(ev))
	{
		return;
	}
	cur_pen = Vec2(ev.x, ev.y);
	on_pen_move();
	last_pen = cur_pen;
}

void Notebook::on_mouse_hover(input::SynMotionEvent& ev)
{

}

void Notebook::on_pen_move()
{
	if(last_pen != OUTSIDE)
	{
		if (cur_pen.y < separator && last_pen.y < separator)
		{
			in_drag = true;
			first_white_cross = true;
			dirty = 1;
			top_dirty = true;

			Vec2i offset = cur_pen - last_pen;
			if(old_center.size() == 0 || center != old_center.back())
			{
				old_center.push_back(center);
			}
			center.x += offset.x;
			center.y += offset.y;
		}
		else
		{
			// Draw a line
			DrawnLine line;
			line.x0 = last_pen.x;
			line.y0 = last_pen.y;
			line.x1 = cur_pen.x;
			line.y1 = cur_pen.y;

			bottom_pages[bottom_page].dirty.push_back(line);
			bottom_dirty = true;
			dirty = 1;
		}
	}
}

void Notebook::render()
{
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
}

Notebook::Notebook(int x, int y, int w, int h) : Widget(x, y, w, h), vfb(w, h)
{
	Page page;
	bottom_pages.push_back(page);

	cur_pen = OUTSIDE;
	last_pen = OUTSIDE;
	center = Vec2f(0.0f, 0.0f);
	zoom = Vec2f(40.0f, 40.0f);

	top_dirty = true;
	drag_finish = -1;
}

void Notebook::draw_bottom()
{
	for(DrawnLine& l : bottom_pages[bottom_page].dirty)
	{
		fb->draw_line(l.x0, l.y0, l.x1, l.y1, 1, BLACK);
	}

	bottom_pages[bottom_page].dirty.clear();
}



void Notebook::draw_graph()
{
	if(drag_finish == 0)
	{
		// Flash black and white
		fb->draw_rect(0, 0, 1404, separator, BLACK, true);
		this_thread::sleep_for(1.5s);
		fb->draw_rect(0, 0, 1404, separator, WHITE, true);
		drag_finish = -1;
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

		bool first = true;
		Vec2i prev;
		// Draw the function
		float left = (-1404.0f * 0.5f + center.x) / zoom.x;
		float right = (1404.0f * 0.5f + center.x) / zoom.x;
		for(float x = left; x < right; x+=0.05f)
		{
			float y = sin(x * 1.0f) + cos(x * 2.0f);
			Vec2i point = transform_point(Vec2f(x, -y));
			if(in_bounds(point) && in_bounds(prev) && !first)
			{
				fb->draw_line(point.x, point.y, prev.x, prev.y, 1, BLACK);
			}
			prev = point;
			first = false;
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
		draw_axis(0, separator, center.x, 0.0f, 0.0f, 1.0f,
				  true, color, size, drag);
	}

	if(in_bounds_y(center))
	{
		draw_axis(0, 1404, center.y, 0.0f, 0.0f, 1.0f,
				  false, color, size, drag);
	}
}


#define RMKIT_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include "rmkit.h"

// Hack to get rmkit to work with multiple .cpp files
#include "widgets/Notebook.cpp"
#include "math/MathExpression.cpp"
#include "widgets/MathKeyboard.cpp"
#include "widgets/Buttons.cpp"

int main()
{
	auto fb = framebuffer::get();
	fb->waveform_mode = WAVEFORM_MODE_INIT;
	fb->clear_screen();
	fb->redraw_screen(true);

	auto notebook_scene = ui::make_scene();
	auto keyboard_scene = ui::make_scene();

	auto notebook = new Notebook(0, 0, 1404, 1872);
	notebook_scene->add(notebook);

	auto kb = new MathKeyboard(0, 0, 1404, 1872);
	keyboard_scene->add(kb);

	ui::MainLoop::set_scene(notebook_scene);

	ui::MainLoop::refresh();
	ui::MainLoop::redraw();
	ui::MainLoop::filter_palm_events = true;
	bool exit = false;
	kb->dirty = 1;

	while(!exit)
	{
		ui::MainLoop::main();
		if(kb->update_graph)
		{
			notebook->bottom_dirty = true;
			notebook->dirty = true;
			kb->update_graph = false;
		}
		else if(kb->is_done)
		{
			ui::MainLoop::hide_overlay();
			kb->is_done = false;
			notebook->on_exit_kb();
		}
		else if(notebook->to_edit && !ui::MainLoop::overlay_is_visible)
		{
			kb->working = notebook->to_edit;
			kb->on_enter_kb();
			ui::MainLoop::show_overlay(keyboard_scene);
		}
		ui::MainLoop::redraw();
		ui::MainLoop::read_input();
	}
}
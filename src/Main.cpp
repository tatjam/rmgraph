#define RMKIT_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include "rmkit.h"

#include "widgets/Notebook.cpp"

int main()
{
	auto fb = framebuffer::get();
	fb->waveform_mode = WAVEFORM_MODE_INIT;
	fb->clear_screen();
	fb->redraw_screen(true);

	auto scene = ui::make_scene();
	ui::MainLoop::set_scene(scene);

	auto text = new ui::Text(0, 0, 200, 50, "Hello, good bad world!");
	scene->add(text);

	auto notebook = new Notebook(0, 0, 1404, 1872);
	scene->add(notebook);

	ui::MainLoop::refresh();
	ui::MainLoop::redraw();
	while(true)
	{
		ui::MainLoop::main();
		ui::MainLoop::redraw();
		ui::MainLoop::read_input();
	}
}

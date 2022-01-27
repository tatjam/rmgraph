#define RMKIT_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include "rmkit.h"

#include "widgets/Notebook.cpp"
#include "math/MathExpression.cpp"

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
	bool exit = false;
	while(!exit)
	{
		ui::MainLoop::main();
		if(notebook->last_pen.x >= 1000)
		{
			printf("EXPR!\n");
			exit = true;
		}
		ui::MainLoop::redraw();
		ui::MainLoop::read_input();
	}
}
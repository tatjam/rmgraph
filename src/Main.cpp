#define RMKIT_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include "rmkit.h"

#include "widgets/Notebook.cpp"
#include "math/MathExpression.cpp"
#include "widgets/MathKeyboard.cpp"

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

	//auto notebook = new Notebook(0, 0, 1404, 1872);
	//scene->add(notebook);

	MathExpression expr = MathExpression();
	auto kb = new MathKeyboard(0, 0, 1404, 1872);
	kb->working = &expr;
	scene->add(kb);


	ui::MainLoop::refresh();
	ui::MainLoop::redraw();
	ui::MainLoop::filter_palm_events = true;
	bool exit = false;
	kb->dirty = 1;

	while(!exit)
	{
		ui::MainLoop::main();
		/*if(notebook->last_pen.x >= 1300 && notebook->last_pen.y <= 100)
		{
			printf("EXPR!\n");
			exit = true;
		}*/
		ui::MainLoop::redraw();
		ui::MainLoop::read_input();
	}
}
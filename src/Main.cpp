#define RMKIT_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include "rmkit.h"

#include "widgets/Notebook.cpp"
#include <seshat/grammar.h>
#include <seshat/sample.h>
#include <seshat/meparser.h>

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
			std::string sg_ink = notebook->bottom_pages[0].generate_scg_ink({0, 0}, {0, 0});
			// Generate a math expression
			printf("EXPR!\n");
			char* mod_array = (char*)malloc(sizeof(char) * sg_ink.size());
			strcpy(mod_array, sg_ink.c_str());
#ifdef DEV
			char* conf = "Config/CONFIG";
#else
			char* conf = "/home/rmmath/Config/CONFIG";
#endif

			Sample smp = Sample(mod_array);
			meParser parser = meParser(conf);
			parser.parse_me(&smp);

			exit = true;
			free(mod_array);
		}
		ui::MainLoop::redraw();
		ui::MainLoop::read_input();
	}
}

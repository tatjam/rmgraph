#define RMKIT_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION

//#define REMARKABLE true
#define DEV true
#include "rmkit.h"

int main()
{
	auto fb = framebuffer::get();
	fb->clear_screen();

	ui::Scene scene = ui::make_scene();
	ui::MainLoop::set_scene(scene);

	auto text = new ui::Text(0, 0, 200, 50, "Hello, world!");
	scene->add(text);

	while(true)
	{
		ui::MainLoop::main();
		ui::MainLoop::redraw();
		ui::MainLoop::read_input();
	}
}

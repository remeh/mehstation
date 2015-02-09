#include "SDL2/SDL.h"
#include "glib-2.0/glib.h"

#include "view/text.h"
#include "view/window.h"
#include "system/settings.h"
#include "system/app.h"

int main(int argc, char* argv[]) {
	App* app = meh_app_create();
	meh_app_init(app);

	SDL_Color black = { 0, 0, 0 };
	SDL_Color green = { 0, 0, 120 };
	meh_window_clear(app->window, black);

	meh_window_render_text(app->window, app->small_font, "mehstation 1.0", green, 50, 50);

	meh_window_render(app->window);

	SDL_Delay(3000);
	
	meh_app_destroy(app);

	return 0;
}

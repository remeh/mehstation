#include "SDL2/SDL.h"
#include "glib-2.0/glib.h"

#include "view/text.h"
#include "view/window.h"
#include "system/settings.h"
#include "system/app.h"

int main(int argc, char* argv[]) {
	App* app = meh_app_create();
	meh_app_init(app);

	SDL_Color pink = { 255, 0, 255 };
	SDL_Texture* texture = meh_font_render_on_texture(app->window->sdl_renderer, app->small_font, "mehstation 1.0", pink);

	SDL_Color black = { 0, 0, 0 };
	meh_window_clear(app->window, black);

	SDL_Rect viewport = { 0, 0, 250, 150 };
	meh_window_render_texture(app->window, texture, viewport);

	SDL_DestroyTexture(texture);
	meh_window_render(app->window);

	SDL_Delay(3000);
	
	meh_app_destroy(app);

	return 0;
}

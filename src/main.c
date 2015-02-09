#include "SDL2/SDL.h"
#include "glib-2.0/glib.h"

#include "view/image.h"
#include "view/text.h"
#include "view/window.h"
#include "system/settings.h"
#include "system/app.h"

int main(int argc, char* argv[]) {
	App* app = meh_app_create();
	meh_app_init(app);

	SDL_Color black = { 0, 0, 0 };
	meh_window_clear(app->window, black);

	SDL_Texture* texture = meh_image_load_file(app->window->sdl_renderer, "./image.png");
	SDL_Rect rect = { 0, 0, 500, 500 };
	meh_window_render_texture(app->window, texture, rect);
	SDL_DestroyTexture(texture);

	meh_window_render_text(app->window, app->small_font, "mehstation 1.0", black, 50, 50);

	meh_window_render(app->window);

	SDL_Delay(3000);
	
	meh_app_destroy(app);

	return 0;
}

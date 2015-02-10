#include "SDL2/SDL.h"

#include "system/app.h"
#include "system/input.h"

/*
 * meh_input_read_event reads the given SDL event 
 * and do things accordingly.
 */
void meh_input_read_event(App* app, SDL_Event* sdl_event) {
	g_assert(app != NULL);

	if (sdl_event == NULL) {
		return;
	}

	Event* event = g_new(Event, 1);

	meh_app_send_event(app, event);
}

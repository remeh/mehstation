#include <stdio.h>
#include <stdlib.h>
#include "SDL2/SDL.h"
#include "glib-2.0/glib.h"

#include "view/text.h"
#include "view/window.h"
#include "system/settings.h"
#include "system/app.h"

int main(int argc, char* argv[]) {
	App* app = meh_app_create();
	meh_app_init(app);

	SDL_Delay(1000);

	meh_app_destroy(app);

	return 0;
}

#include "SDL2/SDL.h"
#include "glib-2.0/glib.h"

#include "system/message.h"
#include "system/input.h"
#include "system/app.h"

/*
 * meh_input_manager_new creates a new InputManager.
 */
InputManager* meh_input_manager_new() {
	InputManager* input_manager = g_new(InputManager, 1);

	input_manager->keyboard_mapping = meh_input_create_default_keyboard_mapping();

	return input_manager;
}

/*
 * meh_input_manager_destroy free the resources of the given input_manager.
 */
void meh_input_manager_destroy(InputManager* input_manager) {
	g_assert(input_manager != NULL);

	g_hash_table_destroy(input_manager->keyboard_mapping);
	g_free(input_manager);
}

/*
 * meh_input_read_event reads the given SDL event 
 * and do things accordingly.
 */
void meh_input_manager_keyboard_read_event(InputManager* input_manager, SDL_Event* sdl_event) {
	g_assert(input_manager != NULL);

	if (sdl_event == NULL) {
		return;
	}

	int sdl_button = sdl_event->key.keysym.sym;

	/* Look wheter the pressed key is in the mapping of the keyboard */
	int* pressed = g_hash_table_lookup(input_manager->keyboard_mapping, &sdl_button);	

	/* Not configured key pressed. Ignore. */
	if (pressed == NULL) {
		return;
	}

	/* This is a known key set it as pressed / unpressed */
	switch (sdl_event->type) {
		case SDL_KEYDOWN:
			input_manager->pressed_buttons[*pressed] = TRUE;
			break;
		case SDL_KEYUP:
			input_manager->pressed_buttons[*pressed] = FALSE;
			break;
	}
}

/*
 * meh_input_manager_generate_messages transforms the input manager
 * states (button pressed) in messages and returns them in a list.
 */
GSList* meh_input_manager_generate_messages(InputManager* input_manager) {
	GSList* list = NULL;

	int i = 0;
	Message *m = NULL;
	for (i = 0; i < MEH_INPUT_END; i++) {
		if (input_manager->pressed_buttons[i] == TRUE) {
			int* data = g_new(int, 1);
			*data = i;
			m = meh_message_new(MEH_MSG_BUTTON_PRESSED, data);
			list = g_slist_append(list, m);
			break;
		}
	}

	return list;
}

/*
 * meh_input_create_default_keyboard_config create an input config
 * from the SDL_Event of the keyboard mapped to the internal
 * mehstation button configuration.
 * Can be used as a fallback.
 */
GHashTable* meh_input_create_default_keyboard_mapping() {
	GHashTable* mapping = g_hash_table_new(g_int_hash, g_int_equal);

	int* sdl = g_new(int, 1); *sdl = SDLK_UP;
	int* button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_UP;
	g_hash_table_insert(mapping, sdl, button_mapping);

	sdl = g_new(int, 1); *sdl = SDLK_DOWN;
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_DOWN;
	g_hash_table_insert(mapping, sdl, button_mapping);

	sdl = g_new(int, 1); *sdl = SDLK_LEFT;
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_LEFT;
	g_hash_table_insert(mapping, sdl, button_mapping);

	sdl = g_new(int, 1); *sdl = SDLK_RIGHT;
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_RIGHT;
	g_hash_table_insert(mapping, sdl, button_mapping);

	sdl = g_new(int, 1); *sdl = SDLK_RETURN;
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_START;
	g_hash_table_insert(mapping, sdl, button_mapping);

	sdl = g_new(int, 1); *sdl = SDLK_SPACE;
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_SELECT;
	g_hash_table_insert(mapping, sdl, button_mapping);

	sdl = g_new(int, 1); *sdl = SDLK_r;
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_A;
	g_hash_table_insert(mapping, sdl, button_mapping);

	sdl = g_new(int, 1); *sdl = SDLK_e;
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_B;
	g_hash_table_insert(mapping, sdl, button_mapping);
	
	sdl = g_new(int, 1); *sdl = SDLK_ESCAPE;
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_SPECIAL_ESCAPE;
	g_hash_table_insert(mapping, sdl, button_mapping);

	return mapping;
}

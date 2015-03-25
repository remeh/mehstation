/*
 * mehstation - Input manager
 *
 * TODO store/restore mapping from sqlite
 * TODO auto-assign the default mappings
 *
 * Copyright © 2015 Rémy Mathieu
 */
#include <glib.h>
#include <SDL2/SDL.h>

#include "system/message.h"
#include "system/input.h"
#include "system/settings.h"

static GHashTable* meh_input_create_default_keyboard_mapping();
static GHashTable* meh_input_create_default_gamepad_mapping();
static InputState* meh_input_manager_get_input_state(InputManager* input_manager, SDL_Event* sdl_event);
static void meh_input_manager_reset_button_state(InputManager* input_manager, int button);

/*
 * meh_input_manager_new creates a new InputManager.
 */
InputManager* meh_input_manager_new(Settings settings) {
	InputManager* input_manager = g_new(InputManager, 1);
	input_manager->settings = settings;

	input_manager->input_states = g_queue_new();

	/* keyboard input state at index 0 */
	InputState* keyboard_state = g_new(InputState, 1);
	keyboard_state->id = g_strdup("keyboard");
	g_queue_push_tail(input_manager->input_states, keyboard_state);
	g_debug("Adding keyboard with id : %s", keyboard_state->id);

	/* default keyboard and gamepad mapping */
	input_manager->keyboard_mapping = meh_input_create_default_keyboard_mapping();
	input_manager->gamepad_mapping = meh_input_create_default_gamepad_mapping();

	/* init the gamepads queue, attach every gamepads and store them into the queue */
	input_manager->gamepads = g_queue_new();

	int gamepad_count = SDL_NumJoysticks();
	g_message("%d gamepads found", gamepad_count);

	for (int i = 0; i < gamepad_count; i++)
	{
		SDL_Joystick* joystick = SDL_JoystickOpen(i);
		gchar* guid = g_new(gchar, 33);
		SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(joystick), guid, 33); /* gets the guid */

		Gamepad* gamepad = g_new(Gamepad, 1);	
		gamepad->joystick = joystick;
		gamepad->instance_id = SDL_JoystickInstanceID(joystick);
		gamepad->guid = guid;
		gamepad->name = SDL_JoystickName(joystick);

		/* create the gamepad input state */
		InputState* gamepad_state = g_new(InputState, 1);
		gamepad_state->id = g_strdup(guid);
		gamepad_state->last_sdl_key = -1;

		g_queue_push_tail(input_manager->gamepads, gamepad);
		g_message("Using gamepad: %s", SDL_JoystickNameForIndex(i));
		g_queue_push_tail(input_manager->input_states, gamepad_state);
		g_debug("Adding gamepad %d with id : %s", i, gamepad_state->id);
	}

	/* TODO resets the button state of every InputState */
	meh_input_manager_reset_buttons_state(input_manager);

	return input_manager;
}

/*
 * meh_input_manager_destroy free the resources of the given input_manager.
 */
void meh_input_manager_destroy(InputManager* input_manager) {
	g_assert(input_manager != NULL);

	/* close all gamepads */
	for (int i = 0; i < g_queue_get_length(input_manager->gamepads); i++) {
		Gamepad* gamepad = g_queue_peek_nth(input_manager->gamepads, i);

		SDL_JoystickClose(gamepad->joystick);
		g_free(gamepad->guid);
		g_free(gamepad);
	}
	g_queue_free(input_manager->gamepads);

	/* release each input state */
	for (int i = 0; i < g_queue_get_length(input_manager->input_states); i++) {
		InputState* state = g_queue_peek_nth(input_manager->input_states, i);
		g_free(state->id); /* free the string used as ID */
		g_free(state);
	}
	g_queue_free(input_manager->input_states);

	g_hash_table_destroy(input_manager->keyboard_mapping);
	g_hash_table_destroy(input_manager->gamepad_mapping);
	g_free(input_manager);
}

Gamepad* meh_input_manager_gamepad_by_guid(InputManager* input_manager, gchar* guid) {
	g_assert(input_manager != NULL);
	g_assert(guid != NULL);

	for (int i = 0; i < g_queue_get_length(input_manager->gamepads); i++) {
		Gamepad* gamepad = g_queue_peek_nth(input_manager->gamepads, i);

		printf("%s %s\n", gamepad->guid, guid);
		if (g_strcmp0(gamepad->guid, guid) == 0) {
			return gamepad;
		}
	}

	return NULL;
}

/*
 * meh_input_manager_reset_buttons_state resets all the state buttons.
 */
void meh_input_manager_reset_buttons_state(InputManager* input_manager) {
	g_assert(input_manager != NULL);

	for (int i = 0; i < MEH_INPUT_END; i++) {
		meh_input_manager_reset_button_state(input_manager, i);
	}
}

static void meh_input_manager_reset_button_state(InputManager* input_manager, int button) {
	g_assert(input_manager != NULL);
	g_assert(button > -1);
	g_assert(button < MEH_INPUT_END);

	for (int j = 0; j < g_queue_get_length(input_manager->input_states); j++) {
		InputState* input_state = g_queue_peek_nth(input_manager->input_states, j);
		input_state->buttons_state[button] = MEH_INPUT_NOT_PRESSED;
		input_state->buttons_next_message[button] = MEH_INPUT_NOT_PRESSED;
	}
}

/*
 * meh_input_manager_get_input_state uses the given SDL Event to
 * return the concerned input state.
 */
static InputState* meh_input_manager_get_input_state(InputManager* input_manager, SDL_Event* sdl_event) {
	g_assert(input_manager != NULL);
	g_assert(sdl_event != NULL);

	/* if it's a joystick */
	gchar* guid = NULL;
	if (sdl_event->type == SDL_JOYBUTTONDOWN || sdl_event->type == SDL_JOYBUTTONUP ||
			sdl_event->type == SDL_JOYAXISMOTION) {
		/* looks which gamepad has done the event */
		for (int i = 0; i < g_queue_get_length(input_manager->gamepads); i++) {
			Gamepad* gamepad = g_queue_peek_nth(input_manager->gamepads, i);

			SDL_JoystickID gamepad_event_id;
			if (sdl_event->type == SDL_JOYAXISMOTION) {
				gamepad_event_id = sdl_event->jaxis.which;	
			} else {
				gamepad_event_id = sdl_event->jbutton.which;	
			}

			/* if this this joystick having done the event ? */
			if (gamepad->instance_id == gamepad_event_id) {
				guid = gamepad->guid;
				break;
			}
		}
	}

	/* the event hasn't been done by a gamepad so,
	 * took the InputState of the keyboard. */
	if (guid == NULL) {
		guid = "keyboard";
	}

	InputState* found = NULL;
	/* yes it is, now, find its input state */
	for (int i = 0; i < g_queue_get_length(input_manager->input_states); i++) {
		InputState* input_state = g_queue_peek_nth(input_manager->input_states, i);
		if (g_strcmp0(guid, input_state->id) == 0) {
			found = input_state;
			break;
		}
	}

	g_assert(found != NULL);

	return found;
}

/*
 * meh_input_read_event reads the given SDL event 
 * and do things accordingly.
 */
void meh_input_manager_read_event(InputManager* input_manager, SDL_Event* sdl_event) {
	g_assert(input_manager != NULL);

	if (sdl_event == NULL) {
		return;
	}

	/* get the use 'input_state' having done the event */
	InputState* input_state = meh_input_manager_get_input_state(input_manager, sdl_event);

	int sdl_button = -1;
	gboolean keyboard = TRUE;
	int* pressed = NULL;

	switch (sdl_event->type) {
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			sdl_button = sdl_event->key.keysym.sym;
			break;
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			keyboard = FALSE;
			sdl_button = sdl_event->jbutton.button;
			break;
		case SDL_JOYAXISMOTION:
			/* Converts the axis + value to a SDLK constant */
			switch (sdl_event->jaxis.axis) {
				case 0: /* X axis */
					if (sdl_event->jaxis.value > MEH_INPUT_MAX_AXIS) {
						sdl_button = SDLK_RIGHT;
					} else if (sdl_event->jaxis.value < -MEH_INPUT_MAX_AXIS) {
						sdl_button = SDLK_LEFT;
					} else if (sdl_event->jaxis.value > -MEH_INPUT_MAX_AXIS && sdl_event->jaxis.value < MEH_INPUT_MAX_AXIS) {
						/* A bit harsh : we there was a motion axis but no value, 
						 * it means that the axis has returned to 0 and so we need
						 * to reset the direction values. */
						meh_input_manager_reset_button_state(input_manager, MEH_INPUT_BUTTON_LEFT);
						meh_input_manager_reset_button_state(input_manager, MEH_INPUT_BUTTON_RIGHT);
					}
					break;

				case 1: /* Y axis */
					if (sdl_event->jaxis.value > MEH_INPUT_MAX_AXIS) {
						sdl_button = SDLK_DOWN;
					} else if (sdl_event->jaxis.value < -MEH_INPUT_MAX_AXIS) {
						sdl_button = SDLK_UP;
					} else if (sdl_event->jaxis.value > -MEH_INPUT_MAX_AXIS && sdl_event->jaxis.value < MEH_INPUT_MAX_AXIS) {
						/* A bit harsh : we there was a motion axis but no value, 
						 * it means that the axis has returned to 0 and so we need
						 * to reset the direction values. */
						meh_input_manager_reset_button_state(input_manager, MEH_INPUT_BUTTON_UP);
						meh_input_manager_reset_button_state(input_manager, MEH_INPUT_BUTTON_DOWN);
					}
					break;
			}
			keyboard = FALSE;
			break;
	}

	/* Apply the mapping */
	/* TODO use the mapping of the InputState */
	if (keyboard == TRUE) {
		pressed = g_hash_table_lookup(input_manager->keyboard_mapping, &sdl_button);
	} else {
		pressed = g_hash_table_lookup(input_manager->gamepad_mapping, &sdl_button);
	}

	/* Not configured key pressed. Just store the last. */
	if (sdl_button == -1) {
		return;
	}

	int key_pressed = MEH_INPUT_SPECIAL_UNKNOWN;
	if (pressed != NULL) {
		key_pressed = *pressed;
	}

	input_state->last_sdl_key = sdl_button;

	/* This is a known key set it as pressed / unpressed */
	switch (sdl_event->type) {
		case SDL_KEYDOWN:
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYAXISMOTION:
			if (sdl_event->key.repeat == 0 || keyboard == FALSE) {
				input_state->buttons_state[key_pressed] = MEH_INPUT_JUST_PRESSED;
			}
			break;
		case SDL_KEYUP:
		case SDL_JOYBUTTONUP:
			input_state->buttons_state[key_pressed] = MEH_INPUT_NOT_PRESSED;
			input_state->buttons_next_message[key_pressed] = 0;
			break;
	}
}

/*
 * meh_input_manager_generate_messages transforms the input manager
 * states (button pressed) in messages and returns them in a list.
 */
GSList* meh_input_manager_generate_messages(InputManager* input_manager) {
	g_assert(input_manager != NULL);

	GSList* list = NULL;
	int i = 0;

	for (int j = 0; j < g_queue_get_length(input_manager->input_states); j++) {
		InputState* input_state = g_queue_peek_nth(input_manager->input_states, j);

		for (i = 0; i < MEH_INPUT_END; i++) {
			switch (input_state->buttons_state[i]) {
				case MEH_INPUT_JUST_PRESSED:
					/* Just pressed, we'll immediatly send a message */
					input_state->buttons_next_message[i] = SDL_GetTicks() + input_manager->settings.input_repeat_delay;
					input_state->buttons_state[i] = MEH_INPUT_HOLD;
					/* creates/appends a button pressed message */
					list = meh_input_manager_append_button_pressed(list, i, input_state->last_sdl_key, input_state->id);
					break;
				case MEH_INPUT_HOLD:
					if (SDL_GetTicks() > input_state->buttons_next_message[i]) {
						/* creates/appends a button pressed message */
						list = meh_input_manager_append_button_pressed(list, i, input_state->last_sdl_key, input_state->id);
						/* compute the next message time */
						input_state->buttons_next_message[i] = SDL_GetTicks() + input_manager->settings.input_repeat_frequency;
					}
					break;
			}
		}
	}

	return list;
}

/*
 * meh_input_manager_append_button_pressed creates a MEH_MSG_BUTTON_PRESSED message
 * and appends the id of the button in the data.
 */
GSList* meh_input_manager_append_button_pressed(GSList* list, int pressed_button, int last_sdl_key, gchar* guid) {
	InputMessageData* message = meh_input_message_new(pressed_button, last_sdl_key, guid);
	Message* m = meh_message_new(MEH_MSG_BUTTON_PRESSED, message);
	list = g_slist_append(list, m);
	return list;
}

InputMessageData* meh_input_message_new(int pressed_button, int last_sdl_key, gchar* guid) {
	InputMessageData* data = g_new(InputMessageData, 1);

	data->button = pressed_button;
	data->sdl_key = last_sdl_key;
	data->guid = g_strdup(guid);

	return data;
}

void meh_input_message_destroy(Message* message) {
	g_assert(message != NULL);

	InputMessageData* data = (InputMessageData*)message->data;

	g_free(data->guid);
	g_free(message->data);
	g_free(message);
}

/*
 * meh_input_create_default_keyboard_config create an input config
 * from the SDL_Event of the keyboard mapped to the internal
 * mehstation button configuration.
 
 */
static GHashTable* meh_input_create_default_keyboard_mapping() {
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
	
	sdl = g_new(int, 1); *sdl = SDLK_i;
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_L;
	g_hash_table_insert(mapping, sdl, button_mapping);
	
	sdl = g_new(int, 1); *sdl = SDLK_o;
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_R;
	g_hash_table_insert(mapping, sdl, button_mapping);
	
	sdl = g_new(int, 1); *sdl = SDLK_ESCAPE;
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_SPECIAL_ESCAPE;
	g_hash_table_insert(mapping, sdl, button_mapping);

	return mapping;
}

/*
 * meh_input_create_default_gamepad_config create an input config
 * from the SDL_Event of the gamepad mapped to the internal
 * mehstation button configuration.
 * It is based on a basic USB gamepad. Can be used as a fallback.
 */
static GHashTable* meh_input_create_default_gamepad_mapping() {
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

	sdl = g_new(int, 1); *sdl = 9;
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_START;
	g_hash_table_insert(mapping, sdl, button_mapping);

	sdl = g_new(int, 1); *sdl = 8;
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_SELECT;
	g_hash_table_insert(mapping, sdl, button_mapping);

	sdl = g_new(int, 1); *sdl = 1;
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_A;
	g_hash_table_insert(mapping, sdl, button_mapping);

	sdl = g_new(int, 1); *sdl = 2;
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_B;
	g_hash_table_insert(mapping, sdl, button_mapping);

	sdl = g_new(int, 1); *sdl = 4;
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_L;
	g_hash_table_insert(mapping, sdl, button_mapping);

	sdl = g_new(int, 1); *sdl = 5;
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_R;
	g_hash_table_insert(mapping, sdl, button_mapping);

	sdl = g_new(int, 1); *sdl = 0;
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_SPECIAL_ESCAPE;
	g_hash_table_insert(mapping, sdl, button_mapping);

	return mapping;
}

/*
 * mehstation - Input manager
 *
 * TODO detection of new device
 *
 * Copyright © 2015 Rémy Mathieu
 */
#include <glib.h>
#include <SDL2/SDL.h>

#include "system/db.h"
#include "system/message.h"
#include "system/input.h"
#include "system/settings.h"
#include "system/db/mapping.h"

static GHashTable* meh_input_create_default_keyboard_mapping();
static GHashTable* meh_input_create_default_gamepad_mapping();
static InputState* meh_input_manager_get_input_state(InputManager* input_manager, SDL_Event* sdl_event);
static void meh_input_manager_reset_button_state(InputManager* input_manager, int button);

/*
 * meh_input_manager_new creates a new InputManager.
 */
InputManager* meh_input_manager_new(DB* db, Settings settings) {
	InputManager* input_manager = g_new(InputManager, 1);
	input_manager->settings = settings;

	input_manager->input_states = g_queue_new();

	/* keyboard input state at index 0 */
	InputState* keyboard_state = g_new(InputState, 1);
	keyboard_state->id = g_strdup("keyboard");
	keyboard_state->mapping = NULL;
	keyboard_state->last_movement.up =
	keyboard_state->last_movement.down =
	keyboard_state->last_movement.left =
	keyboard_state->last_movement.right = 0;
	keyboard_state->last_movement.movement_type = MEH_UNKNOWN;
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
		gamepad_state->mapping = NULL; /* NOTE will be assigned later */
		gamepad_state->last_movement.up =
		gamepad_state->last_movement.down =
		gamepad_state->last_movement.left =
		gamepad_state->last_movement.right = 0;
		gamepad_state->last_movement.movement_type = MEH_UNKNOWN;

		g_queue_push_tail(input_manager->gamepads, gamepad);
		g_message("Using gamepad: %s", SDL_JoystickNameForIndex(i));
		g_debug("Has %d hats", SDL_JoystickNumHats(gamepad->joystick));
		g_queue_push_tail(input_manager->input_states, gamepad_state);
		g_debug("Adding gamepad %d with id : %s", i, gamepad_state->id);
	}

	/* now that we've created all the input state, look in the database 
	 * whether or not they have a mapping configured */
	meh_input_manager_assign_mapping(db, input_manager);

	meh_input_manager_reset_buttons_state(input_manager);

	return input_manager;
}

/*
 * meh_input_has_something_plugged returns true if at least one device 
 * has been mapped */
gboolean meh_input_manager_has_something_plugged(InputManager* input_manager) {
	for (unsigned int i = 0; i < g_queue_get_length(input_manager->input_states); i++) {
		InputState* state = g_queue_peek_nth(input_manager->input_states, i);
		if (state->mapping != NULL) {
			return TRUE;
		}
	}
	return FALSE;
}

/*
 * meh_input_manager_assign_mapping goes through all input states
 * to see whether we can assign them a mapping from the database. */
void meh_input_manager_assign_mapping(DB* db, InputManager* input_manager) {
	g_assert(input_manager != NULL);	

	for (unsigned int i = 0; i < g_queue_get_length(input_manager->input_states); i++) {
		InputState* state =  g_queue_peek_nth(input_manager->input_states, i);

		/* first, free the mapping if any */
		if (state->mapping != NULL) {
			meh_model_mapping_destroy(state->mapping);
			state->mapping = NULL;
		}

		/* look in the db if there is a mapping for this input state */
		const char* name = NULL;
		if (g_strcmp0(state->id, "keyboard") == 0) {
			name = state->id;
		} else {
			Gamepad* gamepad = meh_input_manager_gamepad_by_guid(input_manager, state->id);
			if (gamepad == NULL) {
				continue; /* should never happen */
			}
			name = gamepad->name;
		}

		/* finally assign what is in the database to the InputState */
		Mapping* mapping = meh_db_get_mapping(db, name);

		if (mapping != NULL) {
			state->mapping = mapping;
		}
	}
}

/*
 * meh_input_manager_destroy free the resources of the given input_manager.
 */
void meh_input_manager_destroy(InputManager* input_manager) {
	g_assert(input_manager != NULL);

	/* close all gamepads */
	for (unsigned int i = 0; i < g_queue_get_length(input_manager->gamepads); i++) {
		Gamepad* gamepad = g_queue_peek_nth(input_manager->gamepads, i);

		SDL_JoystickClose(gamepad->joystick);
		g_free(gamepad->guid);
		g_free(gamepad);
	}
	g_queue_free(input_manager->gamepads);

	/* release each input state */
	for (unsigned int i = 0; i < g_queue_get_length(input_manager->input_states); i++) {
		InputState* state = g_queue_peek_nth(input_manager->input_states, i);
		if (state->mapping != NULL) {
			meh_model_mapping_destroy(state->mapping);
		}
		g_free(state->id); /* free the string used as ID */
		g_free(state);
	}
	g_queue_free(input_manager->input_states);

	g_hash_table_destroy(input_manager->keyboard_mapping);
	g_hash_table_destroy(input_manager->gamepad_mapping);

	g_free(input_manager);
}

/*
 * meh_input_manager_gamepad_by_guid finds the instance of Gamepad matching
 * the given ID.
 */
Gamepad* meh_input_manager_gamepad_by_guid(InputManager* input_manager, gchar* guid) {
	g_assert(input_manager != NULL);
	g_assert(guid != NULL);

	for (unsigned int i = 0; i < g_queue_get_length(input_manager->gamepads); i++) {
		Gamepad* gamepad = g_queue_peek_nth(input_manager->gamepads, i);

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

	for (unsigned int j = 0; j < g_queue_get_length(input_manager->input_states); j++) {
		InputState* input_state = g_queue_peek_nth(input_manager->input_states, j);
		input_state->buttons_state[button] = MEH_INPUT_NOT_PRESSED;
		input_state->buttons_next_message[button] = 0;
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
			sdl_event->type == SDL_JOYAXISMOTION || sdl_event->type == SDL_JOYHATMOTION) {
		/* looks which gamepad has done the event */
		for (unsigned int i = 0; i < g_queue_get_length(input_manager->gamepads); i++) {
			Gamepad* gamepad = g_queue_peek_nth(input_manager->gamepads, i);

			SDL_JoystickID gamepad_event_id;
			switch (sdl_event->type) {
				case SDL_JOYAXISMOTION:
					gamepad_event_id = sdl_event->jaxis.which;
					break;
				case SDL_JOYHATMOTION:
					gamepad_event_id = sdl_event->jhat.which;
					break;
				default:
					gamepad_event_id = sdl_event->jbutton.which;
					break;
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
	for (unsigned int i = 0; i < g_queue_get_length(input_manager->input_states); i++) {
		InputState* input_state = g_queue_peek_nth(input_manager->input_states, i);
		if (g_strcmp0(guid, input_state->id) == 0) {
			found = input_state;
			break;
		}
	}

	g_assert(found != NULL);
	return found;
}

static InputDirectionMove meh_input_read_axis_events(InputState* state, SDL_Event* sdl_event) {
	g_assert(state != NULL);
	g_assert(sdl_event != NULL);

	InputDirectionMove move;

	move.up = move.down = move.left = move.right = 0;
	move.movement_type = MEH_UNKNOWN;
	move.tick = SDL_GetTicks();

	switch (sdl_event->type) {
		/* DPAD support */
		case SDL_JOYHATMOTION:
			switch (sdl_event->jhat.value) {
				case SDL_HAT_CENTERED:
					move.movement_type = MEH_HAT;
					move.up = -1;
					move.down = -1;
					move.left = -1;
					move.right = -1;
					break;
				case SDL_HAT_UP:
				case SDL_HAT_RIGHTUP:
				case SDL_HAT_LEFTUP: // FIXME(remy): mehstation doesn't support diagonal
					move.movement_type = MEH_HAT;
					move.up = 1;
					break;
				case SDL_HAT_RIGHT:
					move.movement_type = MEH_HAT;
					move.right = 1;
					break;
				case SDL_HAT_LEFT:
					move.movement_type = MEH_HAT;
					move.left = 1;
					break;
				case SDL_HAT_LEFTDOWN:
				case SDL_HAT_RIGHTDOWN:
				case SDL_HAT_DOWN:
					move.movement_type = MEH_HAT;
					move.down = 1;
					break;
			}
			break;
		/* Stick support */
		case SDL_JOYAXISMOTION:
			switch (sdl_event->jaxis.axis) {
				case 0: /* X axis */
					if (sdl_event->jaxis.value > MEH_INPUT_MAX_AXIS) {
						move.movement_type = MEH_JOYSTICK;
						move.right = 1;
					} else if (sdl_event->jaxis.value < -MEH_INPUT_MAX_AXIS) {
						move.movement_type = MEH_JOYSTICK;
						move.left = 1;
					} else if (sdl_event->jaxis.value > -MEH_INPUT_MAX_AXIS && sdl_event->jaxis.value < MEH_INPUT_MAX_AXIS) {
						/* reset these directions */
						move.movement_type = MEH_JOYSTICK;
						move.left = -1;
						move.right = -1;
					}
					break;
				case 1: /* Y axis */
					if (sdl_event->jaxis.value > MEH_INPUT_MAX_AXIS) {
						move.movement_type = MEH_JOYSTICK;
						move.down = 1;
					} else if (sdl_event->jaxis.value < -MEH_INPUT_MAX_AXIS) {
						move.movement_type = MEH_JOYSTICK;
						move.up = 1;
					} else if (sdl_event->jaxis.value >= -MEH_INPUT_MAX_AXIS && sdl_event->jaxis.value <= MEH_INPUT_MAX_AXIS) {
						move.movement_type = MEH_JOYSTICK;
						move.down = -1;
						move.up = -1;
					}
					break;
			}
	}

	return move;
}


/*
 * meh_input_read_event reads the given SDL event to finally change the state
 * of key pressed / releaseed.
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
		case SDL_JOYHATMOTION:
		case SDL_JOYAXISMOTION:
			keyboard = FALSE;
		break;
	}

	/* Read directions state */

	InputDirectionMove move = meh_input_read_axis_events(input_state, sdl_event);

	if (SDL_GetTicks()-(input_state->last_movement.tick) > 200) { /* ignore jitter */
		if (move.up > 0) {
			input_state->last_movement = move;
			sdl_button = SDLK_UP;
		} else if (move.down > 0) {
			input_state->last_movement = move;
			sdl_button = SDLK_DOWN;
		}  else if (move.left > 0) {
			input_state->last_movement = move;
			sdl_button = SDLK_LEFT;
		} else if (move.right > 0) {
			input_state->last_movement = move;
			sdl_button = SDLK_RIGHT;
		}
	}

	/* reset only if removed by the same axis input */
	if (input_state->last_movement.movement_type == move.movement_type) {
		if (move.up < 0) {
			meh_input_manager_reset_button_state(input_manager, MEH_INPUT_BUTTON_UP);
		}
		if (move.down < 0) {
			meh_input_manager_reset_button_state(input_manager, MEH_INPUT_BUTTON_DOWN);
		}
		if (move.left < 0) {
			meh_input_manager_reset_button_state(input_manager, MEH_INPUT_BUTTON_LEFT);
		}
		if (move.right < 0) {
			meh_input_manager_reset_button_state(input_manager, MEH_INPUT_BUTTON_RIGHT);
		}
	}

	/* Apply the mapping */

	if (input_state->mapping != NULL) {
		pressed = g_hash_table_lookup(input_state->mapping->m, &sdl_button);
	}

	int key_pressed = MEH_INPUT_SPECIAL_UNKNOWN;
	if (pressed != NULL) {
		key_pressed = *pressed;
	}

	if (sdl_button < 0) {
		return;
	}

	input_state->last_sdl_key = sdl_button;

	switch (sdl_event->type) {
		case SDL_KEYDOWN:
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYAXISMOTION:
		case SDL_JOYHATMOTION:
			/* ignore on hat returned to center and
			 * axis released */
			if (sdl_event->type == SDL_JOYHATMOTION && sdl_event->jhat.value == SDL_HAT_CENTERED) {
				break;
			}

			if (move.up < 0 || move.down < 0 || move.right < 0 || move.left < 0) {
				break;
			}

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
	for (unsigned int j = 0; j < g_queue_get_length(input_manager->input_states); j++) {
		InputState* input_state = g_queue_peek_nth(input_manager->input_states, j);

		for (int i = 0; i < MEH_INPUT_END; i++) {
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

/*
 * meh_input_message_new creates the special message to send on input.
 */
InputMessageData* meh_input_message_new(int pressed_button, int last_sdl_key, gchar* guid) {
	InputMessageData* data = g_new(InputMessageData, 1);

	data->button = pressed_button;
	data->sdl_key = last_sdl_key;
	data->guid = g_strdup(guid);

	return data;
}

/*
 * meh_input_message_destroy destroys the special message sent on input.
 */
void meh_input_message_destroy(Message* message) {
	g_assert(message != NULL);

	InputMessageData* data = (InputMessageData*)message->data;

	g_free(data->guid);
	g_free(message->data);
	g_free(message);
}

/*
 * meh_input_create_mapping creates the hash table mapping with the given bindings.
 */
GHashTable* meh_input_create_mapping(int up, int down, int left, int right, int start,
									int select, int a, int b, int l, int r, int escape) {
	GHashTable* mapping = g_hash_table_new_full(
			g_int_hash, 
			g_int_equal,
			(GDestroyNotify)g_free,
			(GDestroyNotify)g_free);

	int* sdl = g_new(int, 1); *sdl = up;
	int* button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_UP;
	g_hash_table_insert(mapping, sdl, button_mapping);

	sdl = g_new(int, 1); *sdl = down;
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_DOWN;
	g_hash_table_insert(mapping, sdl, button_mapping);

	sdl = g_new(int, 1); *sdl = left;
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_LEFT;
	g_hash_table_insert(mapping, sdl, button_mapping);

	sdl = g_new(int, 1); *sdl = right;
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_RIGHT;
	g_hash_table_insert(mapping, sdl, button_mapping);

	sdl = g_new(int, 1); *sdl = start;
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_START;
	g_hash_table_insert(mapping, sdl, button_mapping);

	sdl = g_new(int, 1); *sdl = select;
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_SELECT;
	g_hash_table_insert(mapping, sdl, button_mapping);

	sdl = g_new(int, 1); *sdl = a;
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_A;
	g_hash_table_insert(mapping, sdl, button_mapping);

	sdl = g_new(int, 1); *sdl = b;
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_B;
	g_hash_table_insert(mapping, sdl, button_mapping);
	
	sdl = g_new(int, 1); *sdl = l;
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_L;
	g_hash_table_insert(mapping, sdl, button_mapping);
	
	sdl = g_new(int, 1); *sdl = r;
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_BUTTON_R;
	g_hash_table_insert(mapping, sdl, button_mapping);

	sdl = g_new(int, 1); *sdl = SDLK_ESCAPE;
	if (escape != 0) {
		*sdl = escape;
	}
	button_mapping = g_new(int, 1); *button_mapping = MEH_INPUT_SPECIAL_ESCAPE;
	g_hash_table_insert(mapping, sdl, button_mapping);

	return mapping;
}

/*
 * meh_input_create_default_keyboard_config create an input config
 * from the SDL_Event of the keyboard mapped to the internal
 * mehstation button configuration.
 */
static GHashTable* meh_input_create_default_keyboard_mapping() {
	return meh_input_create_mapping(SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_RETURN,
									SDLK_SPACE, SDLK_r, SDLK_e, SDLK_i, SDLK_o, SDLK_ESCAPE);
}

/*
 * meh_input_create_default_gamepad_config create an input config
 * from the SDL_Event of the gamepad mapped to the internal
 * mehstation button configuration.
 * It is based on a basic USB gamepad. Can be used as a fallback.
 */
static GHashTable* meh_input_create_default_gamepad_mapping() {
	return meh_input_create_mapping(SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, 9,
									8, 1, 2, 4, 5, 0);
}

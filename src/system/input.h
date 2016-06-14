#pragma once

#include "glib-2.0/glib.h"
#include "SDL2/SDL.h"

#include "system/consts.h"
#include "system/message.h"
#include "system/settings.h"
#include "system/db/models.h"

#define MEH_INPUT_NOT_PRESSED 0
#define MEH_INPUT_JUST_PRESSED 1
#define MEH_INPUT_HOLD 2

#define MEH_UNKNOWN 0
#define MEH_HAT 1
#define MEH_JOYSTICK 2

#define MEH_INPUT_MAX_AXIS (32767-1000)

struct DB;

/*
 * InputManager role is to receive events from
 * keyboard, gamepads, ..., and to deal with them.
 */
typedef struct {
	/* application settings */
	Settings settings;

	/* set gamepad mapping */
	GHashTable* keyboard_mapping;
	GHashTable* gamepad_mapping;

	/* the input states of each controller
	 * NOTE we could use a map here but as we're sure there
	 * won't have a lot of entries in this queue, I prefer
	 * a queue and to do a loop to test the id than the use of
	 * a GHashTable and its heavy syntax - remy */
	GQueue* input_states;

	/* attached game pads */
	GQueue* gamepads;
} InputManager;

typedef struct {
	int up;
	int down;
	int left;
	int right;
	int movement_type;
	int tick;
} InputDirectionMove;

/*
 * InputState stores the information on pressed
 * buttons.
 */
typedef struct {
	gchar* id; /* this memory should be freed */
	/* contains all the pressed buttons after reading of the events */
	guint buttons_state[MEH_INPUT_END];
	/* time at which the key produce a new message */
	guint buttons_next_message[MEH_INPUT_END];
	/* mapping of this input state */
	Mapping* mapping;	
	/* last pressed sdl key */
	int last_sdl_key; 
	/* true if the last move has been done by a hat, joystick or unknown */
	InputDirectionMove last_movement;
} InputState;

typedef struct {
	SDL_Joystick* joystick; /* SDL joystick */
	SDL_JoystickID instance_id;
	gchar* guid; 
	const gchar* name;
} Gamepad;

typedef struct {
	int button;
	int sdl_key;
	gchar* guid;
} InputMessageData;

/* input manager */
InputManager* meh_input_manager_new(struct DB*, Settings settings);
void meh_input_manager_destroy(InputManager* input_manager);
void meh_input_manager_reset_buttons_state(InputManager* input_manager);
void meh_input_manager_read_event(InputManager* input_manager, SDL_Event* event);
GSList* meh_input_manager_generate_messages(InputManager* input_manager);
GSList* meh_input_manager_append_button_pressed(GSList* list, int pressed_button, int last_sdl_key, gchar* guid);
Gamepad* meh_input_manager_gamepad_by_guid(InputManager* input_manager, gchar* guid);

InputMessageData* meh_input_message_new(int pressed_button, int last_sdl_key, gchar* guid);
void meh_input_message_destroy(Message* message);
GHashTable* meh_input_create_mapping(int up, int down, int left, int right, int start, int select, int a, int b, int l, int r, int escape);
void meh_input_manager_assign_mapping(struct DB* db, InputManager* input_manager);
gboolean meh_input_manager_has_something_plugged(InputManager* input_manager);

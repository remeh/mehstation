/*
 * mehstation - Transition on values.
 *
 * Copyright © 2015 Rémy Mathieu
 */
#pragma once 

#include <glib.h>

#define MEH_TRANSITION_NONE 0
#define MEH_TRANSITION_LINEAR 1
#define MEH_TRANSITION_CUBIC 2
#define MEH_TRANSITION_QUADRATIC 3

typedef struct {
	guint transition_type;
	float duration; /* duration of the transition */
	float start_tick;
	float start_value; /* the starting value */
	float final_value; /* the final value */
	float value;
	gboolean ended;
} Transition;

Transition meh_transition_start(int transition_type, int start_value, int final_value, int duration);
gboolean meh_transition_update(Transition* transition);
void meh_transitions_update(GQueue* transitions);
void meh_transition_end(Transition* transition);
void meh_transitions_end(GQueue* transition);

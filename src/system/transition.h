#pragma once 

/*
 * mehstation - Easing helpers.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#include <glib.h>

#define MEH_TRANSITION_NONE 0
#define MEH_TRANSITION_LINEAR 1
#define MEH_TRANSITION_CUBIC 2
#define MEH_TRANSITION_QUADRATIC 3

typedef struct {
	guint transition_type;
	gfloat duration; /* duration of the transition */
	gfloat start_tick;
	gfloat start_value; /* the starting value */
	gfloat final_value; /* the final value */
	gfloat value;
	gboolean ended;
} Transition;

Transition meh_transition_new(int transition_type, guint start_value, guint change, guint duration);
void meh_transition_compute(Transition* transition);

/*
 * mehstation - Transition on values.
 *
 * Copyright © 2015 Rémy Mathieu
 */
#include <glib.h>
#include <SDL2/SDL.h>

#include "system/transition.h"

#define MEH_EPSILON 2.0f

/*
 * meh_transition_start prepares and starts a new transition.
 */
Transition meh_transition_start(int transition_type, int start_value, int final_value, int duration) {
	Transition ti;
	ti.transition_type = transition_type;
	ti.start_tick = -1;
	ti.start_value = (float)start_value;
	ti.final_value = (float)final_value;
	ti.duration = (float)duration;
	ti.value = (float)start_value;
	ti.ended = FALSE;
	return ti;
}

/*
 * meh_transitions_update updates a queue of transitions.
 */
void meh_transitions_update(GQueue* transitions) {
	g_assert(transitions != NULL);

	for (int i = 0; i < g_queue_get_length(transitions); i++) {
		Transition* transition = g_queue_peek_nth(transitions, i);
		meh_transition_update(transition);
	}
}

/*
 * meh_transition_update compute the next value for the given transition.
 */
void meh_transition_update(Transition* tvar) {
	if (tvar == NULL) {
		return;
	}

	if (tvar->ended == TRUE) {
		return;
	}

	float time = 0;
	if (tvar->start_tick == -1) {
		tvar->start_tick = (float)SDL_GetTicks();
	} else {
		time = (float)SDL_GetTicks() - tvar->start_tick;
	}

	float change = tvar->final_value - tvar->start_value;

	switch (tvar->transition_type) {
		case MEH_TRANSITION_LINEAR:
			tvar->value = (change*(time / tvar->duration)) + tvar->start_value;
			break;
		case MEH_TRANSITION_CUBIC:
			time /= tvar->duration/2.0f;
			if (time < 1.0f) {
				tvar->value = change/2*time*time*time + tvar->start_value;
			} else {
				time -= 2.0f;
				tvar->value = change/2.0f*(time*time*time + 2.0f) + tvar->start_value;
			}
			break;
		case MEH_TRANSITION_QUADRATIC:
			time /= tvar->duration/2.0f;
			if (time < 1.0f) {
				tvar->value = change/2.0f*time*time + tvar->start_value;
			} else {
				time -= 1.0f;
				tvar->value = -change/2.0f * (time*(time-2.0f) - 1.0f) + tvar->start_value;
			}

			break;
	}

	if (tvar->final_value > tvar->start_value && tvar->final_value - tvar->value < MEH_EPSILON) {
		tvar->ended = TRUE;
	} else if (tvar->final_value <= tvar->start_value && tvar->value - tvar->final_value < MEH_EPSILON) {
		tvar->ended = TRUE;
	}
}



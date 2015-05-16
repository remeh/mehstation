/*
 * mehstation - Transition on values.
 *
 * Copyright © 2015 Rémy Mathieu
 */
#include <glib.h>
#include <SDL2/SDL.h>

#include "system/transition.h"

#define MEH_COEFF 10000.0f /* We apply a coefficient because we're on small numbers. */
#define MEH_EPSILON 2.0f

/*
 * meh_transition_start prepares and starts a new transition.
 */
Transition meh_transition_start(int transition_type, float start_value, float final_value, int duration) {
	Transition ti;
	ti.transition_type = transition_type;
	ti.start_tick = -1;
	ti.start_value = start_value*MEH_COEFF;
	ti.final_value = final_value*MEH_COEFF;
	ti.duration = duration;
	ti.value = start_value;
	ti.ended = FALSE;
	return ti;
}

/*
 * meh_transitions_end ends all the given transitions.
 */
void meh_transitions_end(GQueue* transitions) {
	g_assert(transitions != NULL);

	for (int i = 0; i < g_queue_get_length(transitions); i++) {
		Transition* transition = g_queue_peek_nth(transitions, i);
		meh_transition_end(transition);
	}
}

/*
 * meh_transition_end ends the given transition.
 */
void meh_transition_end(Transition* transition) {
	g_assert(transition != NULL);
	transition->value = transition->final_value/MEH_COEFF;
	transition->ended = TRUE;
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
gboolean meh_transition_update(Transition* transition) {
	if (transition == NULL) {
		return TRUE;
	}

	if (transition->ended == TRUE) {
		return TRUE;
	}

	float time = 0;
	if (transition->start_tick == -1) {
		transition->start_tick = (float)SDL_GetTicks();
	} else {
		time = (float)SDL_GetTicks() - transition->start_tick;
	}

	float change = transition->final_value - transition->start_value;

	switch (transition->transition_type) {
		case MEH_TRANSITION_LINEAR:
			transition->value = ( (change*(time / transition->duration)) + transition->start_value ) / MEH_COEFF;
			break;
		case MEH_TRANSITION_CUBIC:
			time /= transition->duration/2.0f;
			if (time < 1.0f) {
				transition->value = ( change/2.0f*time*time*time + transition->start_value ) / MEH_COEFF;
			} else {
				time -= 2.0f;
				transition->value = ( change/2.0f*(time*time*time + 2.0f) + transition->start_value ) / MEH_COEFF;
			}
			break;
		case MEH_TRANSITION_QUADRATIC:
			time /= transition->duration/2.0f;
			if (time < 1.0f) {
				transition->value = ( change/2.0f*time*time + transition->start_value ) / MEH_COEFF;
			} else {
				time -= 1.0f;
				transition->value = ( -change/2.0f * (time*(time-2.0f) - 1.0f) + transition->start_value ) / MEH_COEFF;
			}

			break;
	}

	/* Perfect value for the end */
	if (transition->final_value > transition->start_value && transition->final_value - (transition->value*MEH_COEFF)< MEH_EPSILON) {
		meh_transition_end(transition);
		return TRUE;
	} else if (transition->final_value <= transition->start_value && (transition->value*MEH_COEFF) - transition->final_value < MEH_EPSILON) {
		meh_transition_end(transition);
		return TRUE;
	}

	return FALSE;
}

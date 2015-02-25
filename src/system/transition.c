#include <glib.h>
#include <SDL2/SDL.h>

#include "system/transition.h"

Transition meh_transition_new(int transition_type, guint start_value, guint final_value, guint duration) {
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

void meh_transition_compute(Transition* tvar) {
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

	if (tvar->final_value > tvar->start_value && tvar->final_value - tvar->value < 0.9f) {
		tvar->ended = TRUE;
	} else if (tvar->final_value <= tvar->start_value && tvar->value - tvar->final_value < 0.9f) {
		tvar->ended = TRUE;
	}
}



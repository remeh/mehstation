#include <stdlib.h>
#include <glib.h>

#include "system/transition.h"
#include "view/widget_text.h"
#include "view/screen.h"

Screen* meh_screen_new() {
	Screen* screen = g_new(Screen, 1);
	screen->parent_screen = NULL;
	screen->data = NULL;
	screen->transitions = g_queue_new();
	screen->destroy_data = NULL;
	return screen;
}

/*
 * meh_screen_destroy releases all the memory of the given screen.
 */
void meh_screen_destroy(Screen* screen) {
	g_assert(screen != NULL);

	if (screen->data != NULL) {
		if (screen->destroy_data != NULL) {
			screen->destroy_data(screen);
		} else {
			g_warning("No destroy_data on Screen '%s', but the data isn't NULL!", screen->name);
		}
	}

	g_queue_free(screen->transitions);
	g_free(screen->name);
	g_free(screen);
}

/*
 * meh_screen_add_transition adds a transition to the screen.
 */
void meh_screen_add_transition(Screen* screen, Transition* transition) {
	g_assert(screen != NULL);
	g_assert(transition != NULL);

	g_queue_push_tail(screen->transitions, transition);
}

/*
 * meh_screen_update_transitions updates all the transitions of the given screen,
 * removing them from the queue if there are ended.
 */
void meh_screen_update_transitions(Screen* screen) {
	g_assert(screen != NULL);

	int length = g_queue_get_length(screen->transitions);
	Transition* to_delete[length];

	for (int i = 0; i < length; i++) {
		to_delete[i] = NULL;
		Transition* transition = g_queue_peek_nth(screen->transitions, i);
		/* update the transition and if the transition is ended,
		 * remove it from the queue */
		if (meh_transition_update(transition)) {
			to_delete[i] = transition;
		}
	}
	
	for (int i = 0; i < length; i++) {
		if (to_delete[i] != NULL) {
			g_queue_remove(screen->transitions, to_delete[i]);
		}
	}
}

/*
 * meh_screen_add_image_transitions adds all the transition contained into the given image widget.
 */
void meh_screen_add_image_transitions(Screen* screen, WidgetImage* image) {
	g_assert(screen != NULL);
	g_assert(image != NULL);

	meh_screen_add_transition(screen, &image->x);
	meh_screen_add_transition(screen, &image->y);
	meh_screen_add_transition(screen, &image->w);
	meh_screen_add_transition(screen, &image->h);
}

/*
 * meh_screen_add_rect_transitions adds all the transition contained into the given rect widget.
 */
void meh_screen_add_rect_transitions(Screen* screen, WidgetRect* rect) {
	g_assert(screen != NULL);
	g_assert(rect != NULL);

	meh_screen_add_transition(screen, &rect->x);
	meh_screen_add_transition(screen, &rect->y);
	meh_screen_add_transition(screen, &rect->w);
	meh_screen_add_transition(screen, &rect->h);

	meh_screen_add_transition(screen, &rect->r);
	meh_screen_add_transition(screen, &rect->g);
	meh_screen_add_transition(screen, &rect->b);
	meh_screen_add_transition(screen, &rect->a);
}

/*
 * meh_screen_add_text_transitions adds all the transition contained into the given text widget.
 */
void meh_screen_add_text_transitions(Screen* screen, WidgetText* text) {
	g_assert(screen != NULL);
	g_assert(text != NULL);

	meh_screen_add_transition(screen, &text->x);
	meh_screen_add_transition(screen, &text->y);

	meh_screen_add_transition(screen, &text->r);
	meh_screen_add_transition(screen, &text->g);
	meh_screen_add_transition(screen, &text->b);
}

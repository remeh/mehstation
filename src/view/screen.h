/*
 * mehstation - Screen.
 *
 * Copyright © 2015 Rémy Mathieu
 */
#pragma once

#include <glib.h>
#include "view/text.h"
#include "view/widget_image.h"
#include "view/widget_rect.h"
#include "view/widget_text.h"
#include "view/window.h"
#include "system/message.h"
#include "system/transition.h"

/* cross-reference. */
struct App;
struct Window;

typedef struct Screen {
	/* To which window this screen is attached. This memory shouldn't be freed. */
	Window* window;
	/* Parent screen of this screen, nullable. */
	struct Screen* parent_screen;
	/* Displayable name of the screen */
	gchar* name;
	/* Transitions used in the screen,
	 * the content of this queue must NOT be freed */
	GQueue* transitions;
	/* The messages handler to use for this Screen */
	int (*messages_handler) (struct App* app, struct Screen* screen, Message* message);
	/* extra data of the screen, if any, a destroy_data method must be attached. */
	void* data;
	void (*destroy_data) ();
} Screen;

Screen* meh_screen_new(Window*);
void meh_screen_destroy(Screen* screen);
void meh_screen_destroy_data_stub(Screen* screen);
void meh_screen_add_transition(Screen* screen, Transition* transition);
void meh_screen_add_image_transitions(Screen* screen, WidgetImage* image);
void meh_screen_add_text_transitions(Screen* screen, WidgetText* text);
void meh_screen_add_rect_transitions(Screen* screen, WidgetRect* rect);
void meh_screen_update_transitions(Screen* screen);

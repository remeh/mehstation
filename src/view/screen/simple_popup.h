/*
 * mehstation - A simple popup.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#pragma once

#include "view/screen.h"
#include "view/widget_rect.h"

struct App;
struct Executable;
struct Platform;

typedef struct {
	/* upon which screen the popup is appearing. */
	Screen* src_screen;

	/* these objects can be either null or valid,
	 * it depends on the context having opened the
	 * popup.
	 * they must be checked before any use in the popup */
	struct Executable* executable;
	struct Platform* platform;
	
	/* Widgets */
	WidgetRect* hover_widget;

	/* popup title */
	WidgetText* title_widget;

	WidgetRect* selection_widget;

    /* popup list of actions */
    GQueue* actions;

	int x;
	int y;

	int height;
	int width;

	/* selected action */
	int action;
} SimplePopupData;

typedef struct {
	gchar* label;
	void (*run) (struct App*, struct Screen*);
} SimplePopupAction;

Screen* meh_simple_popup_new(App* app, Screen* src_screen, struct Platform* platform, struct Executable* executable);
SimplePopupData* meh_simple_popup_get_data(Screen* screen);
void meh_simple_popup_destroy_data(Screen* screen);
int meh_simple_popup_messages_handler(struct App* app, Screen* screen, Message* message);
int meh_simple_popup_update(struct App* app, Screen* screen);
void meh_simple_popup_render(struct App* app, Screen* screen);
void meh_simple_popup_add_action(Screen* screen, gchar* label, void (*func) (struct App*, struct Screen*));

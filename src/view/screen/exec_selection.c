#include "view/screen/exec_selection.h"
#include "view/screen/executable_list.h"

void meh_exec_selection_prepare(App* app, Screen* screen, ExecutableListData* data) {
	g_assert(app != NULL);
	g_assert(screen != NULL);
	g_assert(data != NULL);

	SDL_Color white = { 255, 255, 255, 255 };
	SDL_Color white_transparent = { 255, 255, 255, 50 };

	/* Selection */
	data->selection_widget = meh_widget_rect_new(0, -100, 415, 28, white_transparent, TRUE);
	data->selection_widget->y = meh_transition_start(MEH_TRANSITION_CUBIC, -100, 130, 500);
	meh_screen_add_rect_transitions(screen, data->selection_widget);

	/* Executables */
	for (int i = 0; i < MEH_EXEC_LIST_SIZE; i++) {
		WidgetText* text = meh_widget_text_new(app->small_font, "", 55, 130+(i*32), 350, 30, white, FALSE);
		text->uppercase = TRUE; /* executables name in uppercase */
		g_queue_push_tail(data->executable_widgets, text);
	}
}

void meh_exec_selection_destroy(Screen* screen, ExecutableListData* data) {
	g_assert(screen != NULL);
	g_assert(data != NULL);

	/* Selection */
	meh_widget_rect_destroy(data->selection_widget);

	/* Executables */
	for (unsigned int i = 0; i < g_queue_get_length(data->executable_widgets); i++) {
		meh_widget_text_destroy( g_queue_peek_nth( data->executable_widgets, i) );
	}
	g_queue_free(data->executable_widgets);
}

void meh_exec_selection_render(App* app, Screen* screen, ExecutableListData* data) {
	/* selection */
	meh_widget_rect_render(app->window, data->selection_widget);

	/* render all the executables names. */
	for (unsigned int i = 0; i < g_queue_get_length(data->executable_widgets); i++) {
		meh_widget_text_render(app->window, g_queue_peek_nth(data->executable_widgets, i));
	}
}

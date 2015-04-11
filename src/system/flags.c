/*
 * mehstation - Flags parsing.
 *
 * Copyright © 2015 Rémy Mathieu
 */

#include <glib.h>

#include "system/flags.h"

/*
 * meh_flags_parse parses the commandline options to returns
 * the flags configuration.
 */
Flags meh_flags_parse(int argc, char* argv[]) {
	Flags f;

	/* default values */
	f.configure_mapping = FALSE;

	/* defining the flags */
	GOptionEntry flags[] =
	{
		{ "mapping", 'm', 0, G_OPTION_ARG_NONE, &f.configure_mapping, "Go through the mapping screen when starting.", NULL },
		{ NULL }
	};

	/* reading the flags */
	GError *error = NULL;
	GOptionContext *context = g_option_context_new(NULL);
	g_option_context_add_main_entries(context, flags, NULL);
	if (!g_option_context_parse (context, &argc, &argv, &error))
	{
		g_critical("option parsing failed: %s\n", error->message);
	}

	g_debug("flags read");

	return f;
}

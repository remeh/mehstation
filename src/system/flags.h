#pragma once

/* CLI options */
typedef struct {
	/* do we need to go through the mapping
	 * configuration screen at startup */
	gboolean configure_mapping;
	/* to force the software renderer */
	gboolean force_software;
	/* start on the debug input screen */
	gboolean debug_input;
} Flags;

Flags meh_flags_parse(int argc, char* argv[]);

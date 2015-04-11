#pragma once

/* CLI options */
typedef struct {
	/* do we need to go through the mapping
	 * configuration screen at startup */
	gboolean configure_mapping;
} Flags;

Flags meh_flags_parse(int argc, char* argv[]);

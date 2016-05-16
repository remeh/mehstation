#include <glib.h>

#include "system/utils.h"

static gchar* meh_prepend_zero(gint value);

/* meh_printable_datetime returns string containing
 * a displayable datetime.
 * The gchar* should be freed by the caller.
 */
gchar* meh_displayable_datetime(GDateTime* datetime) {
	gchar* year = meh_prepend_zero(g_date_time_get_year(datetime));
	gchar* month = meh_prepend_zero(g_date_time_get_month(datetime));
	gchar* day = meh_prepend_zero(g_date_time_get_day_of_month(datetime));
	gchar* hour = meh_prepend_zero(g_date_time_get_hour(datetime));
	gchar* minute = meh_prepend_zero(g_date_time_get_minute(datetime));

	gchar* rv = g_strdup_printf(
			"%s-%s-%s %s:%s",
			year,
			month,
			day,
			hour,
			minute
	);

	g_free(year);
	g_free(month);
	g_free(day);
	g_free(hour);
	g_free(minute);

	return rv;
}

static gchar* meh_prepend_zero(gint value) {
	if (value < 10) {
		return g_strdup_printf("0%d", value);
	}
	return g_strdup_printf("%d", value);
}

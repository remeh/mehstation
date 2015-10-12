#include  "os.h"

#ifdef LINUX

void meh_os_shutdown() {
	system("poweroff");
}

#endif

#include  "os.h"

#ifdef WINDOWS

void meh_os_shutdown() {
	system("sudo shutdown -s -t 0");
}

#endif

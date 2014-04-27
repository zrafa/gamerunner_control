#include <X11/Xlib.h>
#include <X11/keysym.h>

#include <stdio.h>
#include <linux/input.h>


#include <signal.h>
#include <errno.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <ctype.h>
#include <dirent.h>
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>

#undef USE_THREADS
#define USE_THREADS
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>

main(int argc, char *argv[])
{
	int om_accel = 0;  // COLOCARLOS DENTRO DEL WHILE !!!!

	struct input_event  val, val2;

	char *k[19]={"g", "Return", "Escape", "Control_L", "Control_R", "Delete", "Page_Down", "End", "Shift", "Left", "Right", "Up", "Down", "space", "a", "s", "d", "f", "g"};

	Window window = 0;
	int res1=0;

	static int event0_fd = -1;
	static int event1_fd = -1;
	struct input_event ev[64];
	struct input_event ev1[64];

	/* power button */
	event0_fd = open("/dev/input/event4", O_RDWR);
	int button=0, x=0, y=0, realx=0, realy=0, i, rd;
        int x1,y1;

	int tecla=0;

	int j=0;
	int doneYet1=0;
	while (!doneYet1) {

		  button=0; x=0; y=0; realx=0; realy=0;

		  rd = read(event0_fd, ev, sizeof(struct input_event) * 64);


		  for (i = 0; i < (rd / sizeof(struct input_event) ); i++) {

			if (EV_KEY == ev[i].type) {
        printf("%ld.%06ld ",
			ev[i].time.tv_sec,
			ev[i].time.tv_usec);
        printf("type %d code %d value %d\n",
			ev[i].type,
			ev[i].code, ev[i].value);
			system("/usr/bin/powerbutton.sh");
	}

}
sleep(1);
}
close(event0_fd);

}

/*
 * gamerunner_control.c : a simple app to control games in 
 * Openmoko Freerunner
 *
 *  Copyright (c) 2009 Rafael Ignacio Zurita <rizurita@yahoo.com>
 *
 *  This program can be distributed under the terms of the GNU GPL.
 *  See the file COPYING.
 *
 * Nada estable todavia. Pero las pruebas muestran que se puede
 * controlar cualquier programa sobre X, usando los acelerometros y 
 * touchscreen.
 * 
 * Este programa es ilegible. Utiliza el acelerometro y touchscreen
 * del telefono Freerunner, y envia eventos de teclado y mouse
 * a X, usando threads.
 */

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

#ifdef USE_THREADS
#include <pthread.h>
#include <sched.h>


pthread_t thread;
pthread_mutex_t mutex;
#endif

#include <sys/soundcard.h>
#include <sys/mman.h>

/* for openmoko */
#include "accelneo.h"
#include "xdotool.c"

int accelerometer_tare = 1;

int accelerometer_zHome = 198;      /* Stores new "Home" position */
int accelerometer_yHome = 54;      /* Stores new "Home" position */

int accelerometer_zwindow=80; // DeadZone in Pixels
int accelerometer_ywindow=140; // DeadZone in Pixels

int accelerometer_zscale=7; // Scaling factor for motion (acceleration)
int accelerometer_yscale=100; // Scaling factor for motion (acceleration)

/* solo util si utilizamos mouse como eventos */
//accelerometer_ywindow=80;
//accelerometer_zwindow=85;



int mode = 0; repeticiones = 0;
char *keys[24];
char *kc[11];



typedef enum
{
	ev_keydown,
	ev_keyup,
	ev_mouse,
	ev_joystick,
	ev_accelerometer,  //Openmoko
} evtype_t;

typedef struct
{
	evtype_t  type;
	int       data1;    // keys / mouse/joystick buttons
	int       data2;    // mouse/joystick x move
	int       data3;    // mouse/joystick y move
} event_t;

int doneYet1=0;
int eventos=0;

SDL_mutex *event_mutex;

int mousex, mousey;
int file_event;


void I_InitAccelerometer(void)
{
	sleep(1);
} 


void I_CloseAccelerometer(void)
{
}


void* vibration(void *arg) {

	system("echo 150 > /sys/class/leds/neo1973:vibrator/brightness; echo 0 > /sys/class/leds/neo1973:vibrator/brightness; ");
	return NULL;
} 

void vibrate() {
	pthread_attr_t tattr;
	pthread_attr_init(&tattr);
	pthread_attr_setdetachstate(&tattr,PTHREAD_CREATE_DETACHED);
	pthread_t p;
	int err = pthread_create(&p, &tattr, vibration, NULL);
}



int get_position(event_t *ev)
{

	char *m = "mouse";
	char *n = "null";
      
	if (accelerometer_tare) {    // Make current position the "Home" position
		accelerometer_tare = 0;             
	}
      
	if ((!strcmp(kc[0],n)) && (!strcmp(kc[2],n))) {
		sleep(5);
		return 0;
	};


	if (!strcmp(kc[0],m)) {

		mousex=0; mousey=0;

		if (ev->data2 >  (accelerometer_yHome + accelerometer_ywindow)) 
			mousex = abs((accelerometer_yscale / 10.0) * ( ev->data2 - (accelerometer_yHome + accelerometer_ywindow) )); // Right            
		else if (ev->data2 <  (accelerometer_yHome - accelerometer_ywindow)) 
			mousex = -abs((accelerometer_yscale / 10.0) * ( ev->data2 - (accelerometer_yHome - accelerometer_ywindow) ));  //Left

		if (ev->data3 >  (accelerometer_zHome + accelerometer_zwindow)) {
			mousey = -abs((accelerometer_zscale / 10.0)* ( ev->data3 - (accelerometer_zHome - accelerometer_zwindow) ));   // Forward
		}
		else if (ev->data3 <  (accelerometer_zHome - accelerometer_zwindow)) {
			mousey = abs((accelerometer_zscale / 10.0) * ( ev->data3 - (accelerometer_zHome - accelerometer_zwindow) ));  // Backward
		}

		xdo_mousemove_relative(xdo, mousex, mousey); 

	} else {
      
		if (ev->data3 > (accelerometer_zHome + accelerometer_zwindow)) {
			if (ev->data2 > (accelerometer_yHome )) return 2;
			else if (ev->data2 < (accelerometer_yHome - accelerometer_ywindow)) return 8;
			else return 1;
		} else if (ev->data3 < (accelerometer_zHome )) {
			if (ev->data2 > (accelerometer_yHome )) return 4;
			else if (ev->data2 <  (accelerometer_yHome - accelerometer_ywindow)) return 6;
			else return 5;
		} else if (ev->data2 > (accelerometer_yHome)) return 3;
		else if (ev->data2 < (accelerometer_yHome - accelerometer_ywindow)) return 7;
	}

	return 0;    // eat events

}



int get_direction_controler(void)
{
	event_t ev;
	const unsigned int report_len = 16;
	unsigned char report[report_len];
	struct accel_3d_t accel;
	accel.val[0] = 0.0;
	accel.val[1] = 0.0;
	accel.val[2] = 0.0;
  
	unsigned short int rel = 1;
  
	file_event = open("/dev/input/event3", O_RDONLY);
	
	while (rel!=0) {
		int read_len = read(file_event, report, report_len);

		/* this was a blocking read */
		if (read_len < 0) {       
			printf("error de lectura\n");

		} else {
			rel = *(unsigned short int *)(report + 8);

			/*
			 * Neo sends three reports on X, Y, and Z with rel = 2
			 * and another one (as a separator) with rel = 0 
			 */
			if (rel == 2) {
				unsigned short int axis_ind = *(short int *)(report + 10);
				int val_mg = *(int *)(report + 12);
				accel.val[axis_ind] = val_mg;
			}
		} 
	}  
	close(file_event);

	ev.type = ev_accelerometer;
	ev.data1 = accel.val[0];
	ev.data2 = accel.val[1];
	ev.data3 = accel.val[2];
      
	return get_position(&ev);
}





#define KEYCODE XK_Escape 



int doneYet2=0;
int events2=0;

int doneYet3=0;
int events3=0;

SDL_mutex *event_mutex2;


int om_direction = 0;  // COLOCARLOS DENTRO DEL WHILE !!!!

int lastk=0;

static int touch(void *nothing)
{
	int om_accel = 0;  // COLOCARLOS DENTRO DEL WHILE !!!!

	struct input_event  val, val2;

	Window window = 0;

	static int event0_fd = -1;
	static int event1_fd = -1;
	struct input_event ev0[64];
	struct input_event ev1[64];

	int button=0, x=0, y=0, realx=0, realy=0, i, rd;
        int x1,y1;

	int tecla=0;

	int j=0;
	while (!doneYet1) {

		button=0; x=0; y=0; realx=0; realy=0;

		event0_fd = open("/dev/input/event1", O_RDWR | O_NONBLOCK );
		rd = read(event0_fd, ev0, sizeof(struct input_event) * 64);
		if (rd>0) {	
		  
		  for (i = 0; i < (rd / sizeof(struct input_event) ); i++) {

			if(ev0[i].type == 3 && ev0[i].code == 0) x = ev0[i].value;
			else if (ev0[i].type == 3 && ev0[i].code == 1) y = ev0[i].value;
			else if (ev0[i].type == 1 && ev0[i].code == 330) button = ev0[i].value << 2;
			else if (ev0[i].type == 0 && ev0[i].code == 0 && ev0[i].value == 0) {
	
				x1 = ((x-110) * 640) / (922-110);                     
                                y1 = ((y-105) * 480) / (928-105);


        			if ((x1>0) && (y1>0)) {

					tecla=0;
					x=x1; y=y1;
					if ((x>0) && (x<210)) {
						if ((y>0) && (y<160)) {
							tecla=4;
						} else if ((y>0) && (y<320)) {
							tecla=6;
						} else if ((y>0) && (y<480)) {
							tecla=8;
						};

					} else if ((x>0) && (x<420)) {
				
						if ((y>0) && (y<160)) {
							/* antes fue el boton de centrado */
							tecla=0;
						} else if ((y>0) && (y<320)) {
							accelerometer_tare = 1;
						} else if ((y>0) && (y<480)) {
							tecla=9;
						};
					} else if ((x>0) && (x<640)) {
						if ((y>0) && (y<160)) {
							tecla=5;
						} else if ((y>0) && (y<320)) {
							tecla=7;
						} else if ((y>0) && (y<480)) {
							tecla=10;
						};
					};


				} else {
					/* nice, nothing to do */
				}

				if (tecla!=0) {
					if (lastk!=tecla) {
						if ( lastk != 0)
							xdo_keysequence_up(xdo, window, kc[lastk]);
					
							xdo_keysequence_down(xdo, window, kc[tecla]);
						i=(rd / sizeof(struct input_event) ) + 1;
						lastk=tecla;
					};
				};

			}
		}
		} else { /* if rf */ 
			if ( lastk != 0)
				xdo_keysequence_up(xdo, window, kc[lastk]);
			lastk=0;
		} 
	
  		close(event0_fd);
		SDL_Delay(20);
	}

	return 0;
}


int skip = 0;
static int accel(void *nothing)
{
	om_direction = 0;  // COLOCARLOS DENTRO DEL WHILE !!!!

	char *m = "mouse";
	char *n = "null";

	Window window = 0;
	int last = 1000;
	while (!doneYet2) {
		om_direction = get_direction_controler();

	if ((strcmp(&kc[0],m)) && (om_direction != last) && (strcmp(&kc[0],n))) {
			if (om_direction==1) {
				xdo_keysequence_up(xdo, window, kc[0]);
				xdo_keysequence_up(xdo, window, kc[1]);
				xdo_keysequence_up(xdo, window, kc[3]);
				xdo_keysequence_down(xdo, window, kc[2]);
			} else if (om_direction==2) {
				xdo_keysequence_up(xdo, window, kc[0]);
				xdo_keysequence_up(xdo, window, kc[3]);
				xdo_keysequence_down(xdo, window, kc[1]);
				xdo_keysequence_down(xdo, window, kc[2]);
			} else if (om_direction==3) {
				xdo_keysequence_up(xdo, window, kc[0]);
				xdo_keysequence_up(xdo, window, kc[2]);
				xdo_keysequence_up(xdo, window, kc[3]);
				xdo_keysequence_down(xdo, window, kc[1]);
			} else if (om_direction==4)  {
				xdo_keysequence_up(xdo, window, kc[0]);
				xdo_keysequence_up(xdo, window, kc[2]);
				xdo_keysequence_down(xdo, window, kc[1]);
				xdo_keysequence_down(xdo, window, kc[3]);
			} else if (om_direction==5) { 
				xdo_keysequence_up(xdo, window, kc[0]);
				xdo_keysequence_up(xdo, window, kc[1]);
				xdo_keysequence_up(xdo, window, kc[2]);
				xdo_keysequence_down(xdo, window, kc[3]);
			} else if (om_direction==6) {
				xdo_keysequence_up(xdo, window, kc[1]);
				xdo_keysequence_up(xdo, window, kc[2]);
				xdo_keysequence_down(xdo, window, kc[0]);
				xdo_keysequence_down(xdo, window, kc[3]);
			} else if (om_direction==7) { 
				xdo_keysequence_up(xdo, window, kc[1]);
				xdo_keysequence_up(xdo, window, kc[2]);
				xdo_keysequence_up(xdo, window, kc[3]);
				xdo_keysequence_down(xdo, window, kc[0]);
			} else if (om_direction==8) {
				xdo_keysequence_up(xdo, window, kc[1]);
				xdo_keysequence_up(xdo, window, kc[3]);
				xdo_keysequence_down(xdo, window, kc[0]);
				xdo_keysequence_down(xdo, window, kc[2]);
			} else {
				xdo_keysequence_up(xdo, window, kc[0]);
				xdo_keysequence_up(xdo, window, kc[1]);
				xdo_keysequence_up(xdo, window, kc[2]);
				xdo_keysequence_up(xdo, window, kc[3]);
			}
		last=om_direction;
	}

		SDL_Delay(20);
	}

	return 0;
}

aux() {

	static int event0_fd = -1;
	struct input_event ev[64];

	event0_fd = open("/dev/input/event0", O_RDWR);
	int rd, i, cant=0;

	while (!doneYet3) {

		rd = read(event0_fd, ev, sizeof(struct input_event) * 64);

		for (i = 0; i < (rd / sizeof(struct input_event) ); i++) {

			if (EV_KEY == ev[i].type) {
				cant++;
				if (cant > 1) {
					cant = 0;
					vibrate();
					if (mode == 1) {
	system("echo 0 > /sys/devices/platform/gta02-led.0/leds/gta02-aux\:red/brightness ");
						mode = 0;
						repeticiones = keys[22];
						for (i=0; i<11; i++)
							kc[i] = keys[i];
					} else {
	system("echo 1 > /sys/devices/platform/gta02-led.0/leds/gta02-aux\:red/brightness ");
						mode = 1;
						repeticiones = keys[23];
						for (i=0; i<11; i++)
							kc[i] = keys[11+i];
					};
				}
			}

		}
		SDL_Delay(300);
	}
	close(event0_fd);
}

SDL_Thread *touch_tr = NULL;
SDL_Thread *accel_tr = NULL;
SDL_Thread *aux_tr = NULL;

void start_control (void)
{
	/* for openmoko */
	I_InitAccelerometer();

	accel_tr = SDL_CreateThread(accel, NULL);
	touch_tr = SDL_CreateThread(touch, NULL);
	aux_tr = SDL_CreateThread(aux, NULL);
	/* end for openmoko */
}



void stop_control (void)
{
	/* for openmoko */
	I_CloseAccelerometer();

	doneYet1 = 1;
	SDL_WaitThread(accel_tr, NULL); // wait for it to die

	doneYet2 = 1;
	SDL_WaitThread(touch_tr, NULL); // wait for it to die

	doneYet3 = 1;
	SDL_WaitThread(aux_tr, NULL); // wait for it to die
	/* end for openmoko */
}

main(int argc, char *argv[])
{
	accelerometer_zHome = atoi(argv[1]);      /* Stores new "Home" position */
	accelerometer_yHome = atoi(argv[2]);      /* Stores new "Home" position */

	accelerometer_zwindow = atoi(argv[3]); // DeadZone in Pixels
	accelerometer_ywindow = atoi(argv[4]); // DeadZone in Pixels

	accelerometer_zscale = atoi(argv[5]); // Scaling factor for motion (acceleration)
	accelerometer_yscale = atoi(argv[6]); // Scaling factor for motion (acceleration)
  

	int i;
	for (i=0; i<22; i++)
		keys[i] = argv[7+i];

	mode = 0;
	keys[22] = atoi(argv[7+22]);
	keys[23] = atoi(argv[7+23]);
	repeticiones = keys[22];
	for (i=0; i<11; i++)
		kc[i] = keys[i];

	xdo = xdo_new(getenv("DISPLAY"));
	if (xdo == NULL) {
		fprintf(stderr, "Failed creating new xdo instance\n");
		return 1;
	}

	start_control();
	for (;;){
		sleep(10000);
	};
	stop_control();

	SDL_Quit();
}

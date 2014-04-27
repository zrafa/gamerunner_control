/*
 * xdo library header
 * - include this if you have code that uses xdo
 *
 * $Id: xdo.h 2254 2009-08-15 23:16:12Z jordansissel $
 */

#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif /* __USE_XOPEN */

#include <sys/types.h>
#include <X11/Xlib.h>

#define SEARCH_VISIBLEONLY (1L << 0)
#define SEARCH_TITLE (1L << 1)
#define SEARCH_CLASS (1L << 2)
#define SEARCH_NAME (1L << 3)

#define SEARCH_IGNORE_TRANSIENTS (1L << 4)
#define SEARCH_IGNORE_WINDOW_INPUTONLY (1L << 5)

#define SIZE_USEHINTS (1L << 0)

/* Map keysym name to actual ascii */
typedef struct keysymcharmap {
  const char *keysym;
  char key;
} keysymcharmap_t;

/* map character to keycode */
typedef struct charcodemap {
  char key;
  KeyCode code;
  int shift;
  int modmask;
} charcodemap_t;

typedef struct xdo {
  Display *xdpy;
  char *display_name;
  charcodemap_t *charcodes;
  XModifierKeymap *modmap;
  int keycode_high; /* highest and lowest keycodes */
  int keycode_low;  /* used by this X server */

  int close_display_when_freed;
} xdo_t;

xdo_t* xdo_new(char *display);
xdo_t* xdo_new_with_opened_display(Display *xdpy, const char *display,
                                   int close_display_when_freed);
void xdo_free(xdo_t *xdo);

int xdo_mousemove(xdo_t *xdo, int x, int y);
int xdo_mousemove_relative(xdo_t *xdo, int x, int y);
int xdo_mousedown(xdo_t *xdo, int button);
int xdo_mouseup(xdo_t *xdo, int button);
int xdo_mouselocation(xdo_t *xdo, int *x, int *y, int *screen_num);

int xdo_click(xdo_t *xdo, int button);

int xdo_type(xdo_t *xdo, Window window, char *string, int delay);
int xdo_keysequence(xdo_t *xdo, Window window, char *keysequence);
int xdo_keysequence_up(xdo_t *xdo, Window window, char *keysequence);
int xdo_keysequence_down(xdo_t *xdo, Window window, char *keysequence);
int xdo_keysequence_list_do(xdo_t *xdo, Window window, charcodemap_t *keys,
                             int nkeys, int pressed, int *modifier);
int xdo_active_modifiers_to_keycode_list(xdo_t *xdo, charcodemap_t **keys,
                                         int *nkeys);

int xdo_window_move(xdo_t *xdo, Window wid, int x, int y);
int xdo_window_setsize(xdo_t *xdo, Window wid, int w, int h, int flags);
int xdo_window_setprop (xdo_t *xdo, Window wid, const char *property, const char *value);
int xdo_window_setclass(xdo_t *xdo, Window wid, const char *name, const char *class);
int xdo_window_focus(xdo_t *xdo, Window wid);
int xdo_window_raise(xdo_t *xdo, Window wid);
int xdo_window_get_focus(xdo_t *xdo, Window *window_ret);
int xdo_window_sane_get_focus(xdo_t *xdo, Window *window_ret);
int xdo_window_activate(xdo_t *xdo, Window wid);

int xdo_window_map(xdo_t *xdo, Window wid);
int xdo_window_unmap(xdo_t *xdo, Window wid);

/* pager-like behaviors */
int xdo_window_get_active(xdo_t *xdo, Window *window_ret);
int xdo_set_number_of_desktops(xdo_t *xdo, long ndesktops);
int xdo_get_number_of_desktops(xdo_t *xdo, long *ndesktops);
int xdo_set_current_desktop(xdo_t *xdo, long desktop);
int xdo_get_current_desktop(xdo_t *xdo, long *desktop);
int xdo_set_desktop_for_window(xdo_t *xdo, Window wid, long desktop);
int xdo_get_desktop_for_window(xdo_t *xdo, Window wid, long *desktop);

/* Returns: windowlist and nwindows */
int xdo_window_list_by_regex(xdo_t *xdo, char *regex, int flags, int max_depth,
                             Window **windowlist, int *nwindows);

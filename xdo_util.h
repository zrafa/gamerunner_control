/* xdo utility pieces 
 *
 * $Id: xdo_util.h 1854 2008-05-21 07:23:29Z jordansissel $
 */

/* human to Keysym string mapping */
const char *symbol_map[] = {
  "alt", "Alt_L",
  "ctrl", "Control_L",
  "control", "Control_L",
  "meta", "Meta_L",
  "super", "Super_L",
  "shift", "Shift_L",
  NULL, NULL,
};

keysymcharmap_t keysymcharmap[] = {
  { "Return", '\n', },
  { "ampersand", '&', },
  { "apostrophe", '\'', },
  { "asciicircum", '^', },
  { "asciitilde", '~', },
  { "asterisk", '*', },
  { "at", '@', },
  { "backslash", '\\', },
  { "bar", '|', },
  { "braceleft", '{', },
  { "braceright", '}', },
  { "bracketleft", '[', },
  { "bracketright", ']', },
  { "colon", ':', },
  { "comma", ',', },
  { "dollar", '$', },
  { "equal", '=', },
  { "exclam", '!', },
  { "grave", '`', },
  { "greater", '>', },
  { "less", '<', },
  { "minus", '-', },
  { "numbersign", '#', },
  { "parenleft", '(', },
  { "parenright", ')', },
  { "percent", '%', },
  { "period", '.', },
  { "plus", '+', },
  { "question", '?', },
  { "quotedbl", '"', },
  { "semicolon", ';', },
  { "slash", '/', },
  { "space", ' ', },
  { "tab", '\t', },
  { "underscore", '_', },
  { NULL, 0, },
};


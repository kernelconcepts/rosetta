 /*
 rosetta.h - global definitions of rosetta handwriting recognition
 (c) 2003 by Ole Reinhardt <ole.reinhardt@kernelconcepts.de>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Library General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*/

#ifndef _rosetta_h_
#define _rosetta_h_

#include <X11/Xlib.h>

#define PROGRAM_VERSION           "0.1"

#ifndef TRUE
  #define TRUE  1
  #define FALSE 0
#endif

#define MAX_POINTS_PER_PIXEL_LINE 25000
#define MAX_POINTS_PER_LINE       2500
#define MAX_LINES                 50
#define MAX_DOTS                  3
#define MAX_WORDS                 350000
#define MAX_WORDSIZE              30
#define MAX_STROKES_PER_CHARACTER 4
#define MAX_SYMBOL_SIZE           15

#define NORM_WIDTH                100
#define NORM_HEIGHT               NORM_WIDTH
#define NORM_ERROR                NORM_WIDTH

#define WRITE_TIMEOUT       300	// microseconds

#define TEST_POINT_COUNT    40//20
#define PRECALC_STEP_SIZE    3

#define WIDTH              240		
#define HEIGHT             85

//#define HEIGHT             180
//#define WIDTH              (HEIGHT * 3)


#define MIN_CHAR_WIDTH     30
#define MIN_CHAR_HEIGHT    30
#define MIN_WORD_HEIGHT    90

#define MODE_RECOGNITION   0x00
#define MODE_LEARNING      0x01
#define MODE_LEARN_SINGLE  0x00
#define MODE_LEARN_CONT    0x01

#define MAX_CONFIG_KEYS    150	
#define MAX_REPEAT         3

#define WORD_X_DISTANCE_FACT 4
#define WORD_Y_DISTANCE_FACT 4

#define OVERLAP_FACT       0.4
#define MAX_DOT_SIZE       20

#define INVALID            0x7FFFFFFF

#define AR_FACT            1.5
#define AR_VERYLONG        3

#define PRECALC_ANGLE_SIZE_X 300
#define PRECALC_ANGLE_SIZE_Y 200
#define PRECALC_ANGLE_OFFS_X (PRECALC_ANGLE_SIZE_X / 2)
#define PRECALC_ANGLE_OFFS_Y (PRECALC_ANGLE_SIZE_Y / 2)

#define KS_INVALID         0x00
#define KS_STR_SPACE       "space"

#define LEARN_SET_GROUP_DB      0x00
#define LEARN_SET_GROUP_CONFIG  0x01

typedef enum {
	ar_heigh      = 0,
	ar_width,
	ar_quadrat,
	ar_veryheigh,
	ar_verywidth
} t_ar;	

typedef enum{
	grp_lowercase = 0,
	grp_uppercase,
	grp_number,
	grp_special,
	grp_function,
	grp_none
} t_keygrp;

typedef struct {	
	short     x;
	short     y;
} __attribute__ ((packed)) t_point;

typedef struct {
	t_point points[MAX_POINTS_PER_PIXEL_LINE];
	short   pcount;
} __attribute__ ((packed)) t_pixelline;

typedef struct {
	t_pixelline pixels;
	
	t_point points[MAX_POINTS_PER_LINE];
	short   pcount;

	short   direction[TEST_POINT_COUNT];
	
	short   xmin;
	short   ymin;
	short   xmax;
	short   ymax;
	short   width;
	short   height;
	
	int     dotcount;
	short   multi;
	short   reference;
	short   newword;
} __attribute__ ((packed)) t_line;

typedef struct {
	t_line  lines[MAX_LINES];
	short   lcount;
	char    words[10][MAX_WORDSIZE];
	short   wcount;
	
	short   xmin;
	short   ymin;
	short   xmax;
	short   ymax;
	short   timeout;
} __attribute__ ((packed)) t_data_struct;	

typedef struct {
	char         text  [3][MAX_SYMBOL_SIZE];
	char         symbol[3][MAX_SYMBOL_SIZE];
	t_keygrp     group;
	short        repeat;
} __attribute__ ((packed)) t_key;

typedef struct {
	KeySym  symbol;
	short   direction[TEST_POINT_COUNT];
	short   xpos[TEST_POINT_COUNT];
	short   ypos[TEST_POINT_COUNT];
} __attribute__ ((packed)) t_chardef;

typedef struct {
	KeySym  symbol;
	int     error;
} __attribute__ ((packed)) t_error;


typedef struct {
	t_point  points[TEST_POINT_COUNT];
	short    direction[TEST_POINT_COUNT];
} __attribute__ ((packed)) t_stroke;

typedef struct {
	KeySym   symbol;
	short    strokecount;
	short    dotcount;
	short    width;
	short    height;
	t_keygrp group;
	t_ar     aspectratio;
	t_stroke strokes[MAX_STROKES_PER_CHARACTER];	
} __attribute__ ((packed)) t_character;

typedef struct {
	short       charcount;
	t_character chars[MAX_LINES];
	short       word_seperator[MAX_LINES];
} __attribute__ ((packed)) t_charlist;

typedef struct {
	short       charcount;
	t_character chars[MAX_CONFIG_KEYS * MAX_REPEAT];
} __attribute__ ((packed)) t_database;

typedef struct {
	KeySym	    symbol;
	int         weight;
} __attribute__ ((packed)) t_weight;

typedef struct {
	t_key       keys[MAX_CONFIG_KEYS];
	short       keycount;
} __attribute__ ((packed)) t_config;

extern t_data_struct input_data;
extern t_charlist    analized_data;
extern t_config      configuration;
extern int     mode;
extern int     learn_mode;
extern int     keycount;
extern int     flag_use_precalc;
extern int     flag_use_dict;
extern int     window_width;
extern int     learn_set_group_mode;
extern char    dict_file_name[100];
extern char    **dictionary;

// declarations for helper.c

void draw_line(int xP, int yP, int xQ, int yQ, t_pixelline *pixels);

// declarations for database.c

int  load_stroke_database(char *filename, t_database *database);
int  save_stroke_database(char *filename, t_database *database);
void generate_emty_stroke_database_from_config(t_database *database);
void add_to_stroke_database(t_character *character, t_database *database);
void set_stroke_database(t_character *character, t_database *database, int charnr);


// declarations for precalc.c

void precalc(void);
int  get_angle(int x1, int y1, int x2, int y2);
int  save_precalc(char *filename);
int  load_precalc(char *filename);

// declarations for matching.c

void init_matching(void);
void match(t_database *database, t_charlist *actual);

// declarations for normalize.c

void normalize_stroke_data(t_data_struct *data);
void convert_strokes_to_normalized_characters(t_data_struct *input_data, t_charlist *analized_data);

// declarations for analize.c

void analize_strokes(t_data_struct *input_data, t_charlist *analized_data);

// declarations for dictionary.c

int  load_dictionary(char *filename);
void load_doted_char_list(char *filename);
int  generate_symbol_list(t_charlist *input, KeySym *output);

// declarations for configfile.c

int  load_configfile(char *filename);

#endif

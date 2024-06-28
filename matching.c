/*
 matching.c - matching filter to charaterize the input
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
       
#include "rosetta.h"

#define  CHAR    0
#define  WEIGHT  1

#define min(a, b) (a<b?a:b) 

int    error_lookup[TEST_POINT_COUNT][TEST_POINT_COUNT];

inline int error_function(int error)
{
	return error*error;
}

int get_position_error(t_stroke *saved, t_stroke *actual, int pa, int ps)
{
	int error = 0;
	error += error_function(saved->points[ps].x-actual->points[pa].x)+
		 error_function(saved->points[ps].y-actual->points[pa].y);

	return error;
}

int get_x_error(t_stroke *saved, t_stroke *actual, int pa, int ps)
{
	return error_function(saved->points[ps].x-actual->points[pa].x);

}

int get_y_error(t_stroke *saved, t_stroke *actual, int pa, int ps)
{
	return error_function(saved->points[ps].y-actual->points[pa].y);
}

int get_direction_error(t_stroke *saved, t_stroke *actual, int pa, int ps)
{
	int    dir = abs(abs(actual->direction[pa])-abs(saved->direction[ps]));
	return error_function(min(dir, NORM_ERROR - dir));
}

int get_error_rec(int (*error)(t_stroke *, t_stroke *, int, int), t_stroke *saved, t_stroke *actual, int pa, int ps)
{
	if (error_lookup[pa][ps] != -1)
		return(error_lookup[pa][ps]);
	if (ps != 1)
		return error_lookup[pa][ps] = 
		       error(saved, actual, pa, ps) +
		       min (min (get_error_rec(error, saved, actual, pa-1, ps), get_error_rec(error, saved, actual, pa-1, ps-1)),
		                 get_error_rec(error, saved, actual, pa-1, ps-2));	
	else
		return error_lookup[pa][ps] =
	               error(saved, actual, pa, ps) + 
	               min (get_error_rec(error, saved, actual, pa-1, 1), get_error_rec(error, saved, actual, pa-1, 0));
}

int get_error(int (*error)(t_stroke *, t_stroke *, int, int), t_stroke *saved, t_stroke *actual, int dynamic)
{	
	register int x, y;
	register int error_sum;
	
	if (dynamic)
	{
		for (x = 1; x < TEST_POINT_COUNT; x++)
			for (y = 1; y < TEST_POINT_COUNT; y++)
				error_lookup[x][y]= -1;
		
		error_lookup[0][0] = error(saved, actual, 0, 0);	
		
		for (x = 1; x < TEST_POINT_COUNT; x++)
		{
			error_lookup[0][x] = error(saved, actual, 0, x) + error_lookup[0][x-1];
			error_lookup[x][0] = error(saved, actual, x, 0) + error_lookup[x-1][0];
		}
		
		return get_error_rec(error, saved, actual, TEST_POINT_COUNT-1, TEST_POINT_COUNT-1);
	} else
	{
		error_sum = 0;
		for (x = 0; x < TEST_POINT_COUNT; x++)
			error_sum += error(saved, actual, x, x);
		return error_sum;		
	}
}

int get_precalc_error(int (*error)(t_stroke *, t_stroke *, int, int), t_stroke *saved, t_stroke *actual)
{	
	register int x;
	register int error_sum;
	
	error_sum = 0;
	for (x = 0; x < TEST_POINT_COUNT; x+=PRECALC_STEP_SIZE)
		error_sum += error(saved, actual, x, x);
	return error_sum;		
}

int  error_compare(const void *e1, const void *e2)
{
	return ((t_error *)e1)->error - ((t_error *)e2)->error;
}

int  weight_compare(const void *w1, const void *w2)
{
	return ((t_weight *)w2)->weight - ((t_weight *)w1)->weight;
}


void  weight(t_error *error, t_weight *weighting, int *weightcount)
{
	int count;
	int wcount;
	int found;
	
	for (count = 0; count < 5; count++)
	{
		found = FALSE;
		for (wcount = 0; wcount < *weightcount; wcount++)
			if (weighting[wcount].symbol == error[count].symbol)
			{
				weighting[wcount].weight += 5-count;
				found = TRUE;
				break;
			}
		if (!found)
		{
			weighting[*weightcount].symbol = error[count].symbol;
			weighting[*weightcount].weight = 5-count;
			*weightcount=*weightcount+1;
		}
	}
}

inline int symbol_in_list(KeySym *list, KeySym testsym, int size)
{
	register int count = 0;
	while (count != size)
	{
		if (testsym == list[count]) return TRUE;
		count++;
	}
	return FALSE;
}

inline int test_chargroup(t_character *actual, t_character *database, int charcount)
{
	if (charcount > 1)
	{
		if ((database->group == grp_lowercase) || (database->group == grp_number)) return TRUE; 
		else return FALSE;
	}
	if (actual->group == grp_lowercase)
	{
		if (database->group==grp_lowercase) return TRUE;
		if (database->group==grp_number) return TRUE;
	} else
	if (actual->group == grp_uppercase)
	{
		if (database->group==grp_uppercase) return TRUE;
		if (database->group==grp_number) return TRUE;
	} else
	if (actual->group == grp_function)
	{
		if (database->group==grp_function) return TRUE;
		if (database->group==grp_special) return TRUE;
	}
	return FALSE;
}

void match(t_database *database, t_charlist *actual)
{
	t_error  error_pos[MAX_CONFIG_KEYS * MAX_REPEAT];
#ifndef NEW_MATCH_CODE
	t_error  error_x  [MAX_CONFIG_KEYS * MAX_REPEAT];
	t_error  error_y  [MAX_CONFIG_KEYS * MAX_REPEAT];
	t_error  error_dir[MAX_CONFIG_KEYS * MAX_REPEAT];
#endif

	int      count;
	int      ccount;
	int      scount;
	t_weight weighting[4*5];
	int      weightcount;
	KeySym   symbols[10];
	int      charcount = 0;
#ifndef NEW_MATCH_CODE
	int      ep;
	int      ed;
#endif
		
#ifdef NEW_MATCH_CODE
	
	for (ccount = 0; ccount < actual->charcount; ccount++)
	{
	
		memset(error_pos, 0, sizeof(error_pos));
		memset(weighting, 0, sizeof(weighting));
		memset(symbols,   0, sizeof(symbols));
		
		for (count = 0; count < database->charcount; count++)
		{
			if ((database->chars[count].strokecount != actual->chars[ccount].strokecount) ||
			    (test_chargroup(&actual->chars[ccount], &database->chars[count], actual->charcount)==FALSE))
			{
				error_pos[count].error  = INVALID;
				error_pos[count].symbol = KS_INVALID;
			} else
			{
				error_pos[count].symbol = database->chars[count].symbol;
				error_pos[count].error = 0;
				
				for (scount = 0; scount < database->chars[count].strokecount; scount++)
				{
					error_pos[count].error += (get_precalc_error(get_position_error,  &database->chars[count].strokes[scount], &actual->chars[ccount].strokes[scount]) *
					                           get_precalc_error(get_direction_error, &database->chars[count].strokes[scount], &actual->chars[ccount].strokes[scount]))/100;
				}	
			}
		}
		
		qsort(error_pos,  database->charcount, sizeof(t_error), error_compare);
			
		weightcount = 0;
	
		weight(error_pos, weighting, &weightcount);	

		qsort(weighting,  weightcount, sizeof(t_weight), weight_compare);	
		
		charcount  = weightcount;
		for (count = 0; count < weightcount; count++)
		{
			symbols[count] = weighting[count].symbol;
	#ifdef DEBUG
			printf("Precalc Weight Keysym %10s -> %d\n", XKeysymToString(weighting[count].symbol), weighting[count].weight);
	#endif
		}
	#ifdef DEBUG
		printf("------------------\n");
	#endif
/*	}	
	
	for (ccount = 0; ccount < actual->charcount; ccount++)
	{
	
*/		memset(error_pos, 0, sizeof(error_pos));
		memset(weighting, 0, sizeof(weighting));
		
		for (count = 0; count < database->charcount; count++)
		{
			if (database->chars[count].strokecount != actual->chars[ccount].strokecount)
			{
				error_pos[count].error = INVALID;
				error_pos[count].symbol = KS_INVALID;
			} else
			if (symbol_in_list(symbols, database->chars[count].symbol, charcount) == FALSE)
			{
				error_pos[count].error = INVALID;
				error_pos[count].symbol = KS_INVALID;
			} else
			{
				error_pos[count].symbol = database->chars[count].symbol;
				error_pos[count].error = 0;
				
				for (scount = 0; scount < database->chars[count].strokecount; scount++)
				{
					error_pos[count].error += (get_error(get_position_error,  &database->chars[count].strokes[scount], &actual->chars[ccount].strokes[scount], TRUE));// *
					                           //get_error(get_direction_error, &database->chars[count].strokes[scount], &actual->chars[ccount].strokes[scount], TRUE)) / 100;
				}				
			}
		}
		
		qsort(error_pos,  database->charcount, sizeof(t_error), error_compare);
	
	#ifdef DEBUG
		for (count = 0; count < 5; count++)
		{
		#ifdef NEW_MATCH_CODE
			printf("Weight %d: p: %5s: %7d\n", count,
				XKeysymToString(error_pos[count].symbol), error_pos[count].error);
		#else
			printf("Weight %d: p: %5s: %7d,  x: %5s: %7d,  y: %5s: %7d,  d: %5s:%7d\n", count,
			       XKeysymToString(error_pos[count].symbol), error_pos[count].error,
			       XKeysymToString(error_x  [count].symbol), error_x  [count].error,
			       XKeysymToString(error_y  [count].symbol), error_y  [count].error,
			       XKeysymToString(error_dir[count].symbol), error_dir[count].error);
		#endif
		}
	#endif

		weightcount = 0;
	
		weight(error_pos, weighting, &weightcount);	
	
	
#else
	for (ccount = 0; ccount < actual->charcount; ccount++)
	{
	
		memset(error_pos, 0, sizeof(error_pos));
		memset(error_dir, 0, sizeof(error_dir));
		memset(weighting, 0, sizeof(weighting));
		memset(symbols,   0, sizeof(symbols));		
		
		for (count = 0; count < database->charcount; count++)
		{
			if (database->chars[count].strokecount != actual->chars[ccount].strokecount)
			{
				error_pos[count].error = INVALID;
				error_dir[count].error = INVALID;

				error_pos[count].symbol = KS_INVALID;
				error_dir[count].symbol = KS_INVALID;
			} else
			{
				error_pos[count].symbol = database->chars[count].symbol;
				error_dir[count].symbol = database->chars[count].symbol;
				
				error_pos[count].error = 0;
				error_dir[count].error = 0;
				
				for (scount = 0; scount < database->chars[count].strokecount; scount++)
				{
					error_pos[count].error += get_precalc_error(get_position_error,  &database->chars[count].strokes[scount], &actual->chars[ccount].strokes[scount]);
					error_dir[count].error += get_precalc_error(get_direction_error, &database->chars[count].strokes[scount], &actual->chars[ccount].strokes[scount]);
				}	
			}
		}
		
		qsort(error_pos,  database->charcount, sizeof(t_error), error_compare);
		qsort(error_dir,  database->charcount, sizeof(t_error), error_compare);	
			
		weightcount = 0;
	
		weight(error_pos, weighting, &weightcount);	
		weight(error_dir, weighting, &weightcount);

		qsort(weighting,  weightcount, sizeof(t_weight), weight_compare);	
		
		charcount  = weightcount;
		for (count = 0; count < weightcount; count++)
		{
			symbols[count] = weighting[count].symbol;
	#ifdef DEBUG
			printf("Precalc Weight %c -> %d\n", weighting[count].character, weighting[count].weight);
	#endif
		}
	#ifdef DEBUG
		printf("------------------\n");
	#endif
	
		memset(error_pos, 0, sizeof(error_pos));
		memset(error_x,   0, sizeof(error_x  ));
		memset(error_y,   0, sizeof(error_y  ));	
		memset(error_dir, 0, sizeof(error_dir));
		memset(weighting, 0, sizeof(weighting));
		
		for (count = 0; count < database->charcount; count++)
		{
			if ((database->chars[count].strokecount != actual->chars[ccount].strokecount) &&
			    (symbol_in_list(symbols, database->chars[count].symbol, charcount)))

			{
				error_pos[count].error = INVALID;
				error_x  [count].error = INVALID;
				error_y  [count].error = INVALID;
				error_dir[count].error = INVALID;
				
				error_pos[count].symbol = KS_INVALID;
				error_x  [count].symbol = KS_INVALID;
				error_y  [count].symbol = KS_INVALID;
				error_dir[count].symbol = KS_INVALID;
			} else
			{
				error_pos[count].symbol = database->chars[count].symbol;
				error_x  [count].symbol = database->chars[count].symbol;
				error_y  [count].symbol = database->chars[count].symbol;
				error_dir[count].symbol = database->chars[count].symbol;
				
				error_pos[count].error = 0;
				error_x  [count].error = 0;
				error_y  [count].error = 0;
				error_dir[count].error = 0;
				
				for (scount = 0; scount < database->chars[count].strokecount; scount++)
				{
					error_pos[count].error += ep = get_error(get_position_error,  &database->chars[count].strokes[scount], &actual->chars[ccount].strokes[scount], TRUE);
					error_x  [count].error += get_error(get_x_error,         &database->chars[count].strokes[scount], &actual->chars[ccount].strokes[scount], TRUE);
					error_y  [count].error += get_error(get_y_error,         &database->chars[count].strokes[scount], &actual->chars[ccount].strokes[scount], TRUE);
					error_dir[count].error += ed = get_error(get_direction_error, &database->chars[count].strokes[scount], &actual->chars[ccount].strokes[scount], TRUE);
				}
				
			}
		}
		
		qsort(error_pos,  database->charcount, sizeof(t_error), error_compare);
		qsort(error_x  ,  database->charcount, sizeof(t_error), error_compare);
		qsort(error_y  ,  database->charcount, sizeof(t_error), error_compare);
		qsort(error_dir,  database->charcount, sizeof(t_error), error_compare);	
		
	
	#ifdef DEBUG
		for (count = 0; count < 5; count++)
		{
			printf("Weight %d: p: %c: %7d,  x: %c: %7d,  y: %c: %7d,  d: %c:%7d\n", count,
			       error_pos[count].character, error_pos[count].error,
			       error_x  [count].character, error_x  [count].error,
			       error_y  [count].character, error_y  [count].error,
			       error_dir[count].character, error_dir[count].error);
		}
	#endif

		
		weightcount = 0;
	
		weight(error_pos, weighting, &weightcount);	
		weight(error_pos, weighting, &weightcount);	
		weight(error_x  , weighting, &weightcount);	
		weight(error_y  , weighting, &weightcount);	
		weight(error_dir, weighting, &weightcount);
		weight(error_dir, weighting, &weightcount);
		
#endif
		qsort(weighting,  weightcount, sizeof(t_weight), weight_compare);	
		
	#ifdef DEBUG
		for (count = 0; count < weightcount; count++)
		{
			printf("Weight %s -> %d\n", XKeysymToString(weighting[count].symbol), weighting[count].weight);
		}
	#endif
		actual->chars[ccount].symbol = weighting[0].symbol;
	}	
}

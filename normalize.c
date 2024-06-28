/*
 normalize.c - normalizing of input data
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
#include <string.h>
#include <math.h>
       
#include "rosetta.h"

void remove_empty_strokes(t_data_struct *data)
{
	int lcount;
	
	lcount = 0;
	while (lcount < data->lcount)
	{
		if (data->lines[lcount].pixels.pcount == 0)
		{
			memmove(&data->lines[lcount], &data->lines[lcount+1], (data->lcount - lcount)*sizeof(t_line));
			data->lcount--;
		} else lcount++;
	}
}

void normalize_stroke_data(t_data_struct *data)
{
	int   lcount;
	int   pcount;
	double pixcount;
	
	// Redraw inputdata as a pixelline and remove empty strokes afterwards
	
	for (lcount = 0; lcount < data->lcount; lcount++)
	{
		for (pcount=1; pcount < data->lines[lcount].pcount-1; pcount++)
		{
			data->lines[lcount].points[pcount].x=
			    (data->lines[lcount].points[pcount-1].x+
			     data->lines[lcount].points[pcount].x+
			     data->lines[lcount].points[pcount+1].x) / 3;
			data->lines[lcount].points[pcount].y=
			    (data->lines[lcount].points[pcount-1].y+
			     data->lines[lcount].points[pcount].y+
			     data->lines[lcount].points[pcount+1].y) / 3;	
		}
		
		
		if (data->lines[lcount].pcount > 1)
			for (pcount=1; pcount < data->lines[lcount].pcount; pcount++)
				draw_line(data->lines[lcount].points[pcount-1].x,
					  data->lines[lcount].points[pcount-1].y,
					  data->lines[lcount].points[pcount].x,
					  data->lines[lcount].points[pcount].y,
					  &data->lines[lcount].pixels);

	}
	
	remove_empty_strokes(data);	

	for (lcount = 0; lcount < data->lcount; lcount++)
	{
		data->lines[lcount].pcount = 0;
		
		pixcount = 0;	
		for (pcount = 0; pcount	< TEST_POINT_COUNT; pcount++)
		{
			data->lines[lcount].points[pcount].x = data->lines[lcount].pixels.points[(int)pixcount].x;			
			data->lines[lcount].points[pcount].y = data->lines[lcount].pixels.points[(int)pixcount].y;	
			pixcount += (double)data->lines[lcount].pixels.pcount / (double)(TEST_POINT_COUNT-1);
			if (pixcount >= data->lines[lcount].pixels.pcount)
				pixcount = data->lines[lcount].pixels.pcount - 1;
		}
		data->lines[lcount].pcount = TEST_POINT_COUNT;		
#ifdef DEBUG
		printf("Line %d contains %d pixels\n", lcount, data->lines[lcount].pixels.pcount);
#endif
	}
}


void normalize_character(t_character *character)
{
	double width;
	double height;
	int   xoffset;
	int   yoffset;
	int   scount;
	int   count;
	
	double xfakt;
	double yfakt;
	double fakt;
	
	width  = character->width;
	height = character->height;
	
	if (width > height*AR_FACT)
	{
		if (width > height * AR_VERYLONG)
			character->aspectratio = ar_verywidth; else
			character->aspectratio = ar_width;
		
	} else
	if (height > width*AR_FACT)
	{
		if (height > width * AR_VERYLONG)
			character->aspectratio = ar_veryheigh; else
			character->aspectratio = ar_heigh;
	} else
		character->aspectratio = ar_quadrat;
	
#ifdef NEW_NORMALIZE_CODE
	if (character->aspectratio == ar_width)
	{
		xfakt   = NORM_WIDTH / width;
		yfakt   = (NORM_HEIGHT / AR_FACT) / height;
		yoffset = (NORM_HEIGHT - height*yfakt) / 2;
		
		for (scount = 0; scount < character->strokecount; scount++)
			for (count = 0; count < TEST_POINT_COUNT; count++)
			{
				character->strokes[scount].points[count].x = 
					character->strokes[scount].points[count].x * xfakt;
				character->strokes[scount].points[count].y = 
					character->strokes[scount].points[count].y * yfakt + yoffset;
			}
	} else
	if (character->aspectratio == ar_heigh)
	{
		xfakt   = (NORM_WIDTH / AR_FACT) / width;
		yfakt   = NORM_HEIGHT / height;
		xoffset = (NORM_WIDTH - width*xfakt) / 2;
		
		for (scount = 0; scount < character->strokecount; scount++)
			for (count = 0; count < TEST_POINT_COUNT; count++)
			{
				character->strokes[scount].points[count].x = 
					character->strokes[scount].points[count].x * xfakt + xoffset;
				character->strokes[scount].points[count].y = 
					character->strokes[scount].points[count].y * yfakt;
			}

	} else
	if (character->aspectratio == ar_verywidth)
	{
		fakt = NORM_WIDTH / width;
		yoffset = (NORM_HEIGHT - character->height*fakt) / 2;

		for (scount = 0; scount < character->strokecount; scount++)
			for (count = 0; count < TEST_POINT_COUNT; count++)
			{
				character->strokes[scount].points[count].x = 
					character->strokes[scount].points[count].x * fakt;
				character->strokes[scount].points[count].y = 
					character->strokes[scount].points[count].y * fakt + yoffset;
			}
	} else
	if (character->aspectratio == ar_veryheigh)
	{	
		fakt = NORM_HEIGHT / height;
		xoffset = (NORM_WIDTH - character->width*fakt) / 2;
		
		for (scount = 0; scount < character->strokecount; scount++)
			for (count = 0; count < TEST_POINT_COUNT; count++)
			{
				character->strokes[scount].points[count].x = 
					character->strokes[scount].points[count].x * fakt + xoffset;

				character->strokes[scount].points[count].y = 
					character->strokes[scount].points[count].y * fakt;
			}
	} else
	{	
		xfakt   = NORM_WIDTH / width;
		yfakt   = NORM_HEIGHT / height;
		
		for (scount = 0; scount < character->strokecount; scount++)
			for (count = 0; count < TEST_POINT_COUNT; count++)
			{
				character->strokes[scount].points[count].x = 
					character->strokes[scount].points[count].x * xfakt;
				character->strokes[scount].points[count].y = 
					character->strokes[scount].points[count].y * yfakt;
			}
	}

#else		
	if (width>height)
	{
		fakt = NORM_WIDTH / width;
		yoffset = (NORM_HEIGHT - character->height*fakt) / 2;

		for (scount = 0; scount < character->strokecount; scount++)
			for (count = 0; count < TEST_POINT_COUNT; count++)
			{
				character->strokes[scount].points[count].x = 
					character->strokes[scount].points[count].x * fakt;
				character->strokes[scount].points[count].y = 
					character->strokes[scount].points[count].y * fakt + yoffset;
			}
	} else
	{	
		fakt = NORM_HEIGHT / height;
		xoffset = (NORM_WIDTH - character->width*fakt) / 2;
		
		for (scount = 0; scount < character->strokecount; scount++)
			for (count = 0; count < TEST_POINT_COUNT; count++)
			{
				character->strokes[scount].points[count].x = 
					character->strokes[scount].points[count].x * fakt + xoffset;

				character->strokes[scount].points[count].y = 
					character->strokes[scount].points[count].y * fakt;
			}
	}
#endif	
}

void convert_strokes_to_normalized_characters(t_data_struct *input_data, t_charlist *analized_data)
{
	int lcount;
	int pcount;
	int ccount;
	int mcount;
	int scount;
	int xmin;
	int xmax;
	int ymin;
	int ymax;


	for (lcount = 0; lcount < input_data->lcount; lcount++)
		if (input_data->lines[lcount].multi == FALSE)
		{
			ccount = analized_data->charcount;

			for (pcount = 0; pcount < TEST_POINT_COUNT; pcount++)
			{
				analized_data->chars[ccount].strokes[analized_data->chars[ccount].strokecount].points[pcount] = 
					input_data->lines[lcount].points[pcount];
				
				analized_data->chars[ccount].strokes[analized_data->chars[ccount].strokecount].direction[pcount] = 
					input_data->lines[lcount].direction[pcount];
				
			}		
			analized_data->chars[ccount].symbol   = KS_INVALID;
			analized_data->chars[ccount].strokecount = 1;
			analized_data->chars[ccount].dotcount = input_data->lines[lcount].dotcount;
			analized_data->word_seperator[ccount] = input_data->lines[lcount].newword;
			
			for (mcount = lcount; mcount < input_data->lcount; mcount++)
			{
				if (input_data->lines[mcount].multi && input_data->lines[mcount].reference == lcount)
				{
					for (pcount = 0; pcount < TEST_POINT_COUNT; pcount++)
					{
						analized_data->chars[ccount].strokes[analized_data->chars[ccount].strokecount].points[pcount] = 
							input_data->lines[mcount].points[pcount];
						
						analized_data->chars[ccount].strokes[analized_data->chars[ccount].strokecount].direction[pcount] = 
							input_data->lines[mcount].direction[pcount];
					}		
					analized_data->chars[ccount].strokecount++;					
				}
				
			}
			
			xmin = window_width;
			ymin = HEIGHT;
			xmax = 0;
			ymax = 0;
			
			for (scount = 0; scount < analized_data->chars[ccount].strokecount; scount++)
				for (pcount = 0; pcount < TEST_POINT_COUNT; pcount++)
				{
					if (xmin > analized_data->chars[ccount].strokes[scount].points[pcount].x)
						xmin = analized_data->chars[ccount].strokes[scount].points[pcount].x;

					if (ymin > analized_data->chars[ccount].strokes[scount].points[pcount].y)
						ymin = analized_data->chars[ccount].strokes[scount].points[pcount].y;

					if (xmax < analized_data->chars[ccount].strokes[scount].points[pcount].x)
						xmax = analized_data->chars[ccount].strokes[scount].points[pcount].x;

					if (ymax < analized_data->chars[ccount].strokes[scount].points[pcount].y)
						ymax = analized_data->chars[ccount].strokes[scount].points[pcount].y;
				}

			for (scount = 0; scount < analized_data->chars[ccount].strokecount; scount++)
				for (pcount = 0; pcount < TEST_POINT_COUNT; pcount++)
				{
					analized_data->chars[ccount].strokes[scount].points[pcount].x -= xmin;
					analized_data->chars[ccount].strokes[scount].points[pcount].y -= ymin;
				}

			if (mode != MODE_LEARNING)
			{
				if ((xmin+xmax) / 2 < window_width / 3)     analized_data->chars[ccount].group = grp_lowercase; else
				if ((xmin+xmax) / 2 > (window_width*2) / 3) analized_data->chars[ccount].group = grp_function; else
									    analized_data->chars[ccount].group = grp_uppercase;
			} else
			{
				if (learn_set_group_mode==LEARN_SET_GROUP_CONFIG)
					analized_data->chars[ccount].group = configuration.keys[keycount].group;   // only for leaning mode of rosetta
			}
				
			analized_data->chars[ccount].width  = xmax - xmin + 1;
			analized_data->chars[ccount].height = ymax - ymin + 1;	
				
			normalize_character(&analized_data->chars[ccount]);
				
			analized_data->charcount++;
		}			
}

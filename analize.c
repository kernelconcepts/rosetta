/*
 analize.c - analizing functions to determine some attribute vectors
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

void calculate_stroke_dimensions(t_data_struct *data)
{
	int lcount;
	int pcount;
	
	data->xmin = window_width;
	data->xmax = 0;
	data->ymin = HEIGHT;
	data->ymax = 0;
	
#ifdef DEBUG	
	printf("Number of strokes: %d\n", data->lcount);
#endif
	
	for (lcount = 0; lcount < data->lcount; lcount++)
	{
		data->lines[lcount].ymin = HEIGHT;
		data->lines[lcount].ymax = 0;
		
		data->lines[lcount].xmin = window_width;
		data->lines[lcount].xmax = 0;
		
		for (pcount = 0; pcount < data->lines[lcount].pcount; pcount++)
		{
			if (data->lines[lcount].points[pcount].x < data->lines[lcount].xmin)
				data->lines[lcount].xmin = data->lines[lcount].points[pcount].x;
			if (data->lines[lcount].points[pcount].x > data->lines[lcount].xmax)
				data->lines[lcount].xmax = data->lines[lcount].points[pcount].x;

			if (data->lines[lcount].points[pcount].y < data->lines[lcount].ymin)
				data->lines[lcount].ymin = data->lines[lcount].points[pcount].y;
			if (data->lines[lcount].points[pcount].y > data->lines[lcount].ymax)
				data->lines[lcount].ymax = data->lines[lcount].points[pcount].y;
		}
		
		if (data->lines[lcount].xmin < data->xmin)
			data->xmin = data->lines[lcount].xmin;
		if (data->lines[lcount].xmax > data->xmax)
			data->xmax = data->lines[lcount].xmax;
		
		if (data->lines[lcount].ymin < data->ymin)
			data->ymin = data->lines[lcount].ymin;
		if (data->lines[lcount].ymax > data->ymax)
			data->ymax = data->lines[lcount].ymax;

		data->lines[lcount].width  = data->lines[lcount].xmax - data->lines[lcount].xmin + 1;
		data->lines[lcount].height = data->lines[lcount].ymax - data->lines[lcount].ymin + 1;
	}
}

void calculate_stroke_directions(t_data_struct *data)
{
	int lcount;
	int pcount;

	int angle;
	for (lcount = 0; lcount < data->lcount; lcount++)
	{
//		data->lines[lcount].sum_angle = 0;

		for (pcount = 1; pcount < data->lines[lcount].pcount; pcount++)
		{
			angle = get_angle(data->lines[lcount].points[pcount-1].x, data->lines[lcount].points[pcount-1].y,
		                          data->lines[lcount].points[pcount].x, data->lines[lcount].points[pcount].y);
			
/*			change= (old_angle - angle  + 2*M_PI);
			if (change > 2*M_PI) change -= 2*M_PI;
			if (change > M_PI)   change = -(2*M_PI-change);
			change = -change;	
			
*/			if (pcount>1) 
			{
				data->lines[lcount].direction[pcount] = angle;   //data->lines[lcount].sum_angle;
			}
			
		}
	}
}

void find_dots(t_data_struct *data)
{
	int count;
	int lcount;
	int xmin;
	int lmindist;
	int dist;

	for (lcount = 0; lcount < data->lcount; lcount++)
		if (data->lines[lcount].pixels.pcount <= MAX_DOT_SIZE)
		{
			data->lines[lcount].dotcount = INVALID;
			data->lines[lcount].multi    = TRUE;
			
			lmindist = INVALID;
			xmin     = INVALID;
			
			for (count = 0; count < data->lcount; count++)
				if ((count != lcount) && 
				    (data->lines[count].pixels.pcount > MAX_DOT_SIZE) &&
				    (data->lines[count].multi == FALSE))
				{
					dist = abs((data->lines[count].xmin + data->lines[count].width / 2) -
					           (data->lines[lcount].xmin+ data->lines[lcount].width / 2));
					
					if (dist < xmin)
					{
						xmin = dist;
						lmindist = count;
					}					
				}
					
			if (lmindist != INVALID)
				data->lines[lmindist].dotcount++;				
		}			
}

void remove_dots(t_data_struct *data)
{
	int lcount;
	
	lcount = 0;
	while (lcount < data->lcount)
	{
		if ((data->lines[lcount].multi == TRUE) && (data->lines[lcount].dotcount == INVALID))
		{
			memmove(&data->lines[lcount], &data->lines[lcount+1], (data->lcount - lcount)*sizeof(t_line));
			data->lcount--;
#ifdef DEBUG			
			printf("Remove dot %d\n", lcount);
#endif
		} else lcount++;
	}
}

void find_multiple_strokes(t_data_struct *data)
{
	int lcount;
	int count;
	int xa1, xa2, ya1, ya2;
	int xb1, xb2, yb1, yb2;
	int wa, wb;
	int ha, hb;
	int height = MIN_WORD_HEIGHT;
		
	// precalculate the maximal height of all characters.
	// needet for differentiasation of the textlines
	for (lcount = 0; lcount < data->lcount; lcount++)
		if (data->lines[lcount].height>height) height = data->lines[lcount].height;
	
	// now compare every stroke with the ones written before to find
	// overlaps. Then combine them. 		
	for (lcount = 0; lcount < data->lcount; lcount++)
		for (count = 0; count < data->lcount; count++)
		{
			if ((data->lines[count].multi == FALSE) && (count != lcount) && (data->lines[lcount].reference!=count))
			{
				xa1 = data->lines[lcount].xmin;
				xa2 = data->lines[lcount].xmax;
				ya1 = data->lines[lcount].ymin;
				ya2 = data->lines[lcount].ymax;
				wa  = data->lines[lcount].width;				
				ha  = data->lines[lcount].height;				

				xb1 = data->lines[count].xmin;
				xb2 = data->lines[count].xmax;
				yb1 = data->lines[count].ymin;
				yb2 = data->lines[count].ymax;
				wb  = data->lines[count].width;								
				hb  = data->lines[count].height;								
				
				// If the strokes are too smal then grow them for the overlapping test
				if (wa < MIN_CHAR_WIDTH)
				{
					xa1 -= (MIN_CHAR_WIDTH-wa)/2;
					xa2 += (MIN_CHAR_WIDTH-wa)/2;
				}
		
				if (ha < MIN_CHAR_HEIGHT)
				{
					ya1 -= (MIN_CHAR_HEIGHT-ha)/2;
					ya2 += (MIN_CHAR_HEIGHT-ha)/2;
				}
				wa  = xa2 - xa1;

				if (wb < MIN_CHAR_WIDTH)
				{
					xb1 -= (MIN_CHAR_WIDTH-wb)/2;
					xb2 += (MIN_CHAR_WIDTH-wb)/2;
				}
		
				if (hb < MIN_CHAR_HEIGHT)
				{
					yb1 -= (MIN_CHAR_HEIGHT-hb)/2;
					yb2 += (MIN_CHAR_HEIGHT-hb)/2;
				}
				wb  = xb2 - xb1;
				
				if (abs(yb1+(yb2-yb1)/2-(ya1+(ya2-ya1)/2)) < height)
				{
					if ((xb1<=xa1 && xb2>=xa2)	||		// Full overlapping, b is larger
					    (xb1>=xa1 && xb2<=xa2)	||		// Full overlapping, b is smaler
					    ((xb1>=xa1 && xb1<=xa2) && (xb2 >= xa2) && 
							((abs(xa2-xb1) >= wa * OVERLAP_FACT) ||
							 (abs(xa2-xb1) >= wb * OVERLAP_FACT))) ||	
					    ((xb2>=xa1 && xb2<=xa2) && (xb1 <= xa1) && 
							((abs(xb2-xa1) >= wa * OVERLAP_FACT) ||
							 (abs(xb2-xa1) >= wb * OVERLAP_FACT))))
					{
#ifdef DEBUG
						printf("Overlapping lines ========> %d : %d\n", lcount, count);
						printf("xa1: %d, xa2: %d, xb1: %d, xb2: %d\n", xa1, xa2, xb1, xb2);
#endif

						data->lines[count].multi = TRUE;
						if (data->lines[lcount].multi == FALSE)
						{
							data->lines[count].reference = lcount;
						}
						else
						{
							data->lines[count].reference = data->lines[lcount].reference;
						}
					}
#ifdef DEBUG
					if (xb1<xa1 && xb2>xa2) printf("Full b>a\n");
					
					if (xb1>xa1 && xb2<xa2)	printf("Full a>b\n");
					
					if ((xb1>xa1 && xb1<xa2) && (xb2 > xa2) && 
							((abs(xa2-xb1) > data->lines[lcount].width * OVERLAP_FACT) ||
							 (abs(xa2-xb1) > data->lines[count].width * OVERLAP_FACT)))
						printf("Partial: b right a, overlap: %d, width: %d\n", abs(xa2-xb1), (int)(data->lines[lcount].width * OVERLAP_FACT));
					
					if ((xb2>xa1 && xb2<xa2) && (xb1 < xa1) && 
							((abs(xb2-xa1) > data->lines[lcount].width * OVERLAP_FACT) ||
							 (abs(xb2-xa1) > data->lines[count].width * OVERLAP_FACT)))
						printf("Partial: b left a, overlap: %d, width: %d\n", abs(xb2-xa1), (int)(data->lines[lcount].width * OVERLAP_FACT));
#endif	
				}
			}
		}
}

int  compare_int(const void *a, const void *b)
{
	return *(int *)a - *(int *)b;
}

void find_words(t_data_struct *data)
{
	int lcount;
	int lastx;
	int lasty;	
	
	int x_distances[MAX_LINES];
	int y_distances[MAX_LINES];
	int distcount;
	int dcount;
	int xdist;
	int ydist;
	int count;
	
	
	lastx  = data->lines[0].xmax;	
	lasty  = data->lines[0].ymax;
	
	if (data->lcount == 0) return;
	
	distcount = 0;
	
	for (lcount = 1; lcount < data->lcount; lcount++)
	{
		if (data->lines[lcount].multi == FALSE)
		{
			x_distances[distcount]=abs(data->lines[lcount-1].xmax - data->lines[lcount].xmin);
			y_distances[distcount++]=abs(data->lines[lcount-1].ymax - data->lines[lcount].ymin);
		}
	}
	
	qsort(x_distances, distcount, sizeof(int), compare_int);
	qsort(y_distances, distcount, sizeof(int), compare_int);
	
	count = 0;
	xdist = 0;
	ydist = 0;
	
	data->lines[data->lcount-1].newword = TRUE;	
	if (distcount < 3) return;
	
	for (dcount = distcount / 3; dcount < (2*distcount) / 3; dcount++)
	{
		xdist += x_distances[dcount];
		ydist += y_distances[dcount];
		count++;		
	}
	
	if (count == 0) 
	{
		xdist = 0; 
		ydist = 0;
	} else
	{
		xdist /= count;
		ydist /= count;
	}

#ifdef DEBUG	
	printf("Distance between characters: X: %d    Y: %d\n", xdist, ydist);
#endif
	
	xdist *= WORD_X_DISTANCE_FACT;
	ydist *= WORD_Y_DISTANCE_FACT;
	
 	if (xdist < MIN_CHAR_WIDTH) xdist = MIN_CHAR_WIDTH;
	if (ydist < MIN_CHAR_HEIGHT) ydist = MIN_CHAR_HEIGHT;
	
#ifdef DEBUG
	printf("Min. Distance between words: X: %d    Y: %d\n", xdist, ydist);
#endif
	
	for (lcount = 1; lcount < data->lcount; lcount++)
		if (data->lines[lcount].multi == FALSE)
		{	
			if (abs(data->lines[lcount].xmin-lastx)  > xdist)
				data->lines[lcount-1].newword = TRUE;
			
			if (abs(data->lines[lcount].ymin-lasty) > ydist)
				data->lines[lcount-1].newword = TRUE;
			
			lastx = data->lines[lcount].xmax;
			lasty = data->lines[lcount].ymax;
		}
}

void analize_strokes(t_data_struct *input_data, t_charlist *analized_data)
{
	normalize_stroke_data(input_data);
	calculate_stroke_dimensions(input_data);
	calculate_stroke_directions(input_data);
	find_dots(input_data);
	remove_dots(input_data);
	find_multiple_strokes(input_data);
	find_words(input_data);
	convert_strokes_to_normalized_characters(input_data, analized_data);
}

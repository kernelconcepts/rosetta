/*
 helper.c - some minor helping functions
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

#include <math.h>
#include <string.h>
#include "rosetta.h"

void draw_line(int xP, int yP, int xQ, int yQ, t_pixelline *pixels) 
{
	int x = xP;
	int y = yP;
	int D = 0;
	int HX = xQ - xP;
	int HY = yQ - yP;
	int xInc = 1;
	int yInc = 1;
	int c = 0;
	int M = 0;
		
	if(HX < 0) 
	{
		xInc = -1; 
		HX = -HX;
	}

	if(HY < 0) 
	{
		yInc = -1; 
		HY = -HY;
	}

	if(HY <= HX) 
	{
		c = 2 * HX; 
		M = 2 * HY;
		
		while(TRUE) 
		{
			pixels->points[pixels->pcount].x = x;
			pixels->points[pixels->pcount].y = y;
			pixels->pcount++;
			if (pixels->pcount >= MAX_POINTS_PER_PIXEL_LINE) break;
			
			if(x == xQ) break;
		
			x += xInc;
			D += M;
			if(D > HX) 
			{
				y += yInc; 
				D -=c;
			}
		}
	} else 
	{
		c = 2 * HY; 
		M = 2 * HX;
		
		while(TRUE) 
		{
			pixels->points[pixels->pcount].x = x;
			pixels->points[pixels->pcount].y = y;
			pixels->pcount++;
			if (pixels->pcount >= MAX_POINTS_PER_PIXEL_LINE) break;

			if(y == yQ) break;
			
			y += yInc;
			D += M;
			if(D > HY) 
			{
				x += xInc; 
				D -=c;
			}
		} 
			
	}
}

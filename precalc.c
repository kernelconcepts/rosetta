/*
 precalc.c - precalculation of floting point values
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
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "rosetta.h"

static short angle[PRECALC_ANGLE_SIZE_X][PRECALC_ANGLE_SIZE_Y];

void precalc(void)
{
	int x;
	int y;

	int xcalc = 0;
	int ycalc = 0;
	
	double val;
	
	for (x = 0; x < PRECALC_ANGLE_SIZE_X; x++)
		for (y = 0; y < PRECALC_ANGLE_SIZE_Y; y++)
		{
			xcalc = x - PRECALC_ANGLE_OFFS_X;
			ycalc = y - PRECALC_ANGLE_OFFS_Y;
			
			val = atan(ycalc/(xcalc+0.0001));

			if (xcalc<0) val = M_PI   + val;
			if (val<0)   val = 2*M_PI + val;
			if (val>M_PI) val = -2*M_PI + val;
			
			angle[x][y] = val / M_PI * NORM_ERROR;	
		}			
} 

int load_precalc(char *filename)
{

	int	fd;
	int    count;
	struct  stat filestat;
	
	
	fd = open(filename, O_RDONLY);
	
	if (fd==-1)
	{
		perror("Error loading precalculations: ");
		return FALSE;
	}

	fstat(fd, &filestat);
	
	if (filestat.st_size != sizeof(angle))
	{
		fprintf(stderr, "Error loading precalculations: Wrong filesize\n");
		close(fd);
		return FALSE;
	}

	for (count=0; count<PRECALC_ANGLE_SIZE_X; count++)
	{
		if (sizeof(short)*PRECALC_ANGLE_SIZE_Y != read(fd, angle[count], sizeof(short)*PRECALC_ANGLE_SIZE_Y))
		{
			perror("Error loading precalculations: ");
			close(fd);
			return FALSE;
		};		
	}
	close(fd);
	return TRUE;
}

int save_precalc(char *filename)
{
	int	fd;
	int    count;
	
	fd=open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

	if (fd==-1)
	{
		perror("Error writing precalculations: ");
		return FALSE;
	}

	for (count=0; count<PRECALC_ANGLE_SIZE_X; count++)
	{
		if (sizeof(short)*PRECALC_ANGLE_SIZE_Y != write(fd, angle[count], sizeof(short)*PRECALC_ANGLE_SIZE_Y))
		{
			perror("Error writing precalculations: ");
			close(fd);
			return FALSE;
		};		
	}
	
	close(fd);
	return TRUE;
}

inline int get_angle(int x1, int y1, int x2, int y2)
{
	int xcalc = 0;
	int ycalc = 0;
	
	double val;	
//	printf("%d\n", angle[x2-x1+PRECALC_ANGLE_OFFS][y1-y2+PRECALC_ANGLE_OFFS]);
	if (flag_use_precalc) return(angle[x2-x1+PRECALC_ANGLE_OFFS_X][y1-y2+PRECALC_ANGLE_OFFS_Y]); else
	{
		xcalc = x2-x1;
		ycalc = y1-y2;
		
		val = atan(ycalc/(xcalc+0.0001));

		if (xcalc<0) val = M_PI   + val;
		if (val<0)   val = 2*M_PI + val;
		if (val>M_PI) val = -2*M_PI + val;
		
		return val / M_PI * NORM_ERROR;				
	}
}

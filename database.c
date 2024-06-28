/*
 database.c - database reading functions
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include "rosetta.h"

int load_stroke_database(char *filename, t_database *database)
{

	int	fd;
	int    count;
	int    charcount;	
	struct  stat filestat;
	
	
	fd = open(filename, O_RDWR);
	
	if (fd==-1)
	{
		perror("Error loading database: ");
		return FALSE;
	}

	fstat(fd, &filestat);
	
	if ((filestat.st_size == 0) || ((filestat.st_size - sizeof(charcount)) % sizeof(t_character)!= 0))
	{
		fprintf(stderr, "Error loading database: Wrong filesize\n");
		close(fd);
		return FALSE;
	}
			
	if (sizeof(charcount) != read(fd, &charcount, sizeof(charcount)))
	{
		perror("Error loading database: ");
		close(fd);
		return FALSE;
	};

	if (charcount != (filestat.st_size - sizeof(charcount)) / sizeof(t_character))
	{
		fprintf(stderr, "Error loading database: Wrong filesize\n");
		close(fd);
		return FALSE;
	}
	
	database->charcount = charcount;
	
	for (count=0; count<charcount; count++)
	{
		if (sizeof(t_character) != read(fd, &database->chars[count], sizeof(t_character)))
		{
			perror("Error loading database: ");
			close(fd);
			return FALSE;
		};		
	}
	printf("Database loaded successfully %s\n", filename);
	close(fd);
	return TRUE;
}

int save_stroke_database(char *filename, t_database *database)
{
	int	fd;
	int    count;
	int    charcount;
	
	fd=open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

	if (fd==-1)
	{
		perror("Error writing database: ");
		return FALSE;
	}

	charcount = database->charcount;
	
	if (sizeof(charcount) != write(fd, &charcount, sizeof(charcount)))
	{
		perror("Error writing database: ");
		close(fd);
		return FALSE;
	};
	
	for (count=0; count<charcount; count++)
	{
		if (sizeof(t_character) != write(fd, &database->chars[count], sizeof(t_character)))
		{
			perror("Error writing database: ");
			close(fd);
			return FALSE;
		};		
	}
	
	close(fd);
	return TRUE;
}

inline void add_to_stroke_database(t_character *character, t_database *database)
{
	database->chars[database->charcount++]=*character;	
}

inline void set_stroke_database(t_character *character, t_database *database, int charnr)
{
	database->chars[charnr]=*character;
}

void generate_emty_stroke_database_from_config(t_database *database)
{
	int repeat;
	int kcount;
	t_character dummy_char;
	
	memset(&dummy_char, 0, sizeof(t_character));
	
	for (kcount = 0; kcount < configuration.keycount; kcount++)
		for (repeat = 0; repeat < configuration.keys[kcount].repeat; repeat++)
		{
			dummy_char.symbol = XStringToKeysym(configuration.keys[kcount].symbol[0]);			
			dummy_char.group  = configuration.keys[kcount].group;			
			add_to_stroke_database(&dummy_char, database);
		}
}

/*
 configfile.c - config file reading functions
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "rosetta.h"

typedef enum {
	state_start     = 0x00,
	state_global,
	state_key,
	state_dot,
	state_dot_dot,
	state_error
} config_state;


int is_comment(char *text)
{
	int count;
	for (count = 0; count < strlen(text); count ++)
		if (text[count] == ' '  ||
		    text[count] == '\t' ||
		    text[count] == '\n' || 
		    text[count] == '#')
		{
			if (text[count]=='#') return TRUE;
		} else return FALSE;
	return TRUE;
}


int load_configfile(char *filename)
{
	FILE            *fd;
	int             repeat;
	config_state    state;
	char            configline[100];
	int             linecount;
	int		errorline;
	
	char            tag[20];
	char            name[20];
	char            value1[20];
	char		value2[20];
	char            value3[20];
	char		value4[20];
	char		value[80];
	int             matchcount;
	int             valcount;
	char            *errorchar = NULL;
	
	fd = fopen(filename, "r");
	
	if (fd == NULL)
	{
		perror("Error loading configuration: ");
		return FALSE;
	}

	state = state_start;
	linecount = 0;
	errorline = -1;
	configuration.keycount = 0;
	repeat    = 1;
	
	while (fgets(configline, 99, fd))
	{
		linecount++;
		if (is_comment(configline)) continue;
			
		switch (state)
		{
			case	state_start:
				if (sscanf(configline, "%20s", tag) != 1)
				{
					errorline = linecount;
					state = state_error;
					break;
				}
				
				if (strcmp(tag, "<global>") == 0)
				{
					state = state_global;
					break;
				}
				
				if (strcmp(tag, "<key>") == 0)
				{
					state = state_key;
					configuration.keycount++;
					if (configuration.keycount > MAX_CONFIG_KEYS)
					{
						printf("Configfile contains too much keydefinitions!\n");
						errorline = linecount;
						state = state_error;
						break;
					}
					configuration.keys[configuration.keycount-1].repeat = repeat;
					strcpy(configuration.keys[configuration.keycount-1].text[0], "");
					strcpy(configuration.keys[configuration.keycount-1].text[1], "");
					strcpy(configuration.keys[configuration.keycount-1].text[2], "");
					strcpy(configuration.keys[configuration.keycount-1].symbol[0], "");
					strcpy(configuration.keys[configuration.keycount-1].symbol[1], "");
					strcpy(configuration.keys[configuration.keycount-1].symbol[2], "");
					configuration.keys[configuration.keycount-1].group = grp_none;
					
					break;
				}

				errorline = linecount;
				state = state_error;
				break;
			
			case	state_global:
				
				matchcount = sscanf(configline, "%s %s %s %s %s", name, value1, value2, value3, value4);
				if (matchcount == 0)
				{				
					errorline = linecount;
					state = state_error;
					break;
				}		
					
				strcpy(value, value1);
				for (valcount=1; valcount < matchcount - 1; valcount++)
				{
					strcat(value, " ");
					switch (valcount)
					{
						case 1:	strcat(value, value2); break;
						case 2: strcat(value, value3); break;
						case 3: strcat(value, value4); break;
					}
				}
				
				if (matchcount == 1)
				{			
					if (strcmp(name, "</global>") == 0)					
					{
						state = state_start;
						break;
					} 
	
					errorline = linecount;
					state = state_error;
					break;
				}	
								
				if (strcmp(name, "repeat") == 0)
				{
					repeat = strtol(value, &errorchar, 0);
					if (repeat > MAX_REPEAT) repeat = MAX_REPEAT;
					if (errorchar[0] != '\0')
					{
						errorline = linecount;
						state = state_error;
						break;
					}
					break;
				}
		
				errorline = linecount;
				state = state_error;
				break;
			
			case	state_key:
				
				matchcount = sscanf(configline, "%s %s %s %s %s", name, value1, value2, value3, value4);
				if (matchcount == 0)
				{				
					errorline = linecount;
					state = state_error;
					break;
				}		
					
				strcpy(value, value1);
				for (valcount=1; valcount < matchcount - 1; valcount++)
				{
					strcat(value, " ");
					switch (valcount)
					{
						case 1:	strcat(value, value2); break;
						case 2: strcat(value, value3); break;
						case 3: strcat(value, value4); break;
					}
				}
				
				if (matchcount == 1)
				{			
					if (strcmp(name, "</key>") == 0)					
					{
						state = state_start;
						break;
					}
					if (strcmp(name, "<.>") == 0)					
					{
						state = state_dot;
						break;
					}
					if (strcmp(name, "<..>") == 0)					
					{
						state = state_dot_dot;
						break;
					}

					errorline = linecount;
					state = state_error;
					break;
				}	
								
				if (strcmp(name, "repeat") == 0)
				{
					configuration.keys[configuration.keycount-1].repeat = strtol(value, &errorchar, 0);
					if (configuration.keys[configuration.keycount-1].repeat > MAX_REPEAT)
						configuration.keys[configuration.keycount-1].repeat = MAX_REPEAT;
					if (errorchar[0] != '\0')
					{
						errorline = linecount;
						state = state_error;
						break;
					}
					break;					
				} else
				if (strcmp(name, "name") == 0)
				{
					strncpy(configuration.keys[configuration.keycount-1].text[0], value, MAX_SYMBOL_SIZE);
					strncpy(configuration.keys[configuration.keycount-1].text[1], value, MAX_SYMBOL_SIZE);
					strncpy(configuration.keys[configuration.keycount-1].text[2], value, MAX_SYMBOL_SIZE);
					break;
				} else				
				if (strcmp(name, "symbol") == 0)
				{
					strncpy(configuration.keys[configuration.keycount-1].symbol[0], value, MAX_SYMBOL_SIZE);
					strncpy(configuration.keys[configuration.keycount-1].symbol[1], value, MAX_SYMBOL_SIZE);
					strncpy(configuration.keys[configuration.keycount-1].symbol[2], value, MAX_SYMBOL_SIZE);
					break;
				} else				
				if (strcmp(name, "group") == 0)
				{
					if (strcmp(value, "grp_lowercase") == 0)
					{
						configuration.keys[configuration.keycount-1].group = grp_lowercase;
						break;
					} else
					if (strcmp(value, "grp_uppercase") == 0)
					{
						configuration.keys[configuration.keycount-1].group = grp_uppercase;
						break;
					} else
					if (strcmp(value, "grp_number") == 0)
					{
						configuration.keys[configuration.keycount-1].group = grp_number;
						break;
					} else
					if (strcmp(value, "grp_special") == 0)
					{
						configuration.keys[configuration.keycount-1].group = grp_special;
						break;
					} else
					if (strcmp(value, "grp_function") == 0)
					{
						configuration.keys[configuration.keycount-1].group = grp_function;
						break;
					} else
					errorline = linecount;
					state = state_error;
					break;					
				}			
				errorline = linecount;
				state = state_error;
				break;
			case	state_dot:
			case	state_dot_dot:
				
				matchcount = sscanf(configline, "%s %s %s %s %s", name, value1, value2, value3, value4);
				if (matchcount == 0)
				{				
					errorline = linecount;
					state = state_error;
					break;
				}		
					
				strcpy(value, value1);
				for (valcount=1; valcount < matchcount - 1; valcount++)
				{
					strcat(value, " ");
					switch (valcount)
					{
						case 1:	strcat(value, value2); break;
						case 2: strcat(value, value3); break;
						case 3: strcat(value, value4); break;
					}
				}
				
				if (matchcount == 1)
				{			
					if (strcmp(name, "</.>") == 0)					
					{
						state = state_key;
						break;
					}
					if (strcmp(name, "</..>") == 0)					
					{
						state = state_key;
						break;
					}

					errorline = linecount;
					state = state_error;
					break;
				}	
								

				if (strcmp(name, "name") == 0)
				{
					if (state == state_dot)
						strncpy(configuration.keys[configuration.keycount-1].text[1], value, MAX_SYMBOL_SIZE); 
					else
						strncpy(configuration.keys[configuration.keycount-1].text[2], value, MAX_SYMBOL_SIZE);
					break;
				} else				
				if (strcmp(name, "symbol") == 0)
				{
					if (state == state_dot)
						strncpy(configuration.keys[configuration.keycount-1].symbol[1], value, MAX_SYMBOL_SIZE);
					else
						strncpy(configuration.keys[configuration.keycount-1].symbol[2], value, MAX_SYMBOL_SIZE);
					break;
				}
				
				errorline = linecount;
				state = state_error;
				break;
			default:				
				errorline = linecount;
				state = state_error;
				break;
		}
		if (state == state_error)
		{
			printf("Error in line #%d in configuration file %s\n", errorline, filename);
			fclose(fd);
			return FALSE;
		}
	}
	
	if (state != state_start)
	{
		printf("Error in configuration file %s\n", filename);
		fclose(fd); 
		return FALSE;
	}

	printf("Configuration loaded successfully: %s\n", filename);
	fclose(fd);
	return TRUE;
}

char *config_get_name_from_keysym(KeySym symbol)
{
	int count;
	int dotcount;
	
	for (count = 0; count < configuration.keycount; count++)
		for (dotcount = 0; dotcount < 3; dotcount++)
			if (symbol == XStringToKeysym(configuration.keys[count].symbol[dotcount]))
				return configuration.keys[count].text[dotcount];	
	return NULL;
}

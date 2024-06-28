/*
 dictionary.c - dictionary reading and correcting functions
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
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "rosetta.h"

char    **dictionary = NULL;
int     wordcount;

int load_dictionary(char *filename)
{

	FILE	*fd;
	int     count;
	
	dictionary = malloc(MAX_WORDS*sizeof(char*));
	for (count = 0; count < MAX_WORDS; count++)
		dictionary[count] = malloc(MAX_WORDSIZE*sizeof(char));
	
	fd = fopen(filename, "r");
	
	if (fd == NULL)
	{
		perror("Error loading dictionary: ");
		return FALSE;
	}
	
	wordcount = 0;
	while (wordcount < MAX_WORDS && fgets(dictionary[wordcount], MAX_WORDSIZE, fd))
	{
		if (dictionary[wordcount][strlen(dictionary[wordcount])-1] == '\n')
			dictionary[wordcount][strlen(dictionary[wordcount])-1] = 0x00;
		wordcount++;
	};

	printf("Dictionary %s loaded with %d words\n", filename, wordcount);
	fclose(fd);
	return TRUE;
}

void lookup_dictionary(KeySym *i, KeySym *o, int length)
{
	int  count;
	int  ccount;
	int  weighting;
	int  maxweighting;
	char outsymbol[2];
	char temp  [MAX_SYMBOL_SIZE];
	char input [MAX_LINES];
	char output[MAX_LINES];
	
	if (length<2)
	{
		memcpy(o, i, sizeof(KeySym) * length);
		return;
	}
	ccount = 0;
	for (count = 0; count < length; count ++)
	{
		strncpy(temp, XKeysymToString(i[count]), MAX_SYMBOL_SIZE);
		if (strlen(temp)!=1)
		{
			memcpy(o, i, sizeof(KeySym) * length);
			return;
		}
		input[ccount++] = temp[0];
	}
	temp[ccount] = 0x00;
	
	strncpy(output, input, MAX_WORDSIZE);
	
	maxweighting = 0;
	for (count=0; count < wordcount; count++)
	{
		if (strlen(dictionary[count]) == length)
		{
			weighting = 0;
			for (ccount = 0; ccount < length; ccount++)
			{
				if (tolower(input[ccount]) == tolower(dictionary[count][ccount])) weighting++;
			}
			if (weighting > maxweighting)
			{
				maxweighting = weighting;
				strncpy(output, dictionary[count], MAX_WORDSIZE);
				if (maxweighting == length) break;
			}
		}
	}
	outsymbol[1] = 0x00;
	for (count = 0; count < strlen(output); count ++)
	{	
		outsymbol[0] = output[count];
		o[count] = XStringToKeysym(outsymbol);
	}
}

KeySym get_doted_char(KeySym character, int dots)
{
	int count;
	if (dots == 0 || dots >= MAX_DOTS) return character;
		
	for (count = 0; count < configuration.keycount; count ++)
		if (character == XStringToKeysym(configuration.keys[count].symbol[0]))
			return XStringToKeysym(configuration.keys[count].symbol[dots]);
	
	return character;
}

int generate_symbol_list(t_charlist *input, KeySym *output)
{
	int    ccount;
	KeySym temp[MAX_LINES];
	KeySym word[MAX_LINES];
	int    tpos;
	int    opos;
	int    count;
	
#ifdef DEBUG
	printf("String: %d characters\n", input->charcount);
#endif
	output[0]=KS_INVALID;
	tpos = 0;
	opos = 0;
	for (ccount = 0; ccount < input->charcount; ccount++)
	{
		temp[tpos]=get_doted_char(input->chars[ccount].symbol, input->chars[ccount].dotcount);
		tpos++;
		if (input->word_seperator[ccount] || (ccount == input->charcount-1))
		{
			//temp[tpos]=KS_INVALID;
			if (flag_use_dict)
			{
				lookup_dictionary(temp, word, tpos); 
				memcpy(temp, word, sizeof(word));
			}

			if (opos != 0) output[opos++] = XStringToKeysym(KS_STR_SPACE);
			for (count = 0; count < tpos; count++)
				output[opos++] = temp[count];
			
			tpos = 0;
		}	
	}
	
	return opos;
}

/*
 * common.c
 *
 *  Created on: Jan 1, 2014
 *      Author: cody
 */

#include <ncurses.h>
#include <dirent.h>
#include <wordexp.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include <stdlib.h>
#include "common.h"
#include "ttysys_api.h"

void getHelpStr(char* out)
{
	sprintf(out,	"%s v%s\n"
					"Window Management:\n"
					"  h            Split current window horizontally.\n"
					"  v            Split current window vertically.\n"
					"  c            Close current window.\n"
					"  Tab          Move to next window in order of creation.\n"
					"  Arrow Keys   Move to next window on screen in direction pressed.\n"
					"  g            Toggle grid for selected window.\n"
					"  e            Toggle value display in current window's title.\n"
					"  t            Toggle display of current window's title bar.\n"
					"  o            Toggle display of current window's label sidebar.\n"
					"  q            Quit this program.\n"
					"\n"
					"Plugins: (+: while in command entry)\n"
					"  ~            Enter command entry.\n"
					" +ESC          Leave command entry.\n"
					" +ENTER        Run command and leave command entry.\n"
					"Plugin command format:\n"
					"  period name(args)\n"
					"     |     |    +--zero or more strings, separated by comma or space\n"
					"     |     +-------plugin name\n"
					"     +-------------number of 10Hz cycles between calls to the plugin\n"
					"Examples:\n"
					"  10 cpu()\n"
					"  10 cpu(0)\n"
					"  10 watch(\"top -b -n1\")\n"
					"\n"
					"Found:\n",
					AppName, AppVers);
	
	DIR* dp;
	struct dirent* dir;
	char fullname[1000];
	
#ifdef USERPLUGINPATH
	wordexp_t we;
	wordexp(USERPLUGINPATH, &we, WRDE_NOCMD);
	
	int len = 0;
	for(int i = 0; i < we.we_wordc; i++)
		len += strlen(we.we_wordv[i]);
	
	char* userpath = malloc(len+1);
	if(!userpath)
		return;
	
	userpath[0] = '\0';
	for(int i = 0; i < we.we_wordc; i++)
		strcat(userpath, we.we_wordv[i]);

	dp = opendir(userpath);
	if(!dp)
	{
		sprintf(out+strlen(out), "errno: %d\n", errno);
		return;
	}
	while ((dir = readdir(dp)) != NULL)
	{
		int len = strlen(dir->d_name);
		if(len > 3 && !strcmp(dir->d_name + len - 3, ".so"))
		{
			sprintf(fullname, "%s/%s", userpath, dir->d_name);

			void* handle = dlopen(fullname, RTLD_LAZY);
			if(!handle)
				continue;
			
			strcat(out, "  ");
			strncat(out, dir->d_name, len - 3);
			strcat(out, " - ");
			char** desc = dlsym(handle, "shortDesc");
			if(desc)
				strcat(out, *desc);
			strcat(out, "\n");
			
			dlclose(handle);
		}
	}
	closedir(dp);
#endif
	
	dp = opendir(SYSTEMPLUGINPATH);
	if(!dp)
	{
		sprintf(out+strlen(out), "errno: %d\n", errno);
		return;
	}
	while ((dir = readdir(dp)) != NULL)
	{
		int len = strlen(dir->d_name);
		if(len > 3 && !strcmp(dir->d_name + len - 3, ".so"))
		{
			sprintf(fullname, "%s/%s", SYSTEMPLUGINPATH, dir->d_name);

			void* handle = dlopen(fullname, RTLD_LAZY);
			if(!handle)
				continue;
			
			strcat(out, "  ");
			strncat(out, dir->d_name, len - 3);
			strcat(out, " - ");
			char** desc = dlsym(handle, "shortDesc");
			if(desc)
				strcat(out, *desc);
			strcat(out, "\n");
			
			dlclose(handle);
		}
	}
	closedir(dp);
}

/**
 * char* getDirectionString(int num)
 * 	num is a number that represents an offset from the surrounding member of a windowlist struct, in sizeof(struct windowlist*) multiples
 * Helps polish the UI.
 */
char* getDirectionString(int num)
{
	switch(num)
	{
		case 0:
			return "Left";
		case 1:
			return "Right";
		case 2:
			return "Up";
		case 3:
			return "Down";
		default:
			return "";
	}
}

/**
 * void listShiftLeftAdd(float* list, int len, float new)
 * 	list is the list to be shifted.
 * 	len is the length of the list.
 * 	new is a new value to be added.
 * shifts list towards beginning and adds new at end.
 */
void listShiftLeftAdd(float* list, int len, float new)
{
	for(int i = 0; i < len-1; i++)
	{
		list[i] = list[i+1]; // shift
	}
	list[len-1] = new; // add
}

/**
 * void listShiftRightAdd(float* list, int len, float new)
 * 	list is the list to be shifted.
 * 	len is the length of the list.
 * 	new is a new value to be added.
 * shifts list towards end and adds new at beginning.
 */
void listShiftRightAdd(float* list, int len, float new)
{
	for(int i = len - 1; i > 0; i--)
	{
		list[i] = list[i-1]; // shift
	}
	list[0] = new; // add
}

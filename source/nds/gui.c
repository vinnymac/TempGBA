/* unofficial gameplaySP kai
 *
 * Copyright (C) 2006 Exophase <exophase@gmail.com>
 * Copyright (C) 2007 takka <takka@tfact.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public Licens e as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/******************************************************************************
 * gui.c
 * GUI処理
 ******************************************************************************/

/******************************************************************************
 * 头文件
 ******************************************************************************/
#include "common.h"
#include "gui.h"
#include "input.h"
#include "draw.h"
#include "cheats.h"

 //program arguments
char  argv[2][MAX_PATH];

// If adding a language, make sure you update the size of the array in
// message.h too.
char *lang[8] =
	{
		"English",					// 0
		"简体中文",					// 1
		"Français", 					// 2
		"Deutsch",					// 3
		"Nederlands",					// 4
		"Español",					// 5
		"Italiano",					// 6
		"Português (Br.)",				// 7
	};

char *language_options[] = { (char *) &lang[0], (char *) &lang[1], (char *) &lang[2], (char *) &lang[3], (char *) &lang[4], (char *) &lang[5], (char *) &lang[6], (char *) &lang[7] };

/******************************************************************************
 * 宏定义
 ******************************************************************************/
#define STATUS_ROWS 0
#define CURRENT_DIR_ROWS 2
//#define FILE_LIST_ROWS 25
#define FILE_LIST_ROWS 5
#define FILE_LIST_ROWS_CENTER   (FILE_LIST_ROWS/2)
#define FILE_LIST_POSITION 50
//#define DIR_LIST_POSITION 360
#define DIR_LIST_POSITION 130
#define PAGE_SCROLL_NUM 5

#define SUBMENU_ROW_NUM	6

#define SAVE_STATE_SLOT_NUM 16

#define NDSGBA_VERSION "1.0"

#define GPSP_CONFIG_FILENAME "SYSTEM/ndsgba.cfg"

// dsGBA的一套头文件
// 标题 4字节
#define GPSP_CONFIG_HEADER  "NGBA1.0"
#define GPSP_CONFIG_HEADER_SIZE 7
const u32 gpsp_config_ver = 0x00010000;
GPSP_CONFIG gpsp_config;

// GAME的一套头文件
// 标题 4字节
#define GAME_CONFIG_HEADER     "GAME1.0"
#define GAME_CONFIG_HEADER_SIZE 7
#define GAME_CONFIG_HEADER_U32 0x67666367
const u32 game_config_ver = 0x00010000;
GAME_CONFIG game_config;

//save state file map
#define RTS_TIMESTAMP_POS   SVS_HEADER_SIZE
static unsigned int savestate_index; // current selection in the saved states menu
static int latest_save; // Slot number of the latest (in time) save for this game, or -1 if none
static unsigned char SavedStateExistenceCached [SAVE_STATE_SLOT_NUM]; // [I] == TRUE if Cache[I] is meaningful
static unsigned char SavedStateExistenceCache [SAVE_STATE_SLOT_NUM];

#ifdef TEST_MODE
#define VER_RELEASE "test"
#else
#define VER_RELEASE "release"
#endif

#define MAKE_MENU(name, init_function, passive_function, key_function, end_function, \
	focus_option, screen_focus)												  \
  MENU_TYPE name##_menu =                                                     \
  {                                                                           \
    init_function,                                                            \
    passive_function,                                                         \
	key_function,															  \
	end_function,															  \
    name##_options,                                                           \
    sizeof(name##_options) / sizeof(MENU_OPTION_TYPE),                        \
	focus_option,															  \
	screen_focus															  \
  }                                                                           \

#define INIT_MENU(name, init_functions, passive_functions, key, end, focus, screen)\
	{																		  \
    name##_menu.init_function = init_functions,								  \
    name##_menu.passive_function = passive_functions,						  \
	name##_menu.key_function = key,											  \
	name##_menu.end_function = end,											  \
    name##_menu.options = name##_options,									  \
    name##_menu.num_options = sizeof(name##_options) / sizeof(MENU_OPTION_TYPE),\
	name##_menu.focus_option = focus,										  \
	name##_menu.screen_focus = screen;										  \
	}																		  \

#define CHEAT_OPTION(action_function, passive_function, number, line_number)  \
{                                                                             \
  action_function,                                                            \
  passive_function,                                                           \
  NULL,                                                                       \
  &cheat_data_ptr[number],                                                    \
  NULL /* enable_disable_options */,                                                     \
  NULL,				                            	  \
  2,                                                                          \
  NULL,                                                                       \
  line_number,                                                                \
  ACTION_TYPE                                         \
}                                                                             \

#define ACTION_OPTION(action_function, passive_function, display_string,      \
 help_string, line_number)                                                    \
{                                                                             \
  action_function,                                                            \
  passive_function,                                                           \
  NULL,                                                                       \
  display_string,                                                             \
  NULL,                                                                       \
  NULL,                                                                       \
  0,                                                                          \
  help_string,                                                                \
  line_number,                                                                \
  ACTION_TYPE                                                               \
}                                                                             \

#define SUBMENU_OPTION(sub_menu, display_string, help_string, line_number)    \
{                                                                             \
  NULL,                                                                       \
  NULL,                                                                       \
  sub_menu,                                                                   \
  display_string,                                                             \
  NULL,                                                                       \
  NULL,                                                                       \
  sizeof(sub_menu) / sizeof(MENU_OPTION_TYPE),                                \
  help_string,                                                                \
  line_number,                                                                \
  SUBMENU_TYPE                                                              \
}                                                                             \

#define SELECTION_OPTION(passive_function, display_string, options,           \
 option_ptr, num_options, help_string, line_number, type)                     \
{                                                                             \
  NULL,                                                                       \
  passive_function,                                                           \
  NULL,                                                                       \
  display_string,                                                             \
  options,                                                                    \
  option_ptr,                                                                 \
  num_options,                                                                \
  help_string,                                                                \
  line_number,                                                                \
  type                                                                        \
}                                                                             \

#define ACTION_SELECTION_OPTION(action_function, passive_function,            \
 display_string, options, option_ptr, num_options, help_string, line_number,  \
 type)                                                                        \
{                                                                             \
  action_function,                                                            \
  passive_function,                                                           \
  NULL,                                                                       \
  display_string,                                                             \
  options,                                                                    \
  option_ptr,                                                                 \
  num_options,                                                                \
  help_string,                                                                \
  line_number,                                                                \
  type | ACTION_TYPE                                                        \
}                                                                             \

#define STRING_SELECTION_OPTION(action_function, passive_function,            \
    display_string, options, option_ptr, num_options, help_string, action, line_number)\
{                                                                             \
  action_function,                                                            \
  passive_function,                                                           \
  NULL,                                                                       \
  display_string,                                                             \
  options,                                                                    \
  option_ptr,                                                                 \
  num_options,                                                                \
  help_string,                                                                \
  line_number,                                                                \
  STRING_SELECTION_TYPE | action                                              \
}

#define NUMERIC_SELECTION_OPTION(passive_function, display_string,            \
 option_ptr, num_options, help_string, line_number)                           \
  SELECTION_OPTION(passive_function, display_string, NULL, option_ptr,        \
   num_options, help_string, line_number, NUMBER_SELECTION_TYPE)              \

#define STRING_SELECTION_HIDEN_OPTION(action_function, passive_function,      \
 display_string, options, option_ptr, num_options, help_string, line_number)  \
  ACTION_SELECTION_OPTION(action_function, passive_function,                  \
   display_string,  options, option_ptr, num_options, help_string,            \
   line_number, (STRING_SELECTION_TYPE | HIDEN_TYPE))                         \

#define NUMERIC_SELECTION_ACTION_OPTION(action_function, passive_function,    \
 display_string, option_ptr, num_options, help_string, line_number)           \
  ACTION_SELECTION_OPTION(action_function, passive_function,                  \
   display_string,  NULL, option_ptr, num_options, help_string,               \
   line_number, NUMBER_SELECTION_TYPE)                                        \

#define NUMERIC_SELECTION_HIDE_OPTION(action_function, passive_function,      \
    display_string, option_ptr, num_options, help_string, line_number)        \
  ACTION_SELECTION_OPTION(action_function, passive_function,                  \
   display_string, NULL, option_ptr, num_options, help_string,                \
   line_number, NUMBER_SELECTION_TYPE)                                        \


typedef enum
{
	NUMBER_SELECTION_TYPE = 0x01,
	STRING_SELECTION_TYPE = 0x02,
	SUBMENU_TYPE          = 0x04,
	ACTION_TYPE           = 0x08,
	HIDEN_TYPE            = 0x10,
	PASSIVE_TYPE          = 0x00,
} MENU_OPTION_TYPE_ENUM;

struct _MENU_OPTION_TYPE
{
	void (* action_function)();				//Active option to process input
	void (* passive_function)();			//Passive function to process this option
	struct _MENU_TYPE *sub_menu;			//Sub-menu of this option
	char **display_string;					//Name and other things of this option
	void *options;							//output value of this option
	u32 *current_option;					//output values
	u32 num_options;						//Total output values
	char **help_string;						//Help string
	u32 line_number;						//Order id of this option in it menu
	MENU_OPTION_TYPE_ENUM option_type;		//Option types
};

struct _MENU_TYPE
{
	void (* init_function)();				//Function to initialize this menu
	void (* passive_function)();			//Function to draw this menu
	void (* key_function)();				//Function to process input
	void (* end_function)();				//End process of this menu
	struct _MENU_OPTION_TYPE *options;		//Options array
	u32	num_options;						//Total options of this menu
	u32	focus_option;						//Option which obtained focus
	u32	screen_focus;						//screen positon of the focus option
};

typedef struct _MENU_OPTION_TYPE MENU_OPTION_TYPE;
typedef struct _MENU_TYPE MENU_TYPE;

/*
typedef enum
{
  MAIN_MENU,
  GAMEPAD_MENU,
  SAVESTATE_MENU,
  FRAMESKIP_MENU,
  CHEAT_MENU,
  ADHOC_MENU,
  MISC_MENU
} MENU_ENUM;
*/

/******************************************************************************
 * 定义全局变量
 ******************************************************************************/
char g_default_rom_dir[MAX_PATH];
char DEFAULT_SAVE_DIR[MAX_PATH];
char DEFAULT_CFG_DIR[MAX_PATH];
char DEFAULT_SS_DIR[MAX_PATH];
char DEFAULT_CHEAT_DIR[MAX_PATH];
u32 game_fast_forward= 0;   //OFF
u32 SAVESTATE_SLOT = 0;


/******************************************************************************
 * 局部变量的定义
 ******************************************************************************/
static char m_font[MAX_PATH];
static char s_font[MAX_PATH];
static u32 menu_cheat_page = 0;
static u32 gamepad_config_line_to_button[] = { 8, 6, 7, 9, 1, 2, 3, 0, 4, 5, 11, 10 };

/******************************************************************************
 * 本地函数的声明
 ******************************************************************************/
static void get_savestate_filelist(void);
static void get_savestate_filename(u32 slot, char *name_buffer);
static unsigned char SavedStateSquareX (u32 slot);
static unsigned char SavedStateFileExists (u32 slot);
static void SavedStateCacheInvalidate (void);
void get_newest_savestate(char *name_buffer);
static int sort_function(const void *dest_str_ptr, const void *src_str_ptr);
static u32 parse_line(char *current_line, char *current_str);
static void print_status(u32 mode);
static void get_timestamp_string(char *buffer, u16 msg_id, u16 year, u16 mon, u16 day, u16 wday, u16 hour, u16 min, u16 sec, u32 msec);
#if 0
static void get_time_string(char *buff, struct rtc *rtcp);
#endif
static u32 save_ss_bmp(u16 *image);
static void init_default_gpsp_config();

void game_disableAudio();
void game_set_frameskip();

static int sort_function(const void *dest_str_ptr, const void *src_str_ptr)
{
  char *dest_str = *((char **)dest_str_ptr);
  char *src_str = *((char **)src_str_ptr);

  if(src_str[0] == '.')
    return 1;

  if(dest_str[0] == '.')
    return -1;

  return strcasecmp(dest_str, src_str);
}

int sort_num= 0;

static int my_array_partion(void *array, int left, int right)
{
    unsigned int pivot= *((unsigned int*)array + left);

    while(left < right)
    {
        while(sort_function((void*)((unsigned int*)array+left), (void*)((unsigned int *)array+right)) < 0) {
            right--;
        }

        if(right== left) break;
        *((unsigned int*)array + left) = *((unsigned int*)array + right);
        *((unsigned int*)array + right) = pivot;

        if(left < right)
        {
            left++;
            if(right== left) break;
        }

        while(sort_function((void*)((unsigned int*)array+right), (void*)((unsigned int *)array+left)) > 0) {
            left++;
        }

        if(left== right) break;
        *((unsigned int*)array + right) = *((unsigned int*)array + left);
        *((unsigned int*)array + left) = pivot;
        right--;
    }

    return left;
}

void my_qsort(void *array, int left, int right)
{
    if(left < right)
    {
        int mid= my_array_partion(array, left, right);
        my_qsort(array, left, mid-1);
        my_qsort(array, mid+1, right);
    }
}

static void strupr(char *str)
{
    while(*str)
    {
        if(*str <= 0x7A && *str >= 0x61) *str -= 0x20;
        str++;
    }
}
/******************************************************************************
 * 全局函数定义
 ******************************************************************************/
//#define FS_FAT_ATTR_DIRECTORY   0x10
#define FILE_LIST_MAX  512
#define DIR_LIST_MAX   64
#define NAME_MEM_SIZE  (320*64)

/*
*	Function: internal function to manage FILE_LIST_INFO structure
*	filelist_infop: a pointer to a predefined FILE_LIST_INFO structure
*	flag: 0	initialize the structure
*			1 increase filename memory size
*			2 increase file name buffer size
*			4 increase directory name buffer size
*			-1 free all allocated memroy
*			other value are invalide
*	return: -1 on failure
*/
static int manage_filelist_info(struct FILE_LIST_INFO *filelist_infop, int flag)
{
	//Initialize
	if(0 == flag)
	{
		filelist_infop->file_list = (char**)malloc(FILE_LIST_MAX*4);
		if(NULL == filelist_infop->file_list)
			return -1;

		filelist_infop->dir_list = (char**)malloc(DIR_LIST_MAX*4);
		if(NULL == filelist_infop->dir_list)
		{
			free((void*)filelist_infop->file_list);
			return -1;
		}

		filelist_infop->filename_mem = (char*)malloc(NAME_MEM_SIZE);
		if(NULL == filelist_infop->filename_mem)
		{
			free((void*)filelist_infop->file_list);
			free((void*)filelist_infop->dir_list);
			return -1;
		}

		filelist_infop->mem_size = NAME_MEM_SIZE;
		filelist_infop->file_size = FILE_LIST_MAX;
		filelist_infop->dir_size = DIR_LIST_MAX;
		return 0;
	}
	//free all memroy
	if(-1 == flag)
	{
		free((void*)filelist_infop->file_list);
		free((void*)filelist_infop->dir_list);
		free((void*)filelist_infop->filename_mem);
		return 0;
	}

	int i;
	void *pt;

	//Increase all
	if(flag & 0x1)
	{
		i = NAME_MEM_SIZE;

		do
		{
			pt = (void*)realloc(filelist_infop->filename_mem, filelist_infop->mem_size+i);
			if(NULL == pt) i /= 2;
		} while(i > 256);

		if(NULL == pt) return -1;

		filelist_infop->mem_size += i;
		filelist_infop->filename_mem = (char*)pt;
		i = (int)pt - (int)filelist_infop->file_list;

		int k;
		char **m;

		k = 0;
		m = filelist_infop->file_list;
		for(k= 0; k < filelist_infop->file_num; k++)
			m[k] += i;

		k = 0;
		m = filelist_infop->dir_list;
		for(k = 0; k < filelist_infop->dir_num; k++)
			m[k] += i;
	}
	//Increase file name buffer
	if(flag & 0x2)
	{
		i = filelist_infop->file_size + FILE_LIST_MAX;

		pt = (void*)realloc(filelist_infop->file_list, i*4);
		if(NULL == pt) return -1;

		filelist_infop->file_list = (char**)pt;
		filelist_infop->file_size = i;
	}
	//Increase directory name buffer
	if(flag & 0x4)
	{
		i = filelist_infop->dir_size + DIR_LIST_MAX;

		pt = (void*)realloc(filelist_infop->dir_list, i*4);
		if(NULL == pt) return -1;

		filelist_infop->dir_list = (char**)pt;
		filelist_infop->dir_size = i;
	}

	return 0;
}

static int load_file_list(struct FILE_LIST_INFO *filelist_infop)
{
    DIR* current_dir;
    char    current_dir_name[MAX_PATH];
    dirent *current_file;
	struct stat st;
    char *file_name;
	unsigned int len;
    unsigned int file_name_length;
    char* name_mem_base;
    char **file_list;
    char **dir_list;
	unsigned int mem_size;
	unsigned int file_size;
	unsigned int dir_size;
    unsigned int num_files;
    unsigned int num_dirs;
    char **wildcards;

    if(filelist_infop -> current_path == NULL)
        return -1;

    name_mem_base = &(filelist_infop -> filename_mem[0]);
    mem_size = filelist_infop -> mem_size;
	file_size = filelist_infop -> file_size;
	dir_size = filelist_infop -> dir_size;
    file_list = filelist_infop -> file_list;
    dir_list = filelist_infop -> dir_list;
    num_files = 0;
    num_dirs = 0;
    wildcards = filelist_infop -> wildcards;

    strcpy(current_dir_name, filelist_infop -> current_path);

	//	path formate should be: "fat:/" or "fat:/dir0" or "fat:", not "fat:/dir0/"
    current_dir = opendir(current_dir_name);
	//Open directory faiure
	if(current_dir == NULL) {
		return -1;
	}

	file_name_length = 0;
	//while((current_file = readdir(current_dir)) != NULL)
	while((current_file = readdir_ex(current_dir, &st)) != NULL)
    {
		//lstat(current_file->d_name, &st);
		file_name = current_file->d_name;

		len = strlen(file_name) +1;
		if((file_name_length+len) > mem_size)
		{
			//get more memory
			if(manage_filelist_info(filelist_infop, 1) == -1)
				break;

			name_mem_base = &(filelist_infop -> filename_mem[0]);
		    mem_size = filelist_infop -> mem_size;
		}
		//Increase file_list
		if(num_files >= file_size) {
			if(manage_filelist_info(filelist_infop, 2) == -1)
				break;
			file_size = filelist_infop -> file_size;
		}
		//Increase dir_list
		if(num_dirs >= dir_size) {
			if(manage_filelist_info(filelist_infop, 4) == -1)
				break;
			dir_size = filelist_infop -> dir_size;
		}

		//If dirctory
		if(S_ISDIR(st.st_mode))
		{
			if(file_name[0] != '.')
			{
				dir_list[num_dirs] = name_mem_base + file_name_length;
				strcpy(dir_list[num_dirs], file_name);
				num_dirs++;
				file_name_length += len;
			}
			//take ".." directory as file
			else if(file_name[1] == '.')
			{
				file_list[num_files] = name_mem_base + file_name_length;
				strcpy(file_list[num_files], file_name);
				num_files++;
				file_name_length += len;
			}
		}
		else
		{
			char *ext_pos;
			unsigned int i;

			ext_pos = (char*)strrchr((const char*)file_name, '.');
			if(NULL != ext_pos)
			for(i = 0; wildcards[i] != NULL; i++)
			{
				if(!strcasecmp(ext_pos, wildcards[i]))
				{
					file_list[num_files] = name_mem_base + file_name_length;
					strcpy(file_list[num_files], file_name);
					num_files++;
					file_name_length += len;
					break;
				}
			}
		}
    }

    closedir(current_dir);

	filelist_infop -> file_num = num_files;
	filelist_infop -> dir_num = num_dirs;
	//printf("%s: num_files %d; num_dirs %d\n", filelist_infop ->current_path, num_files, num_dirs);
#if 0
    my_qsort((void *)file_list, 0, num_files-1);
#else	//to support ".." directory, but take it as file
    my_qsort((void **)file_list, 1, num_files-1);
#endif
    my_qsort((void **)dir_list, 0, num_dirs-1);

    return 0;
}

/*--------------------------------------------------------
  读取文件
--------------------------------------------------------*/

s32 load_file(char **wildcards, char *result, char *default_dir_name)
{
	struct FILE_LIST_INFO filelist_info;
	u32 repeat;
	int return_value;
	gui_action_type gui_action;
	u32 selected_item_on_list;
	u32 selected_item_on_screen;
	u32 to_update_filelist;
	u32 total_items_num;
	int redraw;
    u32 path_scroll;
    u32 num_files;						//File numbers on the directory
    u32 num_dirs;                       //Directory numbers on the dirctory
    char **file_list;
    char **dir_list;
    int flag;

	//Have no path
	if(NULL == default_dir_name)
		return -1;

	//construct filelist_info struct
	if(-1 == manage_filelist_info(&filelist_info, 0))
		return -1;

	result[0] = '\0';
	strcpy(filelist_info.current_path, default_dir_name);

	filelist_info.wildcards = wildcards;
	filelist_info.file_num = 0;
	filelist_info.dir_num = 0;

	flag = load_file_list(&filelist_info);
	if(-1 == flag)
	{
		//deconstruct filelist_info struct
		manage_filelist_info(&filelist_info, -1);
		return -1;
	}

	num_files = filelist_info.file_num;
	num_dirs = filelist_info.dir_num;
	file_list = filelist_info.file_list;
	dir_list = filelist_info.dir_list;

	selected_item_on_list = 0;			//Selected item on list
	selected_item_on_screen = 0;		//Selected item on screen's position
	redraw = -1;						//redraw all
	total_items_num = num_files + num_dirs;
	to_update_filelist = 0;             //to load new file list
	path_scroll = 0x8000;				//path scroll method, first scroll left

	repeat= 1;
	return_value = -1;
	while(repeat)
	{
		struct key_buf inputdata;
		gui_action = get_gui_input();
		int mod;

		// Get KEY_RIGHT and KEY_LEFT separately to allow scrolling
		// the selected file name faster.
		ds2_getrawInput(&inputdata);
		if (inputdata.key & KEY_RIGHT)
			redraw = -3;
		else if (inputdata.key & KEY_LEFT)
			redraw = 3;

		switch(gui_action)
		{
			case CURSOR_TOUCH:
				ds2_getrawInput(&inputdata);
				wait_Allkey_release(0);
				// ___ 33        This screen has 6 possible rows. Touches
				// ___ 60        above or below these are ignored.
				// . . . (+27)
				// ___ 192
				if(inputdata.y <= 33 || inputdata.y > 192)
					break;
				else
					mod = (inputdata.y - 33) / 27;

				if(selected_item_on_list - selected_item_on_screen + mod >= total_items_num)
					break;

				selected_item_on_list = selected_item_on_list - selected_item_on_screen + mod;

				if(selected_item_on_list + 1 <= num_files)
				{
					//The ".." directory is the parent
					if(!strcasecmp(file_list[selected_item_on_list], ".."))
					{
						char *ext_pos;

						strcpy(filelist_info.current_path, default_dir_name);
						ext_pos = strrchr(filelist_info.current_path, '/');
						if(NULL != ext_pos)	//Not root directory
						{
							*ext_pos = '\0';
							to_update_filelist= 1;
						}
					}
					else
					{
						repeat = 0;
						return_value = 0;
						strcpy(default_dir_name, filelist_info.current_path);
						strcpy(result, file_list[selected_item_on_list]);
					}
				}
				//directory selected
				else if(num_dirs > 0)
				{
					unsigned int m;

					m = selected_item_on_list - num_files;
					strcpy(filelist_info.current_path, default_dir_name);
					strcat(filelist_info.current_path, "/");
					strcat(filelist_info.current_path, dir_list[m]);
					to_update_filelist= 1;
				}
				break;
			case CURSOR_UP:
				redraw = 1;
				if(selected_item_on_screen > 0)
				{
					//Not the first item on list
					selected_item_on_list -= 1;
					//Selected item on screen center
					if(FILE_LIST_ROWS_CENTER == selected_item_on_screen) {
						if(selected_item_on_screen > selected_item_on_list)
							selected_item_on_screen -= 1;
					}
					//Selected item on down half screen
					else if(FILE_LIST_ROWS_CENTER < selected_item_on_screen) {
						selected_item_on_screen -= 1;
					}
					//Selected item on up half screen
					else {
						if(selected_item_on_screen < selected_item_on_list)
							selected_item_on_screen += 1;
						else if(selected_item_on_screen > selected_item_on_list)
							selected_item_on_screen -= 1;
					}
				}
				else if(selected_item_on_screen > selected_item_on_list) {
					selected_item_on_screen -= 1;
				}
				else
					redraw = 0;

				break;

			case CURSOR_DOWN:
			{
				unsigned int m, n;
				m = total_items_num - (selected_item_on_list + 1);
				n = FILE_LIST_ROWS - (FILE_LIST_ROWS_CENTER + 1);
				//Selected item on screen center
				if(FILE_LIST_ROWS_CENTER == selected_item_on_screen) {
					if(m > 0 && m < n)
						selected_item_on_screen += 1;
					redraw = 1;
				}
				//Selected item on down half screen
				else if(FILE_LIST_ROWS_CENTER < selected_item_on_screen) {
					if(m > 0) {
						redraw = 1;
						if(m <= n) selected_item_on_screen += 1;
						else if(m > n) selected_item_on_screen -= 1;
						else redraw = 0;	//cancel
					}
				}
				//Selected item on up half screen
				else {
					if(m > 0) {
						selected_item_on_screen += 1;
						redraw = 1;
					}
				}

				//Not reach the last item
				if((selected_item_on_list + 1) < total_items_num)
					selected_item_on_list += 1;

				break;
			}
			//scroll page down
			case CURSOR_RTRIGGER:
			{
				unsigned int m, n;
				//first item on screen in item list's position
				m = selected_item_on_list - selected_item_on_screen;
				n = total_items_num - (m+1);
				//there are more than 1 page after the m item
				if(n >= FILE_LIST_ROWS) {
					m += FILE_LIST_ROWS -1;
					selected_item_on_list = m;

					m = total_items_num - (selected_item_on_list +1);
					if(m >= (FILE_LIST_ROWS_CENTER +1))
						selected_item_on_screen = FILE_LIST_ROWS_CENTER;
					else
						selected_item_on_screen = 0;

					selected_item_on_list += selected_item_on_screen;
					redraw = 1;
				}

				break;
			}
			//scroll page up
			case CURSOR_LTRIGGER:
			{
				unsigned int m;
				//first item on screen in item list's position
				m = selected_item_on_list - selected_item_on_screen;
				//there are more than 1 page befroe the m item
				if((m+1) >= FILE_LIST_ROWS)
				{
					m -= FILE_LIST_ROWS -1;
					selected_item_on_list = m;

					selected_item_on_screen = FILE_LIST_ROWS_CENTER;
					selected_item_on_list += selected_item_on_screen;
					redraw = 1;
				}

				break;
			}
			//scroll string (see above the switch)
			case CURSOR_RIGHT:
				/* fall through */
			//scroll string (see above the switch)
			case CURSOR_LEFT:
				break;

			case CURSOR_SELECT:
				wait_Allkey_release(0);
				//file selected
				if(selected_item_on_list + 1 <= num_files)
				{
					//The ".." directory
					if(!strcasecmp(file_list[selected_item_on_list], ".."))
					{
						char *ext_pos;

						strcpy(filelist_info.current_path, default_dir_name);
						ext_pos = strrchr(filelist_info.current_path, '/');
						if(NULL != ext_pos)	//Not root directory
						{
							*ext_pos = '\0';
							to_update_filelist= 1;
						}
					}
					else
					{
						repeat = 0;
						return_value = 0;
						strcpy(default_dir_name, filelist_info.current_path);
						strcpy(result, file_list[selected_item_on_list]);
					}
				}
				//directory selected
				else if(num_dirs > 0)
				{
					unsigned int m;

					m = selected_item_on_list - num_files;
					strcpy(filelist_info.current_path, default_dir_name);
					strcat(filelist_info.current_path, "/");
					strcat(filelist_info.current_path, dir_list[m]);
					to_update_filelist= 1;
				}
				break;

			case CURSOR_BACK:
			{
				wait_Allkey_release(0);
				char *ext_pos;

				strcpy(filelist_info.current_path, default_dir_name);
				ext_pos = strrchr(filelist_info.current_path, '/');
				if(NULL != ext_pos)	//Not root directory
				{
					*ext_pos = '\0';
					to_update_filelist= 1;
				}
				else { //is root directory
					return_value = -1;
					repeat = 0;
				}
				break;
			}

			case CURSOR_EXIT:
				wait_Allkey_release(0);
				return_value = -1;
				repeat = 0;
				break;

			default:
				break;
		} //end switch

		if(to_update_filelist)
		{
			flag = load_file_list(&filelist_info);
			if(-1 != flag) {
				strcpy(default_dir_name, filelist_info.current_path);
				num_files = filelist_info.file_num;
				num_dirs = filelist_info.dir_num;
				total_items_num = num_files + num_dirs;;

				selected_item_on_list = 0;			//Selected item on list
				selected_item_on_screen = 0;		//Selected item on screen's position

				redraw = -1;						//redraw all
			}
			to_update_filelist = 0;
		}

		//redraw, code reuse
		if(-1 == redraw || 1 == redraw)
		{
			unsigned int m, n, k;
			char *pt;
			unsigned short color;

			//draw background
			show_icon(down_screen_addr, &ICON_SUBBG, 0, 0);
			show_icon(down_screen_addr, &ICON_TITLE, 0, 0);
			show_icon(down_screen_addr, &ICON_TITLEICON, 12, 9);

			//release data struct to draw scrolling string
			//Path
			if(-1 == redraw) {
				draw_hscroll_over(0);
				draw_hscroll_init(down_screen_addr, 49, 10, 199, COLOR_TRANS,
					COLOR_WHITE, default_dir_name);
				path_scroll = 0x8000;		//first scroll left
			}

			for(m = 0; m < MAX_SCROLL_STRING; m++)
				draw_hscroll_over(m+1);

			//file and directory list
			//the first displayed item
			m = selected_item_on_list - selected_item_on_screen;
			//nubers of item to displayed
			n = total_items_num - m;
			if(n > FILE_LIST_ROWS) n = FILE_LIST_ROWS;

			for(k= 0; k < n; k++, m++)
			{
				if(k == selected_item_on_screen) {
					color = COLOR_ACTIVE_ITEM;
					show_icon(down_screen_addr, &ICON_SUBSELA, 6, 35 + k*27);
				}
				else
					color = COLOR_INACTIVE_ITEM;

				//directorys
				if((m+1) > num_files) {
					show_icon(down_screen_addr, &ICON_DIRECTORY, FILE_SELECTOR_ICON_X, 37 + k*27);
					pt = dir_list[m - num_files];
				}
				//files
				else {
					pt= strrchr(file_list[m], '.');

					if(!strcasecmp(pt, ".gba"))
						show_icon(down_screen_addr, &ICON_GBAFILE, FILE_SELECTOR_ICON_X, 37 + k*27);
					else if(!strcasecmp(pt, ".zip"))
						show_icon(down_screen_addr, &ICON_ZIPFILE, FILE_SELECTOR_ICON_X, 37 + k*27);
					else if(!strcasecmp(pt, ".cht"))
						show_icon(down_screen_addr, &ICON_CHTFILE, FILE_SELECTOR_ICON_X, 37 + k*27);
					else if(!strcasecmp(file_list[m], ".."))
						show_icon(down_screen_addr, &ICON_DOTDIR, FILE_SELECTOR_ICON_X, 37 + k*27);
					else //Not recoganized file
						show_icon(down_screen_addr, &ICON_UNKNOW, FILE_SELECTOR_ICON_X, 37 + k*27);

					pt = file_list[m];
				}

				draw_hscroll_init(down_screen_addr, FILE_SELECTOR_NAME_X, 40 + k*27, FILE_SELECTOR_NAME_SX,
					COLOR_TRANS, color, pt);
			}

			redraw = 0;
		} //end if(0 != redraw)
		else if(0 != redraw) {
			unsigned int m, n;
			char *pt;

			m = selected_item_on_screen;
			show_icon(down_screen_addr, &ICON_SUBSELA, 6, 35 + m*27);

			n = selected_item_on_list;
			if((n+1) > num_files)
				show_icon(down_screen_addr, &ICON_DIRECTORY, FILE_SELECTOR_ICON_X, 37 + m*27);
			else {
				pt= strrchr(file_list[n], '.');

				if(!strcasecmp(pt, ".gba"))
					show_icon(down_screen_addr, &ICON_GBAFILE, FILE_SELECTOR_ICON_X, 37 + m*27);
				else if(!strcasecmp(pt, ".zip"))
					show_icon(down_screen_addr, &ICON_ZIPFILE, FILE_SELECTOR_ICON_X, 37 + m*27);
				else if(!strcasecmp(pt, ".cht"))
					show_icon(down_screen_addr, &ICON_CHTFILE, FILE_SELECTOR_ICON_X, 37 + m*27);
				else if(!strcasecmp(file_list[m], ".."))
					show_icon(down_screen_addr, &ICON_DOTDIR, FILE_SELECTOR_ICON_X, 37 + m*27);
				else //Not recoganized file
					show_icon(down_screen_addr, &ICON_UNKNOW, FILE_SELECTOR_ICON_X, 37 + m*27);
			}

			draw_hscroll(m+1, redraw);
			redraw = 0;
		}

		//Path auto scroll
		unsigned int m = path_scroll & 0xFF;
		if(m < 20) //pause 0.5sec
			path_scroll += 1;
		else {
			show_icon(down_screen_addr, &ICON_TITLE, 0, 0);
			show_icon(down_screen_addr, &ICON_TITLEICON, 12, 9);

			if(path_scroll & 0x8000)	//scroll left
			{
				if(draw_hscroll(0, -1) <= 1) path_scroll = 0;	 //scroll right
			}
			else
			{
				if(draw_hscroll(0, 1) <= 1) path_scroll = 0x8000; //scroll left
			}
		}

		ds2_flipScreen(DOWN_SCREEN, DOWN_SCREEN_UPDATE_METHOD);

		mdelay(20);			//about 50ms
	} //end while(repeat)

	unsigned int i;
	for(i= 0; i < (FILE_LIST_ROWS +1); i++)
		draw_hscroll_over(i);

	//deconstruct filelist_info struct
	manage_filelist_info(&filelist_info, -1);

	// ds2_clearScreen(DOWN_SCREEN, COLOR_BLACK);
	// ds2_flipScreen(DOWN_SCREEN, DOWN_SCREEN_UPDATE_METHOD);

	return return_value;
}

/*--------------------------------------------------------
  放映幻灯片
--------------------------------------------------------*/
u32 play_screen_snapshot(void)
{
    struct FILE_LIST_INFO filelist_info;
    char *file_ext[] = { ".bmp", NULL };
    u32 file_num;
    char **file_list;
    s32 flag;
    u32 repeat, i;
    u16 *screenp;
    u32 color_bg;

    screenp= (u16*)malloc(256*192*2);
    if(screenp == NULL)
    {
        screenp = (u16*)down_screen_addr;
        color_bg = COLOR_BG;
    }
    else
    {
        memcpy(screenp, down_screen_addr, 256*192*2);
        color_bg = COLOR16(43, 11, 11);
    }

	//construct filelist_info struct
	if(-1 == manage_filelist_info(&filelist_info, 0))
		return -1;

	strcpy(filelist_info.current_path, DEFAULT_SS_DIR);

	filelist_info.wildcards = file_ext;
	filelist_info.file_num = 0;
	filelist_info.dir_num = 0;

	flag = load_file_list(&filelist_info);
	if(-1 == flag)
	{
		//construct filelist_info struct
		manage_filelist_info(&filelist_info, -1);
		return -1;
	}

    flag = load_file_list(&filelist_info);
    file_num = filelist_info.file_num;
    file_list = filelist_info.file_list;

    if(flag < 0 || file_num== 0)
    {
        draw_message(down_screen_addr, screenp, 28, 31, 227, 165, color_bg);
        draw_string_vcenter(down_screen_addr, 36, 55, 190, COLOR_MSSG, msg[MSG_NO_SLIDE]);
        ds2_flipScreen(DOWN_SCREEN, DOWN_SCREEN_UPDATE_METHOD);

        if(screenp) free((void*)screenp);
        //construct filelist_info struct
		manage_filelist_info(&filelist_info, -1);

        if(draw_yesno_dialog(DOWN_SCREEN, 115, msg[MSG_GENERAL_CONFIRM_WITH_A], msg[MSG_GENERAL_CANCEL_WITH_B]))
            return 1;
        else
            return 0;
    }

    char bmp_path[MAX_PATH];
	BMPINFO SbmpInfo;

    u32 buff[256*192];
    u32 time0= 10;
    u32 pause= 0;
	unsigned int type;

    draw_message(down_screen_addr, screenp, 28, 31, 227, 165, color_bg);
    draw_string_vcenter(down_screen_addr, 36, 70, 190, COLOR_MSSG, msg[MSG_PLAY_SLIDE1]);
    draw_string_vcenter(down_screen_addr, 36, 85, 190, COLOR_MSSG, msg[MSG_PLAY_SLIDE2]);
    draw_string_vcenter(down_screen_addr, 36, 100, 190, COLOR_MSSG, msg[MSG_PLAY_SLIDE3]);
    draw_string_vcenter(down_screen_addr, 36, 115, 190, COLOR_MSSG, msg[MSG_PLAY_SLIDE4]);
    draw_string_vcenter(down_screen_addr, 36, 130, 190, COLOR_MSSG, msg[MSG_PLAY_SLIDE5]);
    draw_string_vcenter(down_screen_addr, 36, 145, 190, COLOR_MSSG, msg[MSG_PLAY_SLIDE6]);
    ds2_flipScreen(DOWN_SCREEN, DOWN_SCREEN_UPDATE_METHOD);

    repeat= 1;
    i= 0;
    while(repeat)
    {
        sprintf(bmp_path, "%s/%s", filelist_info.current_path, file_list[i]);

		flag = openBMP(&SbmpInfo, (const char*)bmp_path);
		if(flag == BMP_OK)
		{
			type = SbmpInfo.bmpHead.bfImghead.imBitpixel >>3;
			if(2 == type || 3 == type)	//2 bytes per pixel
			{
			    u16 *dst, *dst0;
			    u32 w, h, x, y;

				w= SbmpInfo.bmpHead.bfImghead.imBitmapW;
				h= SbmpInfo.bmpHead.bfImghead.imBitmapH;
				if(w > 256) w = 256;
				if(h > 192) h = 192;

				flag = readBMP(&SbmpInfo, 0, 0, w, h, (void*)buff);
				if(0 == flag)
				{
					ds2_clearScreen(UP_SCREEN, 0);

					dst= (unsigned short*)up_screen_addr + (192-h)/2*256 +(256-w)/2;
					dst0 = dst;
					if(2 == type) {
						unsigned short* pt;

						pt = (unsigned short*) buff;
						for(y= 0; y<h; y++) {
	    	            	for(x= 0; x < w; x++) {
		    	                *dst++ = RGB16_15(pt);
		    	                pt += 1;
		    	            }
							dst0 += 256;
							dst = dst0;
						}
					}
					else {
						unsigned char* pt;

						pt= (char*)buff;
	    		        for(y= 0; y< h; y++) {
    			            for(x= 0; x < w; x++) {
    			                *dst++ = RGB24_15(pt);
    			                pt += 3;
    			            }
							dst0 += 256;
							dst = dst0;
    			        }
					}

					ds2_flipScreen(UP_SCREEN, UP_SCREEN_UPDATE_METHOD);
				}

				closeBMP(&SbmpInfo);
			}
			else
				closeBMP(&SbmpInfo);
		}

        if(i+1 < file_num) i++;
        else i= 0;

        gui_action_type gui_action;
        u32 ticks= 0;
        u32 time1;

        time1= time0;
        while(ticks < time1)
        {
            gui_action = get_gui_input();

            switch(gui_action)
            {
                case CURSOR_UP:
                    if(!pause)
                    {
                        if(time0 > 1) time0 -= 1;
                        time1= time0;
                    }
                    break;

                case CURSOR_DOWN:
                    if(!pause)
                    {
                        time0 += 1;
                        time1= time0;
                    }
                    break;

                case CURSOR_LEFT:
                    time1 = ticks;
                    if(i > 1) i -= 2;
                    else if(i == 1) i= file_num -1;
                    else i= file_num -2;
                    break;

                case CURSOR_RIGHT:
                    time1 = ticks;
                    break;

                case CURSOR_SELECT:
                    if(!pause)
                    {
                        time1 = -1;
                        pause= 1;
                    }
                    else
                    {
                        time1 = ticks;
                        pause= 0;
                    }
                    break;

                case CURSOR_BACK:
                    if(screenp) free((void*)screenp);
                    //deconstruct filelist_info struct
					manage_filelist_info(&filelist_info, -1);
					repeat = 0;
					break;

                default: gui_action= CURSOR_NONE;
                    break;
            }

			if(gui_action != CURSOR_BACK)
				mdelay(100);
            if(!pause)
                ticks ++;
        }
    }

	return 0;
}

/*--------------------------------------------------------
  搜索指定名称的目录
--------------------------------------------------------*/


/*
*	Function: search directory on directory_path
*	directory: directory name will be searched
*	directory_path: path, note that the buffer which hold directory_path should
*		be large enough for nested
*	return: 0= found, directory lay on directory_path
*/
int search_dir(char *directory, char* directory_path)
{
    DIR *current_dir;
    dirent *current_file;
	struct stat st;
    int directory_path_len;

    current_dir= opendir(directory_path);
    if(current_dir == NULL)
        return -1;

	directory_path_len = strlen(directory_path);

	//while((current_file = readdir(current_dir)) != NULL)
	while((current_file = readdir_ex(current_dir, &st)) != NULL)
	{
		//Is directory
		if(S_ISDIR(st.st_mode))
		{
			if(strcmp(".", current_file->d_name) || strcmp("..", current_file->d_name))
				continue;

			strcpy(directory_path+directory_path_len, current_file->d_name);

			if(!strcasecmp(current_file->d_name, directory))
			{//dirctory find
				closedir(current_dir);
				return 0;
			}

			if(search_dir(directory, directory_path) == 0)
			{//dirctory find
				closedir(current_dir);
				return 0;
			}

			directory_path[directory_path_len] = '\0';
		}
    }

    closedir(current_dir);
    return -1;
}

//标准按键
const u32 gamepad_config_map_init[MAX_GAMEPAD_CONFIG_MAP] =
{
    BUTTON_ID_A,        /* 0    A */
    BUTTON_ID_B,        /* 1    B */
    BUTTON_ID_SELECT,   /* 2    [SELECT] */
    BUTTON_ID_START,    /* 3    [START] */
    BUTTON_ID_RIGHT,    /* 4    → */
    BUTTON_ID_LEFT,     /* 5    ← */
    BUTTON_ID_UP,       /* 6    ↑ */
    BUTTON_ID_DOWN,     /* 7    ↓ */
    BUTTON_ID_R,        /* 8    [R] */
    BUTTON_ID_L,        /* 9    [L] */
    BUTTON_ID_NONE,     /* 10   FA */
    BUTTON_ID_NONE,     /* 11   FB */
    BUTTON_ID_X,        /* 12   MENU */
    BUTTON_ID_NONE,     /* 13   NONE */
    BUTTON_ID_NONE,     /* 14   NONE */
    BUTTON_ID_NONE      /* 15   NONE */
};

#define BUTTON_MAP_A    gamepad_config_map[0]
#define BUTTON_MAP_B    gamepad_config_map[1]
#define BUTTON_MAP_FA   gamepad_config_map[10]
#define BUTTON_MAP_FB   gamepad_config_map[11]
#define BUTTON_MAP_MENU gamepad_config_map[12]

/* △ ○ × □ ↓ ← ↑ → */

/*--------------------------------------------------------
  game cfg的初始化
--------------------------------------------------------*/
void init_game_config()
{
    u32 i;
    game_config.frameskip_type = 1;   //auto
    game_config.frameskip_value = 2;
    game_config.clock_speed_number = 3;
    game_config.audio_buffer_size_number = 15;
    game_config.update_backup_flag = 0;
    for(i = 0; i < MAX_CHEATS; i++)
    {
	// Cheats are temporarily disabled [Neb]
#if 0
        game_config.cheats_flag[i].cheat_active = NO;
        game_config.cheats_flag[i].cheat_name[0] = '\0';
#endif
    }
    memcpy(game_config.gamepad_config_map, gamepad_config_map_init, sizeof(gamepad_config_map_init));
    game_config.use_default_gamepad_map = 1;
    game_config.gamepad_config_home = BUTTON_ID_X;
    memcpy(gamepad_config_map, game_config.gamepad_config_map, sizeof(game_config.gamepad_config_map));
    gamepad_config_home = game_config.gamepad_config_home;
}

/*--------------------------------------------------------
  gpSP cfg的初始化
--------------------------------------------------------*/
void init_default_gpsp_config()
{
//  int temp;
  game_config.frameskip_type = 1;   //auto
  game_config.frameskip_value = 2;
  gpsp_config.screen_ratio = 1; //orignal
  gpsp_config.enable_audio = 1; //on
  //keypad configure
  memcpy(gpsp_config.gamepad_config_map, gamepad_config_map_init, sizeof(gamepad_config_map_init));
  memcpy(gamepad_config_map, gamepad_config_map_init, sizeof(gamepad_config_map_init));
  gpsp_config.language = 0;     //defalut language= English
#if 1
  gpsp_config.emulate_core = ASM_CORE;
#else
  gpsp_config.emulate_core = C_CORE;
#endif
    gpsp_config.gamepad_config_home = BUTTON_ID_X;
  gpsp_config.debug_flag = NO;
  gpsp_config.fake_fat = NO;
  gpsp_config.rom_file[0]= 0;
  gpsp_config.rom_path[0]= 0;
  memset(gpsp_config.latest_file, 0, sizeof(gpsp_config.latest_file));

  //Removing rom_path due to confusion
  //strcpy(rom_path, main_path);
  gamepak_filename[0] = '\0';
}

/*--------------------------------------------------------
  game cfgファイルの読込み
  3/4修正
--------------------------------------------------------*/
void load_game_config_file(void)
{
    char game_config_filename[MAX_PATH];
    FILE_ID game_config_file;
    char *pt;

    // 设置初始值
    init_game_config();

    sprintf(game_config_filename, "%s/%s", DEFAULT_CFG_DIR, gamepak_filename);
    pt= strrchr(game_config_filename, '.');
    *pt= 0;
    strcat(game_config_filename, "_0.rts");

    FILE_OPEN(game_config_file, game_config_filename, READ);
    if(FILE_CHECK_VALID(game_config_file))
    {
        //Check file header
        pt= game_config_filename;
        FILE_READ(game_config_file, pt, NGBARTS_HEADERA_SIZE);

        if (!strncmp(pt, NGBARTS_HEADERA, NGBARTS_HEADERA_SIZE))
        {
            FILE_READ_VARIABLE(game_config_file, game_config);
            memcpy(gamepad_config_map, game_config.gamepad_config_map, sizeof(game_config.gamepad_config_map));
            gamepad_config_home = game_config.gamepad_config_home;
        }
        FILE_CLOSE(game_config_file);
    }
}

/*--------------------------------------------------------
  gpSP cfg配置文件
--------------------------------------------------------*/
s32 load_config_file()
{
    char gpsp_config_path[MAX_PATH];
    FILE_ID gpsp_config_file;
    char *pt;

    sprintf(gpsp_config_path, "%s/%s", main_path, GPSP_CONFIG_FILENAME);
    FILE_OPEN(gpsp_config_file, gpsp_config_path, READ);

    if(FILE_CHECK_VALID(gpsp_config_file))
    {
        // check the file header
        pt= gpsp_config_path;
        FILE_READ(gpsp_config_file, pt, GPSP_CONFIG_HEADER_SIZE);
        pt[GPSP_CONFIG_HEADER_SIZE]= 0;
        if(!strcmp(pt, GPSP_CONFIG_HEADER))
        {
            FILE_READ_VARIABLE(gpsp_config_file, gpsp_config);
            FILE_CLOSE(gpsp_config_file);
            memcpy(gamepad_config_map, gpsp_config.gamepad_config_map, sizeof(gpsp_config.gamepad_config_map));
            return 0;
        }
    }

    // 如果无法读取配置文件，设置缺省值
    init_default_gpsp_config();
    return -1;
}

void initial_gpsp_config()
{
    //Initial directory path
    sprintf(g_default_rom_dir, "%s/GAMEPAK", main_path);
    sprintf(DEFAULT_SAVE_DIR, "%s/GAMERTS", main_path);
    sprintf(DEFAULT_CFG_DIR, "%s/GAMERTS", main_path);
    sprintf(DEFAULT_SS_DIR, "%s/GAMEPIC", main_path);
    sprintf(DEFAULT_CHEAT_DIR, "%s/GAMECHT", main_path);
}

/*--------------------------------------------------------
  メニューの表示
--------------------------------------------------------*/
u32 menu(u16 *screen, int FirstInvocation)
{
    gui_action_type gui_action;
    u32 i;
    u32 repeat;
    u32 return_value = 0;
    u32 first_load = 0;
    char tmp_filename[MAX_FILE];
    char line_buffer[512];
    char cheat_data_str[MAX_CHEATS][5];
    // ^ Holds the index inside Cheat, as a number in an ASCIIZ string
    char* cheat_data_ptr[MAX_CHEATS];

	// For cheat groups
	char* cheat_group_name_ptr[MAX_CHEATS + 1];
	char cheat_group_names[MAX_CHEATS * CHEAT_NAME_LENGTH];

    MENU_TYPE *current_menu = NULL;
    MENU_OPTION_TYPE *current_option = NULL;
    MENU_OPTION_TYPE *display_option = NULL;

    u32 current_option_num;
//    u32 parent_option_num;
    u32 string_select;

    u16 *bg_screenp;
    u32 bg_screenp_color;

    GAME_CONFIG PreviousGameConfig;  // Compared with current settings to
    GPSP_CONFIG PreviousGpspConfig;  // determine if they need to be saved

	auto void choose_menu();
	auto void menu_return();
	auto void menu_exit();
	auto void menu_load();
	auto void menu_restart();
	auto void menu_save_state();
	auto void menu_load_state();
	auto void others_menu_init();
	auto void main_menu_passive();
	auto void main_menu_key();
	auto void delette_savestate();
	auto void save_screen_snapshot();
	auto void browse_screen_snapshot();
	auto void tools_menu_init();
	auto void load_default_setting();
	auto void check_gbaemu_version();
	auto void load_lastest_played();
	auto void latest_game_menu_passive();
	auto void latest_game_menu_init();
	auto void latest_game_menu_key();
	auto void latest_game_menu_end();
	auto void language_set();
	auto void game_fastforward();
#ifdef ENABLE_FREE_SPACE
	auto void show_card_space();
#endif
	auto void savestate_selitem(u32 sel, u32 y_pos);
	auto void game_state_menu_passive();
	auto void gamestate_delette_menu_passive();
	auto void cheat_menu_init();
	auto void cheat_menu_end();
	auto void reload_cheats_page();
	auto void cheat_option_action();
	auto void cheat_option_passive();

//Local function definition

	void menu_exit()
	{
		HighFrequencyCPU(); // Crank it up, leave quickly
		if(gamepak_filename[0] != 0)
		{
			update_backup_force();
		}
		quit();
	}

	void SaveConfigsIfNeeded()
	{
		if (memcmp(&PreviousGameConfig, &game_config, sizeof(GAME_CONFIG)) != 0)
			save_game_config_file();
		if (memcmp(&PreviousGpspConfig, &gpsp_config, sizeof(GPSP_CONFIG)) != 0)
			save_config_file();
	}

	void PreserveConfigs()
	{
		memcpy(&PreviousGameConfig, &game_config, sizeof(GAME_CONFIG));
		memcpy(&PreviousGpspConfig, &gpsp_config, sizeof(GPSP_CONFIG));
	}

  int LoadGameAndItsData(char *filename) {
    if (gamepak_filename[0] != '\0') {
      update_backup_force();
    }

    draw_message(down_screen_addr, bg_screenp, 28, 31, 227, 165, bg_screenp_color);
    draw_string_vcenter(down_screen_addr, 36, 100, 190, COLOR_MSSG, msg[MSG_PROGRESS_LOADING_GAME]);
    ds2_flipScreen(DOWN_SCREEN, DOWN_SCREEN_UPDATE_METHOD);


    HighFrequencyCPU();
    int load_result = load_gamepak(filename);
    LowFrequencyCPU();

    if(load_result == -1)
    {
      first_load = 1;
      gamepak_filename[0] = '\0';
      return 0;
    }
    else
    {
      reset_gba();
      reg[CHANGED_PC_STATUS] = 1;
    }

    char tempPath[MAX_PATH];
    strcpy(tempPath, filename);

    //update folders and names for settings/config uses
    char *dirEnd = strrchr(tempPath, '/');
    //make sure a valid path was provided
    if(!dirEnd)
      return 0;

    //copy file name as gamepak_filename
    strcpy(gamepak_filename, dirEnd+1);
    //then strip filename from directory path and set it
    *dirEnd = '\0';
    strcpy(g_default_rom_dir, tempPath);

    first_load = 0;
    load_game_config_file();
    PreserveConfigs(); // Make the emulator not save what we've JUST read
    // but it will save the emulator configuration below for latest files

    return_value = 1;
    repeat = 0;

    reorder_latest_file();
    get_savestate_filelist();

    game_fast_forward= 0;
    return 1;
  }

	void menu_load()
	{
		char *file_ext[] = { ".gba", ".bin", ".zip", NULL };

		if(load_file(file_ext, tmp_filename, g_default_rom_dir) != -1)
		{
			strcpy(line_buffer, g_default_rom_dir);
			strcat(line_buffer, "/");
			strcat(line_buffer, tmp_filename);

			LoadGameAndItsData(line_buffer);
		}
		else
		{
			choose_menu(current_menu);
		}
	}

	void menu_restart()
	{
		if(!first_load)
		{
			reset_gba();
			reg[CHANGED_PC_STATUS] = 1;
			return_value = 1;
			repeat = 0;
		}
	}

	void menu_return()
	{
		if(!first_load)
			repeat = 0;
	}

	void clear_savestate_slot(u32 slot_index)
	{
		get_savestate_filename(slot_index, tmp_filename);
		sprintf(line_buffer, "%s/%s", DEFAULT_SAVE_DIR, tmp_filename);
		remove(line_buffer);
		SavedStateCacheInvalidate ();
	}

    void savestate_selitem(u32 selected, u32 y_pos)
    {
        u32 k;

        for(k= 0; k < SAVE_STATE_SLOT_NUM; k++)
        {
		struct gui_iconlist *icon;
		int Exists = SavedStateFileExists (k);
		unsigned char X = SavedStateSquareX (k);

		if (selected == k && Exists)
			icon = &ICON_STATEFULL;
		else if (selected == k && !Exists)
			icon = &ICON_STATEEMPTY;
		else if (selected != k && Exists)
			icon = &ICON_NSTATEFULL;
		else if (selected != k && !Exists)
			icon = &ICON_NSTATEEMPTY;

		show_icon((unsigned short *) down_screen_addr, icon, X, y_pos);
        }
    }

	void game_state_menu_passive()
	{
		unsigned int line[3] = {0, 2, 4};

		//draw background
		show_icon(down_screen_addr, &ICON_SUBBG, 0, 0);
		show_icon(down_screen_addr, &ICON_TITLE, 0, 0);
		show_icon(down_screen_addr, &ICON_TITLEICON, 12, 9);

		if(current_option_num == 0)
			show_icon(down_screen_addr, &ICON_BACK, 229, 10);
		else
			show_icon(down_screen_addr, &ICON_NBACK, 229, 10);

		strcpy(line_buffer, *(display_option->display_string));
		draw_string_vcenter(down_screen_addr, 0, 9, 256, COLOR_ACTIVE_ITEM, line_buffer);

		display_option += 1;
        for(i= 0; i < 3; i++, display_option++)
        {
            unsigned short color;

			if(display_option == current_option)
				show_icon(down_screen_addr, &ICON_SUBSELA, 6, 35 + line[i]*27);

			if(display_option->option_type & NUMBER_SELECTION_TYPE)
			{
				sprintf(line_buffer, *(display_option->display_string),
					*(display_option->current_option)+1);	//ADD 1 here
			}
			else if(display_option->option_type & STRING_SELECTION_TYPE)
			{
				sprintf(line_buffer, *(display_option->display_string),
					*((u32*)(((u32 *)display_option->options)[*(display_option->current_option)])));
			}
			else
			{
				strcpy(line_buffer, *(display_option->display_string));
			}

			if(display_option == current_option)
				color= COLOR_ACTIVE_ITEM;
			else
				color= COLOR_INACTIVE_ITEM;

			PRINT_STRING_BG(down_screen_addr, line_buffer, color, COLOR_TRANS, OPTION_TEXT_X, 40 + line[i]*27);
        }

		unsigned int selected_write, selected_read;

		selected_write = -1;
		selected_read = -1;

		if(current_option_num == 1 /* write */)
			selected_write = savestate_index;
		if(current_option_num == 2 /* read */)
			selected_read = savestate_index;

		savestate_selitem(selected_write, 66);
		savestate_selitem(selected_read, 120);
	}

    u32 delette_savestate_num= 0;
	void gamestate_delette_menu_passive()
	{
		unsigned int line[2] = {0, 1};

		//draw background
		show_icon(down_screen_addr, &ICON_SUBBG, 0, 0);
		show_icon(down_screen_addr, &ICON_TITLE, 0, 0);
		show_icon(down_screen_addr, &ICON_TITLEICON, 12, 9);

		if(current_option_num == 0)
			show_icon(down_screen_addr, &ICON_BACK, 229, 6);
		else
			show_icon(down_screen_addr, &ICON_NBACK, 229, 6);

		strcpy(line_buffer, *(display_option->display_string));
		draw_string_vcenter(down_screen_addr, 0, 9, 256, COLOR_ACTIVE_ITEM, line_buffer);

		display_option += 1;
        for(i= 0; i < 2; i++, display_option++)
        {
            unsigned short color;

			if(display_option == current_option)
				show_icon(down_screen_addr, &ICON_SUBSELA, 6, 35 + line[i]*27);

			if(display_option->option_type & NUMBER_SELECTION_TYPE)
			{
				sprintf(line_buffer, *(display_option->display_string),
					*(display_option->current_option)+1);
			}
			else if(display_option->option_type & STRING_SELECTION_TYPE)
			{
				sprintf(line_buffer, *(display_option->display_string),
					*((u32*)(((u32 *)display_option->options)[*(display_option->current_option)])));
			}
			else
			{
				strcpy(line_buffer, *(display_option->display_string));
			}

			if(display_option == current_option)
				color= COLOR_ACTIVE_ITEM;
			else
				color= COLOR_INACTIVE_ITEM;

			PRINT_STRING_BG(down_screen_addr, line_buffer, color, COLOR_TRANS, OPTION_TEXT_X, 40 + line[i]*27);
        }

		if(current_option_num == 2)
			savestate_selitem(delette_savestate_num, 93);
        else
            savestate_selitem(-1, 93);
	}

	void menu_save_state()
	{
		if(gui_action == CURSOR_SELECT)
		{
			if(!first_load)
			{
				if (SavedStateFileExists (savestate_index))
				{
					draw_message(down_screen_addr, NULL, 28, 31, 227, 165, 0);
					draw_string_vcenter(down_screen_addr, 36, 74, 190, COLOR_MSSG, msg[MSG_SAVESTATE_FULL]);
					if(draw_yesno_dialog(DOWN_SCREEN, 115, msg[MSG_GENERAL_CONFIRM_WITH_A], msg[MSG_GENERAL_CANCEL_WITH_B]) == 0)
						return;

					clear_savestate_slot(savestate_index);
				}

				get_savestate_filename(savestate_index, tmp_filename);

				draw_message(down_screen_addr, NULL, 28, 31, 227, 165, 0);
				draw_string_vcenter(down_screen_addr, 36, 100, 190, COLOR_MSSG, msg[MSG_PROGRESS_SAVED_STATE_CREATING]);
				ds2_flipScreen(DOWN_SCREEN, DOWN_SCREEN_UPDATE_METHOD);

				HighFrequencyCPU();
				int flag = save_state(tmp_filename, (void*)screen);
				LowFrequencyCPU();
				//clear message
				draw_message(down_screen_addr, NULL, 28, 31, 227, 96, 0);
				if(flag < 0)
				{
					draw_string_vcenter(down_screen_addr, 36, 74, 190, COLOR_MSSG, msg[MSG_PROGRESS_SAVED_STATE_CREATION_FAILED]);
				}
				else
				{
					draw_string_vcenter(down_screen_addr, 36, 100, 190, COLOR_MSSG, msg[MSG_PROGRESS_SAVED_STATE_CREATION_SUCCEEDED]);
				}

				ds2_flipScreen(DOWN_SCREEN, DOWN_SCREEN_UPDATE_METHOD);

				SavedStateCacheInvalidate ();

				mdelay(500); // let the progress message linger
			}
		}
	}

	void menu_load_state()
	{
		if(!first_load)
		{
			FILE *fp= NULL;

			if(bg_screenp != NULL)
			{
				bg_screenp_color = COLOR16(43, 11, 11);
				memcpy(bg_screenp, down_screen_addr, 256*192*2);
			}
			else
				bg_screenp_color = COLOR_BG;

			if(SavedStateFileExists(savestate_index))
			{
				get_savestate_filename(savestate_index, tmp_filename);
				sprintf(line_buffer, "%s/%s", DEFAULT_SAVE_DIR, tmp_filename);
				fp= fopen(line_buffer, "r");

				//file error
				if(fp == NULL)
				{
					draw_message(down_screen_addr, bg_screenp, 28, 31, 227, 165, bg_screenp_color);
					draw_string_vcenter(down_screen_addr, 36, 80, 190, COLOR_MSSG, msg[MSG_PROGRESS_SAVED_STATE_CORRUPTED]);
					ds2_flipScreen(DOWN_SCREEN, DOWN_SCREEN_UPDATE_METHOD);

					wait_Allkey_release(0);
					if(gui_action == CURSOR_SELECT)
						wait_Anykey_press(0);
					else
						mdelay(1000);
					return;
				}

				//right
				if(gui_action == CURSOR_SELECT)
				{
					draw_message(down_screen_addr, bg_screenp, 28, 31, 227, 165, bg_screenp_color);
					draw_string_vcenter(up_screen_addr, 36, 75, 190, COLOR_MSSG, msg[MSG_PROGRESS_SAVED_STATE_LOADING]);

					HighFrequencyCPU();
					int flag = load_state(tmp_filename, fp);
					LowFrequencyCPU();
					if(0 == flag)
					{
						return_value = 1;
						repeat = 0;
						draw_string_vcenter(down_screen_addr, 36, 75, 190, COLOR_MSSG, msg[MSG_PROGRESS_SAVED_STATE_LOAD_SUCCEEDED]);
					}
					else
					{
						draw_string_vcenter(down_screen_addr, 36, 75, 190, COLOR_MSSG, msg[MSG_PROGRESS_SAVED_STATE_LOAD_FAILED]);
						mdelay(500); // let the failure show
					}
				}
				else	//load screen snapshot
				{
					HighFrequencyCPU();
					load_game_stat_snapshot(tmp_filename);
					LowFrequencyCPU();
				}
			}
			else
			{
				ds2_clearScreen(UP_SCREEN, COLOR_BLACK);
				draw_string_vcenter(up_screen_addr, 36, 75, 190, COLOR_WHITE, msg[MSG_TOP_SCREEN_NO_SAVED_STATE_IN_SLOT]);
				ds2_flipScreen(UP_SCREEN, UP_SCREEN_UPDATE_METHOD);
			}
		}
	}

	void delette_savestate()
	{
		if(!first_load && (gui_action == CURSOR_SELECT))
		{
			if(bg_screenp != NULL)
			{
				bg_screenp_color = COLOR16(43, 11, 11);
				memcpy(bg_screenp, down_screen_addr, 256*192*2);
			}
			else
				bg_screenp_color = COLOR_BG;

			wait_Allkey_release(0);
			if(current_option_num == 1)         //delette all
			{
				u32 i, flag;

				draw_message(down_screen_addr, bg_screenp, 28, 31, 227, 165, bg_screenp_color);
				draw_string_vcenter(down_screen_addr, 36, 75, 190, COLOR_MSSG, msg[MSG_DIALOG_SAVED_STATE_DELETE_ALL]);

				flag= 0;
				for(i= 0; i < SAVE_STATE_SLOT_NUM; i++)
					if (SavedStateFileExists (i))
					{flag= 1; break;}

				if(flag)
				{
					if(draw_yesno_dialog(DOWN_SCREEN, 115, msg[MSG_GENERAL_CONFIRM_WITH_A], msg[MSG_GENERAL_CANCEL_WITH_B]))
					{
						wait_Allkey_release(0);
						for(i= 0; i < SAVE_STATE_SLOT_NUM; i++)
						{
							get_savestate_filename(i, tmp_filename);
							sprintf(line_buffer, "%s/%s", DEFAULT_SAVE_DIR, tmp_filename);
							remove(line_buffer);
							SavedStateCacheInvalidate ();
						}
						savestate_index= 0;
					}
				}
				else
				{
					draw_message(down_screen_addr, bg_screenp, 28, 31, 227, 165, bg_screenp_color);
					draw_string_vcenter(down_screen_addr, 36, 90, 190, COLOR_MSSG, msg[MSG_PROGRESS_SAVED_STATE_ALREADY_EMPTY]);
					ds2_flipScreen(DOWN_SCREEN, DOWN_SCREEN_UPDATE_METHOD);
					mdelay(500);
				}
			}
			else if(current_option_num == 2)    //delette single
			{
				draw_message(down_screen_addr, bg_screenp, 28, 31, 227, 165, bg_screenp_color);

				if(SavedStateFileExists(delette_savestate_num))
				{
					sprintf(line_buffer, msg[FMT_DIALOG_SAVED_STATE_DELETE_ONE], delette_savestate_num + 1);
					draw_string_vcenter(down_screen_addr, 36, 75, 190, COLOR_MSSG, line_buffer);

					if(draw_yesno_dialog(DOWN_SCREEN, 115, msg[MSG_GENERAL_CONFIRM_WITH_A], msg[MSG_GENERAL_CANCEL_WITH_B])) {
						wait_Allkey_release(0);
						clear_savestate_slot(delette_savestate_num);
}
				}
				else
				{
					draw_string_vcenter(down_screen_addr, 36, 90, 190, COLOR_MSSG, msg[MSG_PROGRESS_SAVED_STATE_ALREADY_EMPTY]);
					ds2_flipScreen(DOWN_SCREEN, DOWN_SCREEN_UPDATE_METHOD);
					mdelay(500);
				}
			}
		}
	}

	MENU_TYPE cheats_menu;
	MENU_TYPE *dynamic_cheat_menu = NULL;
	MENU_OPTION_TYPE *dynamic_cheat_options = NULL;
	unsigned char *dynamic_cheat_msg = NULL;
	unsigned char **dynamic_cheat_pt = NULL;
	unsigned int dynamic_cheat_active;
	int dynamic_cheat_scroll_value= 0;

	void cheat_menu_init()
	{
		// Currently disabled [Neb]
	}

	void cheat_menu_end()
	{
		// Currently disabled [Neb]
	}

	void cheat_option_action()
	{
		// Currently disabled [Neb]
	}

	void cheat_option_passive()
	{
		// Currently disabled [Neb]
	}

	void menu_load_cheat_file()
	{
		// Currently disabled [Neb]
		if (!first_load)
		{
		}
	}

    void save_screen_snapshot()
    {
        if(gui_action == CURSOR_SELECT)
        {
            if(bg_screenp != NULL)
            {
                bg_screenp_color = COLOR16(43, 11, 11);
                memcpy(bg_screenp, down_screen_addr, 256*192*2);
            }
            else
                bg_screenp_color = COLOR_BG;

            draw_message(down_screen_addr, bg_screenp, 28, 31, 227, 165, bg_screenp_color);
            if(!first_load)
            {
                draw_string_vcenter(down_screen_addr, 36, 70, 190, COLOR_MSSG, msg[MSG_PROGRESS_SCREENSHOT_CREATING]);
                ds2_flipScreen(DOWN_SCREEN, DOWN_SCREEN_UPDATE_METHOD);
                if(save_ss_bmp(screen))
                    draw_string_vcenter(down_screen_addr, 36, 90, 190, COLOR_MSSG, msg[MSG_PROGRESS_SCREENSHOT_CREATION_SUCCEEDED]);
                else
                    draw_string_vcenter(down_screen_addr, 36, 90, 190, COLOR_MSSG, msg[MSG_PROGRESS_SCREENSHOT_CREATION_FAILED]);
                ds2_flipScreen(DOWN_SCREEN, DOWN_SCREEN_UPDATE_METHOD);
				mdelay(500);
            }
            else
            {
                draw_string_vcenter(down_screen_addr, 36, 90, 190, COLOR_MSSG, msg[MSG_TOP_SCREEN_NO_SAVED_STATE_IN_SLOT]);
                ds2_flipScreen(DOWN_SCREEN, DOWN_SCREEN_UPDATE_METHOD);
				mdelay(500);
            }
        }
    }

    void browse_screen_snapshot()
    {
        if(current_option_num == 2)
            play_screen_snapshot();
    }

    void load_default_setting()
    {
        if(bg_screenp != NULL)
        {
            bg_screenp_color = COLOR16(43, 11, 11);
            memcpy(bg_screenp, down_screen_addr, 256*192*2);
        }
        else
            bg_screenp_color = COLOR_BG;

        draw_message(down_screen_addr, bg_screenp, 28, 31, 227, 165, bg_screenp_color);
        draw_string_vcenter(down_screen_addr, 36, 70, 190, COLOR_MSSG, msg[MSG_DIALOG_RESET]);

        if(draw_yesno_dialog(DOWN_SCREEN, 115, msg[MSG_GENERAL_CONFIRM_WITH_A], msg[MSG_GENERAL_CANCEL_WITH_B]))
        {
			wait_Allkey_release(0);
            draw_message(down_screen_addr, bg_screenp, 28, 31, 227, 165, bg_screenp_color);
            draw_string_vcenter(down_screen_addr, 36, 80, 190, COLOR_MSSG, msg[MSG_PROGRESS_RESETTING]);
            ds2_flipScreen(DOWN_SCREEN, DOWN_SCREEN_UPDATE_METHOD);

            sprintf(line_buffer, "%s/%s", main_path, GPSP_CONFIG_FILENAME);
            remove(line_buffer);

            first_load= 1;
            init_default_gpsp_config();
            init_game_config();

			ds2_clearScreen(UP_SCREEN, 0);
            draw_string_vcenter(up_screen_addr, 0, 80, 256, COLOR_WHITE, msg[MSG_TOP_SCREEN_NO_GAME_LOADED]);
			ds2_flipScreen(UP_SCREEN, 1);

			// mdelay(500); // Delete this delay
        }
    }

    void check_gbaemu_version()
    {
        if(bg_screenp != NULL)
        {
            bg_screenp_color = COLOR16(43, 11, 11);
            memcpy(bg_screenp, down_screen_addr, 256*192*2);
        }
        else
            bg_screenp_color = COLOR_BG;

        draw_message(down_screen_addr, bg_screenp, 28, 31, 227, 165, bg_screenp_color);
        draw_string_vcenter(down_screen_addr, 36, 80, 190, COLOR_MSSG, msg[MSG_EMULATOR_NAME]);
        sprintf(line_buffer, "%s %s", msg[MSG_WORD_EMULATOR_VERSION], NDSGBA_VERSION);
        draw_string_vcenter(down_screen_addr, 36, 95, 190, COLOR_MSSG, line_buffer);
        ds2_flipScreen(DOWN_SCREEN, DOWN_SCREEN_UPDATE_METHOD);

		wait_Anykey_press(0);
    }

    void language_set()
    {
        if(gui_action == CURSOR_LEFT || gui_action == CURSOR_RIGHT)
        {
            HighFrequencyCPU(); // crank it up

            load_language_msg(LANGUAGE_PACK, gpsp_config.language);

            if(first_load)
            {
				ds2_clearScreen(UP_SCREEN, 0);
                draw_string_vcenter(up_screen_addr, 0, 80, 256, COLOR_WHITE, msg[MSG_TOP_SCREEN_NO_GAME_LOADED]);
				ds2_flipScreen(UP_SCREEN, 1);
            }

            LowFrequencyCPU(); // and back down
        }
    }

#ifdef ENABLE_FREE_SPACE
	unsigned int freespace;
    void show_card_space ()
    {
        u32 line_num;
        u32 num_byte;

        strcpy(line_buffer, *(display_option->display_string));
        line_num= display_option-> line_number;
        PRINT_STRING_BG(down_screen_addr, line_buffer, COLOR_INACTIVE_ITEM, COLOR_TRANS, 27,
            40 + (display_option->line_number)*27);

		num_byte = freespace;

        if(num_byte <= 9999*2)
        { /* < 9999KB */
            sprintf(line_buffer, "%d", num_byte/2);
            if(num_byte & 1)
                strcat(line_buffer, ".5 KB");
            else
                strcat(line_buffer, ".0 KB");
        }
        else if(num_byte <= 9999*1024*2)
        { /* < 9999MB */
            num_byte /= 1024;
            sprintf(line_buffer, "%d", num_byte/2);
            if(num_byte & 1)
                strcat(line_buffer, ".5 MB");
            else
                strcat(line_buffer, ".0 MB");
        }
        else
        {
            num_byte /= 1024*1024;
            sprintf(line_buffer, "%d", num_byte/2);
            if(num_byte & 1)
                strcat(line_buffer, ".5 GB");
            else
                strcat(line_buffer, ".0 GB");
        }

        PRINT_STRING_BG(down_screen_addr, line_buffer, COLOR_INACTIVE_ITEM, COLOR_TRANS, 147,
            40 + (display_option->line_number)*27);
    }
#endif

    char *frameskip_options[] = { (char*)&msg[MSG_VIDEO_FRAME_SKIPPING_AUTOMATIC], (char*)&msg[MSG_VIDEO_FRAME_SKIPPING_0], (char*)&msg[MSG_VIDEO_FRAME_SKIPPING_1], (char*)&msg[MSG_VIDEO_FRAME_SKIPPING_2], (char*)&msg[MSG_VIDEO_FRAME_SKIPPING_3], (char*)&msg[MSG_VIDEO_FRAME_SKIPPING_4], (char*)&msg[MSG_VIDEO_FRAME_SKIPPING_5], (char*)&msg[MSG_VIDEO_FRAME_SKIPPING_6], (char*)&msg[MSG_VIDEO_FRAME_SKIPPING_7], (char*)&msg[MSG_VIDEO_FRAME_SKIPPING_8], (char*)&msg[MSG_VIDEO_FRAME_SKIPPING_9], (char*)&msg[MSG_VIDEO_FRAME_SKIPPING_10] };

    char *cpu_frequency_options[] = { (char*)&msg[MSG_OPTIONS_CPU_FREQUENCY_0], (char*)&msg[MSG_OPTIONS_CPU_FREQUENCY_1], (char*)&msg[MSG_OPTIONS_CPU_FREQUENCY_2], (char*)&msg[MSG_OPTIONS_CPU_FREQUENCY_3], (char*)&msg[MSG_OPTIONS_CPU_FREQUENCY_4], (char*)&msg[MSG_OPTIONS_CPU_FREQUENCY_5] };

    char *on_off_options[] = { (char*)&msg[MSG_GENERAL_OFF], (char*)&msg[MSG_GENERAL_ON] };

    char *sound_seletion[] = { (char*)&msg[MSG_AUDIO_MUTED], (char*)&msg[MSG_AUDIO_ENABLED] };

//    char *snap_frame_options[] = { (char*)&msg[MSG_SNAP_FRAME_0], (char*)&msg[MSG_SNAP_FRAME_1] };

//    char *enable_disable_options[] = { (char*)&msg[MSG_EN_DIS_ABLE_0], (char*)&msg[MSG_EN_DIS_ABLE_1] };

  /*--------------------------------------------------------
    Video & Audio
  --------------------------------------------------------*/
	MENU_OPTION_TYPE graphics_options[] =
	{
	/* 00 */ SUBMENU_OPTION(NULL, &msg[MSG_MAIN_MENU_VIDEO_AUDIO], NULL, 0),

	/* 01 */ STRING_SELECTION_OPTION(game_fastforward, NULL, &msg[FMT_VIDEO_FAST_FORWARD], on_off_options,
		&game_fast_forward, 2, NULL, ACTION_TYPE, 1),

	/* 02 */	STRING_SELECTION_OPTION(game_disableAudio, NULL, &msg[FMT_AUDIO_SOUND], sound_seletion,
		&sound_on, 2, NULL, ACTION_TYPE, 2),

	/* 03 */	STRING_SELECTION_OPTION(game_set_frameskip, NULL, &msg[FMT_VIDEO_FRAME_SKIPPING], frameskip_options,
		&game_config.frameskip_value, 12 /* auto (0) and 0..10 (1..11) make 12 option values */, NULL, ACTION_TYPE, 3),
	};

	MAKE_MENU(graphics, NULL, NULL, NULL, NULL, 0, 0);

  /*--------------------------------------------------------
     Game state -- delette
  --------------------------------------------------------*/
	MENU_TYPE game_state_menu;

	MENU_OPTION_TYPE gamestate_delette_options[] =
	{
	/* 00 */ SUBMENU_OPTION(&game_state_menu, &msg[MSG_SAVED_STATE_DELETE_GENERAL], NULL, 0),

	/* 01 */ ACTION_OPTION(delette_savestate, NULL, &msg[MSG_SAVED_STATE_DELETE_ALL], NULL, 1),

	/* 02 */ NUMERIC_SELECTION_ACTION_OPTION(delette_savestate, NULL,
		&msg[FMT_SAVED_STATE_DELETE_ONE], &delette_savestate_num, SAVE_STATE_SLOT_NUM, NULL, 2)
	};

	MAKE_MENU(gamestate_delette, NULL, gamestate_delette_menu_passive, NULL, NULL, 0, 0);

  /*--------------------------------------------------------
     Game state
  --------------------------------------------------------*/
	MENU_OPTION_TYPE game_state_options[] =
	{
	/* 00 */ SUBMENU_OPTION(NULL, &msg[MSG_MAIN_MENU_SAVED_STATES], NULL, 0),

	// savestate_index is still a signed int
	/* 01 */ NUMERIC_SELECTION_ACTION_OPTION(menu_save_state, NULL, &msg[FMT_SAVED_STATE_CREATE], (u32*) &savestate_index, SAVE_STATE_SLOT_NUM, NULL, 1),

	// savestate_index is still a signed int
	/* 02 */ NUMERIC_SELECTION_ACTION_OPTION(menu_load_state, NULL,
        &msg[FMT_SAVED_STATE_LOAD], (u32*) &savestate_index, SAVE_STATE_SLOT_NUM, NULL, 2),

	/* 03 */ SUBMENU_OPTION(&gamestate_delette_menu, &msg[MSG_SAVED_STATE_DELETE_GENERAL], NULL, 5),
	};

	INIT_MENU(game_state, NULL, game_state_menu_passive, NULL, NULL, 0, 0);

  /*--------------------------------------------------------
     Cheat options
  --------------------------------------------------------*/
	MENU_OPTION_TYPE cheats_options[] =
	{
	/* 00 */ SUBMENU_OPTION(NULL, &msg[MSG_MAIN_MENU_CHEATS], NULL,0),
		// Currently disabled [Neb]
#if 0
	/* 01 */ CHEAT_OPTION(cheat_option_action, cheat_option_passive,
		((CHEATS_PER_PAGE * menu_cheat_page) + 0), 1),
	/* 02 */ CHEAT_OPTION(cheat_option_action, cheat_option_passive,
		((CHEATS_PER_PAGE * menu_cheat_page) + 1), 2),
	/* 03 */ CHEAT_OPTION(cheat_option_action, cheat_option_passive,
		((CHEATS_PER_PAGE * menu_cheat_page) + 2), 3),
	/* 04 */ CHEAT_OPTION(cheat_option_action, cheat_option_passive,
		((CHEATS_PER_PAGE * menu_cheat_page) + 3), 4),

	/* 05 */ NUMERIC_SELECTION_ACTION_OPTION(reload_cheats_page, NULL, &msg[FMT_CHEAT_PAGE],
        &menu_cheat_page, MAX_CHEATS_PAGE, NULL, 5),

	/* 06 */ ACTION_OPTION(menu_load_cheat_file, NULL, &msg[MSG_CHEAT_LOAD_FROM_FILE],
        NULL, 6)
#endif
	};

	INIT_MENU(cheats, cheat_menu_init, NULL, NULL, cheat_menu_end, 0, 0);

  /*--------------------------------------------------------
     Tools-screensanp
  --------------------------------------------------------*/
    MENU_TYPE tools_menu;

    MENU_OPTION_TYPE tools_screensnap_options[] =
    {
	/* 00 */ SUBMENU_OPTION(&tools_menu, &msg[MSG_TOOLS_SCREENSHOT_GENERAL], NULL, 0),

	/* 01 */ ACTION_OPTION(save_screen_snapshot, NULL, &msg[MSG_SCREENSHOT_CREATE], NULL, 1),

	/* 02 */ ACTION_OPTION(browse_screen_snapshot, NULL, &msg[MSG_SCREENSHOT_BROWSE], NULL, 2)
    };

    MAKE_MENU(tools_screensnap, NULL, NULL, NULL, NULL, 0, 0);

  /*--------------------------------------------------------
     Tools
  --------------------------------------------------------*/
    MENU_OPTION_TYPE tools_options[] =
    {
	/* 00 */ SUBMENU_OPTION(NULL, &msg[MSG_MAIN_MENU_TOOLS], NULL, 0),

	/* 01 */ SUBMENU_OPTION(&tools_screensnap_menu, &msg[MSG_TOOLS_SCREENSHOT_GENERAL], NULL, 1),

//	/* 02 */ SUBMENU_OPTION(&tools_keyremap_menu, &msg[MSG_SUB_MENU_31], NULL, 2),

//	/* 03 */ STRING_SELECTION_OPTION(time_backward_action, NULL, &msg[MSG_SUB_MENU_302], on_off_options,
//			&game_config.backward, 2, NULL, ACTION_TYPE, 3),

//	/* 04 */ NUMERIC_SELECTION_ACTION_OPTION(time_period_action, time_period_passive, &msg[MSG_SUB_MENU_32],
//		&game_config.backward_time, 6, NULL, 4)
    };

    INIT_MENU(tools, tools_menu_init, NULL, NULL, NULL, 0, 0);

  /*--------------------------------------------------------
     Others
  --------------------------------------------------------*/
    u32 desert= 0;
	MENU_OPTION_TYPE others_options[] =
	{
	/* 00 */ SUBMENU_OPTION(NULL, &msg[MSG_MAIN_MENU_OPTIONS], NULL, 0),

	//CPU speed (string: shows MHz)
	/* 01 */ STRING_SELECTION_OPTION(NULL, NULL, &msg[FMT_OPTIONS_CPU_FREQUENCY], cpu_frequency_options,
        &game_config.clock_speed_number, 6, NULL, PASSIVE_TYPE, 1),

	/* 02 */ STRING_SELECTION_OPTION(language_set, NULL, &msg[FMT_OPTIONS_LANGUAGE], language_options,
        &gpsp_config.language, sizeof(language_options) / sizeof(language_options[0]) /* number of possible languages */, NULL, ACTION_TYPE, 2),

#ifdef ENABLE_FREE_SPACE
	/* 03 */ STRING_SELECTION_OPTION(NULL, show_card_space, &msg[MSG_OPTIONS_CARD_CAPACITY], NULL,
        &desert, 2, NULL, PASSIVE_TYPE | HIDEN_TYPE, 3),
#endif

	/* 04 */ ACTION_OPTION(load_default_setting, NULL, &msg[MSG_OPTIONS_RESET], NULL,
#ifdef ENABLE_FREE_SPACE
			4
#else
			3
#endif
		),

	/* 05 */ ACTION_OPTION(check_gbaemu_version, NULL, &msg[MSG_OPTIONS_VERSION], NULL,
#ifdef ENABLE_FREE_SPACE
			5
#else
			4
#endif
		),
	};

	MAKE_MENU(others, others_menu_init, NULL, NULL, NULL, 1, 1);

  /*--------------------------------------------------------
     Load_game
  --------------------------------------------------------*/
    MENU_TYPE latest_game_menu;

    MENU_OPTION_TYPE load_game_options[] =
    {
	/* 00 */ SUBMENU_OPTION(NULL, &msg[MSG_LOAD_GAME_MENU_TITLE], NULL, 0),

	/* 01 */ ACTION_OPTION(menu_load, NULL, &msg[MSG_LOAD_GAME_FROM_CARD], NULL, 1),

	/* 02 */ SUBMENU_OPTION(&latest_game_menu, &msg[MSG_LOAD_GAME_RECENTLY_PLAYED], NULL, 2)
    };

    MAKE_MENU(load_game, NULL, NULL, NULL, NULL, 1, 1);

  /*--------------------------------------------------------
     Latest game
  --------------------------------------------------------*/
    MENU_OPTION_TYPE latest_game_options[] =
    {
	/* 00 */ SUBMENU_OPTION(&load_game_menu, &msg[MSG_LOAD_GAME_RECENTLY_PLAYED], NULL, 0),

	/* 01 */ ACTION_OPTION(load_lastest_played, NULL, NULL, NULL, 1),

	/* 02 */ ACTION_OPTION(load_lastest_played, NULL, NULL, NULL, 2),

	/* 03 */ ACTION_OPTION(load_lastest_played, NULL, NULL, NULL, 3),

	/* 04 */ ACTION_OPTION(load_lastest_played, NULL, NULL, NULL, 4),

	/* 05 */ ACTION_OPTION(load_lastest_played, NULL, NULL, NULL, 5)
    };

    INIT_MENU(latest_game, latest_game_menu_init, latest_game_menu_passive,
		latest_game_menu_key, latest_game_menu_end, 0, 0);

  /*--------------------------------------------------------
     MAIN MENU
  --------------------------------------------------------*/
	MENU_OPTION_TYPE main_options[] =
	{
    /* 00 */ SUBMENU_OPTION(&graphics_menu, &msg[MSG_MAIN_MENU_VIDEO_AUDIO], NULL, 0),

    /* 01 */ SUBMENU_OPTION(&game_state_menu, &msg[MSG_MAIN_MENU_SAVED_STATES], NULL, 1),

    /* 02 */ SUBMENU_OPTION(&cheats_menu, &msg[MSG_MAIN_MENU_CHEATS], NULL, 2),

    /* 03 */ SUBMENU_OPTION(&tools_menu, &msg[MSG_MAIN_MENU_TOOLS], NULL, 3),

    /* 04 */ SUBMENU_OPTION(&others_menu, &msg[MSG_MAIN_MENU_OPTIONS], NULL, 4),

    /* 05 */ ACTION_OPTION(menu_exit, NULL, &msg[MSG_MAIN_MENU_EXIT], NULL, 5),

    /* 06 */ SUBMENU_OPTION(&load_game_menu, NULL, NULL, 6),

    /* 07 */ ACTION_OPTION(menu_return, NULL, NULL, NULL, 7),

    /* 08 */ ACTION_OPTION(menu_restart, NULL, NULL, NULL, 8)
	};

	MAKE_MENU(main, NULL, main_menu_passive, main_menu_key, NULL, 6, 0);

	void main_menu_passive()
	{
		show_icon(down_screen_addr, &ICON_MAINBG, 0, 0);
		current_menu -> focus_option = current_option -> line_number;

		//Audio/Video
		strcpy(line_buffer, *(display_option->display_string));
		if(display_option++ == current_option) {
			show_icon(down_screen_addr, &ICON_AVO, 19, 2);
			show_icon(down_screen_addr, &ICON_MSEL, 5, 57);
		}
		else {
			show_icon(down_screen_addr, &ICON_NAVO, 19, 2);
			show_icon(down_screen_addr, &ICON_MNSEL, 5, 57);
		}
		draw_string_vcenter(down_screen_addr, 7, 57, 75, COLOR_WHITE, line_buffer);

		//Save
		strcpy(line_buffer, *(display_option->display_string));
		if(display_option++ == current_option) {
			show_icon(down_screen_addr, &ICON_SAVO, 103, 2);
			show_icon(down_screen_addr, &ICON_MSEL, 89, 57);
		}
		else {
			show_icon(down_screen_addr, &ICON_NSAVO, 103, 2);
			show_icon(down_screen_addr, &ICON_MNSEL, 89, 57);
		}
		draw_string_vcenter(down_screen_addr, 91, 57, 75, COLOR_WHITE, line_buffer);

		//Cheat
		strcpy(line_buffer, *(display_option->display_string));
		if(display_option++ == current_option) {
			show_icon(down_screen_addr, &ICON_CHEAT, 187, 2);
			show_icon(down_screen_addr, &ICON_MSEL, 173, 57);
		}
		else {
			show_icon(down_screen_addr, &ICON_NCHEAT, 187, 2);
			show_icon(down_screen_addr, &ICON_MNSEL, 173, 57);
		}
		draw_string_vcenter(down_screen_addr, 175, 57, 75, COLOR_WHITE, line_buffer);

		//Tools
		strcpy(line_buffer, *(display_option->display_string));
		if(display_option++ == current_option) {
			show_icon(down_screen_addr, &ICON_TOOL, 19, 75);
			show_icon(down_screen_addr, &ICON_MSEL, 5, 131);
		}
		else {
			show_icon(down_screen_addr, &ICON_NTOOL, 19, 75);
			show_icon(down_screen_addr, &ICON_MNSEL, 5, 131);
		}
		draw_string_vcenter(down_screen_addr, 7, 131, 75, COLOR_WHITE, line_buffer);

		//Other
		strcpy(line_buffer, *(display_option->display_string));
		if(display_option++ == current_option) {
			show_icon(down_screen_addr, &ICON_OTHER, 103, 75);
			show_icon(down_screen_addr, &ICON_MSEL, 89, 131);
		}
		else {
			show_icon(down_screen_addr, &ICON_NOTHER, 103, 75);
			show_icon(down_screen_addr, &ICON_MNSEL, 89, 131);
		}
		draw_string_vcenter(down_screen_addr, 91, 131, 75, COLOR_WHITE, line_buffer);

		//Exit
		strcpy(line_buffer, *(display_option->display_string));
		if(display_option++ == current_option) {
			show_icon(down_screen_addr, &ICON_EXIT, 187, 75);
			show_icon(down_screen_addr, &ICON_MSEL, 173, 131);
		}
		else {
			show_icon(down_screen_addr, &ICON_NEXIT, 187, 75);
			show_icon(down_screen_addr, &ICON_MNSEL, 173, 131);
		}
		draw_string_vcenter(down_screen_addr, 175, 131, 75, COLOR_WHITE, line_buffer);

		//New
		if(display_option++ == current_option)
			show_icon(down_screen_addr, &ICON_MAINITEM, 0, 154);
		else
			show_icon(down_screen_addr, &ICON_NMAINITEM, 0, 154);
		draw_string_vcenter(down_screen_addr, 0, 165, 85, COLOR_WHITE, msg[MSG_MAIN_MENU_NEW_GAME]);

		//Restart
		if(display_option++ == current_option)
			show_icon(down_screen_addr, &ICON_MAINITEM, 85, 154);
		else
			show_icon(down_screen_addr, &ICON_NMAINITEM, 85, 154);
		draw_string_vcenter(down_screen_addr, 85, 165, 85, COLOR_WHITE, msg[MSG_MAIN_MENU_RETURN_TO_GAME]);

		//Return
		if(display_option++ == current_option)
			show_icon(down_screen_addr, &ICON_MAINITEM, 170, 154);
		else
			show_icon(down_screen_addr, &ICON_NMAINITEM, 170, 154);
		draw_string_vcenter(down_screen_addr, 170, 165, 85, COLOR_WHITE, msg[MSG_MAIN_MENU_RESET_GAME]);
	}

    void main_menu_key()
    {
        switch(gui_action)
        {
            case CURSOR_DOWN:
				if(current_option_num < 6)	current_option_num += 3;
				else current_option_num -= 6;

                current_option = current_menu->options + current_option_num;
              break;

            case CURSOR_UP:
				if(current_option_num < 3)	current_option_num += 6;
				else current_option_num -= 3;

                current_option = current_menu->options + current_option_num;
              break;

            case CURSOR_RIGHT:
				if(current_option_num == 2)	current_option_num -= 2;
				else if(current_option_num == 5)	current_option_num -= 2;
				else if(current_option_num == 8)	current_option_num -= 2;
				else current_option_num += 1;

                current_option = main_menu.options + current_option_num;
              break;

            case CURSOR_LEFT:
				if(current_option_num == 0)	current_option_num += 2;
				else if(current_option_num == 3)	current_option_num += 2;
				else if(current_option_num == 6)	current_option_num += 2;
				else current_option_num -= 1;

                current_option = main_menu.options + current_option_num;
              break;

            default:
              break;
        }// end swith
    }

	void tools_menu_init()
	{
		/* if(game_config.backward)
			tools_options[4].option_type &= ~HIDEN_TYPE;
		else
			tools_options[4].option_type |= HIDEN_TYPE; */
		// OUT OF BOUNDS MEMORY ACCESS, REENABLE IF NEEDED [NEB]
		if (first_load)
			tools_options[3] /* game hotkeys */.option_type |= HIDEN_TYPE;
		else
			tools_options[3] /* game hotkeys */.option_type &= ~HIDEN_TYPE;
	}

	int lastest_game_menu_scroll_value;
    void latest_game_menu_init()
    {
        u32 k;
        char *ext_pos;

        for(k= 0; k < 5; k++)
        {
            ext_pos= strrchr(gpsp_config.latest_file[k], '/');
            if(ext_pos != NULL)
                draw_hscroll_init(down_screen_addr, OPTION_TEXT_X, 40 + k*27, OPTION_TEXT_SX,
                    COLOR_TRANS, COLOR_INACTIVE_ITEM, ext_pos+1);
			else
				break;
        }

		if(k < 5)
		{
			latest_game_menu.num_options = k+1;
		}
		else
			latest_game_menu.num_options = 6;

		latest_game_menu.num_options = k+1;

        for(; k < 5; k++)
        {
            latest_game_options[k+1].option_type |= HIDEN_TYPE;
        }

		lastest_game_menu_scroll_value = 0;
    }

    void latest_game_menu_passive()
    {
        u32 k;
		//draw background
		show_icon(down_screen_addr, &ICON_SUBBG, 0, 0);
		show_icon(down_screen_addr, &ICON_TITLE, 0, 0);
		show_icon(down_screen_addr, &ICON_TITLEICON, 12, 9);

		if(current_option_num == 0)
			show_icon(down_screen_addr, &ICON_BACK, 229, 10);
		else
		{
			show_icon(down_screen_addr, &ICON_NBACK, 229, 10);
			show_icon(down_screen_addr, &ICON_SUBSELA, 6, 35 + (current_option_num-1)*27);
		}

		strcpy(line_buffer, *(display_option->display_string));
		draw_string_vcenter(down_screen_addr, 0, 9, 256, COLOR_ACTIVE_ITEM, line_buffer);

		for(k= 0; k<5; k++)
        if(gpsp_config.latest_file[k][0] != '\0')
        {
            if(current_option_num != k+1)
                draw_hscroll(k, 0);
            else
			{
                draw_hscroll(k, lastest_game_menu_scroll_value);
				lastest_game_menu_scroll_value = 0;
			}
        }
    }

    void latest_game_menu_end()
    {
        u32 k;

        for(k= 0; k < 5; k++)
        {
            if(gpsp_config.latest_file[k][0] != '\0')
                draw_hscroll_over(k);
        }
    }

	void latest_game_menu_key()
	{
		char *ext_pos;

		switch(gui_action)
        {
            case CURSOR_DOWN:
				//clear last option's bg
				if(current_option_num != 0)
				{
					draw_hscroll_over(current_option_num-1);
	            	ext_pos= strrchr(gpsp_config.latest_file[current_option_num-1], '/');
                	draw_hscroll_init(down_screen_addr, OPTION_TEXT_X, 40 + (current_option_num-1)*27, OPTION_TEXT_SX,
                	    COLOR_TRANS, COLOR_INACTIVE_ITEM, ext_pos+1);
				}

				current_option_num += 1;
				if(current_option_num >= latest_game_menu.num_options)
					current_option_num = 0;
                current_option = current_menu->options + current_option_num;

				//Set current bg
				if(current_option_num != 0)
				{
					draw_hscroll_over(current_option_num-1);
	            	ext_pos= strrchr(gpsp_config.latest_file[current_option_num-1], '/');
                	draw_hscroll_init(down_screen_addr, OPTION_TEXT_X, 40 + (current_option_num-1)*27, OPTION_TEXT_SX,
                	    COLOR_TRANS, COLOR_ACTIVE_ITEM, ext_pos+1);
				}

              break;

            case CURSOR_UP:
				//clear last option's bg
				if(current_option_num != 0)
				{
					draw_hscroll_over(current_option_num-1);
	            	ext_pos= strrchr(gpsp_config.latest_file[current_option_num-1], '/');
                	draw_hscroll_init(down_screen_addr, OPTION_TEXT_X, 40 + (current_option_num-1)*27, OPTION_TEXT_SX,
                	    COLOR_TRANS, COLOR_INACTIVE_ITEM, ext_pos+1);
				}

				if(current_option_num > 0)	current_option_num -= 1;
				else current_option_num = latest_game_menu.num_options -1;
                current_option = current_menu->options + current_option_num;

				//Set current bg
				if(current_option_num != 0)
				{
					draw_hscroll_over(current_option_num-1);
	            	ext_pos= strrchr(gpsp_config.latest_file[current_option_num-1], '/');
                	draw_hscroll_init(down_screen_addr, OPTION_TEXT_X, 40 + (current_option_num-1)*27, OPTION_TEXT_SX,
                	    COLOR_TRANS, COLOR_ACTIVE_ITEM, ext_pos+1);
				}

		      break;

            case CURSOR_RIGHT:
				lastest_game_menu_scroll_value = -5;
              break;

            case CURSOR_LEFT:
				lastest_game_menu_scroll_value = 5;
              break;

            default:
              break;
        }// end swith
	}

    void load_lastest_played()
    {
		char *ext_pos;

		if(bg_screenp != NULL) {
			bg_screenp_color = COLOR16(43, 11, 11);
		}
		else
			bg_screenp_color = COLOR_BG;

		ext_pos= strrchr(gpsp_config.latest_file[current_option_num -1], '/');
		*ext_pos= '\0';
    //Removing rom_path due to confusion, replacing with g_default_rom_dir
		strcpy(g_default_rom_dir, gpsp_config.latest_file[current_option_num -1]);
		*ext_pos= '/';

		ext_pos = gpsp_config.latest_file[current_option_num -1];

		LoadGameAndItsData(ext_pos);

		get_newest_savestate(tmp_filename);
		if(tmp_filename[0] != '\0')
		{
			FILE *fp = fopen(tmp_filename, "rb");
			load_state(tmp_filename, fp);
		}
    }

    void game_fastforward()
    {
    }



	void reload_cheats_page()
	{
		for(i = 0; i < CHEATS_PER_PAGE; i++)
		{
			cheats_options[i+1].display_string = &cheat_data_ptr[(CHEATS_PER_PAGE * menu_cheat_page) + i];
		}
	}

    void others_menu_init()
    {
#ifdef ENABLE_FREE_SPACE
		unsigned int total, used;

		//get card space info
		freespace = 0;
		fat_getDiskSpaceInfo("fat:", &total, &used, &freespace);
#endif
    }

	void choose_menu(MENU_TYPE *new_menu)
	{
		if(new_menu == NULL)
			new_menu = &main_menu;

		if(NULL != current_menu) {
			if(current_menu->end_function)
				current_menu->end_function();
			SaveConfigsIfNeeded();
		}

		current_menu = new_menu;
		current_option_num= current_menu -> focus_option;
		current_option = new_menu->options + current_option_num;
		PreserveConfigs();
		if(current_menu->init_function)
			current_menu->init_function();
	}

//----------------------------------------------------------------------------//
//	Menu Start
	LowFrequencyCPU();
	if (!FirstInvocation)
	{ // assume that the backlight is already at 3 when the emulator starts
		mdelay(100); // to prevent ds2_setBacklight() from crashing
		ds2_setBacklight(3);
		// also allow the user to press A for New Game right away
		wait_Allkey_release(0);
	}

    bg_screenp= (u16*)malloc(256*192*2);

	repeat = 1;

	if(gamepak_filename[0] == 0)
	{
		first_load = 1;
		//try auto loading games passed through argv first
		if(strlen(argv[1]) > 0 && LoadGameAndItsData(argv[1]))
			repeat = 0;
		else
		{
			ds2_clearScreen(UP_SCREEN, COLOR_BLACK);
			draw_string_vcenter(up_screen_addr, 0, 80, 256, COLOR_WHITE, msg[MSG_TOP_SCREEN_NO_GAME_LOADED]);
			ds2_flipScreen(UP_SCREEN, 1);
		}
	}
	else
	{
		/*
		 * It's pretty complicated. These two flips are needed because,
		 * otherwise, the menu freezes if S9xAutoSaveSRAM was called after
		 * loading from a save state.
		 */
		ds2_flipScreen(UP_SCREEN, 1);
		ds2_flipScreen(UP_SCREEN, 1);
	}

	choose_menu(&main_menu);

//	Menu loop

	while(repeat)
	{
		display_option = current_menu->options;
		string_select= 0;

		if(current_menu -> passive_function)
			current_menu -> passive_function();
		else
		{
			u32 line_num, screen_focus, focus_option;

			//draw background
			show_icon(down_screen_addr, &ICON_SUBBG, 0, 0);
			show_icon(down_screen_addr, &ICON_TITLE, 0, 0);
			show_icon(down_screen_addr, &ICON_TITLEICON, 12, 9);

			strcpy(line_buffer, *(display_option->display_string));
			draw_string_vcenter(down_screen_addr, 0, 9, 256, COLOR_ACTIVE_ITEM, line_buffer);

			line_num = current_option_num;
			screen_focus = current_menu -> screen_focus;
			focus_option = current_menu -> focus_option;

			if(focus_option < line_num)	//focus next option
			{
				focus_option = line_num - focus_option;
				screen_focus += focus_option;
				if(screen_focus > SUBMENU_ROW_NUM)	//Reach max row numbers can display
					screen_focus = SUBMENU_ROW_NUM;

				current_menu -> screen_focus = screen_focus;
				focus_option = line_num;
			}
			else if(focus_option > line_num)	//focus last option
			{
				focus_option = focus_option - line_num;
				if(screen_focus > focus_option)
					screen_focus -= focus_option;
				else
					screen_focus = 0;

				if(screen_focus == 0 && line_num > 0)
					screen_focus = 1;

				current_menu -> screen_focus = screen_focus;
				focus_option = line_num;
			}
			current_menu -> focus_option = focus_option;

			i = focus_option - screen_focus;
			display_option += i +1;

			line_num = current_menu->num_options-1;
			if(line_num > SUBMENU_ROW_NUM)
				line_num = SUBMENU_ROW_NUM;

			if(focus_option == 0)
				show_icon(down_screen_addr, &ICON_BACK, 229, 10);
			else
				show_icon(down_screen_addr, &ICON_NBACK, 229, 10);

			for(i= 0; i < line_num; i++, display_option++)
    	    {
    	        unsigned short color;

				if(display_option == current_option)
					show_icon(down_screen_addr, &ICON_SUBSELA, 6, 35 + i*27);

				if(display_option->passive_function)
				{
					display_option->line_number = i;
					display_option->passive_function();
				}
				else if(display_option->option_type & NUMBER_SELECTION_TYPE)
				{
					sprintf(line_buffer, *(display_option->display_string),
						*(display_option->current_option));
				}
				else if(display_option->option_type & STRING_SELECTION_TYPE)
				{
					sprintf(line_buffer, *(display_option->display_string),
						*((u32*)(((u32 *)display_option->options)[*(display_option->current_option)])));
				}
				else
				{
					strcpy(line_buffer, *(display_option->display_string));
				}

				if(display_option->passive_function == NULL)
				{
					if(display_option == current_option)
						color= COLOR_ACTIVE_ITEM;
					else
						color= COLOR_INACTIVE_ITEM;

					PRINT_STRING_BG(down_screen_addr, line_buffer, color, COLOR_TRANS, OPTION_TEXT_X, 40 + i*27);
				}
    	    }
    	}

		struct key_buf inputdata;
		gui_action = get_gui_input();

		switch(gui_action)
		{
			case CURSOR_TOUCH:
				ds2_getrawInput(&inputdata);
				wait_Allkey_release(0);
				/* Back button at the top of every menu but the main one */
				if(current_menu != &main_menu && inputdata.x > 231 && inputdata.y <= 25)
				{
					choose_menu(current_menu->options->sub_menu);
					break;
				}
				/* Main menu */
				if(current_menu == &main_menu)
				{
					// 0     86    172   256
					//  _____ _____ _____  0
					// |0VID_|1SAV_|2CHT_| 80
					// |3TLS_|4OPT_|5EXI_| 160
					// |6NEW_|7RET_|8RST_| 192

					current_option_num = (inputdata.y / 80) * 3 + (inputdata.x / 86);
					current_option = current_menu->options + current_option_num;

					if(current_option -> option_type & HIDEN_TYPE)
						break;
					else if(current_option->option_type & ACTION_TYPE)
						current_option->action_function();
					else if(current_option->option_type & SUBMENU_TYPE)
						choose_menu(current_option->sub_menu);
				}
				/* This is the majority case, covering all menus except save states, screen shots, and game loading */
				else if(current_menu != (main_menu.options + 1)->sub_menu
				&& current_menu != ((main_menu.options +1)->sub_menu->options + 3)->sub_menu
				&& current_menu != (main_menu.options +3)->sub_menu
				&& current_menu != ((main_menu.options +3)->sub_menu->options + 1)->sub_menu
				&& current_menu != (main_menu.options +6)->sub_menu
				&& current_menu != ((main_menu.options +6)->sub_menu->options + 2)->sub_menu)
				{
					if (inputdata.y <= 33 || inputdata.y > 192)
						break;
					// ___ 33        This screen has 6 possible rows. Touches
					// ___ 60        above or below these are ignored.
					// . . . (+27)   The row between 33 and 60 is [1], though!
					// ___ 192
					u32 next_option_num = (inputdata.y - 33) / 27 + 1;
					struct _MENU_OPTION_TYPE *next_option = current_menu->options + next_option_num;

					if (next_option_num >= current_menu->num_options)
						break;

					if(!next_option)
						break;

					if(next_option -> option_type & HIDEN_TYPE)
						break;

					current_option_num = next_option_num;
					current_option = current_menu->options + current_option_num;

					if(current_menu->key_function)
					{
						gui_action = CURSOR_RIGHT;
						current_menu->key_function();
					}
					else if(current_option->option_type & (NUMBER_SELECTION_TYPE | STRING_SELECTION_TYPE))
					{
						gui_action = CURSOR_RIGHT;
						u32 current_option_val = *(current_option->current_option);

						if(current_option_val <  current_option->num_options -1)
							current_option_val++;
						else
							current_option_val= 0;
						*(current_option->current_option) = current_option_val;

						if(current_option->action_function)
							current_option->action_function();
					}
					else if(current_option->option_type & ACTION_TYPE)
						current_option->action_function();
					else if(current_option->option_type & SUBMENU_TYPE)
						choose_menu(current_option->sub_menu);
				}
				/* Save states */
				else if(current_menu == (main_menu.options + 1)->sub_menu)
				{
					u32 next_option_num;
					if(inputdata.y <= 33)
						break;
					else if(inputdata.y <= 60)
						break; // "Create saved state"
					else if(inputdata.y <= 87) // Save cell
						next_option_num = 1;
					else if(inputdata.y <= 114)
						break; // "Load saved state"
					else if(inputdata.y <= 141) // Load cell
						next_option_num = 2;
					else if(inputdata.y <= 168) // Del...
						next_option_num = 3;
					else
						break;

					struct _MENU_OPTION_TYPE *next_option = current_menu->options + next_option_num;

					if(next_option_num == 1 /* write */ || next_option_num == 2 /* read */)
					{
						u32 current_option_val = *(next_option->current_option);
						u32 old_option_val = current_option_val;

						// This row has SAVE_STATE_SLOT_NUM cells for save states, each ICON_STATEFULL.x pixels wide.
						// The left side of a square is at SavedStateSquareX(slot).
						int found_state = FALSE;
						int i;
						for (i = 0; i < SAVE_STATE_SLOT_NUM; i++)
						{
							unsigned char StartX = SavedStateSquareX (i);
							if (inputdata.x >= StartX && inputdata.x < StartX + ICON_STATEFULL.x)
							{
								current_option_val = i;
								found_state = TRUE;
								break;
							}
						}
						if(!found_state)
							break;

						current_option_num = next_option_num;
						current_option = next_option;

						*(current_option->current_option) = current_option_val;

						if(current_option_val == old_option_val)
						{
							gui_action = CURSOR_SELECT;
							if(current_option->option_type & ACTION_TYPE)
								current_option->action_function();
							else if(current_option->option_type & SUBMENU_TYPE)
								choose_menu(current_option->sub_menu);
						}
						else
						{
							// Initial selection of a saved state
							// needs to show its screenshot
							if(current_option->option_type & ACTION_TYPE)
								current_option->action_function();
						}
						break;
					}

					gui_action = CURSOR_SELECT;
					if(next_option -> option_type & HIDEN_TYPE)
						break;

					current_option_num = next_option_num;
					current_option = next_option;

					if(current_option->option_type & ACTION_TYPE)
						current_option->action_function();
					else if(current_option->option_type & SUBMENU_TYPE)
						choose_menu(current_option->sub_menu);
				}
				/* Delete state sub menu */
				else if(current_menu == ((main_menu.options + 1)->sub_menu->options + 3)->sub_menu)
				{
					u32 next_option_num;
					if(inputdata.y <= 33)
						break;
					else if(inputdata.y <= 60)
						next_option_num = 1;
					else if(inputdata.y <= 87)
						break;
					else if(inputdata.y <= 114)
						next_option_num = 2;
					else
						break;

					struct _MENU_OPTION_TYPE *next_option = current_menu->options + next_option_num;

					if(next_option_num == 2)
					{
						u32 current_option_val = *(next_option->current_option);
						u32 old_option_val = current_option_val;

						// This row has SAVE_STATE_SLOT_NUM cells for save states, each ICON_STATEFULL.x pixels wide.
						// The left side of a square is at SavedStateSquareX(slot).
						int found_state = FALSE;
						int i;
						for (i = 0; i < SAVE_STATE_SLOT_NUM; i++)
						{
							unsigned char StartX = SavedStateSquareX (i);
							if (inputdata.x >= StartX && inputdata.x < StartX + ICON_STATEFULL.x)
							{
								current_option_val = i;
								found_state = TRUE;
								break;
							}
						}
						if(!found_state)
							break;

						current_option_num = next_option_num;
						current_option = next_option;

						*(current_option->current_option) = current_option_val;

						if(current_option_val == old_option_val)
						{
							gui_action = CURSOR_SELECT;
							if(current_option->option_type & ACTION_TYPE)
								current_option->action_function();
							else if(current_option->option_type & SUBMENU_TYPE)
								choose_menu(current_option->sub_menu);
						}
						break;
					}

					gui_action = CURSOR_SELECT;
					if(next_option -> option_type & HIDEN_TYPE)
						break;

					current_option_num = next_option_num;
					current_option = next_option;

					if(current_option->option_type & ACTION_TYPE)
						current_option->action_function();
					else if(current_option->option_type & SUBMENU_TYPE)
						choose_menu(current_option->sub_menu);
				}
				/* Screenshots and new game loading */
				else if(current_menu == (main_menu.options + 3)->sub_menu
				|| current_menu == ((main_menu.options +3)->sub_menu->options + 1)->sub_menu
				|| current_menu == (main_menu.options + 6)->sub_menu
				|| current_menu == ((main_menu.options +6)->sub_menu->options + 2)->sub_menu)
				{
					if (inputdata.y <= 33 || inputdata.y > 192)
						break;
					// ___ 33        This screen has 6 possible rows. Touches
					// ___ 60        above or below these are ignored.
					// . . . (+27)   The row between 33 and 60 is [1], though!
					// ___ 192
					u32 next_option_num = (inputdata.y - 33) / 27 + 1;
					if (next_option_num > current_menu->num_options)
						break;

					struct _MENU_OPTION_TYPE *next_option = current_menu->options + next_option_num;

					if(next_option -> option_type & HIDEN_TYPE)
						break;

					current_option_num = next_option_num;
					current_option = next_option;

					if(current_option->option_type & ACTION_TYPE)
						current_option->action_function();
					else if(current_option->option_type & SUBMENU_TYPE)
						choose_menu(current_option->sub_menu);
				}
				break;
			case CURSOR_DOWN:
				if(current_menu->key_function)
					current_menu->key_function();
				else
				{
					current_option_num = (current_option_num + 1) % current_menu->num_options;
					current_option = current_menu->options + current_option_num;

					while(current_option -> option_type & HIDEN_TYPE)
					{
						current_option_num = (current_option_num + 1) % current_menu->num_options;
						current_option = current_menu->options + current_option_num;
					}
				}
				break;

			case CURSOR_UP:
				if(current_menu->key_function)
					current_menu->key_function();
				else
				{
					if(current_option_num)
						current_option_num--;
					else
						current_option_num = current_menu->num_options - 1;
					current_option = current_menu->options + current_option_num;

					while(current_option -> option_type & HIDEN_TYPE)
					{
						if(current_option_num)
							current_option_num--;
						else
							current_option_num = current_menu->num_options - 1;
						current_option = current_menu->options + current_option_num;
					}
				}
				break;

			case CURSOR_RIGHT:
				if(current_menu->key_function)
					current_menu->key_function();
				else
				{
					if(current_option->option_type & (NUMBER_SELECTION_TYPE | STRING_SELECTION_TYPE))
					{
						u32 current_option_val = *(current_option->current_option);

						if(current_option_val <  current_option->num_options -1)
							current_option_val++;
						else
							current_option_val= 0;
						*(current_option->current_option) = current_option_val;

						if(current_option->action_function)
							current_option->action_function();
					}
				}
				break;

			case CURSOR_LEFT:
				if(current_menu->key_function)
					current_menu->key_function();
				else
				{
					if(current_option->option_type & (NUMBER_SELECTION_TYPE | STRING_SELECTION_TYPE))
					{
						u32 current_option_val = *(current_option->current_option);

						if(current_option_val)
							current_option_val--;
						else
							current_option_val = current_option->num_options - 1;
						*(current_option->current_option) = current_option_val;

						if(current_option->action_function)
							current_option->action_function();
					}
				}
				break;

			case CURSOR_EXIT:
				wait_Allkey_release(0);
				break;

			case CURSOR_SELECT:
				wait_Allkey_release(0);
				if(current_option->option_type & ACTION_TYPE)
					current_option->action_function();
				else if(current_option->option_type & SUBMENU_TYPE)
					choose_menu(current_option->sub_menu);
				break;

			case CURSOR_BACK:
				wait_Allkey_release(0);
				if(current_menu != &main_menu)
					choose_menu(current_menu->options->sub_menu);
				else
					menu_return();
				break;

			default:
				break;
		} // end swith

		ds2_flipScreen(DOWN_SCREEN, DOWN_SCREEN_UPDATE_METHOD);
	} // end while

	if (current_menu && current_menu->end_function)
		current_menu->end_function();
	SaveConfigsIfNeeded();

	if(bg_screenp != NULL) free((void*)bg_screenp);

	ds2_clearScreen(DOWN_SCREEN, 0);
	ds2_flipScreen(DOWN_SCREEN, DOWN_SCREEN_UPDATE_METHOD);
	gui_copy_screen(up_screen_addr, (void*) screen, 0, 0, 256, 192);
	ds2_flipScreen(UP_SCREEN, UP_SCREEN_UPDATE_METHOD);
	wait_Allkey_release(0);

	mdelay(100); // to prevent ds2_setBacklight() from crashing
	ds2_setBacklight(2);

	GameFrequencyCPU();

	return return_value;
}

u32 load_dircfg(char *file_name)  // TODO: Working directory configure
{
  int loop;
  int next_line;
  char current_line[256];
  char current_str[256];
  FILE *msg_file;
  char msg_path[MAX_PATH];

  sprintf(msg_path, "%s/%s", main_path, file_name);
  msg_file = fopen(msg_path, "r");

  next_line = 0;
  if(msg_file)
  {
    loop = 0;
    while(fgets(current_line, 256, msg_file))
    {

      if(parse_line(current_line, current_str) != -1)
      {
        sprintf(current_line, "%s/%s", main_path, current_str+2);

        switch(loop)
        {
          case 0:
            strcpy(g_default_rom_dir, current_line);
            if(opendir(current_line) == NULL)
            {
              printf("can't open rom dir : %s\n", g_default_rom_dir);
              strcpy(g_default_rom_dir, main_path);
//              *g_default_rom_dir = (char)NULL;
            }
            loop++;
            break;

          case 1:
            strcpy(DEFAULT_SAVE_DIR, current_line);
            if(opendir(current_line) == NULL)
            {
              printf("can't open save dir : %s\n", DEFAULT_SAVE_DIR);
              strcpy(DEFAULT_SAVE_DIR, main_path);
//              *DEFAULT_SAVE_DIR = (char)NULL;
            }
            loop++;
            break;

          case 2:
            strcpy(DEFAULT_CFG_DIR, current_line);
            if(opendir(current_line) == NULL)
            {
              printf("can't open cfg dir : %s\n", DEFAULT_CFG_DIR);
              strcpy(DEFAULT_CFG_DIR, main_path);
//              *DEFAULT_CFG_DIR = (char)NULL;
            }
            loop++;
            break;

          case 3:
            strcpy(DEFAULT_SS_DIR, current_line);
            if(opendir(current_line) == NULL)
            {
              printf("can't open screen shot dir : %s\n", current_line);
              strcpy(DEFAULT_SS_DIR, main_path);
//              *DEFAULT_SS_DIR = (char)NULL;
            }
            loop++;
            break;

          case 4:
            strcpy(DEFAULT_CHEAT_DIR, current_line);
            if(opendir(current_line) == NULL)
            {
              printf("can't open cheat dir : %s\n", current_line);
              strcpy(DEFAULT_CHEAT_DIR, main_path);
//              *DEFAULT_CHEAT_DIR = (char)NULL;
            }
            loop++;
            break;
        }
      }
    }

    fclose(msg_file);
    if (loop == 5)
    {
      return 0;
    }
    else
    {
      return -1;
    }
  }
  fclose(msg_file);
  return -1;
}

u32 load_fontcfg(char *file_name)  // TODO:スマートな実装に書き直す
{
  int loop;
  int next_line;
  char current_line[256];
  char current_str[256];
  FILE *msg_file;
  char msg_path[MAX_PATH];

  sprintf(msg_path, "%s/%s", main_path, file_name);

  m_font[0] = '\0';
  s_font[0] = '\0';

  msg_file = fopen(msg_path, "rb");

  next_line = 0;
  if(msg_file)
  {
    loop = 0;
    while(fgets(current_line, 256, msg_file))
    {
      if(parse_line(current_line, current_str) != -1)
      {
        sprintf(current_line, "%s/%s", main_path, current_str+2);
printf("main_path %s\n", main_path);
printf("current_str %s\n", current_str);
        switch(loop)
        {
          case 0:
            strcpy(m_font, current_line);
            loop++;
            break;
          case 1:
            strcpy(s_font, current_line);
            loop++;
            break;
        }
      }
    }

    fclose(msg_file);
    if (loop > 0)
      return 0;
    else
      return -1;
  }
  fclose(msg_file);
  return -1;
}

/*--------------------------------------------------------
--------------------------------------------------------*/
int load_language_msg(char *filename, u32 language)
{
    FILE *fp;
    char msg_path[MAX_PATH];
    char string[256];
    char start[32];
    char end[32];
    char *pt, *dst;
    u32 cmplen;
    u32 loop, offset, len;
    int ret;

    sprintf(msg_path, "%s/%s", main_path, filename);
    fp = fopen(msg_path, "rb");
    if(fp == NULL)
        return -1;

    switch(language)
    {
    case ENGLISH:
        strcpy(start, "STARTENGLISH");
        strcpy(end, "ENDENGLISH");
        cmplen= 12;
        break;
    default:
        strcpy(start, "STARTENGLISH");
        strcpy(end, "ENDENGLISH");
        cmplen= 12;
        break;
    }
    //find the start flag
    ret= 0;
    while(1)
    {
        pt= fgets(string, 256, fp);
        if(pt == NULL)
        {
            ret= -2;
            goto load_language_msg_error;
        }

        if(!strncmp(pt, start, cmplen))
            break;
    }

    loop= 0;
    offset= 0;
    dst= msg_data;
    msg[0]= dst;

    while(loop != MSG_END)
    {
        while(1)
        {
            pt = fgets(string, 256, fp);
            if(pt[0] == '#' || pt[0] == 0x0D || pt[0] == 0x0A)
                continue;
            if(pt != NULL)
                break;
            else
            {
                ret = -3;
                goto load_language_msg_error;
            }
        }

        if(!strncmp(pt, end, cmplen-2))
            break;

        len= strlen(pt);
        memcpy(dst, pt, len);

        dst += len;
        //at a line return, when "\n" paded, this message not end
        if(*(dst-1) == 0x0A)
        {
            pt = strrchr(pt, '\\');
            if((pt != NULL) && (*(pt+1) == 'n'))
            {
                if(*(dst-2) == 0x0D)
                {
                    *(dst-4)= '\n';
                    dst -= 3;
                }
                else
                {
                    *(dst-3)= '\n';
                    dst -= 2;
                }
            }
            else//a message end
            {
                if(*(dst-2) == 0x0D)
                    dst -= 1;
                *(dst-1) = '\0';
                msg[++loop] = dst;
            }
        }
    }

/*
unsigned char *ppt;
len= 0;
ppt= msg[0];
printf("------\n");
while(*ppt)
    printf("%02x ", *ppt++);
*/

load_language_msg_error:
  fclose(fp);
  return ret;
}

/*--------------------------------------------------------
  加载字体库
--------------------------------------------------------*/
u32 load_font()
{
    return (u32)BDF_font_init();
}

void get_savestate_filename_noshot(u32 slot, char *name_buffer)
{
  char savestate_ext[16];

  sprintf(savestate_ext, "_%d.rts", (int)slot);
  change_ext(gamepak_filename, name_buffer, savestate_ext);
}

/*--------------------------------------------------------
  游戏的设置文件
--------------------------------------------------------*/
s32 save_game_config_file()
{
    char game_config_filename[MAX_PATH];
    FILE_ID game_config_file;
    char *pt;

    if(gamepak_filename[0] == 0) return -1;

    memcpy(game_config.gamepad_config_map, gamepad_config_map, sizeof(game_config.gamepad_config_map));
    game_config.gamepad_config_home = gamepad_config_home;

    sprintf(game_config_filename, "%s/%s", DEFAULT_CFG_DIR, gamepak_filename);
    pt = strrchr(game_config_filename, '.');
    *pt = '\0';
    strcat(pt, "_0.rts");

    FILE_OPEN(game_config_file, game_config_filename, WRITE);
    if(FILE_CHECK_VALID(game_config_file))
    {
        FILE_WRITE(game_config_file, NGBARTS_HEADERA, NGBARTS_HEADERA_SIZE);
        FILE_WRITE_VARIABLE(game_config_file, game_config);
        FILE_CLOSE(game_config_file);
        return 0;
    }

    return -1;
}

/*--------------------------------------------------------
  dsGBA 的配置文件
--------------------------------------------------------*/
s32 save_config_file()
{
    char gpsp_config_path[MAX_PATH];
    FILE_ID gpsp_config_file;

    sprintf(gpsp_config_path, "%s/%s", main_path, GPSP_CONFIG_FILENAME);

    if(game_config.use_default_gamepad_map == 1)
        memcpy(gpsp_config.gamepad_config_map, gamepad_config_map, sizeof(gpsp_config.gamepad_config_map));

    FILE_OPEN(gpsp_config_file, gpsp_config_path, WRITE);
    if(FILE_CHECK_VALID(gpsp_config_file))
    {
        FILE_WRITE(gpsp_config_file, GPSP_CONFIG_HEADER, GPSP_CONFIG_HEADER_SIZE);
        FILE_WRITE_VARIABLE(gpsp_config_file, gpsp_config);
        FILE_CLOSE(gpsp_config_file);
        return 0;
    }

    return -1;
}

/******************************************************************************
 * local function definition
 ******************************************************************************/
void reorder_latest_file(void)
{
    char *pt, *ext_pos, *ext_pos1;
    u32 i, len;
    char full_file[512];

    if(gamepak_filename[0] == '\0')
        return;

    for(i= 0; i < 5; i++)
    {
        ext_pos= strrchr(gpsp_config.latest_file[i], '/');
        if(ext_pos != NULL)
        {
            ext_pos1= strrchr(ext_pos + 1, '.');
            len= ext_pos1 - ext_pos -1;
            if(!strncasecmp(ext_pos + 1, gamepak_filename, len))
                break;
        }
    }

    //some one match, ly to last
    if(i < 5)
    {
        if(i < 4)
        {
            strcpy(full_file, gpsp_config.latest_file[i]);
            for(i+=1; i < 5; i++)
            {
                if(gpsp_config.latest_file[i][0] != '\0')
                    strcpy(gpsp_config.latest_file[i-1], gpsp_config.latest_file[i]);
                else
                    break;
            }

            strcpy(gpsp_config.latest_file[i-1], full_file);
        }
        return ;
    }

    //none match
    for(i= 0; i < 5; i++)
    {
        if(gpsp_config.latest_file[i][0] == '\0')
        {
            //Removing rom_path due to confusion, replacing with g_default_rom_dir
            sprintf(gpsp_config.latest_file[i], "%s/%s", g_default_rom_dir, gamepak_filename);
            break;
        }
    }

    if(i < 5) return;

    //queue full
    for(i=1; i < 5; i++)
        strcpy(gpsp_config.latest_file[i-1], gpsp_config.latest_file[i]);

    //Removing rom_path due to confusion, replacing with g_default_rom_dir
    sprintf(gpsp_config.latest_file[i-1], "%s/%s", g_default_rom_dir, gamepak_filename);
}


static int rtc_time_cmp(struct rtc *t1, struct rtc *t2)
{
    int result;

    result= (int)((unsigned int)(t1 -> year) - (unsigned int)(t2 -> year));
    if(result != 0)
        return result;
    result= (int)((unsigned int)(t1 -> month) - (unsigned int)(t2 -> month));
    if(result != 0)
        return result;
    result= (int)((unsigned int)(t1 -> day) - (unsigned int)(t2 -> day));
    if(result != 0)
        return result;
    result= (int)((unsigned int)(t1 -> weekday) - (unsigned int)(t2 -> weekday));
    if(result != 0)
        return result;
    result= (int)((unsigned int)(t1 -> hours) - (unsigned int)(t2 -> hours));
    if(result != 0)
        return result;
    result= (int)((unsigned int)(t1 -> minutes) - (unsigned int)(t2 -> minutes));
    if(result != 0)
        return result;
    result= (int)((unsigned int)(t1 -> seconds) - (unsigned int)(t2 -> seconds));
    if(result != 0)
        return result;
}


int load_game_stat_snapshot(char* file)
{
	FILE* fp;
	char tmp_path[MAX_PATH];
	unsigned int n, m;

	sprintf(tmp_path, "%s/%s", DEFAULT_SAVE_DIR, file);
	fp = fopen(tmp_path, "r");
	if(NULL == fp)
		return -1;

	m = fread((void*)&n, 1, 4, fp);
	if(m < 4)
	{
		fclose(fp);
		return - 2;
	}

	fseek(fp, n+sizeof(struct rtc), SEEK_SET);

	m = fread(up_screen_addr, 1, 256*192*2, fp);
	if(m < 256*192*2)
	{
		fclose(fp);
		return -4;
	}

	fclose(fp);
	ds2_flipScreen(UP_SCREEN, UP_SCREEN_UPDATE_METHOD);
	return 0;
}

static void get_savestate_filelist(void)
{
	int i;
	char savestate_path[MAX_PATH];
	char postdrix[8];
	char *pt;
	FILE *fp;
	unsigned int n, m;
	// Which is the latest?
	/* global */ latest_save = -1;
	struct rtc latest_save_time, current_time;
	memset(&latest_save_time, 0, sizeof (struct rtc));

	sprintf(savestate_path, "%s/%s", DEFAULT_SAVE_DIR, gamepak_filename);
	pt= strrchr(savestate_path, '.');
	for(i= 0; i < SAVE_STATE_SLOT_NUM; i++)
	{
		sprintf(postdrix, "_%d.rts", i+1);
		strcpy(pt, postdrix);

		fp= fopen(savestate_path, "r");
		if (fp != NULL)
		{
			SavedStateExistenceCache [i] = TRUE;
			m = fread((void*)&n, 1, 4, fp);
			if(m < 4) {
				fclose(fp);
				continue;
			}

			fseek(fp, n, SEEK_SET);
			/* Read back the time stamp */
			fread((char*)&current_time, 1, sizeof(struct rtc), fp);
			if (rtc_time_cmp (&current_time, &latest_save_time) > 0)
			{
				latest_save = i;
				latest_save_time = current_time;
			}
			fclose(fp);
		}
		else
			SavedStateExistenceCache [i] = FALSE;

		SavedStateExistenceCached [i] = TRUE;
	}

	if(latest_save < 0)
		savestate_index = 0;
	else
		savestate_index = latest_save;
}

static void get_savestate_filename(u32 slot, char *name_buffer)
{
	char savestate_ext[16];
	char *pt;

	sprintf(savestate_ext, "_%d.rts", slot+1);
	pt = strrchr(gamepak_filename, '/');
	if(NULL == pt)
		pt = gamepak_filename;
	else
		pt += 1;

	change_ext(pt, name_buffer, savestate_ext);
}

unsigned char SavedStateSquareX (u32 slot)
{
	return (SCREEN_WIDTH * (slot + 1) / (SAVE_STATE_SLOT_NUM + 1))
		- ICON_STATEFULL.x / 2;
}

unsigned char SavedStateFileExists (u32 slot)
{
	if (SavedStateExistenceCached [slot])
		return SavedStateExistenceCache [slot];

	char BaseName [MAX_PATH + 1];
	char FullName [MAX_PATH + 1];
	get_savestate_filename(slot, BaseName);
	sprintf(FullName, "%s/%s", DEFAULT_SAVE_DIR, BaseName);
	FILE *SavedStateFile = fopen(FullName, "r");
	unsigned char Result = SavedStateFile != NULL;
	if (Result)
	{
		fclose(SavedStateFile);
	}
	SavedStateExistenceCache [slot] = Result;
	SavedStateExistenceCached [slot] = TRUE;
	return Result;
}

void SavedStateCacheInvalidate (void)
{
	int i;
	for (i = 0; i < SAVE_STATE_SLOT_NUM; i++)
		SavedStateExistenceCached [i] = FALSE;
}

void get_newest_savestate(char *name_buffer)
{
    if (latest_save < 0)
    {
        name_buffer[0]= '\0';
        return;
    }

    get_savestate_filename(latest_save, name_buffer);
}

static u32 parse_line(char *current_line, char *current_str)
{
  char *line_ptr;
  char *line_ptr_new;

  line_ptr = current_line;
  /* NULL or comment or other */
  if((current_line[0] == 0) || (current_line[0] == '#') || (current_line[0] != '!'))
    return -1;

  line_ptr++;

  line_ptr_new = strchr(line_ptr, '\r');
  while (line_ptr_new != NULL)
  {
    *line_ptr_new = '\n';
    line_ptr_new = strchr(line_ptr, '\r');
  }

  line_ptr_new = strchr(line_ptr, '\n');
  if (line_ptr_new == NULL)
    return -1;

  *line_ptr_new = 0;

  // "\n" to '\n'
  line_ptr_new = strstr(line_ptr, "\\n");
  while (line_ptr_new != NULL)
  {
    *line_ptr_new = '\n';
    memmove((line_ptr_new + 1), (line_ptr_new + 2), (strlen(line_ptr_new + 2) + 1));
    line_ptr_new = strstr(line_ptr_new, "\\n");
  }

  strcpy(current_str, line_ptr);
  return 0;
}

static void print_status(u32 mode)
{
    char print_buffer_1[256];
//  char print_buffer_2[256];
//  char utf8[1024];
    struct rtc  current_time;
//  pspTime current_time;

	// Disabled [Neb]
    //get_nds_time(&current_time);
//  sceRtcGetCurrentClockLocalTime(&current_time);
//  int wday = sceRtcGetDayOfWeek(current_time.year, current_time.month , current_time.day);

//    get_timestamp_string(print_buffer_1, MSG_MENU_DATE_FMT_0, current_time.year+2000,
//        current_time.month , current_time.day, current_time.weekday, current_time.hours,
//        current_time.minutes, current_time.seconds, 0);
//    sprintf(print_buffer_2,"%s%s", msg[MSG_MENU_DATE], print_buffer_1);

	// Disabled [Neb]
    // get_time_string(print_buffer_1, &current_time);
    //PRINT_STRING_BG(down_screen_addr, print_buffer_1, COLOR_BLACK, COLOR_TRANS, 44, 176);
    //PRINT_STRING_BG(down_screen_addr, print_buffer_1, COLOR_HELP_TEXT, COLOR_TRANS, 43, 175);

//    sprintf(print_buffer_1, "nGBA Ver:%d.%d", VERSION_MAJOR, VERSION_MINOR);
//    PRINT_STRING_BG(down_screen_addr, print_buffer_1, COLOR_HELP_TEXT, COLOR_BG, 0, 0);

//  sprintf(print_buffer_1, msg[MSG_MENU_BATTERY], scePowerGetBatteryLifePercent(), scePowerGetBatteryLifeTime());
//  PRINT_STRING_BG(down_screen_addr, print_buffer_1, COLOR_HELP_TEXT, COLOR_BG, 240, 0);

//    sprintf(print_buffer_1, "MAX ROM BUF: %02d MB Ver:%d.%d %s %d Build %d",
//        (int)(gamepak_ram_buffer_size/1024/1024), VERSION_MAJOR, VERSION_MINOR,
//        VER_RELEASE, VERSION_BUILD, BUILD_COUNT);
//    PRINT_STRING_BG(down_screen_addr, print_buffer_1, COLOR_HELP_TEXT, COLOR_BG, 10, 0);

//  if (mode == 0)
//  {
//    strncpy(print_buffer_1, gamepak_filename, 40);
//    PRINT_STRING_BG_SJIS(utf8, print_buffer_1, COLOR_ROM_INFO, COLOR_BG, 10, 10);
//    sprintf(print_buffer_1, "%s  %s  %s  %0X", gamepak_title, gamepak_code,
//        gamepak_maker, (unsigned int)gamepak_crc32);
//    PRINT_STRING_BG(down_screen_addr, print_buffer_1, COLOR_ROM_INFO, COLOR_BG, 10, 20);
//  }
}

#if 0
#define PSP_SYSTEMPARAM_DATE_FORMAT_YYYYMMDD	0
#define PSP_SYSTEMPARAM_DATE_FORMAT_MMDDYYYY	1
#define PSP_SYSTEMPARAM_DATE_FORMAT_DDMMYYYY	2

static void get_timestamp_string(char *buffer, u16 msg_id, u16 year, u16 mon,
    u16 day, u16 wday, u16 hour, u16 min, u16 sec, u32 msec)
{
/*
  char *weekday_strings[] =
  {
    msg[MSG_WDAY_0], msg[MSG_WDAY_1], msg[MSG_WDAY_2], msg[MSG_WDAY_3],
    msg[MSG_WDAY_4], msg[MSG_WDAY_5], msg[MSG_WDAY_6], ""
  };

  switch(date_format)
  {
    case PSP_SYSTEMPARAM_DATE_FORMAT_YYYYMMDD:
      sprintf(buffer, msg[msg_id    ], year, mon, day, weekday_strings[wday], hour, min, sec, msec / 1000);
      break;
    case PSP_SYSTEMPARAM_DATE_FORMAT_MMDDYYYY:
      sprintf(buffer, msg[msg_id + 1], weekday_strings[wday], mon, day, year, hour, min, sec, msec / 1000);
      break;
    case PSP_SYSTEMPARAM_DATE_FORMAT_DDMMYYYY:
      sprintf(buffer, msg[msg_id + 2], weekday_strings[wday], day, mon, year, hour, min, sec, msec / 1000);
      break;
  }
*/
    char *weekday_strings[] =
    {
        "SUN", "MON", "TUE", "WED", "TUR", "FRI", "SAT"
    };

    sprintf(buffer, "%s %02d/%02d/%04d %02d:%02d:%02d", weekday_strings[wday],
        day, mon, year, hour, min, sec);
}

static void get_time_string(char *buff, struct rtc *rtcp)
{
    get_timestamp_string(buff, NULL,
                            rtcp -> year +2000,
                            rtcp -> month,
                            rtcp -> day,
                            rtcp -> weekday,
                            rtcp -> hours,
                            rtcp -> minutes,
                            rtcp -> seconds,
                            0);
}
#endif

void LowFrequencyCPU()
{
	ds2_setCPUclocklevel(0); // 60 MHz
}

void HighFrequencyCPU()
{
	ds2_setCPUclocklevel(13); // 396 MHz
}

void GameFrequencyCPU()
{
	u32 clock_speed_table[6] = {6, 9, 10, 11, 12, 13};	//240, 300, 336, 360, 384, 396

	if(game_config.clock_speed_number <= 5)
		ds2_setCPUclocklevel(clock_speed_table[game_config.clock_speed_number]);
}

static u32 save_ss_bmp(u16 *image)
{
    static unsigned char header[] ={ 'B',  'M',  0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00,
                                    0x28, 0x00, 0x00, 0x00,  0, 0x01, 0x00,
                                    0x00,  192, 0x00, 0x00, 0x00, 0x01, 0x00,
                                    0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00};

    char ss_filename[MAX_FILE];
    char save_ss_path[MAX_PATH];
    struct rtc current_time;
    char rgb_data[256*192*3];
    unsigned int x,y;
    unsigned short col;
    unsigned char r,g,b;

    change_ext(gamepak_filename, ss_filename, "_");
	ds2_getTime(&current_time);
    sprintf(save_ss_path, "%s/%s%02d%02d%02d%02d%02d.bmp", DEFAULT_SS_DIR, ss_filename,
    current_time.month, current_time.day, current_time.hours, current_time.minutes, current_time.seconds);

    for(y = 0; y < 192; y++)
    {
        for(x = 0; x < 256; x++)
        {
            col = image[x + y * 256];
            r = (col >> 10) & 0x1F;
            g = (col >> 5) & 0x1F;
            b = (col) & 0x1F;

            rgb_data[(191-y)*256*3+x*3+2] = b << 3;
            rgb_data[(191-y)*256*3+x*3+1] = g << 3;
            rgb_data[(191-y)*256*3+x*3+0] = r << 3;
        }
    }

    FILE *ss = fopen( save_ss_path, "wb" );
    if( ss == NULL ) return 0;
    fwrite( header, sizeof(header), 1, ss );
    fwrite( rgb_data, 1, 256*192*3, ss);
    fclose( ss );

    return 1;
}

// Fill these in [Neb]
void game_disableAudio() {}
void game_set_frameskip() {}

/*
* GUI Initialize
*/
static bool Get_Args(char *file, char **filebuf){
  FILE* dat = fat_fopen(file, "rb");
  if(dat){
    int i = 0;
    while(!fat_feof (dat)){
      fat_fgets(filebuf[i], 512, dat);
      int len = strlen(filebuf[i]);
      if(filebuf[i][len - 1] == '\n')
        filebuf[i][len - 1] = '\0';
      i++;
    }

    fat_fclose(dat);
    fat_remove(file);
    return i;
  }
  return 0;
}

int CheckLoad_Arg(){
  argv[0][0] = '\0';  // Initialise the first byte to be a NULL in case
  argv[1][0] = '\0';  // there are no arguments to avoid uninit. memory
  char *argarray[2];
  argarray[0] = argv[0];
  argarray[1] = argv[1];

  if(!Get_Args("/plgargs.dat", argarray))
    return 0;

  fat_remove("plgargs.dat");
  return 1;
}

int gui_init(u32 lang_id)
{
	int flag;

	HighFrequencyCPU(); // Crank it up. When the menu starts, -> 0.

    //Find the "TEMPGBA" system directory
    DIR *current_dir;

    if(CheckLoad_Arg()){
      //copy new folder location
      strcpy(main_path, "fat:");
      strcat(main_path, argv[0]);
      //strip off the binary name
      char *endStr = strrchr(main_path, '/');
      *endStr = '\0';

      //do a check to make sure the folder is a valid catsfc folder
      char tempPath[MAX_PATH];
      strcpy(tempPath, main_path);
      strcat(tempPath, "/system/gui");
      DIR *testDir = opendir(tempPath);
      if(!testDir)
        //not a valid catsfc install
        strcpy(main_path, "fat:/TEMPGBA");
      else//test was successful, do nothing
        closedir(testDir);
    }
    else
      strcpy(main_path, "fat:/TEMPGBA");



    current_dir = opendir(main_path);
    if(current_dir)
        closedir(current_dir);
    else
    {
        strcpy(main_path, "fat:/_SYSTEM/PLUGINS/TEMPGBA");
        current_dir = opendir(main_path);
        if(current_dir)
            closedir(current_dir);
        else
        {
            strcpy(main_path, "fat:");
            if(search_dir("TEMPGBA", main_path) == 0)
            {
                printf("Found TEMPGBA directory\r\nDossier TEMPGBA trouve\r\n\r\n%s\r\n", main_path);
            }
            else
            {
				err_msg(DOWN_SCREEN, "/TEMPGBA: Directory missing\r\nPress any key to return to\r\nthe menu\r\n\r\n/TEMPGBA: Dossier manquant\r\nAppuyer sur une touche pour\r\nretourner au menu");
                goto gui_init_err;
            }
        }
    }

	show_log(down_screen_addr);
	ds2_flipScreen(DOWN_SCREEN, DOWN_SCREEN_UPDATE_METHOD);

  //should probably configure the language before using it
  load_config_file();
  lang_id = gpsp_config.language;

	flag = icon_init(lang_id);
	if(0 != flag)
	{
		err_msg(DOWN_SCREEN, "Some icons are missing\r\nLoad them onto your card\r\nPress any key to return to\r\nthe menu\r\n\r\nDes icones sont manquantes\r\nChargez-les sur votre carte\r\nAppuyer sur une touche pour\r\nretourner au menu");
		goto gui_init_err;
	}

	flag = load_font();
	if(0 != flag)
	{
		char message[512];
		sprintf(message, "Font library initialisation\r\nerror (%d)\r\nPress any key to return to\r\nthe menu\r\n\r\nErreur d'initalisation de la\r\npolice de caracteres (%d)\r\nAppuyer sur une touche pour\r\nretourner au menu", flag, flag);
		err_msg(DOWN_SCREEN, message);
		goto gui_init_err;
	}


	flag = load_language_msg(LANGUAGE_PACK, lang_id);
	if(0 != flag)
	{
		char message[512];
		sprintf(message, "Language pack initialisation\r\nerror (%d)\r\nPress any key to return to\r\nthe menu\r\n\r\nErreur d'initalisation du\r\npack de langue (%d)\r\nAppuyer sur une touche pour\r\nretourner au menu", flag, flag);
		err_msg(DOWN_SCREEN, message);
		goto gui_init_err;
	}

	// initial_path_config(); // Enable this later [Neb]

  //gpsp_main expecting a return value
	return 1;

gui_init_err:
	ds2_flipScreen(DOWN_SCREEN, DOWN_SCREEN_UPDATE_METHOD);
	wait_Anykey_press(0);
	quit();
}

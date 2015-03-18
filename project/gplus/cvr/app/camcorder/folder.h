#include <dirent.h>

#define TIME_IDX_FILE_NAME	0
#define DATETIME_FILE_NAME	1

typedef struct folderInfo_s {
	struct dirent** folder_list;
	struct dirent** file_list;
	int folder_num;
	int old_file_num;
	int delete_folder_idx;
	int delete_idx;
	int name_format;
	char working_path[256];
	char root_path[256];
	char folder_name[256];
	char* postfix;
} folderInfo_t;

void* cam_folder_init(char* path, char* folder_name, char* postfix, int* file_num, int name_format);
int cam_get_new_file_name(void* handle, char* filename, char* ext, int idx);
int cam_delete_file(void* handle, int size);
int cam_folder_close(void* handle);
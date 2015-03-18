#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include "mach/typedef.h"
#include "folder.h"

static int folder_select(const struct dirent* f)
{
	int fnum;
	char* tmp;
	
	if (strlen(f->d_name) != 8)
		return 0;
		
	tmp = strdup(f->d_name);
	tmp[3] = 0;

	fnum = (int)strtoul(tmp, NULL, 10);
	free(tmp);
		
	if (!fnum)
		return 0;
		
	return 1;
}

static int folder_check(struct dirent* f, char* working_path)
{
	struct stat st;
	char filename[256];
	
	sprintf(filename, "%s/%s", working_path, f->d_name);
	if (stat(filename, &st) < 0)
		return -1;
		
	if (S_ISDIR(st.st_mode))
		return 0;
	else
		return -1;	
}

//static int folder_select(const struct dirent* f)
//{
//	struct stat st;
//	char filename[256];
//	
//	if (strlen(f->d_name) != 8)
//		return 0;
//
//	sprintf(filename, "%s/%s", working_path, f->d_name);
//	if (stat(filename, &st) < 0)
//		return 0;
//		
//	if (S_ISDIR(st.st_mode))
//	{
//		int fnum;
//		char* tmp;
//		
//		tmp = strdup(f->d_name);
//		tmp[3] = 0;
//
//		fnum = (int)strtoul(tmp, NULL, 10);
//		free(tmp);
//		
//		if (!fnum)
//			return 0;
//	}
//	else
//		return 0;
//	
//	return 1;
//}

static int folder_compar(const struct dirent** f1,	const struct dirent** f2)
{
	int fnum1, fnum2;
	
	fnum1 = (int)strtoul((*f1)->d_name, NULL, 10);
	fnum2 = (int)strtoul((*f2)->d_name, NULL, 10);
	
	return fnum1 - fnum2;
}

static int file_select(const struct dirent* f)
{
	char* tmp;
	int fnum;
	
	if (strlen(f->d_name) != 12)
		return 0;

	if (strcasecmp(&f->d_name[9], "MOV") != 0 && strcasecmp(&f->d_name[9], "JPG") != 0)
		return 0;
		
	tmp = strdup(f->d_name);
	tmp[9] = 0;
			
	fnum = (int)strtoul(&tmp[4], NULL, 10);
	
	free(tmp);
	
	if (!fnum)
		return 0;
				
	return 1;
}

static int file_compar(const struct dirent** f1,	const struct dirent** f2)
{
	int fnum1, fnum2;
	char *s1, *s2;
	
	s1 = strdup((*f1)->d_name);
	s2 = strdup((*f2)->d_name);
	
	s1[9] = 0;
	s2[9] = 0;
	
	fnum1 = (int)strtoul(&s1[4], NULL, 10);
	fnum2 = (int)strtoul(&s2[4], NULL, 10);
	
	free(s1);
	free(s2);
	return fnum1 - fnum2;
}

static int file_select2(const struct dirent* f)
{
	char* tmp;
	int fnum;
	
	if (strlen(f->d_name) != 11)
		return 0;

	if (strcasecmp(&f->d_name[8], "MOV") != 0 && strcasecmp(&f->d_name[8], "JPG") != 0)
		return 0;
		
	tmp = strdup(f->d_name);
	tmp[7] = 0;
			
	fnum = (int)strtoul(&tmp[0], NULL, 10);
	
	free(tmp);
	
	if (!fnum)
		return 0;
				
	return 1;
}

static int file_compar2(const struct dirent** f1,	const struct dirent** f2)
{
	int fnum1, fnum2;
	char *s1, *s2;
	
	s1 = strdup((*f1)->d_name);
	s2 = strdup((*f2)->d_name);
	
	s1[7] = 0;
	s2[7] = 0;
	
	fnum1 = (int)strtoul(&s1[0], NULL, 10);
	fnum2 = (int)strtoul(&s2[0], NULL, 10);
	
	free(s1);
	free(s2);
	return fnum1 - fnum2;
}

static int folder_scan(folderInfo_t* fi)
{
	DIR *dir;
	char path[256], subpath[10];
	struct dirent* ptr;
	struct dirent** list = NULL;
	int fldidx = 0, fidx = 0;
	int fnum, file_num = 0;
	int i, j;
	
	memset(fi->working_path, 0, 256);
	
	dir = opendir(fi->root_path);
	
	if (!dir)
	{
		printf("[folder] open directory %s fail: %s\n", fi->root_path, strerror(errno));
		return -1;
	}
	
	closedir(dir);
		
	if (fi->root_path[strlen(fi->root_path)-1] == '/')
		fi->root_path[strlen(fi->root_path)-1] = 0;
		
	sprintf(path, "%s/%s", fi->root_path, fi->folder_name);
	
	dir = opendir(path);
		
	if (!dir)
	{
		if (errno == ENOENT)
		{
			printf("[folder] directory not found, create %s\n", path);
			if (0 > mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO))
			{
				printf("[folder] canot create directory %s. error: %s\n", path, strerror(errno));
				return -1;
			}			
		}
		
		dir = opendir(path);
		
		if (!dir)
		{
			printf("[folder] open directory %s fail. error: %s\n", path, strerror(errno));
			return -1;
		}
	}
	
	closedir(dir);
	
	strcpy(fi->working_path, path);

	fi->folder_num = scandir(path, &fi->folder_list, folder_select, folder_compar);
	if (0 > fi->folder_num)
	{
		printf("[folder] cannot scan directory %s. errno: %s\n", path, strerror(errno));
		return -1;
	}

	for (i = 0; i < fi->folder_num; i++)
	{
		if (0 > folder_check(fi->folder_list[i], fi->working_path))
		{
			printf("[folder scan] skipped %s\n", fi->folder_list[i]->d_name);
			free(fi->folder_list[i]);
			fi->folder_list[i] = NULL;
		}
	}
	for (i = 0, j = 0; i < fi->folder_num; i++)
	{
		if (fi->folder_list[i])
		{
			if (j < i)
			{
				fi->folder_list[j+1] = fi->folder_list[i];
				fi->folder_list[i] = NULL;
			}
			printf("[folder scan] found %s\n", fi->folder_list[j]->d_name);
			j++;
		}
	}
	fi->folder_num = j;
				
	fi->delete_folder_idx = 0;
	
	if (fi->folder_num)
	{
		strcat(path, "/");
		strcat(path, fi->folder_list[fi->folder_num-1]->d_name);
	
		if (fi->name_format == 1)
			file_num = scandir(path, &list, file_select2, file_compar2);
		else
			file_num = scandir(path, &list, file_select, file_compar);
		if (0 > file_num)
		{
			printf("[folder] cannot scan directory %s. errno: %s\n", path, strerror(errno));
			return -1;
		}
		
		//for (i = 0; i < file_num; i++)
			//printf("[file scan] %s\n", list[i]->d_name);
	}
	
	if (file_num)
	{
		char* tmp;
		
		if (fi->name_format == 1)
			fnum = file_num;
		else
		{
			tmp = strdup(list[file_num-1]->d_name);
			tmp[8] = 0;
			fnum = (int)strtoul(&tmp[4], NULL, 10);
			free(tmp);
			fnum++;

			if (fnum > 9999)
				fnum = 1;
		}
			
		if (list)
		{
			while(file_num)
				free(list[--file_num]);
			
			free(list);
		}
	}
	else
		fnum = 1;
		
	return fnum;
}

static void get_local_time(struct tm* tm)
{
	static struct timeval global_tv;
	static struct tm global_tm;
	static int first = 1;

	struct timeval current_tv;
	struct tm current_tm;
	time_t t;
	
	gettimeofday(&current_tv, NULL);
	time(&t);
	current_tm = *localtime(&t);
	
	if (first)
	{
		first = 0;
		global_tv = current_tv;
		global_tm = current_tm;
	}
	
	if (current_tv.tv_sec - global_tv.tv_sec < 3)
	{
		*tm = global_tm;
	}
	else
	{
		*tm = current_tm;
		global_tm = current_tm;
		global_tv = current_tv;
	}
}

static int get_index_folder_name(char* foldername, int index, char* path)
{
	struct tm tm;
	int year,month,day;

	get_local_time(&tm);
	
	year = tm.tm_year % 10;
	month = tm.tm_mon + 1;
	day = tm.tm_mday;
	
	sprintf(foldername, "%s/%03d%d%02d%02d", path, index, year, month, day);
	
	return 0;
}

static int get_date_folder_name(char* foldername, char* path)
{
	struct tm tm;
	int year,month,day;
	
	get_local_time(&tm);
	
	year = tm.tm_year + 1900;
	month = tm.tm_mon + 1;
	day = tm.tm_mday;
	
	sprintf(foldername, "%s/%4d%02d%02d", path, year, month, day);
	
	return 0;
}

static int get_index_file_name(char* filename, int index, char* path, char* ext)
{
	struct tm tm;
	int hour, minute;
	
	get_local_time(&tm);
	
	hour = tm.tm_hour;
	minute = tm.tm_min;
	
	sprintf(filename, "%s/%02d%02d%04d.%s", path, hour, minute, index, ext);
	
	return 0;
}

static int get_time_file_name(char* filename, char* path, char* postfix, char* ext)
{
	struct tm tm;
	int hour, minute, second;
	
	get_local_time(&tm);
	
	hour = tm.tm_hour;
	minute = tm.tm_min;
	second = tm.tm_sec;
	
	if (postfix)
		sprintf(filename, "%s/%02d%02d%02d%s.%s", path, hour, minute, second, postfix, ext);
	else
		sprintf(filename, "%s/%02d%02d%02d.%s", path, hour, minute, second, ext);
	
	return 0;
}

static int get_current_folder2(folderInfo_t* fi, char* folderpath)
{
	char path[256];
	int fnum = 0;
	DIR *dir;
	
	get_date_folder_name(path, fi->working_path);
	
	dir = opendir(path);
		
	if (!dir)
	{
		if (errno == ENOENT)
		{
			if (0 > mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO))
			{
				printf("[folder] canot create directory %s. error: %s\n", path, strerror(errno));
				return -1;
			}			
		}
		
		dir = opendir(path);
		
		if (!dir)
		{
			printf("[folder] open directory %s fail. error: %s\n", path, strerror(errno));
			return -1;
		}
		fnum = folder_scan(fi);
	}
	
	closedir(dir);
	
	strcpy(folderpath, path);
	
	return fnum;
}

static int get_current_folder(folderInfo_t* fi, char* folderpath)
{
	int fnum = 0, new_folder = 0;
	char tmp[256], tmp2[256];
	
	if (!fi->folder_num)
	{
		fnum = 100;
		new_folder = 1;
		get_index_folder_name(tmp, fnum, fi->working_path);
	}
	else
	{
		strcpy(tmp, fi->folder_list[fi->folder_num-1]->d_name);
		tmp[3] = 0;
		fnum = (int)strtoul(tmp, NULL, 10);
		
		sprintf(tmp2, "%s/%s", fi->working_path, fi->folder_list[fi->folder_num-1]->d_name);
		get_index_folder_name(tmp, fnum, fi->working_path);
		
		if (strcmp(tmp, tmp2))
		{
			new_folder = 1;
			fnum++;
			get_index_folder_name(tmp, fnum, fi->working_path);
		}
	}
	
	strcpy(folderpath, tmp);

	if (new_folder)
	{
		printf("[folder] create directory %s\n", folderpath);
		if (0 > mkdir(folderpath, S_IRWXU | S_IRWXG | S_IRWXO))
		{
			printf("[folder] cannot create directory %s, error: %s\n", folderpath, strerror(errno));
			if (errno != EEXIST)
				return -1;
		}
		fnum = folder_scan(fi);
	}
	else
		fnum = 0;
	
	return fnum;
}

int cam_get_new_file_name(void* handle, char* filename, char* ext, int idx)
{
	folderInfo_t* fi = (folderInfo_t*)handle;
	int ret;
	char folder[256];
	
	if (fi->name_format == 1)
	{
		ret = get_current_folder2(fi, folder);

		if (0 > ret)
			return -1;
			
		get_time_file_name(filename, folder, fi->postfix, ext);
		
		return 0;
	}
	else
	{
		ret = get_current_folder(fi, folder);

		if (0 > ret)
			return -1;
	
		if (ret)
			idx = ret;
		
		get_index_file_name(filename, idx, folder, ext);
		
		return idx;
	}
}

int cam_delete_file(void* handle, int size)
{
	folderInfo_t* fi = (folderInfo_t*)handle;
	char folder[256];
	char file[256];
	int i;
	UINT64 del_space = 0;
	int current_folder = 0;
	
	if (!fi->folder_num)
		return 0;

	if (fi->delete_folder_idx == fi->folder_num)
		folder_scan(fi);

	if (!size)
		size = 1;

	//printf("[folder] delete minimum size = %u, old_file_num = %d\n", size, old_file_num);
	
	while(1)
	{
		sprintf(folder, "%s/%s", fi->working_path, fi->folder_list[fi->delete_folder_idx]->d_name);
		if (fi->old_file_num < 1)
		{
			fi->delete_idx = 0;    
    		printf("[folder] scan directory [%d] to delete: %s\n", fi->delete_folder_idx, folder);	
			if (fi->name_format == 1)
				fi->old_file_num = scandir(folder, &fi->file_list, file_select2, file_compar2);
			else
				fi->old_file_num = scandir(folder, &fi->file_list, file_select, file_compar);
			if (0 > fi->old_file_num)
			{
				printf("[folder] cannot scan directory %s. errno: %s\n", folder, strerror(errno));
				return -1;
			}
			
			current_folder = 0;
			if (fi->delete_folder_idx == (fi->folder_num - 1))
			{
				current_folder = 1;	
				fi->old_file_num --;
			}
				
			//for (i = 0; i < old_file_num; i++)
				//printf("[folder scan] [%d] %s\n", i, file_list[i]->d_name);
		}
					
		for (i = fi->delete_idx; i < fi->old_file_num; i++)
		{
			struct stat stbuf;
			
			if (del_space >= size)
				break;

			sprintf(file, "%s/%s", folder, fi->file_list[i]->d_name);
			
			stat(file, &stbuf);
			
			if (stbuf.st_mode & S_IWUSR)
			{
				printf("[folder] del the file [%d] %s\n", i, file);
				if(unlink(file) == 0)
					del_space += stbuf.st_size;
			}
		}
		
		fi->delete_idx = i;
		
		if (i == fi->old_file_num || i == 0)
		{
			if (current_folder)
				fi->old_file_num++;
				
			for (i = 0; i < fi->old_file_num; i++)
				free(fi->file_list[i]);

			if (fi->file_list)
			{
				free(fi->file_list);
				fi->file_list = NULL;
			}
					
			if (!current_folder)
			{
				printf("[folder] remove directory %s\n", folder);
				if (0 > rmdir(folder))
					printf("[folder] cannot delete folder %s, error:%s\n", folder, strerror(errno));
				fi->delete_folder_idx ++;
			}
			fi->delete_idx = 0;
			fi->old_file_num = 0;
		}
		
		if (del_space >= size || fi->delete_folder_idx == fi->folder_num || current_folder)
			break;
	}
	
	printf("[folder] del %llu bytes\n", del_space);
	return del_space;
}

void* cam_folder_init(char* path, char* folder_name, char* postfix, int* file_num, int name_format)
{
	folderInfo_t* fi;
	int ret;
	
	fi = malloc(sizeof(folderInfo_t));
	
	if (!fi)
	{
		printf("[%s] out of memory!\n", __FUNCTION__);
		return NULL;
	}
	
	memset(fi, 0, sizeof(folderInfo_t));
	
	strcpy(fi->root_path, path);
	strcpy(fi->folder_name, folder_name);
	fi->name_format = name_format;
	if (postfix)
		fi->postfix = strdup(postfix);
	printf("[%s] path = %s, directory = %s\n", __FUNCTION__, path, folder_name);	
	ret = folder_scan(fi);
	if (0 > ret)
	{
		printf("[%s] folder scan fail!\n", __FUNCTION__);
		free(fi);
		fi = NULL;
	}
	
	if(file_num)
		*file_num = ret;
	return (void*)fi;
}

int cam_folder_close(void* handle)
{
	folderInfo_t* fi = (folderInfo_t*)handle;
	int i;

	if (fi->file_list)
	{	
		for (i = 0; i < fi->old_file_num; i++)
			free(fi->file_list[i]);
		
		free(fi->file_list);
		fi->file_list = NULL;
	}
		
	if (fi->folder_list)
	{
		for (i = 0; i < fi->folder_num; i++)
			free(fi->folder_list[i]);
		
		free(fi->folder_list);
		fi->folder_list = NULL;
	}
		
	fi->old_file_num = 0;
	fi->folder_num = 0;
	
	if (fi->postfix)
		free(fi->postfix);
		
	free(fi);
}
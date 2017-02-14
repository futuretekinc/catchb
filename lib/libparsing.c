#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libsha1.h>
#include "catchb_trace.h"

#define DOTDOT ".."
#define DOT "."
#define LOST_FOUND "lost+found"
#define ARP_PATH "/proc/net/arp"
#define MAXLINE 128


int delDirFile(char *folder)
{
    DIR* dp;
    struct dirent *dirp;
    struct stat file_stat;
    char targetfile[512];

    if((dp = opendir(folder)) == NULL)
    {
        return -1;
    }
    while(1)
    {

        if(!(dirp = readdir(dp)))
        {
            closedir(dp);
            remove(folder);
            return -1;
		}

		if(strcmp(dirp->d_name, DOTDOT) == 0
				|| strcmp(dirp->d_name, DOT) == 0 || strcmp(dirp->d_name, LOST_FOUND) == 0)
			continue;

		sprintf(targetfile, "%s/%s", folder, dirp->d_name);
		stat(targetfile, &file_stat);

		if ( S_ISDIR(file_stat.st_mode) )        
        {
            if ( delDirFile(targetfile) == -1 ) 
            {
                rmdir(targetfile);
            }

            continue;
        }
        if ( unlink(targetfile) == -1 )
        {
            return -1; 
        }

        continue;
    }

    int result = rmdir(folder);
    closedir(dp);

    return result;
}


int cctv_make_folder(char *path)
{
 
    int nResult = mkdir(path, 0755);
	if (nResult != 0)
	{
		if (errno == 17)
		{
			nResult = delDirFile(path);
			if (nResult == 0)
			{
				nResult = mkdir(path, 0755);
			}

		}
		else if(errno != 0)
		{
			ERROR(errno, "Failed to make dir.");
			return 1;
 		}
	}

 return nResult;
}

int cctv_file_open(char * path, char * string, int rw)
{
	FILE *f;

	if(!rw)
	{
		f = fopen(path, "w");
		fprintf(f, "%s",string);
		fclose(f);
	}
	else
	{
		f = fopen(path, "r");
		while(!feof(f))
		{
			fgets(string, 100, f);
		}
		fclose(f);


	}

	return 0;
}

int cctv_input(char * input)
{ 
    scanf("%s", input);
    return 0;
}


int cctv_hash_sha1(char * h_string, char * h_value, int size)
{
    SHA1Context sha;
    int i;
    int len;


    /*
     *  Perform test A
     */
    SHA1Reset(&sha);
    SHA1Input(&sha, (const unsigned char *) h_string, size);
    if (!SHA1Result(&sha))
    {
		ERROR(0, "ERROR-- could not compute message digest\n");
    }
    else
    {
        for(i = 0; i < 5 ; i++)
        {


            sprintf(h_value,"%s%x",h_value, sha.Message_Digest[i]);
        }

        len = strlen(h_value);
        for(i =0 ; i < len; i++){
            if(!isdigit(h_value[i])){
                if(h_value[i] >= 'a')
                    h_value[i]=h_value[i]-32;
                else 
                    h_value[i] = h_value[i]+32;

            }
        }

    }

    return 0;
}



int cctv_value_match(char * f_string, char * t_string)
{
	if(!strncmp(f_string, t_string, strlen(f_string)-5)){
		return 0;
	}
	else{ 
		return 1;
	}
}

int arp_parsing(char * check_ip, char *mac)
{
    FILE *fp;
    char *ptr = NULL;

    char buff[128];
    char pa_string[] = " ";
    int i;

    memset(buff, 0x00, sizeof(buff));

    fp = fopen(ARP_PATH, "r");

    if(fp == NULL)
        return 1;


    while(fgets(buff, MAXLINE, fp))
    {
        if(strstr(buff, check_ip))
        {

            ptr = strtok(buff, pa_string);

            i = 0;

            while(ptr != NULL){

                if(i == 3){

                    memcpy(mac, ptr, 48);

                    break;
                }
                ptr = strtok(NULL, pa_string);
                i++;
            }

            break;
        }
    }
    fclose(fp);
    return 0;
}

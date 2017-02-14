#ifndef	LIB_PARSING_H_
#define	LIB_PARSING_H_

int delDirFile(char *folder);
int cctv_make_folder(char *path);
int cctv_file_open(char * path, char * string, int rw);
int cctv_input(char * input);
int cctv_hash_sha1(char * h_string, char * h_value, int size);
int cctv_value_match(char * f_string, char * t_string);
int arp_parsing(char * check_ip, char *mac);

#endif

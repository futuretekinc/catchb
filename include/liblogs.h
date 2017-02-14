#ifndef __LIBLOGS_H__
#define __LIBLOGS_H__


#include <string.h>
#include <errno.h>

/*-----------------fopen() 옵션 정의---------------------------------*/
// 읽기 전용으로 파일 오픈
// 파일이 없으면 NULL 리턴)
#define FILE_READ_ONLY   "r"

// 쓰기 전용으로 파일 오픈
// 파일이 없으면 생성, 있으면 내용 삭제)
#define FILE_WRITE_ONLY   "w"

// Append모드
// 파일이 없으면 생성. 이미 존재하면 파일 끝부분에 file pointer위치.
// 이 위치부터 뒤로 write 가능. 읽기 불가능
#define FILE_APPEND_ONLY   "a"

// 읽고 쓰기 모드
// 파일이 없으면 NULL 리턴
#define FILE_READ_WRITE   "r+"

// 읽고 쓰기 모드
// 단, 파일이 없으면 만들고, 있으면 기존내용 지움
// write를 먼저 한 후 동일 파일 포인터로 읽기 수행이 필요한 경우 사용.
// 보통은 읽기 전용, 혹은 쓰기 전용으로 fopen 하므로 거의 쓸일 없음.
#define FILE_READ_WRITE_CREATE   "w+"

// Append모드, 읽고 쓰기 가능.
// 파일이 이미 존재할 경우 그 파일의 끝부분에서부터 추가된 내용을 씀
// 읽기는 fseek로 지정한 file pointer위치에서 가능
// 쓰기는 파일 끝부분에서만 가능
#define FILE_APPEND_READ   "a+"

/*---------------------------------------------------------------------------*/

//#define LOG_PATH  "/opt/XANUS/logs/"
#define DEBUG_PATH  "/opt/XANUS/tmp/"

#define DEBUG_FILE    DEBUG_PATH "webcure_debug"

//#define LOG_FILE       LOG_PATH "exampled.log"   //각각의 기능/데몬/ 별로 각각의LOG_FILE 정의 후 사용

#define ERRIO   "입출력 오류"

//로그
#define SIGNATURE_UNUSAL_LOG     "SIGNATURE NOT MATCH CCTV"
#define NOMALITY_LOG             "NORMALITY CCTV"
#define PING_UNUSAL_LOG          "NETWORK ERROR CCTV"

//분석결과 상태
#define NOMALITY    1   //정상
#define UNUSAL      2   //비정상



void log_message(char *, const char *, ...);
void cctv_system_debugger(char *, const char *, ...);
void cctv_system_error(const char *msg, ...);
void sendlog(char *filename, int alarm, const char *message, ...);

#endif

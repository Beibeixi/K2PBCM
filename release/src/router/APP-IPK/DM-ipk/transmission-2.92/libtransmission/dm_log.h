#ifndef __LOGS_H
#define __LOGS_H

//#pragma (1)

#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>

#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

#define LocalHost       "127.0.0.1"
#define Serv_Port       8936

#define MAX_NAMELEN  256
#define MAX_SHORTNAMELEN  128
#define MAX_SHORTLEN  32

//Allen 20100919+++
#define SIZE_4G		(int64_t)4*1024*1024*1024  
//char mnt_type[MAX_SHORTLEN];
//int64_t Available_space;

                                                                                                                                               
#define FILE_MODE  (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

#define HTTP	0x60
#define FTP	0x61
#define BT 	0x62
#define NZB 	0x63
#define NNTP 	0x64	//Allen NNTP
#define ED2K 	0x65

#define S_PROCESSING		0x70	// process status is processing
#define S_PAUSED		0x71	// process is paused
#define S_COMPLETED		0x72	// download completed and sucessful
#define S_DEAD_OF_DELETED	0x73	// download failed because of user delete
#define S_DEAD_OF_ERROR		0x74	// downlaod failed because of download error
#define S_INITIAL		0x75	// download initial, usual in bt
#define S_NOTBEGIN		0x76	// download queue
#define S_PARSER_FAIL           0x77    // parser url failed add by gauss
#define S_NEEDACCOUNT		0x78    // ftp need usrname or password 
#define S_CONNECTFAIL           0x79   //nzb connected fail
#define S_SSLFAIL               0X80    //nzb ssl fail
#define S_ACCOUNTFAIL           0x81    //nzb account fail
#define S_RECVFAIL              0x82    //nzb soket rece fail
#define S_NOTCOMPLETED           0x83    //nzb downloaded file is incomplete
#define S_DISKFULL				0x84     //disk is full
#define S_TYPE_OF_ERROR				0x85     //disk is Vfat File more than 4G
#define S_HASH                  0x86        //the flag for creating hash
#define S_MOVE4GBERROR          0x87    //files more than 4GB in NTFS move to FAT32
#define S_MOVEDISKFULL          0x88
#define S_SEEDING		0x89	// download completed and sucessful Seeding

//struct sockaddr_in  servaddr;
//char share_dir[MAX_NAMELEN];
//char logname[MAX_NAMELEN];
//char semName[MAX_NAMELEN];
//int  log_use, semUse;

#define BT_LOG_LEN	(sizeof(bt_log) - 1)
#define HF_LOG_LEN	(sizeof(hf_log) - 1)

typedef struct INFO_BT{
        int id;
        uint32_t pieceSize;
        uint32_t pieceCount;
	bool isPrivate;
        char comment[1024];
	char creator[32];
	time_t dateCreated;
	char downloadDir[64];
	uint64_t have;
	uint64_t haveValid;
	float  availability;
	uint64_t  downloadedsize;
	uint64_t  uploadedsize;
	float ratio;
	char errorString[256];
        double  downloadSpeed;		// current download speed
        double  uploadSpeed;		// current upload speed
        int  uploadpeers;
        int  downloadpeers;
	double health;	//Health ratio
        uint32_t fileCount;
        uint32_t trackCount;
        char serial[200];
        char pid[5];
        char vid[5];
        int partitionnum;
}Info_bt;

typedef struct INFO_FTP{
	char  proxy[MAX_SHORTNAMELEN];
	char  host[MAX_SHORTNAMELEN];
	uint32_t  port;
	char  user_name[MAX_SHORTLEN];
	char  password[MAX_SHORTLEN];
	double  elapsed;
	float  rate;  // K/s
}Info_ftp;

typedef struct INFO_HTTP{
        char  proxy[MAX_SHORTNAMELEN];
        double  elapsed;
        float  rate;  // K/s
}Info_http;

typedef struct INFO_NZB{
        double  elapsed;
        float  rate;  // K/s
}Info_nzb;

typedef union FILE_INFO{
	Info_bt  iBt;
	Info_ftp  iFtp;
        Info_http  iHttp;
        Info_nzb inzb;
}File_info;

#define INFO_SIZE  (sizeof(File_info));

typedef struct LOG_STRUCT{
        char id[256];
        time_t  begin_t;
        time_t  end_t;
	time_t  now_t;
	char  fullname[MAX_NAMELEN];	// be errstring when err occurs
        char  filename[MAX_SHORTNAMELEN];
	char  store_dst[MAX_NAMELEN];
        uint64_t  filesize;
        uint8_t  download_type;	// BT, HTTP, FTP, NZB
        uint8_t  status;	// S_PROCESSING, S_PAUSED, S_COMPLETED, S_DEAD_OF_DELETED, S_DEAD_OF_ERROR
        int  error;		// S_DEAD_OF_ERROR
        float  progress;
	File_info  ifile;
}Log_struc;

typedef struct TORRENT_FILES
{
    char  filename[512];
    double  progress;
    uint64_t  filecompletedsize;
    uint64_t  filetotalsize;
}Tr_files;


#define LOG_SIZE  (sizeof(Log_struc))

#endif

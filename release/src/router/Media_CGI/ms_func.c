#include "ms.h"
#include "ms_hook.h"
#include "ms_func.h"
#include <sys/statvfs.h>

int Sem_close(Sem_t *sem)
{
    if (sem->sem_magic != SEM_MAGIC)
    {
        errno = EINVAL;
        return(-1);
    }

    sem->sem_magic = 0;             // in case caller tries to use it later

    if (close(sem->sem_fd[0]) == -1 || close(sem->sem_fd[1]) == -1)
    {
        memset(sem, '\0', sizeof(Sem_t));
        return(-1);
    }

    memset(sem, '\0', sizeof(Sem_t));

    return(0);
}

int Close_sem(char* logid)
{
    int  i;

    for(i=0; i<MAX_NSEM; i++)
        if(strcmp(Sem[i].logid, logid)==0)
            break;

    if(i == MAX_NSEM){
        return -1;
    }

    if(Sem_close(&Sem[i]) < 0)
        return -1;

    return 0;
}

void Clear_log(char* id)
{
    char  delname[MAX_NAMELEN];

    memset(delname, '\0', sizeof(delname));
    sprintf(delname, "%s/.sems/sem.%s", Share_dir, id);
    unlink(delname);

    memset(delname, '\0', sizeof(delname));
    sprintf(delname, "%s/.logs/%s", Share_dir, id);
    unlink(delname);

    if(Close_sem(id) < 0)
        fprintf(stderr, "close sem %s fail\n", id);
}

int Sem_wait(Sem_t *sem)
{

    char    c;

    if (sem->sem_magic != SEM_MAGIC)
    {
        errno = EINVAL;
        return(-1);
    }

    if (read(sem->sem_fd[0], &c, 1) == 1)
        return(0);

    return(-1);
}

int Sem_post(Sem_t *sem)
{

    char    c;

    if (sem->sem_magic != SEM_MAGIC)
    {
        errno = EINVAL;
        return(-1);
    }

    if (write(sem->sem_fd[1], &c, 1) == 1)
        return(0);
    return(-1);
}


int Sem_open(Sem_t *sem, const char *pathname, int oflag, ... )
{

    int     i, flags, save_errno;
    char    c;
    mode_t  mode;
    va_list ap;
    unsigned int    value = 0;
    if (oflag & O_CREAT) {
        va_start(ap, oflag);            // init ap to final named argument
        mode = va_arg(ap, mode_t);
        value = va_arg(ap, unsigned int);
        va_end(ap);

        if (mkfifo(pathname, mode) < 0) {
            if (errno == EEXIST && (oflag & O_EXCL) == 0)
                oflag &= ~O_CREAT;      // already exists, OK
            else
                return -1;
        }
    }

    if(sem == NULL)
    {

        return -1;
    }

    memset(sem, '\0', sizeof(Sem_t));
    sem->sem_fd[0] = sem->sem_fd[1] = -1;

    if ( (sem->sem_fd[0] = open(pathname, O_RDONLY | O_NONBLOCK)) < 0)
        goto error;
    if ( (sem->sem_fd[1] = open(pathname, O_WRONLY | O_NONBLOCK)) < 0)
        goto error;

    if ( (flags = fcntl(sem->sem_fd[0], F_GETFL, 0)) < 0)
        goto error;
    flags &= ~O_NONBLOCK;
    if (fcntl(sem->sem_fd[0], F_SETFL, flags) < 0)
        goto error;

    if (oflag & O_CREAT) {          // initialize semaphore
        for (i = 0; i < value; i++)
            if (write(sem->sem_fd[1], &c, 1) != 1)
                goto error;
    }

    sem->sem_magic = SEM_MAGIC;

    return 0;

    error:

    save_errno = errno;
    if (oflag & O_CREAT)
        unlink(pathname);               // if we created FIFO
    close(sem->sem_fd[0]);          // ignore error
    close(sem->sem_fd[1]);          // ignore error
    memset(sem, '\0',sizeof(Sem_t));
    errno = save_errno;

    return -1;
}

char *right(char *dst,char *src, int n)
{
    char *p=src;
    char *q =dst;
    int len =strlen(src);
    if(n>len) n=len;
    p+=(len-n);
    while(*(q++) = *(p++));
    return dst;
}

int chk_on_process(char *download_name, char *infohash)
{
    DIR *log_dir;
    struct dirent *log_ptr;
    Log_struc slog;
    char pid[MAX_HASHLEN]={0};

    if((log_dir = opendir(Log_dir)) == NULL)
    {
        return 1;
    }

    while((log_ptr = readdir(log_dir)) != NULL){
        sprintf(pid, "%s" , getlogid(log_ptr->d_name));
        if((strcmp(pid,"none"))&&(strncmp(pid,"-",1)!=0)){
            memset(&slog, '\0', sizeof(slog));
            if(read_log(&slog, pid) > 0){
                if(isOnProc(slog.status)){
                    if(slog.download_type == BT){
                        //if((!download_name) || (!slog.filename) || (!infohash) || (!slog.store_dst))
			if((!download_name) || (!slog.filename) || (!slog.store_dst))
                        {
                            continue;
                        }
			
			if(strstr(download_name,slog.id)!=NULL){
                            closedir(log_dir);
                            return 1;
			}
                        if(strncmp(download_name, slog.filename, strlen(download_name)) == 0){
                            closedir(log_dir);
                            return 1;
                        }
			if(infohash){
                            if(strncmp(infohash, slog.id, strlen(infohash)) == 0){
                                remove_torrent(download_name, BT_EXIST);	//Allen 20090826
                                closedir(log_dir);
                                return 1;
                            }
			}
                    }else if((slog.download_type == HTTP) || (slog.download_type == FTP)){
                        if((!download_name) || (!slog.filename))
                            continue;
                        if(strncmp(download_name, slog.filename, strlen(download_name)) == 0){
                            closedir(log_dir);
                            return 1;
                        }
                    }else if(slog.download_type == NZB){
                        if((!download_name) || (!slog.fullname))
                            continue;
                        if(strncmp(download_name, slog.fullname, strlen(download_name)-4) == 0){
                            closedir(log_dir);
                            return 1;
                        }

                    }else if(slog.download_type == ED2K){
			char infohash_tmp[1024];
			memset(infohash_tmp,0,sizeof(infohash_tmp));
			right(infohash_tmp,download_name,34);
			//fprintf(stderr,"\ninfohash_tmp=%s\n",infohash_tmp);
                        if(strncmp(infohash_tmp, slog.id, strlen(infohash_tmp)-2) == 0){
                            closedir(log_dir);
                            return 1;
                        }

                    }
                }
            }
        }
    }
    closedir(log_dir);
    return 0;
}

void init_path()
{
    memset(Base_dir, '\0', sizeof(Base_dir));
    memset(Share_dir, '\0', sizeof(Share_dir));
    memset(In_dir, '\0', sizeof(In_dir));
    memset(Seeds_dir, '\0', sizeof(Seeds_dir));
    memset(Log_dir, '\0', sizeof(Log_dir));
    memset(Sem_dir, '\0', sizeof(Sem_dir));
    memset(jqs_dir, '\0', sizeof(jqs_dir));
    memset(jqs_file, '\0', sizeof(jqs_file));

    //sprintf(Base_dir, "%s", getbasepath());
    //2016.8.17 tina modify{
    //sprintf(Base_dir, "%s", getrouterconfig()); //20120821 magic modify
    char *routerconfig = getrouterconfig();
    if(routerconfig != NULL)
    {
        sprintf(Base_dir, "%s", routerconfig);
        free(routerconfig);
    }
    //{end tina
    sprintf(Share_dir, "%s/Download2", Base_dir);
    sprintf(In_dir, "%s/InComplete", Share_dir);
    sprintf(Seeds_dir, "%s/Seeds", Share_dir);
    sprintf(Log_dir, "%s/.logs", Share_dir);
    sprintf(Sem_dir, "%s/.sems", Share_dir);
    sprintf(jqs_dir, "%s/.sems/sem.jqs", Share_dir);
    sprintf(jqs_file, "%s/dm.jqs", Log_dir);

}

char *getrouterconfig()
{
    //2016.8.17 tina add{
    char *result = NULL;
    result = (char *)malloc(sizeof(char)*64);
    if (result == NULL)
        return NULL;
    memset(result, 0, sizeof(char)*64);
    //}end tina
    if (access("/tmp/asus_router.conf",0) == 0)
    {
        memset(lan_ip_addr, '\0', sizeof(lan_ip_addr));
	memset(miscr_httpport_x_check, '\0', sizeof(miscr_httpport_x_check));
	memset(miscr_http_x_check, '\0', sizeof(miscr_http_x_check));
	memset(productid_check, '\0', sizeof(productid_check));
	memset(apps_dev_path, '\0', sizeof(apps_dev_path));
	memset(wan_ip_check, '\0', sizeof(wan_ip_check));
	memset(ddns_enable_x_check, '\0', sizeof(ddns_enable_x_check));
	memset(ddns_hostname_x_check, '\0', sizeof(ddns_hostname_x_check));
        memset(rfw_enable_x_check, '\0', sizeof(rfw_enable_x_check));
        memset(device_type_check, '\0', sizeof(device_type_check));
        memset(utility_ver_check, '\0', sizeof(utility_ver_check));
        memset(local_domain_check,'\0',sizeof(local_domain_check));
        memset(http_autologout_check, '\0', sizeof(http_autologout_check));

        int fd, len, i=0;
         //2016.8.17 tina modify{
        //char ch, tmp[256], name[256], content[256] ,result[64];
        char ch, tmp[256], name[256], content[256];
        //} end tina
        memset(tmp, 0, sizeof(tmp));
        memset(name, 0, sizeof(name));
        memset(content, 0, sizeof(content));
        //memset(result, 0, sizeof(result)); //2016.8.17 tina modify

        if((fd = open("/tmp/asus_router.conf", O_RDONLY | O_NONBLOCK)) < 0)
        {
            fprintf(stderr,"\nread conf error!\n");
        }
        else
        {

            while((len = read(fd, &ch, 1)) > 0)
            {
                if(ch == '=')
                {
                    strcpy(name, tmp);
                    //printf("name is %s\n",name);
                    memset(tmp, 0, sizeof(tmp));
                    i = 0;
                    continue;
                }
                else if(ch == '\n')
                {
                    strcpy(content, tmp);
                    //printf("content is [%s] \n",content);
                    memset(tmp, 0, sizeof(tmp));
                    i = 0;
                    if(!strcmp(name, "BASE_PATH"))
                    {
                        sprintf(result, "%s", content);

                    }
                    else if(!strcmp(name, "LAN_IP"))
                    {
                        sprintf(lan_ip_addr, "%s", content);
                    }
                    else if(!strcmp(name, "MISCR_HTTPPORT_X"))
                    {
                        sprintf(miscr_httpport_x_check, "%s", content);
                    }
                    else if(!strcmp(name, "MISCR_HTTP_X"))
                    {
                        sprintf(miscr_http_x_check, "%s", content);
                    }
                    else if(!strcmp(name, "PRODUCTID"))
                    {
                        sprintf(productid_check, "%s", content);
                    }
                    else if(!strcmp(name, "APPS_DEV"))
                    {
                        sprintf(apps_dev_path, "%s", content);
                    }
                    else if(!strcmp(name, "WAN_IP"))
                    {
                        sprintf(wan_ip_check, "%s", content);
                    }
                    else if(!strcmp(name, "DDNS_ENABLE_X"))
                    {
                        sprintf(ddns_enable_x_check, "%s", content);
                    }
                    else if(!strcmp(name, "DDNS_HOSTNAME_X"))
                    {
                        sprintf(ddns_hostname_x_check, "%s", content);
                    }
                    else if(!strcmp(name, "RFW_ENABLE_X"))
                    {
                        sprintf(rfw_enable_x_check, "%s", content);
                    }
                    else if(!strcmp(name, "DEVICE_TYPE"))
                    {
                        sprintf(device_type_check, "%s", content);
                    }
                    else if(!strcmp(name, "Utility_Ver"))
                    {
                        sprintf(utility_ver_check, "%s", content);
                    }
                    else if(!strcmp(name, "local_domain"))
                    {
                        sprintf(local_domain_check,"%s",content);
                    }
                    else if(!strcmp(name, "HTTP_AUTOLOGOUT"))
                    {
                        sprintf(http_autologout_check, "%s", content);
                    }

                    continue;
                }


                memcpy(tmp+i, &ch, 1);
                i++;
            }
            close(fd);
            return result;
        }
    }
    else
        //2016.8.17 tina modify{
        {
        //return "none";
        sprintf(result,"%s","none");
        return result;
        }
        //}end tina
}


char *getdmconfig()
{
    //2016.8.17 tina add{
    char *result = NULL;
    result = (char *)malloc(sizeof(char)*64);
    if (result == NULL)
        return NULL;
    memset(result, 0, sizeof(char)*64);
    //}end tina
    if (access("/opt/etc/mes_general.conf",0) == 0)
    {
        memset(nv_enable_time, '\0', sizeof(nv_enable_time));
        memset(nv_data, '\0', sizeof(nv_data));
        memset(nv_time1, '\0', sizeof(nv_time1));
        memset(nv_time2, '\0', sizeof(nv_time2));
        int fd, len, i=0;
        //2016.8.17 tina modify{
        //char ch, tmp[256], name[256], content[256] ,result[64];
        char ch, tmp[256], name[256], content[256];
        //}end tina
        memset(tmp, 0, sizeof(tmp));
        memset(name, 0, sizeof(name));
        memset(content, 0, sizeof(content));
        //memset(result, 0, sizeof(result)); //2016.8.17 tina modify
        sprintf(result, "1");
        if((fd = open("/opt/etc/mes_general.conf", O_RDONLY | O_NONBLOCK)) < 0)
        {
            fprintf(stderr,"\nread conf error!\n");
        }
        else
        {

            while((len = read(fd, &ch, 1)) > 0)
            {
                if(ch == '=')
                {
                    strcpy(name, tmp);
                    memset(tmp, 0, sizeof(tmp));
                    i = 0;
                    continue;
                }
                else if(ch == '\n')
                {
                    strcpy(content, tmp);
                    memset(tmp, 0, sizeof(tmp));
                    i = 0;

                    if(!strcmp(name, "Enable_time"))
                    {
                        sprintf(nv_enable_time, "%s", content);
                    }
                    else  if(!strcmp(name, "Day"))
                    {
                        sprintf(nv_data, "%s", content);
                    }
                    else  if(!strcmp(name, "MAX_ON_HEAVY"))
                    {
                        MAX_ON_HEAVY = atoi(content);
                    }
                    else  if(!strcmp(name, "MAX_QUEUES"))
                    {
                        MAX_QUEUES = atoi(content);
                    }
                    else  if(!strcmp(name, "MAX_ON_ED2K"))
                    {
                        MAX_ON_ED2K = atoi(content);
                    }
                    else  if(!strcmp(name, "dm_radio_time_x"))
                    {
                        sprintf(nv_time1, "%s", content);
                    }
                    else  if(!strcmp(name, "dm_radio_time2_x"))
                    {
                        sprintf(nv_time2, "%s", content);
                    }

                    continue;
                }


                memcpy(tmp+i, &ch, 1);
                i++;
            }
            close(fd);
            return result;
        }
    }
    else
        //2016.8.17 tina modify{
    {
        //return "none";
        sprintf(result,"%s","none");
        return result;
    }
        //}end tina
}

char *getlogid(const char *logname)
{
    char *ptr;

    if((ptr = strstr(logname, "snarf_")) != NULL){
    }
    else if((ptr = strstr(logname, "transm_")) != NULL){
    }
    else if((ptr = strstr(logname, "nzb_")) != NULL){
    }
    else if((ptr = strstr(logname, "ed2k_")) != NULL){
    }
    //else if((ptr = strstr(logname, "error_")) != NULL){
    //}
    else{
        return "none";
    }
    return ptr;
}



int isOnProc(uint8_t status)
{

    return ((status == S_INITIAL)||(status == S_PROCESSING)||(status == S_PAUSED)||(status == S_COMPLETED)||(status == S_DISKFULL)||(status == S_NEEDACCOUNT)||(status == S_CONNECTFAIL)||(status == S_SSLFAIL)||(status == S_ACCOUNTFAIL)||(status == S_RECVFAIL) ||(status == S_HASH) ) ? (1) : (0);
}

int read_log(Log_struc *slog, char *id)
{
    Sem_t  sem;
    int  use_sem = 1;
    int  fd, flags, n;
    char  logname[MAX_NAMELEN];
    char  semname[MAX_NAMELEN];
    int is_bt;
    is_bt = (strlen(id)>5) ? 1 : 0;           // 2009.03 SZ-Alicia modified for HTTP/FTP's pid

    memset(logname, '\0',sizeof(logname));
    memset(semname, '\0',sizeof(semname));
    sprintf(logname, "%s/%s", Log_dir, id);
    sprintf(semname, "%s/sem.%s", Sem_dir, id);
    flags = O_RDONLY;
    if(Sem_open(&sem, semname, flags) == -1)
    {
        use_sem = 0;
    }

    if(use_sem)
        Sem_wait(&sem);

    if((fd = open(logname, O_RDONLY)) > 0)
    {
        if((n = read(fd, slog, sizeof(Log_struc))) == 1)
            slog->status = S_INITIAL;
        close(fd);
    }
    else
    {
        return -1;
    }
    if(use_sem)
        Sem_post(&sem);

    Sem_close(&sem);
    return 1;
}

int remove_torrent(char*  torrent_name, uint8_t ack)
{
    char torrent_pos[MAX_NAMELEN];
    memset(torrent_pos, '\0', sizeof(torrent_pos));
    //sprintf(torrent_pos, "%s/%s", In_dir, torrent_name);
    sprintf(torrent_pos, "%s/%s", Seeds_dir, torrent_name);
    unlink(torrent_pos);
    return 0;
}

void check_alive()
{
    DIR *log_dir;
    struct dirent *log_ptr;
    Log_struc slog;
    char pid[MAX_HASHLEN]={0};

    if((log_dir = opendir(Log_dir)) == NULL){
        return;
    }

    on_heavy_counts = 0;
    on_light_counts = 0;
    on_nntp_counts = 0;			//Allen NNTP
    on_ed2k_counts = 0;
    on_hash_counts =0;//2012.06.27 eric added
    //total_heavy_counts=0;

    while((log_ptr = readdir(log_dir)) != NULL)
    {
        sprintf(pid, "%s" , getlogid(log_ptr->d_name));
        if((strcmp(pid,"none"))&&(strncmp(pid,"-",1)!=0))
        {
            memset(&slog, '\0', sizeof(slog));
            if(read_log(&slog, pid) > 0)
            {
                if(isOnProc(slog.status))
                {
                    if((slog.download_type == BT) && slog.progress < 1 ){
			//if(slog.download_type == BT){
                        //if(slog.status!=S_PAUSED){
                        on_heavy_counts++;
                        //}
                        if(slog.status == S_HASH){
                            on_hash_counts++;//2012.06.27 eric added
                        }
                    }
                    else if(((slog.download_type == HTTP)||(slog.download_type == FTP)) && slog.progress < 1){
                        //if(slog.status!=S_PAUSED){
                        on_light_counts++;
                        //}
                    }
                    else if((slog.download_type == NZB) && slog.progress < 1 ){		//Allen NNTP
                        //if(slog.status!=S_PAUSED){
                        on_nntp_counts++;
                        //}
                    }
                    else if((slog.download_type == ED2K) && slog.progress < 1 ){		//Allen NNTP
                        //if(slog.status!=S_PAUSED){
                        on_ed2k_counts++;
                        //}
                    }
                }
		if(((strstr(pid, "transm_")) != NULL) && slog.status==0)
		{
                    on_heavy_counts++;
		}
		if(((strstr(pid, "snarf_")) != NULL) && slog.status==0)
		{
                    on_light_counts++;
		}
		if(((strstr(pid, "nzb_")) != NULL) && slog.status==0)
		{
                    on_nntp_counts++;
		}
		if(((strstr(pid, "ed2k_")) != NULL) && slog.status==0)
		{
                    on_ed2k_counts++;
		}
            }
            else
            {
                Clear_log(pid);
            }

        }
    }
    closedir(log_dir);
}

void delet(char *s,int d)
{

    int i,j;
    for(i=0;s[i]!='\0';i++)
	if(s[i]==d)
            for(j=i;s[j]!='\0';j++)
                s[j]=s[j+1];
}


void char_to_ascii(char *output, char *input)
{
    int i;
    char tmp[10];
    char *ptr;

    ptr = output;

    for ( i=0; i<strlen(input); i++ )
    {
        if ((input[i]>='0' && input[i] <='9')
            ||(input[i]>='A' && input[i]<='Z')
            ||(input[i] >='a' && input[i]<='z')
            || input[i] == '!' || input[i] == '*'
                    || input[i] == '(' || input[i] == ')'
                    || input[i] == '_' || input[i] == '-'
                    || input[i] == "'" || input[i] == '.')
            {
            *ptr = input[i];
            ptr++;
        }
        else
        {
            memset(tmp, '\0', sizeof(tmp));
            sprintf(tmp, "%%%.02X", input[i]);
            strcpy(ptr, tmp);
            ptr+=3;
        }
    }

    *ptr = '\0';
}

int check_download_time(){
    if (access("/opt/etc/mes_general.conf",0) == 0)
    {

        int fd, len, i=0;
        char ch, tmp[256], name[256], content[256];
        memset(tmp, 0, sizeof(tmp));
        memset(name, 0, sizeof(name));
        memset(content, 0, sizeof(content));

        if((fd = open("/opt/etc/mes_general.conf", O_RDONLY | O_NONBLOCK)) < 0)
        {
            fprintf(stderr,"\nread conf error!\n");
        }
        else
        {

            while((len = read(fd, &ch, 1)) > 0)
            {
                if(ch == '=')
                {
                    strcpy(name, tmp);
                    //printf("name is %s\n",name);
                    memset(tmp, 0, sizeof(tmp));
                    i = 0;
                    continue;
                }
                else if(ch == '\n')
                {
                    strcpy(content, tmp);
                    //printf("content is [%s] \n",content);
                    memset(tmp, 0, sizeof(tmp));
                    i = 0;

                    if(!strcmp(name, "Enable_time"))
                    {
                        tmp_enable = atoi(content);
                    }
                    else  if(!strcmp(name, "Start_hour"))
                    {
                        tmp_shour = atoi(content);
                    }
                    else  if(!strcmp(name, "Start_minute"))
                    {
                        tmp_smin = atoi(content);
                    }
                    else  if(!strcmp(name, "End_hour"))
                    {
                        tmp_ehour = atoi(content);
                    }
                    else  if(!strcmp(name, "End_minute"))
                    {
                        tmp_emin = atoi(content);
                    }
                    else  if(!strcmp(name, "Day"))
                    {
                        tmp_day = atoi(content);
                    }

                    continue;
                }


                memcpy(tmp+i, &ch, 1);
                i++;
            }
            close(fd);

            time_t timter;
            timter = time((time_t*)0);
            struct tm *t_tm;
            t_tm=localtime(&timter);

            if(tmp_enable==0)
            {
                return 1;
            }

            if(tmp_day<7){
                if(tmp_day!=t_tm->tm_wday){
                    return 0;
                }
            }
            if(tmp_day==8){
                if(t_tm->tm_wday==6 && t_tm->tm_wday==0)
                {
                    return 0;
                }
            }
            if(tmp_day==9){
                if(0<t_tm->tm_wday && t_tm->tm_wday<6)
                {
                    return 0;
                }
            }

            if (tmp_shour > t_tm->tm_hour || tmp_ehour < t_tm->tm_hour){
                return 0;
            }
            else if(tmp_shour < t_tm->tm_hour && tmp_ehour > t_tm->tm_hour){
                return 1;
            }
            else if(tmp_shour == t_tm->tm_hour){
                if(tmp_smin > t_tm->tm_min)
                {
                    return 0;
                }
                else if(tmp_ehour == t_tm->tm_hour){
                    if(tmp_emin >= t_tm->tm_min){
                        return 1;
                    }
                    else{
                        return 0;
                    }
                }
                else if(tmp_ehour > t_tm->tm_hour){
                    return 1;

                }
                else{
                    return 0;
                }

            }
            else if(tmp_ehour == t_tm->tm_hour){
                if(tmp_emin >= t_tm->tm_min){
                    return 1;
                }
                else{
                    return 0;
                }

            }
            else{
                return 0;
            }
        }
    }
    else{
        return 0;
    }
}


#define DAYSTART (0)
#define DAYEND (60*60*23 + 60*59 + 59) // 86399
static int in_sched(int now_mins, int now_dow, int sched_begin, int sched_end, int sched_begin2, int sched_end2, int sched_dow)
{
    //cprintf("%s: now_mins=%d sched_begin=%d sched_end=%d sched_begin2=%d sched_end2=%d now_dow=%d sched_dow=%d\n", __FUNCTION__, now_mins, sched_begin, sched_end, sched_begin2, sched_end2, now_dow, sched_dow);
    int restore_dow = now_dow; // orig now day of week

    // wday: 0
    if((now_dow & 0x40) != 0){
        // under Sunday's sched time
        if(((now_dow & sched_dow) != 0) && (now_mins >= sched_begin2) && (now_mins <= sched_end2) && (sched_begin2 < sched_end2))
            return 1;

        // under Sunday's sched time and cross-night
        if(((now_dow & sched_dow) != 0) && (now_mins >= sched_begin2) && (sched_begin2 >= sched_end2))
            return 1;

        // under Saturday's sched time
        now_dow >>= 6; // Saturday
        if(((now_dow & sched_dow) != 0) && (now_mins <= sched_end2) && (sched_begin2 >= sched_end2))
            return 1;

        // reset now_dow, avoid to check now_day = 0000001 (Sat)
        now_dow = restore_dow;
    }

    // wday: 1
    if((now_dow & 0x20) != 0){
        // under Monday's sched time
        if(((now_dow & sched_dow) != 0) && (now_mins >= sched_begin) && (now_mins <= sched_end) && (sched_begin < sched_end))
            return 1;

        // under Monday's sched time and cross-night
        if(((now_dow & sched_dow) != 0) && (now_mins >= sched_begin) && (sched_begin >= sched_end))
            return 1;

        // under Sunday's sched time
        now_dow <<= 1; // Sunday
        if(((now_dow & sched_dow) != 0) && (now_mins <= sched_end2) && (sched_begin2 >= sched_end2))
            return 1;
    }

    // wday: 2-5
    if((now_dow & 0x1e) != 0){
        // under today's sched time
        if(((now_dow & sched_dow) != 0) && (now_mins >= sched_begin) && (now_mins <= sched_end) && (sched_begin < sched_end))
            return 1;

        // under today's sched time and cross-night
        if(((now_dow & sched_dow) != 0) && (now_mins >= sched_begin) && (sched_begin >= sched_end))
            return 1;

        // under yesterday's sched time
        now_dow <<= 1; // yesterday
        if(((now_dow & sched_dow) != 0) && (now_mins <= sched_end) && (sched_begin >= sched_end))
            return 1;
    }

    // wday: 6
    if((now_dow & 0x01) != 0){
        // under Saturday's sched time
        if(((now_dow & sched_dow) != 0) && (now_mins >= sched_begin2) && (now_mins <= sched_end2) && (sched_begin2 < sched_end2))
            return 1;

        // under Saturday's sched time and cross-night
        if(((now_dow & sched_dow) != 0) && (now_mins >= sched_begin2) && (sched_begin2 >= sched_end2))
            return 1;

        // under Friday's sched time
        now_dow <<= 1; // Friday
        if(((now_dow & sched_dow) != 0) && (now_mins <= sched_end) && (sched_begin >= sched_end))
            return 1;
    }

    return 0;
}

int timecheck_item(char *activeDate, char *activeTime, char *activeTime2)
{

    //fprintf(stderr,"\nnv_enable_time=%s\n",nv_enable_time);
    if(!strcmp(nv_enable_time,"0")){
        return 1;
    }

    int current, active, activeTimeStart, activeTimeEnd;
    int activeTimeStart2, activeTimeEnd2;
    int now_dow, sched_dow=0;
    time_t now;
    struct tm *tm;
    int i;

    //setenv("TZ", nvram_safe_get("time_zone_x"), 1);

    time(&now);
    tm = localtime(&now);
    current = tm->tm_hour * 60 + tm->tm_min;
    active = 0;

    // weekdays time
    activeTimeStart = ((activeTime[0]-'0')*10 + (activeTime[1]-'0'))*60 + (activeTime[2]-'0')*10 + (activeTime[3]-'0');
    activeTimeEnd = ((activeTime[4]-'0')*10 + (activeTime[5]-'0'))*60 + (activeTime[6]-'0')*10 + (activeTime[7]-'0');

    // weekend time
    activeTimeStart2 = ((activeTime2[0]-'0')*10 + (activeTime2[1]-'0'))*60 + (activeTime2[2]-'0')*10 + (activeTime2[3]-'0');
    activeTimeEnd2 = ((activeTime2[4]-'0')*10 + (activeTime2[5]-'0'))*60 + (activeTime2[6]-'0')*10 + (activeTime2[7]-'0');

    // now day of week
    now_dow = 1<< (6-tm->tm_wday);

    // schedule day of week
    sched_dow = 0;
    for(i=0;i<=6;i++){
        sched_dow += (activeDate[i]-'0') << (6-i);
    }

    active = in_sched(current, now_dow, activeTimeStart, activeTimeEnd, activeTimeStart2, activeTimeEnd2, sched_dow);

    //cprintf("[watchdoe] active: %d\n", active);
    //fprintf(stderr,"[CGI] active: %d\n", active);
    return active;
}

void small_sleep(float nsec){
    struct timeval  tval;

    tval.tv_sec = (int)(nsec*1000000) / 1000000;
    tval.tv_usec = (int)(nsec*1000000) % 1000000;

    select(0, NULL, NULL, NULL, &tval);
}

int check_ed2k_length(char *url){
    /*int64_t total_len=0;

	if(total_len >= check_disk_space(Base_dir) - (int64_t)50*1024*1024){	//rtorrent will close when less then 50Mb
				//gdc_err_job("File size is larger then the space left on the partition", pid_rtn, torrent);
				return 1;
			}*/
    return 0;
}

int decode_path(char *url)
{

    //printf("start decode url \n");

    int len ;
    int i,k;
    char temp_url[512];

    memset( temp_url,0,sizeof(temp_url) );

    len = strlen(url);

    for( i = 0 , k= 0 ; i < len ; i++ ,k++)
    {
        if( url[i] == '/')
        {
            temp_url[k] = '\\';
            temp_url[k+1] = '/';
            k++;
        }
        temp_url[k] = url[i];
    }

    //int size = strlen(temp_url);
    temp_url[k+1] = '\0';

    //fprintf(stderr,"temp url is %s \n",temp_url);


    strcpy(url,temp_url);

}

int detect_process(char * process_name)  
{
    FILE *ptr;
    char buff[512];
    char ps[128];
    sprintf(ps,"ps | grep -c %s",process_name);
    strcpy(buff,"ABNORMAL");
    if((ptr=popen(ps, "r")) != NULL)
    {
        while (fgets(buff, 512, ptr) != NULL)
        {
            if(atoi(buff)>2)
            {
                pclose(ptr);
                //fprintf(stderr,"\n#########dm2_transmission-daemon is running\n");
                return 1;
            }
        }
    }
    if(strcmp(buff,"ABNORMAL")==0)  /*ps command error*/
    {
        //fprintf(stderr,"\n#########ps error\n");
        return 0;
    }
    pclose(ptr);
    //fprintf(stderr,"\n#########dm2_transmission-daemon not running\n");
    return 0;
}

int64_t check_disk_space(char *path)
{
    struct statvfs diskdata;
    int64_t free_disk_space;
    if (!statvfs(path, &diskdata))
    {
        free_disk_space = (long long)diskdata.f_bsize * (long long)diskdata.f_bavail;
        //fprintf(stderr,"free disk space is %lld \n",free_disk_space);
        return free_disk_space;
    }
    else
    {
        //fprintf(stderr,"obtain disk space is failed ,path is %s \n",path);
        return -1;
    }
}

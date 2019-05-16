/*
 *  This file is part of nzbget
 *
 *  Copyright (C) 2004 Sven Henkel <sidddy@users.sourceforge.net>
 *  Copyright (C) 2007-2010 Andrei Prygounkov <hugbug@users.sourceforge.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Revision: 1.3 $
 * $Date: 2011/06/24 08:13:34 $
 *
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WIN32
#include "win32.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <cstdio>
//add by gauss {
#include <iostream>
#include <fstream>
//#include <string>
#include <vector>
#include <map>
#include <utility>
//ad by gauss }
#ifdef WIN32
#include <direct.h>
#else
#include <unistd.h>
#include <getopt.h>
#endif

#include "nzbget.h"
#include "Util.h"
#include "Options.h"
#include "Log.h"
#include "ServerPool.h"
#include "NewsServer.h"
#include "MessageBase.h"
#include "Scheduler.h"

//add by gauss{
typedef struct ITEM {
    char name[128];
    char value[128];
} Item;
using namespace std;
//add by gauss}
//2012.12 magic{
Diskinfo disksave;
//2012.12 magic}

extern ServerPool* g_pServerPool;
extern Scheduler* g_pScheduler;
int is_update_basepath = 0 ;
char maindir[50];
char ex_maindir[50];


#ifdef HAVE_GETOPT_LONG
static struct option long_options[] =
    {
	    {"help", no_argument, 0, 'h'},
	    {"configfile", required_argument, 0, 'c'},
	    {"noconfigfile", no_argument, 0, 'n'},
	    {"printconfig", no_argument, 0, 'p'},
	    {"server", no_argument, 0, 's' },
	    {"daemon", no_argument, 0, 'D' },
	    {"version", no_argument, 0, 'v'},
	    {"serverversion", no_argument, 0, 'V'},
	    {"option", required_argument, 0, 'o'},
	    {"append", no_argument, 0, 'A'},
	    {"list", no_argument, 0, 'L'},
	    {"pause", no_argument, 0, 'P'},
	    {"unpause", no_argument, 0, 'U'},
	    {"rate", required_argument, 0, 'R'},
	    {"debug", no_argument, 0, 'B'},
	    {"log", required_argument, 0, 'G'},
	    {"top", no_argument, 0, 'T'},
	    {"edit", required_argument, 0, 'E'},
	    {"connect", no_argument, 0, 'C'},
	    {"quit", no_argument, 0, 'Q'},
	    {"write", required_argument, 0, 'W'},
	    {"category", required_argument, 0, 'K'},
	    {"scan", no_argument, 0, 'S'},
	    {0, 0, 0, 0}
    };
#endif

static char short_options[] = "c:hno:psvAB:DCE:G:K:LPR:STUQVW:";

// Program options
static const char* OPTION_CONFIGFILE		= "ConfigFile";
static const char* OPTION_APPBIN			= "AppBin";
static const char* OPTION_APPDIR			= "AppDir";
static const char* OPTION_VERSION			= "Version";
static const char* OPTION_DESTDIR			= "DestDir";
static const char* OPTION_TEMPDIR			= "TempDir";
static const char* OPTION_QUEUEDIR			= "QueueDir";
static const char* OPTION_NZBDIR			= "NzbDir";
static const char* OPTION_CREATELOG			= "CreateLog";
static const char* OPTION_LOGFILE			= "LogFile";
static const char* OPTION_LOGSDIR                       = "LogsDir"; // Walf add
static const char* OPTION_COMPLETEDIR                       = "CompleteDir"; //Gauss add
static const char* OPTION_BASEDIR                       = "BaseDir"; //Gauss add
static const char* OPTION_SEMSDIR                       = "SemsDir";
static const char* OPTION_APPENDNZBDIR		= "AppendNzbDir";
static const char* OPTION_APPENDCATEGORYDIR	= "AppendCategoryDir";
static const char* OPTION_LOCKFILE			= "LockFile";
static const char* OPTION_DAEMONUSERNAME	= "DaemonUserName";
static const char* OPTION_OUTPUTMODE		= "OutputMode";
static const char* OPTION_DUPECHECK			= "DupeCheck";
static const char* OPTION_DOWNLOADRATE		= "DownloadRate";
static const char* OPTION_RENAMEBROKEN		= "RenameBroken";
static const char* OPTION_SERVERIP			= "ServerIp";
static const char* OPTION_SERVERPORT		= "ServerPort";
static const char* OPTION_SERVERPASSWORD	= "ServerPassword";
static const char* OPTION_CONNECTIONTIMEOUT	= "ConnectionTimeout";
static const char* OPTION_SAVEQUEUE			= "SaveQueue";
static const char* OPTION_RELOADQUEUE		= "ReloadQueue";
static const char* OPTION_RELOADPOSTQUEUE	= "ReloadPostQueue";
static const char* OPTION_CREATEBROKENLOG	= "CreateBrokenLog";
static const char* OPTION_RESETLOG			= "ResetLog";
static const char* OPTION_DECODE			= "Decode";
static const char* OPTION_RETRIES			= "Retries";
static const char* OPTION_RETRYINTERVAL		= "RetryInterval";
static const char* OPTION_TERMINATETIMEOUT	= "TerminateTimeout";
static const char* OPTION_CONTINUEPARTIAL	= "ContinuePartial";
static const char* OPTION_LOGBUFFERSIZE		= "LogBufferSize";
static const char* OPTION_INFOTARGET		= "InfoTarget";
static const char* OPTION_WARNINGTARGET		= "WarningTarget";
static const char* OPTION_ERRORTARGET		= "ErrorTarget";
static const char* OPTION_DEBUGTARGET		= "DebugTarget";
static const char* OPTION_DETAILTARGET		= "DetailTarget";
static const char* OPTION_LOADPARS			= "LoadPars";
static const char* OPTION_PARCHECK			= "ParCheck";
static const char* OPTION_PARREPAIR			= "ParRepair";
static const char* OPTION_POSTPROCESS		= "PostProcess";
static const char* OPTION_NZBPROCESS		= "NZBProcess";
static const char* OPTION_STRICTPARNAME		= "StrictParName";
static const char* OPTION_UMASK				= "UMask";
static const char* OPTION_UPDATEINTERVAL	= "UpdateInterval";
static const char* OPTION_CURSESNZBNAME		= "CursesNzbName";
static const char* OPTION_CURSESTIME		= "CursesTime";
static const char* OPTION_CURSESGROUP		= "CursesGroup";
static const char* OPTION_CRCCHECK			= "CrcCheck";
static const char* OPTION_RETRYONCRCERROR	= "RetryOnCrcError";
static const char* OPTION_THREADLIMIT		= "ThreadLimit";
static const char* OPTION_DIRECTWRITE		= "DirectWrite";
static const char* OPTION_WRITEBUFFERSIZE	= "WriteBufferSize";
static const char* OPTION_NZBDIRINTERVAL	= "NzbDirInterval";
static const char* OPTION_NZBDIRFILEAGE		= "NzbDirFileAge";
static const char* OPTION_PARCLEANUPQUEUE	= "ParCleanupQueue";
static const char* OPTION_DISKSPACE			= "DiskSpace";
static const char* OPTION_PROCESSLOGKIND	= "ProcessLogKind";
static const char* OPTION_ALLOWREPROCESS	= "AllowReProcess";
static const char* OPTION_DUMPCORE			= "DumpCore";
static const char* OPTION_PARPAUSEQUEUE		= "ParPauseQueue";
static const char* OPTION_POSTPAUSEQUEUE	= "PostPauseQueue";
static const char* OPTION_NZBCLEANUPDISK	= "NzbCleanupDisk";
static const char* OPTION_DELETECLEANUPDISK	= "DeleteCleanupDisk";
static const char* OPTION_MERGENZB			= "MergeNzb";
static const char* OPTION_PARTIMELIMIT		= "ParTimeLimit";
static const char* OPTION_KEEPHISTORY		= "KeepHistory";

// obsolete options
static const char* OPTION_POSTLOGKIND		= "PostLogKind";
static const char* OPTION_NZBLOGKIND		= "NZBLogKind";

const char* BoolNames[] = { "yes", "no", "true", "false", "1", "0", "on", "off", "enable", "disable", "enabled", "disabled" };
const int BoolValues[] = { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 };
const int BoolCount = 12;

#ifndef WIN32
const char* PossibleConfigLocations[] =
        {
                "/opt/etc/dm2_nzbget.conf",
		"~/.nzbget",
		"/etc/nzbget.conf",
		"/usr/etc/nzbget.conf",
		"/usr/local/etc/nzbget.conf",
		"/opt/etc/nzbget.conf",
		NULL
	};
#endif

Options::OptEntry::OptEntry()
{
	m_szName = NULL;
	m_szValue = NULL;
}

Options::OptEntry::~OptEntry()
{
	if (m_szName)
	{
		free(m_szName);
	}
	if (m_szValue)
	{
		free(m_szValue);
	}
}

void Options::OptEntry::SetName(const char* szName)
{
	if (m_szName)
	{
		free(m_szName);
        }
        m_szName = strdup(szName);
}

void Options::OptEntry::SetValue(const char* szValue)
{
	if (m_szValue)
	{
		free(m_szValue);
	}
	m_szValue = strdup(szValue);
}


Options::Options(int argc, char* argv[])
{
	// initialize options with default values

	m_bConfigInitialized	= false;
	m_szConfigFilename		= NULL;
	m_szDestDir				= NULL;
	m_szTempDir				= NULL;
	m_szQueueDir			= NULL;
	m_szNzbDir				= NULL;
	m_eInfoTarget			= mtScreen;
	m_eWarningTarget		= mtScreen;
	m_eErrorTarget			= mtScreen;
	m_eDebugTarget			= mtScreen;
	m_eDetailTarget			= mtScreen;
	m_bDecode				= true;
	m_bPauseDownload		= false;
	m_bPauseDownload2		= false;
	m_bPausePostProcess		= false;
	m_bPauseScan			= false;
	m_bCreateBrokenLog		= false;
	m_bResetLog				= false;
	m_fDownloadRate			= 0;
	m_iEditQueueAction		= 0;
	m_pEditQueueIDList		= NULL;
	m_iEditQueueIDCount		= 0;
	m_iEditQueueOffset		= 0;
	m_szEditQueueText			= NULL;
	m_szArgFilename			= NULL;
	m_szLastArg				= NULL;
	m_szCategory			= NULL;
	m_iConnectionTimeout	= 0;
	m_iTerminateTimeout		= 0;
	m_bServerMode			= false;
	m_bDaemonMode			= false;
	m_bRemoteClientMode		= false;
	m_bPrintOptions			= false;
	m_bAddTop				= false;
	m_bAppendNZBDir			= false;
	m_bAppendCategoryDir	= false;
	m_bContinuePartial		= false;
	m_bRenameBroken			= false;
	m_bSaveQueue			= false;
	m_bDupeCheck			= false;
	m_iRetries				= 0;
	m_iRetryInterval		= 0;
	m_szServerPort			= 0;
	m_szServerIP			= NULL;
	m_szServerPassword		= NULL;
	m_szLockFile			= NULL;
	m_szDaemonUserName		= NULL;
	m_eOutputMode			= omLoggable;
	m_bReloadQueue			= false;
	m_bReloadPostQueue		= false;
	m_iLogBufferSize		= 0;
	m_iLogLines				= 0;
	m_iWriteLogKind			= 0;
	m_bCreateLog			= false;
	m_szLogFile				= NULL;
        m_szLogsDir				= NULL;
        m_szCompleteDir                         =NULL; //add by gauss
        m_szBaseDir                         =NULL; //add by gauss
        m_szSemsDir                         =NULL;
	m_eLoadPars				= lpAll;
	m_bParCheck				= false;
	m_bParRepair			= false;
	m_szPostProcess			= NULL;
	m_szNZBProcess			= NULL;
	m_bStrictParName		= false;
	m_bNoConfig				= false;
	m_iUMask				= 0;
	m_iUpdateInterval		= 0;
	m_bCursesNZBName		= false;
	m_bCursesTime			= false;
	m_bCursesGroup			= false;
	m_bCrcCheck				= false;
	m_bRetryOnCrcError		= false;
	m_bDirectWrite			= false;
	m_iThreadLimit			= 0;
	m_iWriteBufferSize		= 0;
	m_iNzbDirInterval		= 0;
	m_iNzbDirFileAge		= 0;
	m_bParCleanupQueue		= false;
	m_iDiskSpace			= 0;
	m_eProcessLogKind		= slNone;
	m_bAllowReProcess		= false;
	m_bTestBacktrace		= false;
	m_bTLS					= false;
	m_bDumpCore				= false;
	m_bParPauseQueue		= false;
	m_bPostPauseQueue		= false;
	m_bNzbCleanupDisk		= false;
	m_bDeleteCleanupDisk	= false;
	m_bMergeNzb				= false;
	m_iParTimeLimit			= 0;
	m_iKeepHistory			= 0;

	// Option "ConfigFile" will be initialized later, but we want
	// to see it at the top of option list, so we add it first
	SetOption(OPTION_CONFIGFILE, "");

	char szFilename[MAX_PATH + 1];
#ifdef WIN32
	GetModuleFileName(NULL, szFilename, sizeof(szFilename));
#else
	Util::ExpandFileName(argv[0], szFilename, sizeof(szFilename));
#endif
	Util::NormalizePathSeparators(szFilename);
	SetOption(OPTION_APPBIN, szFilename);
	char* end = strrchr(szFilename, PATH_SEPARATOR);
	if (end) *end = '\0';
	SetOption(OPTION_APPDIR, szFilename);

	SetOption(OPTION_VERSION, Util::VersionRevision());

	InitDefault();
	InitCommandLine(argc, argv);

	if (argc == 1)
	{
		PrintUsage(argv[0]);
	}
	if (!m_szConfigFilename && !m_bNoConfig)
	{
		if (argc == 1)
		{
			printf("\n");
		}
		printf("No configuration-file found\n");
#ifdef WIN32
		printf("Please put configuration-file \"nzbget.conf\" into the directory with exe-file\n");
#else
		printf("Please use option \"-c\" or put configuration-file in one of the following locations:\n");
		int p = 0;
		while (const char* szFilename = PossibleConfigLocations[p++])
		{
			printf("%s\n", szFilename);
		}
#endif
		exit(-1);
	}
	if (argc == 1)
	{
		exit(-1);
	}

	InitOptions();

	if (!m_bPrintOptions)
	{
		InitFileArg(argc, argv);
	}
	
	InitServers();
	InitScheduler();
	CheckOptions();

	if (m_bPrintOptions)
	{
		Dump();
		exit(-1);
	}
}

Options::~Options()
{
	if (m_szConfigFilename)
	{
		free(m_szConfigFilename);
	}
	if (m_szDestDir)
	{
		free(m_szDestDir);
	}
	if (m_szTempDir)
	{
		free(m_szTempDir);
	}
	if (m_szQueueDir)
	{
		free(m_szQueueDir);
	}
	if (m_szNzbDir)
	{
		free(m_szNzbDir);
	}
	if (m_szArgFilename)
	{
		free(m_szArgFilename);
	}
	if (m_szCategory)
	{
		free(m_szCategory);
	}
	if (m_szEditQueueText)
	{
		free(m_szEditQueueText);
	}
	if (m_szLastArg)
	{
		free(m_szLastArg);
	}
	if (m_szServerIP)
	{
		free(m_szServerIP);
	}
	if (m_szServerPassword)
	{
		free(m_szServerPassword);
	}
	if (m_szLogFile)
	{
		free(m_szLogFile);
	}
        if (m_szLogsDir)
        {
                free(m_szLogsDir);
        }
        if (m_szCompleteDir) //add by gauss
        {
                free(m_szCompleteDir);
        }
        if (m_szBaseDir) //add by gauss
        {
                free(m_szBaseDir);
        }
        if (m_szSemsDir)
        {
                free(m_szSemsDir);
        }
	if (m_szLockFile)
	{
		free(m_szLockFile);
	}
	if (m_szDaemonUserName)
	{
		free(m_szDaemonUserName);
	}
	if (m_szPostProcess)
	{
		free(m_szPostProcess);
	}
	if (m_szNZBProcess)
	{
		free(m_szNZBProcess);
	}
	if (m_pEditQueueIDList)
	{
		free(m_pEditQueueIDList);
	}

	for (OptEntries::iterator it = m_OptEntries.begin(); it != m_OptEntries.end(); it++)
	{
		delete *it;
	}
	m_OptEntries.clear();
}

void Options::Dump()
{
	for (OptEntries::iterator it = m_OptEntries.begin(); it != m_OptEntries.end(); it++)
	{
		OptEntry* pOptEntry = *it;
		printf("%s = \"%s\"\n", pOptEntry->GetName(), pOptEntry->GetValue());
	}
}

void Options::InitDefault()
{
#ifdef WIN32
	SetOption(OPTION_TEMPDIR, "${AppDir}\\temp");
	SetOption(OPTION_DESTDIR, "${AppDir}\\dest");
	SetOption(OPTION_QUEUEDIR, "${AppDir}\\queue");
	SetOption(OPTION_NZBDIR, "${AppDir}\\nzb");
	SetOption(OPTION_LOGFILE, "${AppDir}\\nzbget.log");
	SetOption(OPTION_LOCKFILE, "${AppDir}\\nzbget.lock");
#else
	SetOption(OPTION_TEMPDIR, "~/nzbget/temp");
	SetOption(OPTION_DESTDIR, "~/nzbget/dest");
	SetOption(OPTION_QUEUEDIR, "~/nzbget/queue");
	SetOption(OPTION_NZBDIR, "~/nzbget/nzb");
	SetOption(OPTION_LOGFILE, "~/nzbget/nzbget.log");
        SetOption(OPTION_LOGSDIR, "~/nzbget/./logs");
        SetOption(OPTION_COMPLETEDIR,"~/nzbget/complete"); //add by gauss
        SetOption(OPTION_BASEDIR,"~/nzbget/complete"); //add by gauss
        SetOption(OPTION_SEMSDIR,"~/nzbget/.sems");
	SetOption(OPTION_LOCKFILE, "/tmp/nzbget.lock");
#endif
	SetOption(OPTION_CREATELOG, "yes");
	SetOption(OPTION_APPENDNZBDIR, "yes");
	SetOption(OPTION_APPENDCATEGORYDIR, "yes");
	SetOption(OPTION_OUTPUTMODE, "loggable");
	SetOption(OPTION_DUPECHECK, "yes");
	SetOption(OPTION_DOWNLOADRATE, "0");
	SetOption(OPTION_RENAMEBROKEN, "no");
	SetOption(OPTION_SERVERIP, "localhost");
	SetOption(OPTION_SERVERPASSWORD, "tegbzn");
	SetOption(OPTION_SERVERPORT, "6789");
	SetOption(OPTION_CONNECTIONTIMEOUT, "60");
	SetOption(OPTION_SAVEQUEUE, "yes");
	SetOption(OPTION_RELOADQUEUE, "yes");
	SetOption(OPTION_RELOADPOSTQUEUE, "yes");
	SetOption(OPTION_CREATEBROKENLOG, "no");
	SetOption(OPTION_RESETLOG, "no");
	SetOption(OPTION_DECODE, "yes");
	SetOption(OPTION_RETRIES, "5");
	SetOption(OPTION_RETRYINTERVAL, "10");
	SetOption(OPTION_TERMINATETIMEOUT, "600");
	SetOption(OPTION_CONTINUEPARTIAL, "no");
	SetOption(OPTION_LOGBUFFERSIZE, "1000");
	SetOption(OPTION_INFOTARGET, "both");
	SetOption(OPTION_WARNINGTARGET, "both");
	SetOption(OPTION_ERRORTARGET, "both");
	SetOption(OPTION_DEBUGTARGET, "none");
	SetOption(OPTION_DETAILTARGET, "both");
	SetOption(OPTION_LOADPARS, "all");
	SetOption(OPTION_PARCHECK, "no");
	SetOption(OPTION_PARREPAIR, "no");	
	SetOption(OPTION_POSTPROCESS, "");
	SetOption(OPTION_NZBPROCESS, "");
	SetOption(OPTION_STRICTPARNAME, "yes");
	SetOption(OPTION_DAEMONUSERNAME, "root");
	SetOption(OPTION_UMASK, "1000");
	SetOption(OPTION_UPDATEINTERVAL, "200");
	SetOption(OPTION_CURSESNZBNAME, "yes");
	SetOption(OPTION_CURSESTIME, "no");
	SetOption(OPTION_CURSESGROUP, "no");
	SetOption(OPTION_CRCCHECK, "yes");
	SetOption(OPTION_RETRYONCRCERROR, "no");
	SetOption(OPTION_THREADLIMIT, "100");
	SetOption(OPTION_DIRECTWRITE, "no");
	SetOption(OPTION_WRITEBUFFERSIZE, "0");
	SetOption(OPTION_NZBDIRINTERVAL, "5");
	SetOption(OPTION_NZBDIRFILEAGE, "60");
	SetOption(OPTION_PARCLEANUPQUEUE, "no");
	SetOption(OPTION_DISKSPACE, "0");
	SetOption(OPTION_PROCESSLOGKIND, "none");
	SetOption(OPTION_ALLOWREPROCESS, "no");
	SetOption(OPTION_DUMPCORE, "no");
	SetOption(OPTION_PARPAUSEQUEUE, "no");
	SetOption(OPTION_POSTPAUSEQUEUE, "no");
	SetOption(OPTION_NZBCLEANUPDISK, "no");
	SetOption(OPTION_DELETECLEANUPDISK, "no");
	SetOption(OPTION_MERGENZB, "no");
	SetOption(OPTION_PARTIMELIMIT, "0");	
	SetOption(OPTION_KEEPHISTORY, "0");	
}

void Options::InitOptFile()
{
	if (m_bConfigInitialized)
	{
		return;
	}

	if (!m_szConfigFilename && !m_bNoConfig)
	{
		// search for config file in default locations
#ifdef WIN32
		char szFilename[MAX_PATH + 1];
		GetModuleFileName(NULL, szFilename, MAX_PATH + 1);
		szFilename[MAX_PATH] = '\0';
		Util::NormalizePathSeparators(szFilename);
		char* end = strrchr(szFilename, PATH_SEPARATOR);
		if (end) end[1] = '\0';
		strcat(szFilename, "nzbget.conf");

		if (Util::FileExists(szFilename))
		{
			m_szConfigFilename = strdup(szFilename);
		}
#else
		int p = 0;
		while (const char* szFilename = PossibleConfigLocations[p++])
		{
			// substitute HOME-variable
			char szExpandedFilename[1024];
			if (Util::ExpandHomePath(szFilename, szExpandedFilename, sizeof(szExpandedFilename)))
			{
				szFilename = szExpandedFilename;
			}

			if (Util::FileExists(szFilename))
			{
				m_szConfigFilename = strdup(szFilename);
				break;
			}
		}
#endif
	}

        //get $MAINDIR and $EX_MAINDIR from dm2_general.conf
        //eric added 2012.11.19{
        FILE *fp;
        fp = fopen("/opt/etc/dm2_general.conf","r");
        char *a;
        a = (char *)malloc(sizeof(char)*256);//2016.8.26 tina add
        memset(a, 0, sizeof(char)*256);//2016.8.26 tina add
        char *b = a;//2016.8.26 tina add
        memset(maindir,'\0',sizeof(maindir));
        memset(ex_maindir,'\0',sizeof(ex_maindir));
        if(fp)
        {
            while(!feof(fp))
            {
                //fgets(conf_buf,sizeof(conf_buf),fp);
 fscanf(fp,"%[^\n]%*c",a);
                if(a[0] != '$')
                    continue;
                else
                {
                    if(strncmp(a,"$MAINDIR",8) == 0)
                    {
                        a = a + 9;
                        strncpy(maindir,a,strlen(a));
                    }
                    else
                    {
                        a = a + 12;
                        strncpy(ex_maindir,a,strlen(a));
			break;
                    }
                }
            }
             fclose(fp);//2016.8.29 tina add
        }
        free(b);//2016.8.29 tina add
        //fclose(fp);//2016.8.29 tina modify
        //printf("\nmaindir=%s\n",maindir);
        //printf("\nex_maindir=%s\n",ex_maindir);

        if(!m_szConfigFilename)
        {
            m_szConfigFilename = strdup("/opt/etc/dm2_nzbget.conf");
            char s[] = "\nDestDir=${EX_MAINDIR}/Download2/InComplete\nNzbDir=${EX_MAINDIR}/Download2/config/nzbget/nzb\n"
                       "QueueDir=${EX_MAINDIR}/Download2/config/nzbget/queue\nTempDir=${EX_MAINDIR}/Download2/config/nzbget/tmp\nLockFile=/tmp/nzbget.lock\nLogFile=${EX_MAINDIR}/Download2/.logs/nzbget.log\n"
                       "LogsDir=${EX_MAINDIR}/Download2/.logs\nSemsDir=${EX_MAINDIR}/Download2/.sems\nCompleteDir=${EX_MAINDIR}/${MAINDIR}\nServer1.Level=0\nServer1.Host=news.newshosting.com\nServer1.Port=119\n"
                       "Server1.Username=nzb\nServer1.Password=123456\nServer1.JoinGroup=yes\nServer1.Encryption=no\nServer1.Connections=4\nDaemonUserName=root\nUMask=1000\nAppendCategoryDir=yes\n"
                       "AppendNzbDir=yes\nNzbDirInterval=0\nNzbDirFileAge=60\nMergeNzb=no\nNzbProcess=\nDupeCheck=no\nSaveQueue=yes\nReloadQueue=yes\nReloadPostQueue=yes\nContinuePartial=yes\nRenameBroken=no\n"
                       "Decode=yes\nDirectWrite=no\nCrcCheck=yes\nRetries=4\nRetryInterval=10\nRetryOnCrcError=no\nConnectionTimeout=60\nTerminateTimeout=600\nThreadLimit=100\nDownloadRate=0\nWriteBufferSize=0\n"
                       "DiskSpace=250\nDeleteCleanupDisk=no\nKeepHistory=1\nCreateLog=yes\nResetLog=no\nErrorTarget=both\nWarningTarget=both\nInfoTarget=both\nDetailTarget=both\nDebugTarget=both\nProcessLogKind=detail\n"
                       "LogBufferSize=1000\nCreateBrokenLog=yes\nDumpCore=no\nOutputMode=curses\nCursesNzbName=yes\nCursesGroup=no\nCursesTime=no\nUpdateInterval=200\nServerIp=127.0.0.1\nServerPort=6789\nServerPassword=tegbzn6789\n"
                       "LoadPars=one\nParCheck=no\nParRepair=yes\nStrictParName=yes\nParTimeLimit=0\nParPauseQueue=no\nParCleanupQueue=yes\nNzbCleanupDisk=no\nPostProcess=\nAllowReProcess=no\nPostPauseQueue=no";
            unsigned int s_len= strlen(s);
            char s_s[s_len+100];
            memset(s_s,'\0',sizeof(s_s));
            fp = fopen(m_szConfigFilename,"w+");
            if(fp)
            {
                sprintf(s_s,"$MAINDIR=%s\n$EX_MAINDIR=%s%s",maindir,ex_maindir,s);
                fprintf(fp,"%s",s_s);
            }
            fclose(fp);

            //free(s_s);
            //free(s);
            //free(maindir);
            //free(ex_maindir);
        }
        //eric added 2012.11.19}

	if (m_szConfigFilename)
	{
		SetOption(OPTION_CONFIGFILE, m_szConfigFilename);
		LoadConfig(m_szConfigFilename);
	}

	m_bConfigInitialized = true;
}

void Options::CheckDir(char** dir, const char* szOptionName, bool bAllowEmpty)
{
	char* usedir = NULL;
        const char* tempdir = GetOption(szOptionName);
	if (tempdir && strlen(tempdir) > 0)
	{
		int len = strlen(tempdir);
		usedir = (char*) malloc(len + 2);
		strcpy(usedir, tempdir);
		char ch = usedir[len-1];
		if (ch == ALT_PATH_SEPARATOR)
		{
			usedir[len-1] = PATH_SEPARATOR;
		}
		else if (ch != PATH_SEPARATOR)
		{
			usedir[len] = PATH_SEPARATOR;
			usedir[len + 1] = '\0';
		}
		Util::NormalizePathSeparators(usedir);
	}
	else
	{
		if (!bAllowEmpty)
		{
			abort("FATAL ERROR: Wrong value for option \"%s\"\n", szOptionName);
		}
		return;
	}

	// Ensure the dir is created
	if (!Util::ForceDirectories(usedir))
	{
		abort("FATAL ERROR: Directory \"%s\" (option \"%s\") does not exist and could not be created\n", usedir, szOptionName);
	}
	*dir = usedir;
}

void Options::InitOptions()
{
        CheckDir(&m_szDestDir, OPTION_DESTDIR, false);
	CheckDir(&m_szTempDir, OPTION_TEMPDIR, false);
        CheckDir(&m_szQueueDir, OPTION_QUEUEDIR, false);

	m_szPostProcess = strdup(GetOption(OPTION_POSTPROCESS));
	m_szNZBProcess = strdup(GetOption(OPTION_NZBPROCESS));
	
	m_fDownloadRate			= (float)atof(GetOption(OPTION_DOWNLOADRATE));
	m_iConnectionTimeout	= atoi(GetOption(OPTION_CONNECTIONTIMEOUT));
	m_iTerminateTimeout		= atoi(GetOption(OPTION_TERMINATETIMEOUT));
	m_iRetries				= atoi(GetOption(OPTION_RETRIES));
	m_iRetryInterval		= atoi(GetOption(OPTION_RETRYINTERVAL));
	m_szServerPort			= atoi(GetOption(OPTION_SERVERPORT));
	m_szServerIP			= strdup(GetOption(OPTION_SERVERIP));
	m_szServerPassword		= strdup(GetOption(OPTION_SERVERPASSWORD));
	m_szLockFile			= strdup(GetOption(OPTION_LOCKFILE));
	m_szDaemonUserName		= strdup(GetOption(OPTION_DAEMONUSERNAME));
	m_iLogBufferSize		= atoi(GetOption(OPTION_LOGBUFFERSIZE));
	m_szLogFile				= strdup(GetOption(OPTION_LOGFILE));
        m_szLogsDir				= strdup(GetOption(OPTION_LOGSDIR));
        m_szCompleteDir				= strdup(GetOption(OPTION_COMPLETEDIR)); //add by gauss
        m_szBaseDir				= strdup(GetOption(OPTION_BASEDIR)); //add by gauss
        m_szSemsDir				= strdup(GetOption(OPTION_SEMSDIR));
	m_iUMask				= strtol(GetOption(OPTION_UMASK), NULL, 8);
	m_iUpdateInterval		= atoi(GetOption(OPTION_UPDATEINTERVAL));
	m_iThreadLimit			= atoi(GetOption(OPTION_THREADLIMIT));
	m_iWriteBufferSize		= atoi(GetOption(OPTION_WRITEBUFFERSIZE));
	m_iNzbDirInterval		= atoi(GetOption(OPTION_NZBDIRINTERVAL));
	m_iNzbDirFileAge		= atoi(GetOption(OPTION_NZBDIRFILEAGE));
	m_iDiskSpace			= atoi(GetOption(OPTION_DISKSPACE));
	m_iParTimeLimit			= atoi(GetOption(OPTION_PARTIMELIMIT));
	m_iKeepHistory			= atoi(GetOption(OPTION_KEEPHISTORY));

	CheckDir(&m_szNzbDir, OPTION_NZBDIR, m_iNzbDirInterval == 0);
	CheckDir(&m_szLogsDir, OPTION_LOGSDIR, false);//add by gauss
        CheckDir(&m_szSemsDir, OPTION_SEMSDIR, false);
        //CheckDir(&m_szCompleteDir,OPTION_COMPLETEDIR,false); //20130206 magic del
        //CheckDir(&m_szBaseDir,OPTION_BASEDIR,false);

	m_bCreateBrokenLog		= (bool)ParseOptionValue(OPTION_CREATEBROKENLOG, BoolCount, BoolNames, BoolValues);
	m_bResetLog				= (bool)ParseOptionValue(OPTION_RESETLOG, BoolCount, BoolNames, BoolValues);
	m_bAppendNZBDir			= (bool)ParseOptionValue(OPTION_APPENDNZBDIR, BoolCount, BoolNames, BoolValues);
	m_bAppendCategoryDir	= (bool)ParseOptionValue(OPTION_APPENDCATEGORYDIR, BoolCount, BoolNames, BoolValues);
	m_bContinuePartial		= (bool)ParseOptionValue(OPTION_CONTINUEPARTIAL, BoolCount, BoolNames, BoolValues);
	m_bRenameBroken			= (bool)ParseOptionValue(OPTION_RENAMEBROKEN, BoolCount, BoolNames, BoolValues);
	m_bSaveQueue			= (bool)ParseOptionValue(OPTION_SAVEQUEUE, BoolCount, BoolNames, BoolValues);
	m_bDupeCheck			= (bool)ParseOptionValue(OPTION_DUPECHECK, BoolCount, BoolNames, BoolValues);
	m_bCreateLog			= (bool)ParseOptionValue(OPTION_CREATELOG, BoolCount, BoolNames, BoolValues);
	m_bParCheck				= (bool)ParseOptionValue(OPTION_PARCHECK, BoolCount, BoolNames, BoolValues);
	m_bParRepair			= (bool)ParseOptionValue(OPTION_PARREPAIR, BoolCount, BoolNames, BoolValues);
	m_bStrictParName		= (bool)ParseOptionValue(OPTION_STRICTPARNAME, BoolCount, BoolNames, BoolValues);
	m_bReloadQueue			= (bool)ParseOptionValue(OPTION_RELOADQUEUE, BoolCount, BoolNames, BoolValues);
	m_bReloadPostQueue		= (bool)ParseOptionValue(OPTION_RELOADPOSTQUEUE, BoolCount, BoolNames, BoolValues);
	m_bCursesNZBName		= (bool)ParseOptionValue(OPTION_CURSESNZBNAME, BoolCount, BoolNames, BoolValues);
	m_bCursesTime			= (bool)ParseOptionValue(OPTION_CURSESTIME, BoolCount, BoolNames, BoolValues);
	m_bCursesGroup			= (bool)ParseOptionValue(OPTION_CURSESGROUP, BoolCount, BoolNames, BoolValues);
	m_bCrcCheck				= (bool)ParseOptionValue(OPTION_CRCCHECK, BoolCount, BoolNames, BoolValues);
	m_bRetryOnCrcError		= (bool)ParseOptionValue(OPTION_RETRYONCRCERROR, BoolCount, BoolNames, BoolValues);
	m_bDirectWrite			= (bool)ParseOptionValue(OPTION_DIRECTWRITE, BoolCount, BoolNames, BoolValues);
	m_bParCleanupQueue		= (bool)ParseOptionValue(OPTION_PARCLEANUPQUEUE, BoolCount, BoolNames, BoolValues);
	m_bDecode				= (bool)ParseOptionValue(OPTION_DECODE, BoolCount, BoolNames, BoolValues);
	m_bAllowReProcess		= (bool)ParseOptionValue(OPTION_ALLOWREPROCESS, BoolCount, BoolNames, BoolValues);
	m_bDumpCore				= (bool)ParseOptionValue(OPTION_DUMPCORE, BoolCount, BoolNames, BoolValues);
	m_bParPauseQueue		= (bool)ParseOptionValue(OPTION_PARPAUSEQUEUE, BoolCount, BoolNames, BoolValues);
	m_bPostPauseQueue		= (bool)ParseOptionValue(OPTION_POSTPAUSEQUEUE, BoolCount, BoolNames, BoolValues);
	m_bNzbCleanupDisk		= (bool)ParseOptionValue(OPTION_NZBCLEANUPDISK, BoolCount, BoolNames, BoolValues);
	m_bDeleteCleanupDisk	= (bool)ParseOptionValue(OPTION_DELETECLEANUPDISK, BoolCount, BoolNames, BoolValues);
	m_bMergeNzb				= (bool)ParseOptionValue(OPTION_MERGENZB, BoolCount, BoolNames, BoolValues);

	const char* OutputModeNames[] = { "loggable", "logable", "log", "colored", "color", "ncurses", "curses" };
	const int OutputModeValues[] = { omLoggable, omLoggable, omLoggable, omColored, omColored, omNCurses, omNCurses };
	const int OutputModeCount = 7;
	m_eOutputMode = (EOutputMode)ParseOptionValue(OPTION_OUTPUTMODE, OutputModeCount, OutputModeNames, OutputModeValues);

	const char* LoadParsNames[] = { "none", "one", "all", "1", "0" };
	const int LoadParsValues[] = { lpNone, lpOne, lpAll, lpOne, lpNone };
	const int LoadParsCount = 4;
	m_eLoadPars = (ELoadPars)ParseOptionValue(OPTION_LOADPARS, LoadParsCount, LoadParsNames, LoadParsValues);

	const char* TargetNames[] = { "screen", "log", "both", "none" };
	const int TargetValues[] = { mtScreen, mtLog, mtBoth, mtNone };
	const int TargetCount = 4;
	m_eInfoTarget = (EMessageTarget)ParseOptionValue(OPTION_INFOTARGET, TargetCount, TargetNames, TargetValues);
	m_eWarningTarget = (EMessageTarget)ParseOptionValue(OPTION_WARNINGTARGET, TargetCount, TargetNames, TargetValues);
	m_eErrorTarget = (EMessageTarget)ParseOptionValue(OPTION_ERRORTARGET, TargetCount, TargetNames, TargetValues);
	m_eDebugTarget = (EMessageTarget)ParseOptionValue(OPTION_DEBUGTARGET, TargetCount, TargetNames, TargetValues);
	m_eDetailTarget = (EMessageTarget)ParseOptionValue(OPTION_DETAILTARGET, TargetCount, TargetNames, TargetValues);

	const char* ScriptLogKindNames[] = { "none", "detail", "info", "warning", "error", "debug" };
	const int ScriptLogKindValues[] = { slNone, slDetail, slInfo, slWarning, slError, slDebug };
	const int ScriptLogKindCount = 6;
	m_eProcessLogKind = (EScriptLogKind)ParseOptionValue(OPTION_PROCESSLOGKIND, ScriptLogKindCount, ScriptLogKindNames, ScriptLogKindValues);
}

int Options::ParseOptionValue(const char* OptName, int argc, const char * argn[], const int argv[])
{
	OptEntry* pOptEntry = FindOption(OptName);
	if (!pOptEntry)
	{
		abort("FATAL ERROR: Undefined value for option \"%s\"\n", OptName);
	}

	for (int i = 0; i < argc; i++)
	{
		if (!strcasecmp(pOptEntry->GetValue(), argn[i]))
		{
			// normalizing option value in option list, for example "NO" -> "no"
			if (pOptEntry)
			{
				for (int j = 0; j < argc; j++)
				{
					if (argv[j] == argv[i])
					{
						if (strcmp(argn[j], pOptEntry->GetValue()))
						{
							pOptEntry->SetValue(argn[j]);
						}
						break;
					}
				}
			}

			return argv[i];
		}
	}
	
	abort("FATAL ERROR: Wrong value \"%s\" for option \"%s\"\n", pOptEntry->GetValue(), OptName);
	return -1;
}

void Options::InitCommandLine(int argc, char* argv[])
{
	m_eClientOperation = opClientNoOperation; // default

	// reset getopt
        optind = 1;

	while (true)
	{
		int c;

#ifdef HAVE_GETOPT_LONG
		int option_index  = 0;
		c = getopt_long(argc, argv, short_options, long_options, &option_index);
#else
		c = getopt(argc, argv, short_options);
#endif

		if (c == -1) break;

		switch (c)
		{
			case 'c':
				m_szConfigFilename = strdup(optarg);
				break;
			case 'n':
				m_szConfigFilename = NULL;
				m_bNoConfig = true;
				break;
			case 'h':
				PrintUsage(argv[0]);
				exit(0);
				break;
			case 'v':
				printf("nzbget version: %s\n", Util::VersionRevision());
				exit(1);
				break;
			case 'p':
				m_bPrintOptions = true;
				break;
			case 'o':
				InitOptFile();
				if (!SetOptionString(optarg))
				{
					abort("FATAL ERROR: could not set option: %s\n", optarg);
				}
				break;
			case 's':
				m_bServerMode = true;
				break;
			case 'D':
				m_bServerMode = true;
				m_bDaemonMode = true;
				break;
			case 'A':
				m_eClientOperation = opClientRequestDownload;
				break;
			case 'L':
				optind++;
				optarg = optind > argc ? NULL : argv[optind-1];
				if (!optarg || !strncmp(optarg, "-", 1))
				{
					m_eClientOperation = opClientRequestListFiles;
					optind--;
				}
				else if (!strcmp(optarg, "F"))
				{
					m_eClientOperation = opClientRequestListFiles;
				}
				else if (!strcmp(optarg, "G"))
				{
					m_eClientOperation = opClientRequestListGroups;
				}
				else if (!strcmp(optarg, "O"))
				{
					m_eClientOperation = opClientRequestPostQueue;
				}
				else if (!strcmp(optarg, "S"))
				{
					m_eClientOperation = opClientRequestListStatus;
				}
				else if (!strcmp(optarg, "H"))
				{
					m_eClientOperation = opClientRequestHistory;
				}
				else
				{
					abort("FATAL ERROR: Could not parse value of option 'L'\n");
				}
				break;
			case 'P':
			case 'U':
				optind++;
				optarg = optind > argc ? NULL : argv[optind-1];
				if (!optarg || !strncmp(optarg, "-", 1))
				{
					m_eClientOperation = c == 'P' ? opClientRequestDownloadPause : opClientRequestDownloadUnpause;
					optind--;
				}
				else if (!strcmp(optarg, "D"))
				{
					m_eClientOperation = c == 'P' ? opClientRequestDownloadPause : opClientRequestDownloadUnpause;
				}
				else if (!strcmp(optarg, "D2"))
				{
					m_eClientOperation = c == 'P' ? opClientRequestDownload2Pause : opClientRequestDownload2Unpause;
				}
				else if (!strcmp(optarg, "O"))
				{
					m_eClientOperation = c == 'P' ? opClientRequestPostPause : opClientRequestPostUnpause;
				}
				else if (!strcmp(optarg, "S"))
				{
					m_eClientOperation = c == 'P' ? opClientRequestScanPause : opClientRequestScanUnpause;
				}
				else
				{
					abort("FATAL ERROR: Could not parse value of option '%c'\n", c);
				}
				break;
			case 'R':
				m_eClientOperation = opClientRequestSetRate;
				m_fSetRate = (float)atof(optarg);
				break;
			case 'B':
				if (!strcmp(optarg, "dump"))
				{
					m_eClientOperation = opClientRequestDumpDebug;
				}
				else if (!strcmp(optarg, "trace"))
				{
					m_bTestBacktrace = true;
				}
				else
				{
					abort("FATAL ERROR: Could not parse value of option 'B'\n");
				}
				break;
			case 'G':
				m_eClientOperation = opClientRequestLog;
				m_iLogLines = atoi(optarg);
				if (m_iLogLines == 0)
				{
					abort("FATAL ERROR: Could not parse value of option 'G'\n");
				}
				break;
			case 'T':
				m_bAddTop = true;
				break;
			case 'C':
				m_bRemoteClientMode = true;
				break;
			case 'E':
			{
				m_eClientOperation = opClientRequestEditQueue;
				bool bGroup = !strcasecmp(optarg, "G");
				bool bPost = !strcasecmp(optarg, "O");
				bool bHistory = !strcasecmp(optarg, "H");
				if (bGroup || bPost || bHistory)
				{
					optind++;
					if (optind > argc)
					{
						abort("FATAL ERROR: Could not parse value of option 'E'\n");
					}
					optarg = argv[optind-1];
				}

				if (bPost)
				{
					// edit-commands for post-processor-queue
					if (!strcasecmp(optarg, "T"))
					{
						m_iEditQueueAction = eRemoteEditActionPostMoveTop;
					}
					else if (!strcasecmp(optarg, "B"))
					{
						m_iEditQueueAction = eRemoteEditActionPostMoveBottom;
					}
					else if (!strcasecmp(optarg, "D"))
					{
						m_iEditQueueAction = eRemoteEditActionPostDelete;
					}
					else
					{
						m_iEditQueueOffset = atoi(optarg);
						if (m_iEditQueueOffset == 0)
						{
							abort("FATAL ERROR: Could not parse value of option 'E'\n");
						}
						m_iEditQueueAction = eRemoteEditActionPostMoveOffset;
					}
				}
				else if (bHistory)
				{
					// edit-commands for history
					if (!strcasecmp(optarg, "D"))
					{
						m_iEditQueueAction = eRemoteEditActionHistoryDelete;
					}
					else if (!strcasecmp(optarg, "R"))
					{
						m_iEditQueueAction = eRemoteEditActionHistoryReturn;
					}
					else if (!strcasecmp(optarg, "P"))
					{
						m_iEditQueueAction = eRemoteEditActionHistoryProcess;
					}
					else
					{
						abort("FATAL ERROR: Could not parse value of option 'E'\n");
					}
				}
				else
				{
					// edit-commands for download-queue
					if (!strcasecmp(optarg, "T"))
					{
						m_iEditQueueAction = bGroup ? eRemoteEditActionGroupMoveTop : eRemoteEditActionFileMoveTop;
					}
					else if (!strcasecmp(optarg, "B"))
					{
						m_iEditQueueAction = bGroup ? eRemoteEditActionGroupMoveBottom : eRemoteEditActionFileMoveBottom;
					}
					else if (!strcasecmp(optarg, "P"))
					{
						m_iEditQueueAction = bGroup ? eRemoteEditActionGroupPause : eRemoteEditActionFilePause;
					}
					else if (!strcasecmp(optarg, "A"))
					{
						m_iEditQueueAction = bGroup ? eRemoteEditActionGroupPauseAllPars : eRemoteEditActionFilePauseAllPars;
					}
					else if (!strcasecmp(optarg, "R"))
					{
						m_iEditQueueAction = bGroup ? eRemoteEditActionGroupPauseExtraPars : eRemoteEditActionFilePauseExtraPars;
					}
					else if (!strcasecmp(optarg, "U"))
					{
						m_iEditQueueAction = bGroup ? eRemoteEditActionGroupResume : eRemoteEditActionFileResume;
					}
					else if (!strcasecmp(optarg, "D"))
					{
						m_iEditQueueAction = bGroup ? eRemoteEditActionGroupDelete : eRemoteEditActionFileDelete;
					}
					else if (!strcasecmp(optarg, "K"))
					{
						if (!bGroup)
						{
							abort("FATAL ERROR: Category can be set only for groups\n");
						}
						m_iEditQueueAction = eRemoteEditActionGroupSetCategory;

						optind++;
						if (optind > argc)
						{
							abort("FATAL ERROR: Could not parse value of option 'E'\n");
						}
						m_szEditQueueText = strdup(argv[optind-1]);
					}
					else if (!strcasecmp(optarg, "M"))
					{
						if (!bGroup)
						{
							abort("FATAL ERROR: only groups can be merged\n");
						}
						m_iEditQueueAction = eRemoteEditActionGroupMerge;
					}
					else if (!strcasecmp(optarg, "O"))
					{
						if (!bGroup)
						{
							abort("FATAL ERROR: Post-process parameter can be set only for groups\n");
						}
						m_iEditQueueAction = eRemoteEditActionGroupSetParameter;

						optind++;
						if (optind > argc)
						{
							abort("FATAL ERROR: Could not parse value of option 'E'\n");
						}
						m_szEditQueueText = strdup(argv[optind-1]);

						if (!strchr(m_szEditQueueText, '='))
						{
							abort("FATAL ERROR: Could not parse value of option 'E'\n");
						}
					}
					else
					{
						m_iEditQueueOffset = atoi(optarg);
						if (m_iEditQueueOffset == 0)
						{
							abort("FATAL ERROR: Could not parse value of option 'E'\n");
						}
						m_iEditQueueAction = bGroup ? eRemoteEditActionGroupMoveOffset : eRemoteEditActionFileMoveOffset;
					}
				}
				break;
			}
			case 'Q':
				m_eClientOperation = opClientRequestShutdown;
				break;
			case 'V':
				m_eClientOperation = opClientRequestVersion;
				break;
			case 'W':
				m_eClientOperation = opClientRequestWriteLog;
				if (!strcmp(optarg, "I")) {
					m_iWriteLogKind = (int)Message::mkInfo;
				}
				else if (!strcmp(optarg, "W")) {
					m_iWriteLogKind = (int)Message::mkWarning;
				}
				else if (!strcmp(optarg, "E")) {
					m_iWriteLogKind = (int)Message::mkError;
				}
				else if (!strcmp(optarg, "D")) {
					m_iWriteLogKind = (int)Message::mkDetail;
				}
				else if (!strcmp(optarg, "G")) {
					m_iWriteLogKind = (int)Message::mkDebug;
				} 
				else
				{
					abort("FATAL ERROR: Could not parse value of option 'W'\n");
				}
				break;
			case 'K':
				if (m_szCategory)
				{
					free(m_szCategory);
				}
				m_szCategory = strdup(optarg);
				break;
			case 'S':
				m_eClientOperation = opClientRequestScan;
				break;
			case '?':
				exit(-1);
				break;
		}
	}

	if (m_bServerMode && (m_eClientOperation == opClientRequestDownloadPause ||
		m_eClientOperation == opClientRequestDownload2Pause))
	{
		m_bPauseDownload = m_eClientOperation == opClientRequestDownloadPause;
		m_bPauseDownload2 = m_eClientOperation == opClientRequestDownload2Pause;
		m_eClientOperation = opClientNoOperation;
	}

	InitOptFile();
}

void Options::PrintUsage(char* com)
{
	printf("Usage:\n"
		"  %s [switches]\n\n"
		"Switches:\n"
	    "  -h, --help                Print this help-message\n"
	    "  -v, --version             Print version and exit\n"
		"  -c, --configfile <file>   Filename of configuration-file\n"
		"  -n, --noconfigfile        Prevent loading of configuration-file\n"
		"                            (required options must be passed with --option)\n"
		"  -p, --printconfig         Print configuration and exit\n"
		"  -o, --option <name=value> Set or override option in configuration-file\n"
		"  -s, --server              Start nzbget as a server in console-mode\n"
#ifndef WIN32
		"  -D, --daemon              Start nzbget as a server in daemon-mode\n"
#endif
	    "  -V, --serverversion       Print server's version and exit\n"
		"  -Q, --quit                Shutdown server\n"
		"  -A, --append <nzb-file>   Send file to server's download queue\n"
		"  -C, --connect             Attach client to server\n"
		"  -L, --list    [F|G|O|S|H] Request list of downloads from server\n"
		"                 F          list individual files and server status (default)\n"
		"                 G          list groups (nzb-files) and server status\n"
		"                 O          list post-processor-queue\n"
		"                 H          list history\n"
		"                 S          print only server status\n"
		"  -P, --pause   [D|D2|O|S]  Pause server:\n"
		"                 D          pause download queue (default)\n"
		"                 D2         pause download queue via second pause-register\n"
		"                 O          pause post-processor queue\n"
		"                 S          pause scan of incoming nzb-directory\n"
		"  -U, --unpause [D|D2|O|S]  Unpause server:\n"
		"                 D          unpause download queue (default)\n"
		"                 D2         unpause download queue via second pause-register\n"
		"                 O          unpause post-processor queue\n"
		"                 S          unpause scan of incoming nzb-directory\n"
		"  -R, --rate <speed>        Set download rate on server, in KB/s\n"
		"  -T, --top                 Add file to the top (begining) of queue\n"
		"                            (for using with switch --append)\n"
		"  -K, --category <name>     Assign category to nzb-file\n"
		"                            (for using with switch --append)\n"
		"  -G, --log <lines>         Request last <lines> lines from server's screen-log\n"
		"  -W, --write <D|I|W|E|G> \"Text\" Send text to server's log\n"
		"  -S, --scan                Scan incoming nzb-directory on server\n"
		"  -E, --edit [G|O|H] <action> <IDs> Edit items on server\n"
		"              G             Affect all files in the group (same nzb-file)\n"
		"              O             Edit post-processor-queue\n"
		"              H             Edit history\n"
		"    <action> is one of:\n"
		"       <+offset|-offset>    Move file(s)/group(s)/post-job in queue relative to\n"
		"                            current position, offset is an integer value\n"
		"       T                    Move file(s)/group(s)/post-job to top of queue\n"
		"       B                    Move file(s)/group(s)/post-job to bottom of queue\n"
		"       P                    Pause file(s)/group(s)/\n"
		"                            Postprocess history-item(s) again\n"
		"       U                    Resume (unpause) file(s)/group(s)\n"
		"       A                    Pause all pars (for groups)\n"
		"       R                    Pause extra pars (for groups)/\n"
		"                            Return history-item(s) back to download queue\n"
		"       D                    Delete file(s)/group(s)/post-job(s)/history-item(s)\n"
		"       K <name>             Set category (for groups)\n"
		"       M                    Merge (for groups)\n"
		"       O <name>=<value>     Set post-process parameter (for groups)\n"
		"    <IDs>                   Comma-separated list of file-ids or ranges\n"
		"                            of file-ids, e. g.: 1-5,3,10-22\n",
		Util::BaseFileName(com));
}

void Options::InitFileArg(int argc, char* argv[])
{
	if (optind >= argc)
	{
		// no nzb-file passed
		if (!m_bServerMode && !m_bRemoteClientMode &&
		        (m_eClientOperation == opClientNoOperation ||
		         m_eClientOperation == opClientRequestDownload ||
				 m_eClientOperation == opClientRequestWriteLog))
		{
			if (m_eClientOperation == opClientRequestWriteLog)
			{
				printf("Log-text not specified\n");
			}
			else
			{
				printf("Nzb-file not specified\n");
			}
			exit(-1);
		}
	}
	else if (m_eClientOperation == opClientRequestEditQueue)
	{
		ParseFileIDList(argc, argv, optind);
	}
	else
	{
		m_szLastArg = strdup(argv[optind]);

		// Check if the file-name is a relative path or an absolute path
		// If the path starts with '/' its an absolute, else relative
		const char* szFileName = argv[optind];

#ifdef WIN32
			m_szArgFilename = strdup(szFileName);
#else
		if (szFileName[0] == '/')
		{
			m_szArgFilename = strdup(szFileName);
		}
		else
		{
			// TEST
			char szFileNameWithPath[1024];
			getcwd(szFileNameWithPath, 1024);
			strcat(szFileNameWithPath, "/");
			strcat(szFileNameWithPath, szFileName);
			m_szArgFilename = strdup(szFileNameWithPath);
		}
#endif

		if (m_bServerMode || m_bRemoteClientMode ||
		        !(m_eClientOperation == opClientNoOperation ||
		          m_eClientOperation == opClientRequestDownload ||
				  m_eClientOperation == opClientRequestWriteLog))
		{
			printf("Too many arguments\n");
			exit(-1);
		}
	}
}

void Options::SetOption(const char* optname, const char* value)
{
	OptEntry* pOptEntry = FindOption(optname);
	if (!pOptEntry)
	{

               pOptEntry = new OptEntry();
		pOptEntry->SetName(optname);
		m_OptEntries.push_back(pOptEntry);
	}

	char* curvalue = NULL;

#ifndef WIN32
	if (value && (value[0] == '~') && (value[1] == '/'))
	{
		char szExpandedPath[1024];
		if (!Util::ExpandHomePath(value, szExpandedPath, sizeof(szExpandedPath)))
		{
			abort("FATAL ERROR: Unable to determine home-directory, option \"%s\"\n", optname);
		}
		curvalue = strdup(szExpandedPath);
	}
	else
#endif
	{
		curvalue = strdup(value);
	}

	// expand variables
	while (char* dollar = strstr(curvalue, "${"))
	{
		char* end = strchr(dollar, '}');
		if (end)
		{
			int varlen = (int)(end - dollar - 2);
			char variable[101];
			int maxlen = varlen < 100 ? varlen : 100;
			strncpy(variable, dollar + 2, maxlen);
			variable[maxlen] = '\0';
			const char* varvalue = GetOption(variable);
			if (varvalue)
			{
				int newlen = strlen(varvalue);
				char* newvalue = (char*)malloc(strlen(curvalue) - varlen - 3 + newlen + 1);
				strncpy(newvalue, curvalue, dollar - curvalue);
				strncpy(newvalue + (dollar - curvalue), varvalue, newlen);
				strcpy(newvalue + (dollar - curvalue) + newlen, end + 1);
				free(curvalue);
				curvalue = newvalue;
			}
			else
			{
				abort("FATAL ERROR: Variable \"%s\" not found, option \"%s\"\n", variable, optname);
			}
		}
		else
		{
			abort("FATAL ERROR: Syntax error in variable-substitution, option \"%s=%s\"\n", optname, curvalue);
		}
	}

	pOptEntry->SetValue(curvalue);

	free(curvalue);
}

Options::OptEntry* Options::FindOption(const char* optname)
{
	if (!optname)
	{
		return NULL;
	}

	for (OptEntries::iterator it = m_OptEntries.begin(); it != m_OptEntries.end(); it++)
	{
		OptEntry* pOptEntry = *it;
		if (!strcasecmp(pOptEntry->GetName(), optname))
		{
			// normalize option name in option list; for example "server1.joingroup" -> "Server1.JoinGroup"
			if (strcmp(pOptEntry->GetName(), optname))
			{
				pOptEntry->SetName(optname);
			}

			return pOptEntry;
		}
	}

	return NULL;
}

const char* Options::GetOption(const char* optname)
{
	OptEntry* pOptEntry = FindOption(optname);
	if (pOptEntry)
        {
		return pOptEntry->GetValue();
	}
	return NULL;
}

void Options::InitServers()
{
	int n = 1;
	while (true)
	{
		char optname[128];

		sprintf(optname, "Server%i.Level", n);
		const char* nlevel = GetOption(optname);

		sprintf(optname, "Server%i.Host", n);
		const char* nhost = GetOption(optname);

		sprintf(optname, "Server%i.Port", n);
		const char* nport = GetOption(optname);

		sprintf(optname, "Server%i.Username", n);
                const char* nusername = GetOption(optname);

		sprintf(optname, "Server%i.Password", n);
                const char* npassword = GetOption(optname);

		sprintf(optname, "Server%i.JoinGroup", n);
                const char* njoingroup = GetOption(optname);
		bool bJoinGroup = true;
		if (njoingroup)
		{
			bJoinGroup = (bool)ParseOptionValue(optname, BoolCount, BoolNames, BoolValues);
		}

		sprintf(optname, "Server%i.Encryption", n);
		const char* ntls = GetOption(optname);
		bool bTLS = false;
		if (ntls)
		{
			bTLS = (bool)ParseOptionValue(optname, BoolCount, BoolNames, BoolValues);
#ifdef DISABLE_TLS
			if (bTLS)
			{
				abort("FATAL ERROR: Program was compiled without TLS/SSL-support. Invalid value for option \"%s\"\n", optname);
			}
#endif
			m_bTLS |= bTLS;
		}

		sprintf(optname, "Server%i.Connections", n);
		const char* nconnections = GetOption(optname);

		bool definition = nlevel || nhost || nport || nusername || npassword || nconnections || njoingroup || ntls;
		bool completed = nlevel && nhost && nport && nconnections;

		if (!definition)
		{
			break;
		}

		if (definition && !completed)
		{
			abort("FATAL ERROR: Server definition not complete\n");
                }

                NewsServer* pNewsServer = new NewsServer(nhost, atoi(nport), nusername, npassword,
                        bJoinGroup, bTLS, atoi((char*)nconnections), atoi((char*)nlevel));
		g_pServerPool->AddServer(pNewsServer);

		n++;
	}

	g_pServerPool->SetTimeout(GetConnectionTimeout());
}

void Options::InitScheduler()
{
	int n = 1;
	while (true)
	{
		char optname[128];

		sprintf(optname, "Task%i.Time", n);
		const char* szTime = GetOption(optname);

		sprintf(optname, "Task%i.WeekDays", n);
		const char* szWeekDays = GetOption(optname);

		sprintf(optname, "Task%i.Command", n);
		const char* szCommand = GetOption(optname);

		sprintf(optname, "Task%i.DownloadRate", n);
		const char* szDownloadRate = GetOption(optname);

		sprintf(optname, "Task%i.Process", n);
		const char* szProcess = GetOption(optname);

		bool definition = szTime || szWeekDays || szCommand || szDownloadRate || szProcess;
		bool completed = szTime && szCommand;

		if (!definition)
		{
			break;
		}

		if (definition && !completed)
		{
			abort("FATAL ERROR: Task definition not complete for Task%i\n", n);
		}

		if (szProcess && strlen(szProcess) > 0 && !Util::SplitCommandLine(szProcess, NULL))
		{
			abort("FATAL ERROR: Invalid value for option Task%i.Process\n", n);
		}

		sprintf(optname, "Task%i.Command", n);
		const char* CommandNames[] = { "pausedownload", "pause", "unpausedownload", "resumedownload", "unpause", "resume", "downloadrate", "setdownloadrate", 
			"rate", "speed", "script", "process", "pausescan", "unpausescan", "resumescan" };
		const int CommandValues[] = { Scheduler::scPauseDownload, Scheduler::scPauseDownload, Scheduler::scUnpauseDownload, Scheduler::scUnpauseDownload, Scheduler::scUnpauseDownload, Scheduler::scUnpauseDownload, Scheduler::scDownloadRate, Scheduler::scDownloadRate, 
			Scheduler::scDownloadRate, Scheduler::scDownloadRate, Scheduler::scProcess, Scheduler::scProcess, Scheduler::scPauseScan, Scheduler::scUnpauseScan, Scheduler::scUnpauseScan };
		const int CommandCount = 15;
		Scheduler::ECommand eCommand = (Scheduler::ECommand)ParseOptionValue(optname, CommandCount, CommandNames, CommandValues);

		int iWeekDays = 0;
		if (szWeekDays && !ParseWeekDays(szWeekDays, &iWeekDays))
		{
			abort("FATAL ERROR: Invalid value for option Task%i.WeekDays\n", n);
		}

		int iDownloadRate = 0;
		if (eCommand == Scheduler::scDownloadRate)
		{
			if (!szDownloadRate)
			{
				abort("FATAL ERROR: Task definition not complete for Task%i. Option Task%i.DownloadRate missing.\n", n, n);
			}
			char* szErr;
			iDownloadRate = strtol(szDownloadRate, &szErr, 10);
			if (!szErr || *szErr != '\0' || iDownloadRate < 0)
			{
				abort("FATAL ERROR: Invalid value for option Task%i.DownloadRate\n", n);
			}
		}

		if (eCommand == Scheduler::scProcess && (!szProcess || strlen(szProcess) == 0))
		{
			abort("FATAL ERROR: Task definition not complete for Task%i. Option Task%i.Process missing.\n", n, n);
		}

		int iHours, iMinutes;
		const char** pTime = &szTime;
		while (*pTime)
		{
			if (!ParseTime(pTime, &iHours, &iMinutes))
			{
				abort("FATAL ERROR: Invalid value for option Task%i.Time\n", n);
			}

			if (iHours == -1)
			{
				for (int iEveryHour = 0; iEveryHour < 24; iEveryHour++)
				{
					Scheduler::Task* pTask = new Scheduler::Task(iEveryHour, iMinutes, iWeekDays, eCommand, iDownloadRate, szProcess);
					g_pScheduler->AddTask(pTask);
				}
			}
			else
			{
				Scheduler::Task* pTask = new Scheduler::Task(iHours, iMinutes, iWeekDays, eCommand, iDownloadRate, szProcess);
				g_pScheduler->AddTask(pTask);
			}
		}

		n++;
	}
}

/*
* Parses Time string and moves current string pointer to the next time token.
*/
bool Options::ParseTime(const char** pTime, int* pHours, int* pMinutes)
{
	const char* szTime = *pTime;
	const char* szComma = strchr(szTime, ',');

	int iColons = 0;
	const char* p = szTime;
	while (*p && (!szComma || p != szComma))
	{
		if (!strchr("0123456789: *", *p))
		{
			return false;
		}
		if (*p == ':')
		{
			iColons++;
		}
		p++;
	}

	if (iColons != 1)
	{
		return false;
	}

	const char* szColon = strchr(szTime, ':');
	if (!szColon)
	{
		return false;
	}

	if (szTime[0] == '*')
	{
		*pHours = -1;
	}
	else
	{
		*pHours = atoi(szTime);
		if (*pHours < 0 || *pHours > 23)
		{
			return false;
		}
	}

	if (szColon[1] == '*')
	{
		return false;
	}
	*pMinutes = atoi(szColon + 1);
	if (*pMinutes < 0 || *pMinutes > 59)
	{
		return false;
	}

	if (szComma)
	{
		*pTime = szComma + 1;
	}
	else
	{
		*pTime = NULL;
	}

	return true;
}

bool Options::ParseWeekDays(const char* szWeekDays, int* pWeekDaysBits)
{
	*pWeekDaysBits = 0;
	const char* p = szWeekDays;
	int iFirstDay = 0;
	bool bRange = false;
	while (*p)
	{
		if (strchr("1234567", *p))
		{
			int iDay = *p - '0';
			if (bRange)
			{
				if (iDay <= iFirstDay || iFirstDay == 0)
				{
					return false;
				}
				for (int i = iFirstDay; i <= iDay; i++)
				{
					*pWeekDaysBits |= 1 << (i - 1);
				}
				iFirstDay = 0;
			}
			else
			{
				*pWeekDaysBits |= 1 << (iDay - 1);
				iFirstDay = iDay;
			}
			bRange = false;
		}
		else if (*p == ',')
		{
			bRange = false;
		}
		else if (*p == '-')
		{
			bRange = true;
		}
		else if (*p == ' ')
		{
			// skip spaces
		}
		else
		{
			return false;
		}
		p++;
	}
	return true;
}
//add by gauss {

int  fileToVector(const char *fileName, Item pitem[])
{

  FILE *fp = fopen(fileName,"rb");
  if (!fp)
  {
          //abort("FATAL ERROR: Could not open file %s\n", fileName);
          //printf("can't find filenaem \n",fileName);
          return 0;
  }

  //printf("obtain name and value \n");

  int Line = 0 ;
  char buf[1024];
  memset(buf,'\0',sizeof(buf));

      while (fgets(buf, sizeof(buf) - 1, fp))
      {

              if (buf[strlen(buf)-1] == '\n')
              {
                      buf[strlen(buf)-1] = 0; // remove traling '\n'

              }

              memset(pitem[Line].name,0,sizeof(pitem[Line].name));
              memset(pitem[Line].value,0,sizeof(pitem[Line].value));

              char *p = strchr(buf,'=');
              if(p != NULL)
              {
                  strncpy(pitem[Line].name,buf,strlen(buf)-strlen(p));
                  strcpy(pitem[Line].value,++p);

              }
              else
              {
                  strcpy(pitem[Line].name,buf);
                  strcpy(pitem[Line].value,"");
              }
              p = NULL;
              memset(buf,'\0',sizeof(buf));
              Line++;

      }

      fclose(fp);

      return Line;



}


int UpdateConfigEntry(const char *ConfigName,const char *NewConfigName)
{

    Item config_items[880],newconfig_items[100];

    int line = fileToVector(ConfigName,config_items);
    //printf("line is %d \n",line);
    int newline = fileToVector(NewConfigName,newconfig_items);

    for(int i = 0; i < line; i++)
    {
       for(int j = 0 ; j < newline ; j++)
        {
           //if(strlen(config_items[i].value) ==0 || config_items[i].name[0] == '#' ||
              //config_items[i].name[0] == ' ')
           if( config_items[i].name[0] == '#' || config_items[i].name[0] == ' ')
               continue;

           if(!strcmp(config_items[i].name ,"$EX_MAINDIR") && is_update_basepath)
               continue;

           if ( strcmp( config_items[i].name ,newconfig_items[j].name) == 0 )
           {

              if(strcmp( config_items[i].value ,newconfig_items[j].value) != 0 && strlen(newconfig_items[j].value) != 0)
              {
                  strcpy(config_items[i].value ,newconfig_items[j].value);
              }
          }

       }
    }



    FILE *fp = fopen(ConfigName,"w");

    //printf("write value to file \n");

    for(int k = 0; k < line; k++)
    {

        if( strcmp(config_items[k].value,"") !=0 )
            fprintf(fp,"%s=%s\n",config_items[k].name,config_items[k].value);
        else
        {
            //if( strcmp(config_items[k].name,"") != 0 && config_items[k].name[0] != '#')
            //if( strcmp(config_items[k].name,"PostProcess") == 0 ||  strcmp(config_items[k].name,"NzbProcess") == 0)
            if(config_items[k].name[0] != '#' &&  strlen(config_items[k].name) != 0)
                fprintf(fp,"%s=\n",config_items[k].name);
            else
                fprintf(fp,"%s\n",config_items[k].name);
        }
    }


    fclose(fp);




}


int CompareConfigfile(char* configfile)
{

  char general_file[256],config_ex_file[256],filepath[256];
  char *router_config = "/tmp/asus_router.conf";


  memset(general_file,'\0',sizeof(general_file));
  memset(config_ex_file,'\0',sizeof(config_ex_file));
  memset(filepath,'\0',sizeof(filepath));

  char *p = strrchr(configfile,'/');

  strncpy(filepath,configfile,strlen(configfile)-strlen(p));
  sprintf(config_ex_file,"%s/dm2_nzbget_EX.conf",filepath);
  sprintf(general_file,"%s/dm2_general.conf",filepath);

  UpdateConfigEntry(configfile,router_config);
  is_update_basepath = 1;

  UpdateConfigEntry(configfile,config_ex_file);

  //printf("general_file is %s \n",general_file);
  UpdateConfigEntry(configfile,general_file);

 is_update_basepath = 0;

  return 0;
}

//add by gauss }
void Options::LoadConfig(char * configfile)
{

    CompareConfigfile(configfile); //add by gauss

    FILE* infile = fopen(configfile, "rb");

	if (!infile)
	{
		abort("FATAL ERROR: Could not open file %s\n", configfile);
	}

	int Errors = 0;
	int Line = 0;
	char buf[1024];
	while (fgets(buf, sizeof(buf) - 1, infile))
	{
		Line++;

		if (buf[0] != 0 && buf[strlen(buf)-1] == '\n')
		{
			buf[strlen(buf)-1] = 0; // remove traling '\n'
		}
		if (buf[0] != 0 && buf[strlen(buf)-1] == '\r')
		{
			buf[strlen(buf)-1] = 0; // remove traling '\r' (for windows line endings)
		}

		if (buf[0] == 0 || buf[0] == '#' || strspn(buf, " ") == strlen(buf))
		{
			continue;
		}

		if (!SetOptionString(buf))
		{
			printf("Error in config-file: line %i: %s\n", Line, buf);
			Errors++;
		}
	}

	fclose(infile);

	if (Errors)
	{
                //abort("FATAL ERROR: %i Error(s) in config-file detected\n", Errors);
                //printf("FATAL ERROR: %i Error(s) in config-file detected\n", Errors);
                char buf_s[] = "\nDestDir=${EX_MAINDIR}/Download2/InComplete\nNzbDir=${EX_MAINDIR}/Download2/config/nzbget/nzb\n"
                           "QueueDir=${EX_MAINDIR}/Download2/config/nzbget/queue\nTempDir=${EX_MAINDIR}/Download2/config/nzbget/tmp\nLockFile=/tmp/nzbget.lock\nLogFile=${EX_MAINDIR}/Download2/.logs/nzbget.log\n"
                           "LogsDir=${EX_MAINDIR}/Download2/.logs\nSemsDir=${EX_MAINDIR}/Download2/.sems\nCompleteDir=${EX_MAINDIR}/${MAINDIR}\nServer1.Level=0\nServer1.Host=news.newshosting.com\nServer1.Port=119\n"
                           "Server1.Username=nzb\nServer1.Password=123456\nServer1.JoinGroup=yes\nServer1.Encryption=no\nServer1.Connections=4\nDaemonUserName=root@UMask=1000\nAppendCategoryDir=yes\n"
                           "AppendNzbDir=yes\nNzbDirInterval=0\nNzbDirFileAge=60\nMergeNzb=no\nNzbProcess=\nDupeCheck=no\nSaveQueue=yes\nReloadQueue=yes\nReloadPostQueue=yes\nContinuePartial=yes\nRenameBroken=no\n"
                           "Decode=yes\nDirectWrite=no\nCrcCheck=yes\nRetries=4\nRetryInterval=10\nRetryOnCrcError=no\nConnectionTimeout=60\nTerminateTimeout=600\nThreadLimit=100\nDownloadRate=0\nWriteBufferSize=0\n"
                           "DiskSpace=250\nDeleteCleanupDisk=no\nKeepHistory=1\nCreateLog=yes\nResetLog=no\nErrorTarget=both\nWarningTarget=both\nInfoTarget=both\nDetailTarget=both\nDebugTarget=both\nProcessLogKind=detail\n"
                           "LogBufferSize=1000\nCreateBrokenLog=yes\nDumpCore=no\nOutputMode=curses\nCursesNzbName=yes\nCursesGroup=no\nCursesTime=no\nUpdateInterval=200\nServerIp=127.0.0.1\nServerPort=6789\nServerPassword=tegbzn6789\n"
                           "LoadPars=one\nParCheck=no\nParRepair=yes\nStrictParName=yes\nParTimeLimit=0\nParPauseQueue=no\nParCleanupQueue=yes\nNzbCleanupDisk=no\nPostProcess=\nAllowReProcess=no\nPostPauseQueue=no";
                unsigned int buf_len= strlen(buf_s);
                char buf_ss[buf_len+100];
                memset(buf_ss,'\0',sizeof(buf_ss));
                sprintf(buf_ss,"$MAINDIR=%s\n$EX_MAINDIR=%s%s",maindir,ex_maindir,buf_s);
                //printf("buf is \n%s\n",buf_ss);
                char *p;
                int i = 0;
                p = strtok(buf_ss,"\n");
                while(p)
                {
                    i++;
                    printf("num is %d\n",i);
                    printf("buf is %s\n",p);
                    if(!SetOptionString(p))
                    {
                        //printf("Error in config-file: line %i: %s eirc and angus\n", Line, buf);
                    }
                    p = strtok(NULL,"\n");
                }
                remove(configfile);
	}
}



bool Options::SetOptionString(const char * option)
{
        //debug("option: %s", option);//add by gauss
    const char* eq = strchr(option, '=');
	if (eq)
	{
		char optname[1001];
		char optvalue[1001];
		int maxlen = (int)(eq - option < 1000 ? eq - option : 1000);
		strncpy(optname, option, maxlen);
		optname[maxlen] = '\0';
		strncpy(optvalue, eq + 1, 1000);
		optvalue[1000]  = '\0';
		if (strlen(optname) > 0)
		{
			if (!ValidateOptionName(optname))
			{
                                //abort("FATAL ERROR: Invalid option \"%s\"\n", optname);
                                printf("FATAL ERROR: Invalid option \"%s\"\n",optname);
                                return false;
			}
			char* optname2 = optname;
			if (optname2[0] ==  '$')
			{
				optname2++;
			}
                        //debug("optionname: %s | optvalue: %s", optname2,optvalue);//add by gauss
                        SetOption(optname2, optvalue);
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool Options::ValidateOptionName(const char * optname)
{
	if (!strcasecmp(optname, OPTION_CONFIGFILE) || !strcasecmp(optname, OPTION_APPBIN) ||
		!strcasecmp(optname, OPTION_APPDIR) || !strcasecmp(optname, OPTION_VERSION))
	{
		// read-only options
		return false;
	}

	const char* v = GetOption(optname);
	if (v)
	{
		// it's predefined option, OK
		return true;
	}

	if (optname[0] == '$')
	{
		// it's variable, OK
		return true;
	}

	if (!strncasecmp(optname, "server", 6))
	{
		char* p = (char*)optname + 6;
		while (*p >= '0' && *p <= '9') p++;
		if (p &&
			(!strcasecmp(p, ".level") || !strcasecmp(p, ".host") ||
			!strcasecmp(p, ".port") || !strcasecmp(p, ".username") ||
			!strcasecmp(p, ".password") || !strcasecmp(p, ".joingroup") ||
			!strcasecmp(p, ".encryption") || !strcasecmp(p, ".connections")))
		{
			return true;
		}
	}

	if (!strncasecmp(optname, "task", 4))
	{
		char* p = (char*)optname + 4;
		while (*p >= '0' && *p <= '9') p++;
		if (p && (!strcasecmp(p, ".time") || !strcasecmp(p, ".weekdays") || 
			!strcasecmp(p, ".command") || !strcasecmp(p, ".downloadrate") || !strcasecmp(p, ".process")))
		{
			return true;
		}
	}

	// suppress abort on obsolete options; print a warning message instead
	if (!strcasecmp(optname, OPTION_POSTLOGKIND) || !strcasecmp(optname, OPTION_NZBLOGKIND))
	{
		warn("Option \"%s\" is obsolete. Use \"%s\" instead.", optname, OPTION_PROCESSLOGKIND);
		return true;
	}

	return false;
}

void Options::CheckOptions()
{
#ifdef DISABLE_PARCHECK
	if (m_bParCheck)
	{
		abort("FATAL ERROR: Program was compiled without parcheck-support. Invalid value for option \"%s\"\n", OPTION_PARCHECK);
	}
#endif
	
#ifdef DISABLE_CURSES
	if (m_eOutputMode == omNCurses)
	{
		abort("FATAL ERROR: Program was compiled without curses-support. Can not use \"curses\" frontend (option \"%s\")\n", OPTION_OUTPUTMODE);
	}
#endif

	if (!m_bDecode)
	{
		m_bDirectWrite = false;
	}
}

void Options::ParseFileIDList(int argc, char* argv[], int optind)
{
	std::vector<int> IDs;
	IDs.clear();

	while (optind < argc)
	{
		char* szWritableFileIDList = strdup(argv[optind++]);

		char* optarg = strtok(szWritableFileIDList, ", ");
		while (optarg)
		{
			int iEditQueueIDFrom = 0;
			int iEditQueueIDTo = 0;
			const char* p = strchr(optarg, '-');
			if (p)
			{
				char buf[101];
				int maxlen = (int)(p - optarg < 100 ? p - optarg : 100);
				strncpy(buf, optarg, maxlen);
				buf[maxlen] = '\0';
				iEditQueueIDFrom = atoi(buf);
				iEditQueueIDTo = atoi(p + 1);
				if (iEditQueueIDFrom <= 0 || iEditQueueIDTo <= 0)
				{
					abort("FATAL ERROR: invalid list of file IDs\n");
				}
			}
			else
			{
				iEditQueueIDFrom = atoi(optarg);
				if (iEditQueueIDFrom <= 0)
				{
					abort("FATAL ERROR: invalid list of file IDs\n");
				}
				iEditQueueIDTo = iEditQueueIDFrom;
			}

			int iEditQueueIDCount = 0;
			if (iEditQueueIDTo != 0)
			{
				if (iEditQueueIDFrom < iEditQueueIDTo)
				{
					iEditQueueIDCount = iEditQueueIDTo - iEditQueueIDFrom + 1;
				}
				else
				{
					iEditQueueIDCount = iEditQueueIDFrom - iEditQueueIDTo + 1;
				}
			}
			else
			{
				iEditQueueIDCount = 1;
			}

			for (int i = 0; i < iEditQueueIDCount; i++)
			{
				if (iEditQueueIDFrom < iEditQueueIDTo || iEditQueueIDTo == 0)
				{
					IDs.push_back(iEditQueueIDFrom + i);
				}
				else
				{
					IDs.push_back(iEditQueueIDFrom - i);
				}
			}

			optarg = strtok(NULL, ", ");
		}

		free(szWritableFileIDList);
	}

	m_iEditQueueIDCount = IDs.size();
	m_pEditQueueIDList = (int*)malloc(sizeof(int) * m_iEditQueueIDCount);
	for (int i = 0; i < m_iEditQueueIDCount; i++)
	{
		m_pEditQueueIDList[i] = IDs[i];
	}
}

Options::OptEntries* Options::LockOptEntries()
{
	m_mutexOptEntries.Lock();
	return &m_OptEntries;
}

void Options::UnlockOptEntries()
{
	m_mutexOptEntries.Unlock();
}

int Diskinfo::init_diskinfo_struct()
{
    int len = 0;
    FILE *fp;
    if(access("/tmp/usbinfo",0)==0)
    {
        fp =fopen("/tmp/usbinfo","r");
        if(fp)
        {
            fseek(fp,0,SEEK_END);
            len = ftell(fp);
            fseek(fp,0,SEEK_SET);
        }
    }
    else
        return -1;

    char buf[len+1];
    memset(buf,'\0',sizeof(buf));
    fread(buf,1,len,fp);
    fclose(fp);

    if(initial_disk_data(&follow_disk_tmp) == NULL){
        return -1;
    }
    if(initial_disk_data(&follow_disk_info_start) == NULL){
        return -1;
    }

    follow_disk_info = follow_disk_info_start;
    //get diskname and mountpath
    char a[1024];
    char *p,*q;
    fp = fopen("/proc/mounts","r");
    if(fp)
    {
       while(!feof(fp))
        {
           memset(a,'\0',sizeof(a));
           fscanf(fp,"%[^\n]%*c",a);
           if((strlen(a) != 0)&&((p=strstr(a,"/dev/sd")) != NULL))
           {
               singledisk++;
               if(singledisk != 1){
                   initial_disk_data(&follow_disk_tmp);
               }
               p = p + 5;
               follow_disk_tmp->diskname=(char *)malloc(5);
               memset(follow_disk_tmp->diskname,'\0',5);
               strncpy(follow_disk_tmp->diskname,p,4);

               p = p + 3;
               follow_disk_tmp->partitionport=atoi(p);
               if((q=strstr(p,"/tmp")) != NULL)
               {
                   if((p=strstr(q," ")) != NULL)
                   {
                       follow_disk_tmp->mountpath=(char *)malloc(strlen(q)-strlen(p)+1);
                       memset(follow_disk_tmp->mountpath,'\0',strlen(q)-strlen(p)+1);
                       strncpy(follow_disk_tmp->mountpath,q,strlen(q)-strlen(p));
                   }

                   p++;//eric added for disktype
                   if((q=strstr(p," ")) != NULL)
                   {
                       follow_disk_tmp->disktype=(char *)malloc(strlen(p)-strlen(q)+1);
                       memset(follow_disk_tmp->disktype,'\0',strlen(p)-strlen(q)+1);
                       strncpy(follow_disk_tmp->disktype,p,strlen(p)-strlen(q));
                   }//eric added for disktype
               }
               char diskname_tmp[4];
               memset(diskname_tmp,'\0',sizeof(diskname_tmp));
               strncpy(diskname_tmp,follow_disk_tmp->diskname,3);
               if((p=strstr(buf,diskname_tmp)) != NULL)
               {
                   p = p - 6;
                   follow_disk_tmp->port=atoi(p);
                   follow_disk_tmp->diskpartition=(char *)malloc(4);
                   memset(follow_disk_tmp->diskpartition,'\0',4);
                   strncpy(follow_disk_tmp->diskpartition,diskname_tmp,3);
                   q=strstr(p,"_serial");
                   q = q + 8;
                   p=strstr(q,"_pid");
                   follow_disk_tmp->serialnum=(char *)malloc(strlen(q)-strlen(p)-4);
                   memset(follow_disk_tmp->serialnum,'\0',strlen(q)-strlen(p)-4);
                   strncpy(follow_disk_tmp->serialnum,q,strlen(q)-strlen(p)-5);
                   p = p + 5;
                   q=strstr(p,"_vid");
                   follow_disk_tmp->product=(char *)malloc(strlen(p)-strlen(q)-4);
                   memset(follow_disk_tmp->product,'\0',5);
                   strncpy(follow_disk_tmp->product,p,strlen(p)-strlen(q)-5);
                   q = q + 5;
                   follow_disk_tmp->vendor=(char *)malloc(5);
                   memset(follow_disk_tmp->vendor,'\0',5);
                   strncpy(follow_disk_tmp->vendor,q,4);
               }

               follow_disk_info->next =  follow_disk_tmp;
               follow_disk_info = follow_disk_tmp;
           }
       }
    }
    fclose(fp);
	return 0;
}

struct disk_info * Diskinfo::initial_disk_data(struct disk_info **disk)
{
    struct disk_info *follow_disk;

    if(disk == NULL)
        return NULL;

    *disk = (struct disk_info *)malloc(sizeof(struct disk_info));
    if(*disk == NULL)
        return NULL;

    follow_disk = *disk;

    follow_disk->diskname = NULL;
    follow_disk->diskpartition = NULL;
    follow_disk->mountpath = NULL;
    follow_disk->serialnum = NULL;
    follow_disk->product = NULL;
    follow_disk->vendor = NULL;
    follow_disk->next = NULL;
    follow_disk->port = (unsigned int)0;
    follow_disk->partitionport = (unsigned int )0;
    follow_disk->disktype = NULL;

    return follow_disk;
}

void  Diskinfo::free_disk_struc(struct disk_info **disk) //2012.12.24 magic
{
     info("magic free_disk_struc start!!!!");
    struct disk_info *follow_disk, *old_disk;

    if(disk == NULL){
        info("magic free_disk_struc disk == NULL!!!!");
        return;
    }

    follow_disk = *disk;
    while(follow_disk != NULL)
    {
        info("magic follow_disk->diskname = %s!!!!",follow_disk->diskname);
        if(follow_disk->diskname != NULL)
            free(follow_disk->diskname);
        if(follow_disk->diskpartition != NULL)
            free(follow_disk->diskpartition);
        if(follow_disk->mountpath != NULL)
            free(follow_disk->mountpath);
        if(follow_disk->serialnum != NULL)
            free(follow_disk->serialnum);
        if(follow_disk->product != NULL)
            free(follow_disk->product);
        if(follow_disk->vendor != NULL)
            free(follow_disk->vendor);
        if(follow_disk->disktype != NULL)
            free(follow_disk->disktype);

        old_disk = follow_disk;
        follow_disk = follow_disk->next;
        free(old_disk);
    }
}

char * Diskinfo::my_nstrchr(const char chr,char *str,int n){

    if(n<1)
    {
        printf("my_nstrchr need n>=1\n");
        return NULL;
    }

    char *p1,*p2;
    int i = 1;
    p1 = str;

    do{
        p2 = strchr(p1,chr);
        p1 = p2;
        p1++;
        i++;
    }while(p2!=NULL && i<=n);

    if(i<n)
    {
        return NULL;
    }

    if(p2 == NULL)
        return "\0";

    return p2;
}

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
 * $Revision: 1.2 $
 * $Date: 2011/06/24 08:13:34 $
 *
 */


#ifndef OPTIONS_H
#define OPTIONS_H

#include <vector>
#include "Thread.h"

//2012.12 magic{
#define GENERALFILE "/opt/etc/dm2_general.conf"
#define LOGFILE "/tmp/dm2_log"
struct disk_info{
    int port;
    int partitionport;
    char *disktype;//eric added for disktype
    char *product;
    char *vendor;
    char *serialnum;
    char *mountpath;
    char *diskname;
    char *diskpartition;
    struct disk_info *next;
};


class Diskinfo
{
public:
    struct disk_info *initial_disk_data(struct disk_info **disk);
    void free_disk_struc(struct disk_info **disk);
    int init_diskinfo_struct();
    char * my_nstrchr(const char chr,char *str,int n);
    char generalbuf[100];
    char serialtmp[50];
    char producttmp[10];
    char vondertmp[10];
    int partitiontmp;
    int singledisk;
    char dldir[100];
    struct disk_info *follow_disk_info;
    struct disk_info *follow_disk_info_start;
    struct disk_info *follow_disk_tmp;
};
//2012.12 magic}

class Options
{
public:
	enum EClientOperation
	{
		opClientNoOperation,
		opClientRequestDownload,
		opClientRequestListFiles,
		opClientRequestListGroups,
		opClientRequestListStatus,
		opClientRequestSetRate,
		opClientRequestDumpDebug,
		opClientRequestEditQueue,
		opClientRequestLog,
		opClientRequestShutdown,
		opClientRequestVersion,
		opClientRequestPostQueue,
		opClientRequestWriteLog,
		opClientRequestScan,
		opClientRequestDownloadPause,
		opClientRequestDownloadUnpause,
		opClientRequestDownload2Pause,
		opClientRequestDownload2Unpause,
		opClientRequestPostPause,
		opClientRequestPostUnpause,
		opClientRequestScanPause,
		opClientRequestScanUnpause,
		opClientRequestHistory
	};
	enum EMessageTarget
	{
		mtNone,
		mtScreen,
		mtLog,
		mtBoth
	};
	enum EOutputMode
	{
		omLoggable,
		omColored,
		omNCurses
	};
	enum ELoadPars
	{
		lpNone,
		lpOne,
		lpAll
	};
	enum EScriptLogKind
	{
		slNone,
		slDetail,
		slInfo,
		slWarning,
		slError,
		slDebug
	};

	class OptEntry
	{
	private:
		char*			m_szName;
		char*			m_szValue;

		void			SetName(const char* szName);
		void			SetValue(const char* szValue);

		friend class Options;

	public:
						OptEntry();
						~OptEntry();
		const char*		GetName() { return m_szName; }
		const char*		GetValue() { return m_szValue; }
	};
	
	typedef std::vector<OptEntry*>  OptEntries;

private:
	OptEntries			m_OptEntries;
	bool				m_bConfigInitialized;
	Mutex				m_mutexOptEntries;

	// Options
	char*				m_szConfigFilename;
	char*				m_szDestDir;
	char*				m_szTempDir;
	char*				m_szQueueDir;
	char*				m_szNzbDir;
	EMessageTarget		m_eInfoTarget;
	EMessageTarget		m_eWarningTarget;
	EMessageTarget		m_eErrorTarget;
	EMessageTarget		m_eDebugTarget;
	EMessageTarget		m_eDetailTarget;
	bool				m_bDecode;
	bool				m_bCreateBrokenLog;
	bool				m_bResetLog;
	int					m_iConnectionTimeout;
	int					m_iTerminateTimeout;
	bool				m_bAppendNZBDir;
	bool				m_bAppendCategoryDir;
	bool				m_bContinuePartial;
	bool				m_bRenameBroken;
	int					m_iRetries;
	int					m_iRetryInterval;
	bool				m_bSaveQueue;
	bool				m_bDupeCheck;
	char*				m_szServerIP;
	char*				m_szServerPassword;
	int					m_szServerPort;
	char*				m_szLockFile;
	char*				m_szDaemonUserName;
	EOutputMode			m_eOutputMode;
	bool				m_bReloadQueue;
	bool				m_bReloadPostQueue;
	int					m_iLogBufferSize;
	bool				m_bCreateLog;
	char*				m_szLogFile;
        char*                           m_szLogsDir; // Walf add
        char*                           m_szCompleteDir;//gauss add
        char*                           m_szBaseDir;//gauss add
        char*                           m_szSemsDir;
	ELoadPars			m_eLoadPars;
	bool				m_bParCheck;
	bool				m_bParRepair;
	char*				m_szPostProcess;
	char*				m_szNZBProcess;
	bool				m_bStrictParName;
	bool				m_bNoConfig;
	int					m_iUMask;
	int					m_iUpdateInterval;
	bool				m_bCursesNZBName;
	bool				m_bCursesTime;
	bool				m_bCursesGroup;
	bool				m_bCrcCheck;
	bool				m_bRetryOnCrcError;
	int					m_iThreadLimit;
	bool				m_bDirectWrite;
	int					m_iWriteBufferSize;
	int					m_iNzbDirInterval;
	int					m_iNzbDirFileAge;
	bool				m_bParCleanupQueue;
	int					m_iDiskSpace;
	EScriptLogKind		m_eProcessLogKind;
	bool				m_bAllowReProcess;
	bool				m_bTLS;
	bool				m_bDumpCore;
	bool				m_bParPauseQueue;
	bool				m_bPostPauseQueue;
	bool				m_bNzbCleanupDisk;
	bool				m_bDeleteCleanupDisk;
	bool				m_bMergeNzb;
	int					m_iParTimeLimit;
	int					m_iKeepHistory;

	// Parsed command-line parameters
	bool				m_bServerMode;
	bool				m_bDaemonMode;
	bool				m_bRemoteClientMode;
	int					m_iEditQueueAction;
	int					m_iEditQueueOffset;
	int*				m_pEditQueueIDList;
	int					m_iEditQueueIDCount;
	char*				m_szEditQueueText;
	char*				m_szArgFilename;
	char*				m_szCategory;
	char*				m_szLastArg;
	bool				m_bPrintOptions;
	bool				m_bAddTop;
	float				m_fSetRate;
	int					m_iLogLines;
	int					m_iWriteLogKind;
	bool				m_bTestBacktrace;

	// Current state
	bool				m_bPauseDownload;
	bool				m_bPauseDownload2;
	bool				m_bPausePostProcess;
	bool				m_bPauseScan;
	float				m_fDownloadRate;
        EClientOperation	m_eClientOperation;

	void				InitDefault();
	void				InitOptFile();
	void				InitCommandLine(int argc, char* argv[]);
	void				InitOptions();
	void				InitFileArg(int argc, char* argv[]);
	void				InitServers();
	void				InitScheduler();
	void				CheckOptions();
	void				PrintUsage(char* com);
	void				Dump();
	int					ParseOptionValue(const char* OptName, int argc, const char* argn[], const int argv[]);
	OptEntry*			FindOption(const char* optname);
	const char*			GetOption(const char* optname);
	void				SetOption(const char* optname, const char* value);
	bool				SetOptionString(const char* option);
	bool				ValidateOptionName(const char* optname);
        void				LoadConfig(char* configfile);
	void				CheckDir(char** dir, const char* szOptionName, bool bAllowEmpty);
	void				ParseFileIDList(int argc, char* argv[], int optind);
	bool				ParseTime(const char** pTime, int* pHours, int* pMinutes);
	bool				ParseWeekDays(const char* szWeekDays, int* pWeekDaysBits);

public:
						Options(int argc, char* argv[]);
						~Options();

	// Options
	OptEntries*			LockOptEntries();
	void				UnlockOptEntries();
	const char*			GetDestDir() { return m_szDestDir; }
        void				SetDestDir(char* szDestDir) { m_szDestDir = szDestDir; }
	const char*			GetTempDir() { return m_szTempDir; }
	const char*			GetQueueDir() { return m_szQueueDir; }
	const char*			GetNzbDir() { return m_szNzbDir; }
	bool				GetCreateBrokenLog() const { return m_bCreateBrokenLog; }
	bool				GetResetLog() const { return m_bResetLog; }
	EMessageTarget		GetInfoTarget() const { return m_eInfoTarget; }
	EMessageTarget		GetWarningTarget() const { return m_eWarningTarget; }
	EMessageTarget		GetErrorTarget() const { return m_eErrorTarget; }
	EMessageTarget		GetDebugTarget() const { return m_eDebugTarget; }
	EMessageTarget		GetDetailTarget() const { return m_eDetailTarget; }
	int					GetConnectionTimeout() { return m_iConnectionTimeout; }
	int					GetTerminateTimeout() { return m_iTerminateTimeout; }
	bool				GetDecode() { return m_bDecode; };
	bool				GetAppendNZBDir() { return m_bAppendNZBDir; }
	bool				GetAppendCategoryDir() { return m_bAppendCategoryDir; }
	bool				GetContinuePartial() { return m_bContinuePartial; }
	bool				GetRenameBroken() { return m_bRenameBroken; }
	int					GetRetries() { return m_iRetries; }
	int					GetRetryInterval() { return m_iRetryInterval; }
	bool				GetSaveQueue() { return m_bSaveQueue; }
	bool				GetDupeCheck() { return m_bDupeCheck; }
	const char*			GetServerIP() { return m_szServerIP; }
	const char*			GetServerPassword() { return m_szServerPassword; }
	int					GetServerPort() { return m_szServerPort; }
	const char*			GetLockFile() { return m_szLockFile; }
	const char*			GetDaemonUserName() { return m_szDaemonUserName; }
	EOutputMode			GetOutputMode() { return m_eOutputMode; }
	bool				GetReloadQueue() { return m_bReloadQueue; }
	bool				GetReloadPostQueue() { return m_bReloadPostQueue; }
	int					GetLogBufferSize() { return m_iLogBufferSize; }
	bool				GetCreateLog() { return m_bCreateLog; }
	const char*			GetLogFile() { return m_szLogFile; }
        const char*			GetLogsDir() { return m_szLogsDir; }
        const char*                     GetCompleteDir() { return m_szCompleteDir;} //add by gauss
        const char*                     GetSemsDir() { return m_szSemsDir;} //add by gauss
        const char*                     GetBaseDir() { return m_szBaseDir;} //add by gauss
	ELoadPars			GetLoadPars() { return m_eLoadPars; }
	bool				GetParCheck() { return m_bParCheck; }
	bool				GetParRepair() { return m_bParRepair; }
	const char*			GetPostProcess() { return m_szPostProcess; }
	const char*			GetNZBProcess() { return m_szNZBProcess; }
	bool				GetStrictParName() { return m_bStrictParName; }
	int					GetUMask() { return m_iUMask; }
	int					GetUpdateInterval() {return m_iUpdateInterval; }
	bool				GetCursesNZBName() { return m_bCursesNZBName; }
	bool				GetCursesTime() { return m_bCursesTime; }
	bool				GetCursesGroup() { return m_bCursesGroup; }
	bool				GetCrcCheck() { return m_bCrcCheck; }
	bool				GetRetryOnCrcError() { return m_bRetryOnCrcError; }
	int					GetThreadLimit() { return m_iThreadLimit; }
	bool				GetDirectWrite() { return m_bDirectWrite; }
	int					GetWriteBufferSize() { return m_iWriteBufferSize; }
	int					GetNzbDirInterval() { return m_iNzbDirInterval; }
	int					GetNzbDirFileAge() { return m_iNzbDirFileAge; }
	bool				GetParCleanupQueue() { return m_bParCleanupQueue; }
	int					GetDiskSpace() { return m_iDiskSpace; }
	EScriptLogKind		GetProcessLogKind() { return m_eProcessLogKind; }
	bool				GetAllowReProcess() { return m_bAllowReProcess; }
	bool				GetTLS() { return m_bTLS; }
	bool				GetDumpCore() { return m_bDumpCore; }
	bool				GetParPauseQueue() { return m_bParPauseQueue; }
	bool				GetPostPauseQueue() { return m_bPostPauseQueue; }
	bool				GetNzbCleanupDisk() { return m_bNzbCleanupDisk; }
	bool				GetDeleteCleanupDisk() { return m_bDeleteCleanupDisk; }
	bool				GetMergeNzb() { return m_bMergeNzb; }
	int					GetParTimeLimit() { return m_iParTimeLimit; }
	int					GetKeepHistory() { return m_iKeepHistory; }

	// Parsed command-line parameters
	bool				GetServerMode() { return m_bServerMode; }
	bool				GetDaemonMode() { return m_bDaemonMode; }
	bool				GetRemoteClientMode() { return m_bRemoteClientMode; }
	EClientOperation	GetClientOperation() { return m_eClientOperation; }
	int					GetEditQueueAction() { return m_iEditQueueAction; }
	int					GetEditQueueOffset() { return m_iEditQueueOffset; }
	int*				GetEditQueueIDList() { return m_pEditQueueIDList; }
	int					GetEditQueueIDCount() { return m_iEditQueueIDCount; }
	const char*			GetEditQueueText() { return m_szEditQueueText; }
	const char*			GetArgFilename() { return m_szArgFilename; }
	const char*			GetCategory() { return m_szCategory; }
	const char*			GetLastArg() { return m_szLastArg; }
	bool				GetAddTop() { return m_bAddTop; }
	float				GetSetRate() { return m_fSetRate; }
	int					GetLogLines() { return m_iLogLines; }
	int					GetWriteLogKind() { return m_iWriteLogKind; }
	bool				GetTestBacktrace() { return m_bTestBacktrace; }

	// Current state
	void				SetPauseDownload(bool bPauseDownload) { m_bPauseDownload = bPauseDownload; }
	bool				GetPauseDownload() const { return m_bPauseDownload; }
	void				SetPauseDownload2(bool bPauseDownload2) { m_bPauseDownload2 = bPauseDownload2; }
	bool				GetPauseDownload2() const { return m_bPauseDownload2; }
	void				SetPausePostProcess(bool bPausePostProcess) { m_bPausePostProcess = bPausePostProcess; }
	bool				GetPausePostProcess() const { return m_bPausePostProcess; }
	void				SetPauseScan(bool bPauseScan) { m_bPauseScan = bPauseScan; }
	bool				GetPauseScan() const { return m_bPauseScan; }
	void				SetDownloadRate(float fRate) { m_fDownloadRate = fRate; }
	float				GetDownloadRate() const { return m_fDownloadRate; }
};

#endif

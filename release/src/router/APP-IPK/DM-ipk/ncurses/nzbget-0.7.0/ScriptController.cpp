/*
 *  This file is part of nzbget
 *
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
 * $Revision: 1.1.1.1 $
 * $Date: 2011/03/21 01:46:15 $
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
#include <ctype.h>
#ifndef WIN32
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#endif
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>

#include "nzbget.h"
#include "ScriptController.h"
#include "Log.h"
#include "Util.h"

extern Options* g_pOptions;
extern char* (*szEnvironmentVariables)[];
extern DownloadQueueHolder* g_pDownloadQueueHolder;

static const int POSTPROCESS_PARCHECK_CURRENT = 91;
static const int POSTPROCESS_PARCHECK_ALL = 92;
static const int POSTPROCESS_SUCCESS = 93;
static const int POSTPROCESS_ERROR = 94;
static const int POSTPROCESS_NONE = 95;

#ifndef WIN32
#define CHILD_WATCHDOG 1
#endif

#ifdef CHILD_WATCHDOG
/**
 * Sometimes the forked child process doesn't start properly and hangs
 * just during the starting. I didn't find any explanation about what
 * could cause that problem except of a general advice, that
 * "a forking in a multithread application is not recommended".
 *
 * Workaround:
 * 1) child process prints a line into stdout directly after the start;
 * 2) parent process waits for a line for 60 seconds. If it didn't receive it
 *    the cild process assumed to hang and will be killed. Another attempt
 *    will be made.
 */
 
class ChildWatchDog : public Thread
{
private:
	pid_t			m_hProcessID;
protected:
	virtual void	Run();
public:
	void			SetProcessID(pid_t hProcessID) { m_hProcessID = hProcessID; }
};

void ChildWatchDog::Run()
{
	static const int WAIT_SECONDS = 60;
	time_t tStart = time(NULL);
	while (!IsStopped() && (time(NULL) - tStart) < WAIT_SECONDS)
	{
		usleep(10 * 1000);
	}

	if (!IsStopped())
	{
		info("Restarting hanging child process");
		kill(m_hProcessID, SIGKILL);
	}
}
#endif


EnvironmentStrings::EnvironmentStrings()
{
}

EnvironmentStrings::~EnvironmentStrings()
{
	for (Strings::iterator it = m_strings.begin(); it != m_strings.end(); it++)
	{
		free(*it);
	}
	m_strings.clear();
}

void EnvironmentStrings::InitFromCurrentProcess()
{
	for (int i = 0; (*szEnvironmentVariables)[i]; i++)
	{
		char* szVar = (*szEnvironmentVariables)[i];
		Append(strdup(szVar));
	}
}

void EnvironmentStrings::Append(char* szString)
{
	m_strings.push_back(szString);
}

#ifdef WIN32
/*
 * Returns environment block in format suitable for using with CreateProcess. 
 * The allocated memory must be freed by caller using "free()".
 */
char* EnvironmentStrings::GetStrings()
{
	int iSize = 1;
	for (Strings::iterator it = m_strings.begin(); it != m_strings.end(); it++)
	{
		char* szVar = *it;
		iSize += strlen(szVar) + 1;
	}

	char* szStrings = (char*)malloc(iSize);
	char* szPtr = szStrings;
	for (Strings::iterator it = m_strings.begin(); it != m_strings.end(); it++)
	{
		char* szVar = *it;
		strcpy(szPtr, szVar);
		szPtr += strlen(szVar) + 1;
	}
	*szPtr = '\0';

	return szStrings;
}

#else

/*
 * Returns environment block in format suitable for using with execve 
 * The allocated memory must be freed by caller using "free()".
 */
char** EnvironmentStrings::GetStrings()
{
	char** pStrings = (char**)malloc((m_strings.size() + 1) * sizeof(char*));
	char** pPtr = pStrings;
	for (Strings::iterator it = m_strings.begin(); it != m_strings.end(); it++)
	{
		char* szVar = *it;
		*pPtr = szVar;
		pPtr++;
	}
	*pPtr = NULL;

	return pStrings;
}
#endif


ScriptController::ScriptController()
{
	m_szScript = NULL;
	m_szWorkingDir = NULL;
	m_szArgs = NULL;
	m_bFreeArgs = false;
	m_szInfoName = NULL;
	m_szDefaultKindPrefix = NULL;
	m_bTerminated = false;
	m_environmentStrings.InitFromCurrentProcess();
}

ScriptController::~ScriptController()
{
	if (m_bFreeArgs)
	{
		for (const char** szArgPtr = m_szArgs; *szArgPtr; szArgPtr++)
		{
			free((char*)*szArgPtr);
		}
		free(m_szArgs);
	}
}

void ScriptController::SetEnvVar(const char* szName, const char* szValue)
{
	int iLen = strlen(szName) + strlen(szValue) + 2;
	char* szVar = (char*)malloc(iLen);
	snprintf(szVar, iLen, "%s=%s", szName, szValue);
	m_environmentStrings.Append(szVar);
}

void ScriptController::PrepareEnvironmentStrings()
{
	Options::OptEntries* pOptEntries = g_pOptions->LockOptEntries();

	for (Options::OptEntries::iterator it = pOptEntries->begin(); it != pOptEntries->end(); it++)
	{
		Options::OptEntry* pOptEntry = *it;
		char szVarname[1024];
		snprintf(szVarname, sizeof(szVarname), "NZBOP_%s", pOptEntry->GetName());

		// convert to upper case; replace "." with "_".
		for (char* szPtr = szVarname; *szPtr; szPtr++)
		{
			if (*szPtr == '.')
			{
				*szPtr = '_';
			}
			*szPtr = toupper(*szPtr);
		}

		szVarname[1024-1] = '\0';
		SetEnvVar(szVarname, pOptEntry->GetValue());
	}

	g_pOptions->UnlockOptEntries();
}

int ScriptController::Execute()
{
	if (!Util::FileExists(m_szScript))
	{
		error("Could not start %s: could not find file %s", m_szInfoName, m_szScript);
		return -1;
	}

	PrepareEnvironmentStrings();

	int iExitCode = 0;
	int pipein;

#ifdef CHILD_WATCHDOG
	bool bChildConfirmed = false;
	while (!bChildConfirmed && !m_bTerminated)
	{
#endif

#ifdef WIN32
	// build command line
	char szCmdLine[2048];
	int iUsedLen = 0;
	for (const char** szArgPtr = m_szArgs; *szArgPtr; szArgPtr++)
	{
		snprintf(szCmdLine + iUsedLen, 2048 - iUsedLen, "\"%s\" ", *szArgPtr);
		iUsedLen += strlen(*szArgPtr) + 3;
	}
	szCmdLine[iUsedLen < 2048 ? iUsedLen - 1 : 2048 - 1] = '\0';
	
	// create pipes to write and read data
	HANDLE hReadPipe, hWritePipe;
	SECURITY_ATTRIBUTES SecurityAttributes;
	memset(&SecurityAttributes, 0, sizeof(SecurityAttributes));
	SecurityAttributes.nLength = sizeof(SecurityAttributes);
	SecurityAttributes.bInheritHandle = TRUE;

	CreatePipe(&hReadPipe, &hWritePipe, &SecurityAttributes, 0);

	STARTUPINFO StartupInfo;
	memset(&StartupInfo, 0, sizeof(StartupInfo));
	StartupInfo.cb = sizeof(StartupInfo);
	StartupInfo.dwFlags = STARTF_USESTDHANDLES;
	StartupInfo.hStdInput = 0;
	StartupInfo.hStdOutput = hWritePipe;
	StartupInfo.hStdError = hWritePipe;

	PROCESS_INFORMATION ProcessInfo;
	memset(&ProcessInfo, 0, sizeof(ProcessInfo));

	char* szEnvironmentStrings = m_environmentStrings.GetStrings();

	BOOL bOK = CreateProcess(NULL, szCmdLine, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, szEnvironmentStrings, m_szWorkingDir, &StartupInfo, &ProcessInfo);
	if (!bOK)
	{
		DWORD dwErrCode = GetLastError();
		char szErrMsg[255];
		szErrMsg[255-1] = '\0';
		if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM || FORMAT_MESSAGE_IGNORE_INSERTS || FORMAT_MESSAGE_ARGUMENT_ARRAY, 
			NULL, dwErrCode, 0, szErrMsg, 255, NULL))
		{
			error("Could not start %s: %s", m_szInfoName, szErrMsg);
		}
		else
		{
			error("Could not start %s: error %i", m_szInfoName, dwErrCode);
		}
		free(szEnvironmentStrings);
		return -1;
	}

	free(szEnvironmentStrings);

	debug("Child Process-ID: %i", (int)ProcessInfo.dwProcessId);

	m_hProcess = ProcessInfo.hProcess;

	// close unused "write" end
	CloseHandle(hWritePipe);

	pipein = _open_osfhandle((intptr_t)hReadPipe, _O_RDONLY);

#else

	int p[2];
	int pipeout;

	// create the pipe
	if (pipe(p))
	{
		error("Could not open pipe: errno %i", errno);
		return -1;
	}

	char** pEnvironmentStrings = m_environmentStrings.GetStrings();

	pipein = p[0];
	pipeout = p[1];

	debug("forking");
	pid_t pid = fork();

	if (pid == -1)
	{
		error("Could not start %s: errno %i", m_szInfoName, errno);
		free(pEnvironmentStrings);
		return -1;
	}
	else if (pid == 0)
	{
		// here goes the second instance

		// create new process group (see Terminate() where it is used)
		setsid();
			
		// close up the "read" end
		close(pipein);
      			
		// make the pipeout to be the same as stdout and stderr
		dup2(pipeout, 1);
		dup2(pipeout, 2);
		
		close(pipeout);

#ifdef CHILD_WATCHDOG
		fwrite("\n", 1, 1, stdout);
		fflush(stdout);
#endif

		execve(m_szScript, (char* const*)m_szArgs, (char* const*)pEnvironmentStrings);
		fprintf(stdout, "[ERROR] Could not start script: %s", strerror(errno));
		fflush(stdout);
		_exit(-1);
	}

	// continue the first instance
	debug("forked");
	debug("Child Process-ID: %i", (int)pid);

	free(pEnvironmentStrings);

	m_hProcess = pid;

	// close unused "write" end
	close(pipeout);
#endif

	// open the read end
	FILE* readpipe = fdopen(pipein, "r");
	if (!readpipe)
	{
		error("Could not open pipe to %s", m_szInfoName);
		return -1;
	}
	
#ifdef CHILD_WATCHDOG
	debug("Creating child watchdog");
	ChildWatchDog* pWatchDog = new ChildWatchDog();
	pWatchDog->SetAutoDestroy(false);
	pWatchDog->SetProcessID(pid);
	pWatchDog->Start();
#endif
	
	char* buf = (char*)malloc(10240);

	debug("Entering pipe-loop");
	while (!feof(readpipe) && !m_bTerminated)
	{
		if (fgets(buf, 10240, readpipe))
		{
#ifdef CHILD_WATCHDOG
			if (!bChildConfirmed)
			{
				bChildConfirmed = true;
				pWatchDog->Stop();
				debug("Child confirmed");
			}
#endif
			ProcessOutput(buf);
		}
	}
	debug("Exited pipe-loop");

#ifdef CHILD_WATCHDOG
	debug("Destroying WatchDog");
	if (!bChildConfirmed)
	{
		pWatchDog->Stop();
	}
	while (pWatchDog->IsRunning())
	{
		usleep(1 * 1000);
	}
	delete pWatchDog;
#endif
	
	free(buf);
	fclose(readpipe);

	if (m_bTerminated)
	{
		warn("Interrupted %s", m_szInfoName);
	}

	iExitCode = 0;

#ifdef WIN32
	WaitForSingleObject(m_hProcess, INFINITE);
	DWORD dExitCode = 0;
	GetExitCodeProcess(m_hProcess, &dExitCode);
	iExitCode = dExitCode;
#else
	int iStatus = 0;
	waitpid(m_hProcess, &iStatus, 0);
	if (WIFEXITED(iStatus))
	{
		iExitCode = WEXITSTATUS(iStatus);
	}
#endif
	
#ifdef CHILD_WATCHDOG
	}	// while (!bChildConfirmed && !m_bTerminated)
#endif
	
	if (!m_bTerminated)
	{
		info("Completed %s", m_szInfoName);
		debug("Exit code %i", iExitCode);
	}

	return iExitCode;
}

void ScriptController::Terminate()
{
	debug("Stopping %s", m_szInfoName);
	m_bTerminated = true;

#ifdef WIN32
	BOOL bOK = TerminateProcess(m_hProcess, -1);
#else
	pid_t hKillProcess = m_hProcess;
	if (getpgid(hKillProcess) == hKillProcess)
	{
		// if the child process has its own group (setsid() was successful), kill the whole group
		hKillProcess = -hKillProcess;
	}
	bool bOK = kill(hKillProcess, SIGKILL) == 0;
#endif

	if (bOK)
	{
		debug("Terminated %s", m_szInfoName);
	}
	else
	{
		error("Could not terminate %s", m_szInfoName);
	}

	debug("Stopped %s", m_szInfoName);
}

void ScriptController::ProcessOutput(char* szText)
{
	debug("Processing output received from script");

	for (char* pend = szText + strlen(szText) - 1; pend >= szText && (*pend == '\n' || *pend == '\r' || *pend == ' '); pend--) *pend = '\0';

	if (strlen(szText) == 0)
	{
		// skip empty lines
		return;
	}

	if (!strncmp(szText, "[INFO] ", 7))
	{
		AddMessage(Message::mkInfo, false, g_pOptions->GetInfoTarget(), szText + 7);
	}
	else if (!strncmp(szText, "[WARNING] ", 10))
	{
		AddMessage(Message::mkWarning, false, g_pOptions->GetWarningTarget(), szText + 10);
	}
	else if (!strncmp(szText, "[ERROR] ", 8))
	{
		AddMessage(Message::mkError, false, g_pOptions->GetErrorTarget(), szText + 8);
	}
	else if (!strncmp(szText, "[DETAIL] ", 9))
	{
		AddMessage(Message::mkDetail, false, g_pOptions->GetDetailTarget(), szText + 9);
	}
	else if (!strncmp(szText, "[DEBUG] ", 8))
	{
		AddMessage(Message::mkDebug, false, g_pOptions->GetDebugTarget(), szText + 8);
	}
	else 
	{	
		switch (m_eDefaultLogKind)
		{
			case Options::slNone:
				break;

			case Options::slDetail:
				AddMessage(Message::mkDetail, true, g_pOptions->GetDetailTarget(), szText);
				break;

			case Options::slInfo:
				AddMessage(Message::mkInfo, true, g_pOptions->GetInfoTarget(), szText);
				break;

			case Options::slWarning:
				AddMessage(Message::mkWarning, true, g_pOptions->GetWarningTarget(), szText);
				break;

			case Options::slError:
				AddMessage(Message::mkError, true, g_pOptions->GetErrorTarget(), szText);
				break;

			case Options::slDebug:
				AddMessage(Message::mkDebug, true, g_pOptions->GetDebugTarget(), szText);
				break;
		}
	}

	debug("Processing output received from script - completed");
}

void ScriptController::AddMessage(Message::EKind eKind, bool bDefaultKind, Options::EMessageTarget eMessageTarget, const char* szText)
{
	switch (eKind)
	{
		case Message::mkDetail:
			detail("%s%s", (bDefaultKind && m_szDefaultKindPrefix ? m_szDefaultKindPrefix : ""), szText);
			break;

		case Message::mkInfo:
			info("%s%s", (bDefaultKind && m_szDefaultKindPrefix ? m_szDefaultKindPrefix : ""), szText);
			break;

		case Message::mkWarning:
			warn("%s%s", (bDefaultKind && m_szDefaultKindPrefix ? m_szDefaultKindPrefix : ""), szText);
			break;

		case Message::mkError:
			error("%s%s", (bDefaultKind && m_szDefaultKindPrefix ? m_szDefaultKindPrefix : ""), szText);
			break;

		case Message::mkDebug:
			debug("%s%s", (bDefaultKind && m_szDefaultKindPrefix ? m_szDefaultKindPrefix : ""), szText);
			break;
	}
}

void PostScriptController::StartScriptJob(PostInfo* pPostInfo, const char* szScript, bool bNZBFileCompleted, bool bHasFailedParJobs)
{
	info("Executing post-process-script for %s", pPostInfo->GetInfoName());

	PostScriptController* pScriptController = new PostScriptController();
	pScriptController->m_pPostInfo = pPostInfo;
	pScriptController->SetScript(szScript);
	pScriptController->SetWorkingDir(g_pOptions->GetDestDir());
	pScriptController->m_bNZBFileCompleted = bNZBFileCompleted;
	pScriptController->m_bHasFailedParJobs = bHasFailedParJobs;
	pScriptController->SetAutoDestroy(false);

	pPostInfo->SetScriptThread(pScriptController);

	pScriptController->Start();
}

void PostScriptController::Run()
{
	// the locking is needed for accessing the memebers of NZBInfo
	g_pDownloadQueueHolder->LockQueue();

	char szParStatus[10];
	snprintf(szParStatus, 10, "%i", m_pPostInfo->GetParStatus());
	szParStatus[10-1] = '\0';

	char szCollectionCompleted[10];
	snprintf(szCollectionCompleted, 10, "%i", (int)m_bNZBFileCompleted);
	szCollectionCompleted[10-1] = '\0';

	char szHasFailedParJobs[10];
	snprintf(szHasFailedParJobs, 10, "%i", (int)m_bHasFailedParJobs);
	szHasFailedParJobs[10-1] = '\0';

	char szDestDir[1024];
	strncpy(szDestDir, m_pPostInfo->GetNZBInfo()->GetDestDir(), 1024);
	szDestDir[1024-1] = '\0';
	
	char szNZBFilename[1024];
	strncpy(szNZBFilename, m_pPostInfo->GetNZBInfo()->GetFilename(), 1024);
	szNZBFilename[1024-1] = '\0';
	
	char szParFilename[1024];
	strncpy(szParFilename, m_pPostInfo->GetParFilename(), 1024);
	szParFilename[1024-1] = '\0';

	char szCategory[1024];
	strncpy(szCategory, m_pPostInfo->GetNZBInfo()->GetCategory(), 1024);
	szCategory[1024-1] = '\0';

	char szInfoName[1024];
	snprintf(szInfoName, 1024, "post-process-script for %s", m_pPostInfo->GetInfoName());
	szInfoName[1024-1] = '\0';
	SetInfoName(szInfoName);

	SetDefaultKindPrefix("Post-Process: ");
	SetDefaultLogKind(g_pOptions->GetProcessLogKind());

	const char* szArgs[9];
	szArgs[0] = GetScript();
	szArgs[1] = szDestDir;
	szArgs[2] = szNZBFilename;
	szArgs[3] = szParFilename;
	szArgs[4] = szParStatus;
	szArgs[5] = szCollectionCompleted;
	szArgs[6] = szHasFailedParJobs;
	szArgs[7] = szCategory;
	szArgs[8] = NULL;
	SetArgs(szArgs, false);

	SetEnvVar("NZBPP_DIRECTORY", szDestDir);
	SetEnvVar("NZBPP_NZBFILENAME", szNZBFilename);
	SetEnvVar("NZBPP_PARFILENAME", szParFilename);
	SetEnvVar("NZBPP_PARSTATUS", szParStatus);
	SetEnvVar("NZBPP_NZBCOMPLETED", szCollectionCompleted);
	SetEnvVar("NZBPP_PARFAILED", szHasFailedParJobs);
	SetEnvVar("NZBPP_CATEGORY", szCategory);

	for (NZBParameterList::iterator it = m_pPostInfo->GetNZBInfo()->GetParameters()->begin(); it != m_pPostInfo->GetNZBInfo()->GetParameters()->end(); it++)
	{
		NZBParameter* pParameter = *it;
		char szVarname[1024];
		snprintf(szVarname, sizeof(szVarname), "NZBPR_%s", pParameter->GetName());
		szVarname[1024-1] = '\0';
		SetEnvVar(szVarname, pParameter->GetValue());
	}

	g_pDownloadQueueHolder->UnlockQueue();

	int iResult = Execute();

	switch (iResult)
	{
		case POSTPROCESS_SUCCESS:
			info("%s sucessful", szInfoName);
			m_pPostInfo->SetScriptStatus(PostInfo::srSuccess);
			break;

		case POSTPROCESS_ERROR:
			info("%s failed", szInfoName);
			m_pPostInfo->SetScriptStatus(PostInfo::srFailure);
			break;

		case POSTPROCESS_NONE:
			info("%s skipped", szInfoName);
			m_pPostInfo->SetScriptStatus(PostInfo::srNone);
			break;

#ifndef DISABLE_PARCHECK
		case POSTPROCESS_PARCHECK_ALL:
			if (m_pPostInfo->GetParCheck())
			{
				error("%s requested par-check/repair for all collections, but they were already checked", szInfoName);
				m_pPostInfo->SetScriptStatus(PostInfo::srFailure);
			}
			else if (!m_bNZBFileCompleted)
			{
				error("%s requested par-check/repair for all collections, but it was not the call for the last collection", szInfoName);
				m_pPostInfo->SetScriptStatus(PostInfo::srFailure);
			}
			else
			{
				info("%s requested par-check/repair for all collections", szInfoName);
				m_pPostInfo->SetRequestParCheck(PostInfo::rpAll);
				m_pPostInfo->SetScriptStatus(PostInfo::srSuccess);
			}
			break;

		case POSTPROCESS_PARCHECK_CURRENT:
			if (m_pPostInfo->GetParCheck())
			{
				error("%s requested par-check/repair for current collection, but it was already checked", szInfoName);
				m_pPostInfo->SetScriptStatus(PostInfo::srFailure);
			}
			else if (strlen(m_pPostInfo->GetParFilename()) == 0)
			{
				error("%s requested par-check/repair for current collection, but it doesn't have any par-files", szInfoName);
				m_pPostInfo->SetScriptStatus(PostInfo::srFailure);
			}
			else
			{
				info("%s requested par-check/repair for current collection", szInfoName);
				m_pPostInfo->SetRequestParCheck(PostInfo::rpCurrent);
				m_pPostInfo->SetScriptStatus(PostInfo::srSuccess);
			}
			break;
#endif

		default:
			info("%s terminated with unknown status", szInfoName);
			m_pPostInfo->SetScriptStatus(PostInfo::srUnknown);
	}

	m_pPostInfo->SetStage(PostInfo::ptFinished);
	m_pPostInfo->SetWorking(false);
}

void PostScriptController::AddMessage(Message::EKind eKind, bool bDefaultKind, Options::EMessageTarget eMessageTarget, const char* szText)
{
	if (!strncmp(szText, "[HISTORY] ", 10))
	{
		if (eMessageTarget == Options::mtScreen || eMessageTarget == Options::mtBoth)
		{
			m_pPostInfo->GetNZBInfo()->AppendMessage(eKind, 0, szText + 10);
		}
	}
	else
	{
		ScriptController::AddMessage(eKind, bDefaultKind, eMessageTarget, szText);

		if (eMessageTarget == Options::mtScreen || eMessageTarget == Options::mtBoth)
		{
			m_pPostInfo->AppendMessage(eKind, szText);
		}
	}


	if (g_pOptions->GetPausePostProcess())
	{
		time_t tStageTime = m_pPostInfo->GetStageTime();
		time_t tStartTime = m_pPostInfo->GetStartTime();
		time_t tWaitTime = time(NULL);

		// wait until Post-processor is unpaused
		while (g_pOptions->GetPausePostProcess() && !IsStopped())
		{
			usleep(100 * 1000);

			// update time stamps

			time_t tDelta = time(NULL) - tWaitTime;

			if (tStageTime > 0)
			{
				m_pPostInfo->SetStageTime(tStageTime + tDelta);
			}

			if (tStartTime > 0)
			{
				m_pPostInfo->SetStartTime(tStartTime + tDelta);
			}
		}
	}
}

void PostScriptController::Stop()
{
	debug("Stopping post-process-script");
	Thread::Stop();
	Terminate();
}

void NZBScriptController::ExecuteScript(const char* szScript, const char* szNZBFilename, 
	const char* szDirectory, char** pCategory, NZBParameterList* pParameterList)
{
	info("Executing nzb-process-script for %s", Util::BaseFileName(szNZBFilename));

	NZBScriptController* pScriptController = new NZBScriptController();
	pScriptController->SetScript(szScript);
	pScriptController->m_pCategory = pCategory;
	pScriptController->m_pParameterList = pParameterList;

	char szInfoName[1024];
	snprintf(szInfoName, 1024, "nzb-process-script for %s", Util::BaseFileName(szNZBFilename));
	szInfoName[1024-1] = '\0';
	pScriptController->SetInfoName(szInfoName);

	// remove trailing slash
	char szDir[1024];
	strncpy(szDir, szDirectory, 1024);
	szDir[1024-1] = '\0';
	int iLen = strlen(szDir);
	if (szDir[iLen-1] == PATH_SEPARATOR)
	{
		szDir[iLen-1] = '\0';
	}

	pScriptController->SetDefaultKindPrefix("NZB-Process: ");
	pScriptController->SetDefaultLogKind(g_pOptions->GetProcessLogKind());

	const char* szArgs[4];
	szArgs[0] = szScript;
	szArgs[1] = szDir;
	szArgs[2] = szNZBFilename;
	szArgs[3] = NULL;
	pScriptController->SetArgs(szArgs, false);

	pScriptController->SetEnvVar("NZBNP_DIRECTORY", szDir);
	pScriptController->SetEnvVar("NZBNP_FILENAME", szNZBFilename);

	pScriptController->Execute();

	delete pScriptController;
}

void NZBScriptController::AddMessage(Message::EKind eKind, bool bDefaultKind, Options::EMessageTarget eMessageTarget, const char* szText)
{
	if (!strncmp(szText, "[NZB] ", 6))
	{
		debug("Command %s detected", szText + 6);
		if (!strncmp(szText + 6, "CATEGORY=", 9))
		{
			free(*m_pCategory);
			*m_pCategory = strdup(szText + 6 + 9);
		}
		else if (!strncmp(szText + 6, "NZBPR_", 6))
		{
			char* szParam = strdup(szText + 6 + 6);
			char* szValue = strchr(szParam, '=');
			if (szValue)
			{
				*szValue = '\0';
				m_pParameterList->SetParameter(szParam, szValue + 1);
			}
			else
			{
				error("Invalid command \"%s\" received from %s", szText, GetInfoName());
			}
			free(szParam);
		}
		else
		{
			error("Invalid command \"%s\" received from %s", szText, GetInfoName());
		}
	}
	else
	{
		ScriptController::AddMessage(eKind, bDefaultKind, eMessageTarget, szText);
	}
}

void SchedulerScriptController::StartScript(const char* szCommandLine)
{
	char** argv = NULL;
	if (!Util::SplitCommandLine(szCommandLine, &argv))
	{
		error("Could not execute scheduled process-script, failed to parse command line: %s", szCommandLine);
		return;
	}

	info("Executing scheduled process-script %s", Util::BaseFileName(argv[0]));

	SchedulerScriptController* pScriptController = new SchedulerScriptController();
	pScriptController->SetScript(argv[0]);
	pScriptController->SetArgs((const char**)argv, true);
	pScriptController->SetAutoDestroy(true);

	pScriptController->Start();
}

void SchedulerScriptController::Run()
{
	char szInfoName[1024];
	snprintf(szInfoName, 1024, "scheduled process-script %s", Util::BaseFileName(GetScript()));
	szInfoName[1024-1] = '\0';
	SetInfoName(szInfoName);

	SetDefaultKindPrefix("Scheduled Process: ");
	SetDefaultLogKind(g_pOptions->GetProcessLogKind());

	Execute();
}

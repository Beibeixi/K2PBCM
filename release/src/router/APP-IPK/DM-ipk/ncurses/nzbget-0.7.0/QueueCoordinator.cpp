/*
 *  This file is part of nzbget
 *
 *  Copyright (C) 2005 Bo Cordes Petersen <placebodk@users.sourceforge.net>
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
 * $Revision: 1.7 $
 * $Date: 2011/08/15 07:34:39 $
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
#include <cstdio>
#include <sys/stat.h>
#ifndef WIN32
#include <unistd.h>
#include <sys/time.h>
#endif

#include "nzbget.h"
#include "QueueCoordinator.h"
#include "Options.h"
#include "ServerPool.h"
#include "ArticleDownloader.h"
#include "DiskState.h"
#include "Log.h"
#include "Util.h"
#include "Decoder.h"
#include <stdio.h>
#include "logs.h"
#include "DownloadInfo.h"
#include "ex.h"

extern Options* g_pOptions;
extern ServerPool* g_pServerPool;
extern DiskState* g_pDiskState;
extern QueueCoordinator* g_pQueueCoordinator;
//int semUse = 1;

QueueCoordinator::QueueCoordinator()
{
	debug("Creating QueueCoordinator");

	m_bHasMoreJobs = true;
	ResetSpeedStat();

	m_iAllBytes = 0;
	m_tStartServer = 0;
	m_tStartDownload = 0;
	m_tPausedFrom = 0;
	m_bStandBy = true;

	YDecoder::Init();
}

QueueCoordinator::~QueueCoordinator()
{
	debug("Destroying QueueCoordinator");
	// Cleanup

	debug("Deleting DownloadQueue");
	for (FileQueue::iterator it = m_DownloadQueue.GetFileQueue()->begin(); it != m_DownloadQueue.GetFileQueue()->end(); it++)
	{
		delete *it;
	}
	m_DownloadQueue.GetFileQueue()->clear();

	debug("Deleting ArticleDownloaders");
	for (ActiveDownloads::iterator it = m_ActiveDownloads.begin(); it != m_ActiveDownloads.end(); it++)
	{
		delete *it;
	}
	m_ActiveDownloads.clear();

	YDecoder::Final();

	debug("QueueCoordinator destroyed");
}

#if 0
//add by gauss
int SecondToDate(int sec,char *time)
{
  int second,minute,hour;
  char csec[3],cmin[3],chour[3];
  hour = sec / 3600;
  minute = (sec - hour * 3600) / 60 ;
  second = sec - hour * 3600 - minute *60;

 sprintf(time,"%2d:%2d:%2d",hour,minute,second);

 int i;
 for(i=0;time[i] != '\0';i++)
        if(time[i]== ' ')
                time[i] = '0';

}

char  *cut_path(char *path , int n)
{

    int i;

    char *new_path = NULL;
    new_path = path;

    for(i= 0;i< n ;i++)
    {
        new_path = strchr(new_path,'/');
        new_path++;
    }

    //strcpy(info_dest_path,new_path);

    return new_path;
}
#endif

void QueueCoordinator::Run()
{
	debug("Entering QueueCoordinator-loop");

	m_mutexDownloadQueue.Lock();

        // Walf add,


            struct dirent* ent = NULL;
            DIR *pDir;
            pDir=opendir(g_pOptions->GetLogsDir());
            while (NULL != (ent=readdir(pDir)))
            {
                if (ent->d_reclen==20)
                {
                    if (ent->d_type==8)
                    {
                        char tmp[256];
                        memset(tmp, 0, sizeof(tmp));
                        sprintf(tmp, "%s/%s", g_pOptions->GetLogsDir(), ent->d_name);
                        printf("rm file:%s\n", ent->d_name);
                        int er = remove(tmp);

                    }
                }
            }
            if(pDir)
               closedir(pDir);

            // Walf add


	if (g_pOptions->GetServerMode() && g_pOptions->GetSaveQueue() && g_pDiskState->DownloadQueueExists())
	{
		if (g_pOptions->GetReloadQueue())
		{
			g_pDiskState->LoadDownloadQueue(&m_DownloadQueue);
		}
		else
		{
			g_pDiskState->DiscardDownloadQueue();
		}
	}

	g_pDiskState->CleanupTempDir(&m_DownloadQueue);

	m_mutexDownloadQueue.Unlock();

	m_tStartServer = time(NULL);
	m_tLastCheck = m_tStartServer;
	bool bWasStandBy = true;
	bool bArticeDownloadsRunning = false;
	int iResetCounter = 0;

	while (!IsStopped())
	{
		if (!(g_pOptions->GetPauseDownload() || g_pOptions->GetPauseDownload2()))
		{
			NNTPConnection* pConnection = g_pServerPool->GetConnection(0);
			if (pConnection)
			{
				// start download for next article
				FileInfo* pFileInfo;
				ArticleInfo* pArticleInfo;

				m_mutexDownloadQueue.Lock();
                                bool bHasMoreArticles = GetNextArticle(pFileInfo, pArticleInfo);

				bArticeDownloadsRunning = !m_ActiveDownloads.empty();
				m_bHasMoreJobs = bHasMoreArticles || bArticeDownloadsRunning;
				if (bHasMoreArticles && !IsStopped() && Thread::GetThreadCount() < g_pOptions->GetThreadLimit())
				{
                                    StartArticleDownload(pFileInfo, pArticleInfo, pConnection);
                                    bArticeDownloadsRunning = true;
				}
				else
				{

					g_pServerPool->FreeConnection(pConnection, false);
				}
				m_mutexDownloadQueue.Unlock();
			}
		}
		else
		{
			m_mutexDownloadQueue.Lock();
			bArticeDownloadsRunning = !m_ActiveDownloads.empty();
			m_mutexDownloadQueue.Unlock();
                }


		bool bStandBy = !bArticeDownloadsRunning;
		if (bStandBy ^ bWasStandBy)
		{
			EnterLeaveStandBy(bStandBy);
			bWasStandBy = bStandBy;
		}


		// sleep longer in StandBy
		int iSleepInterval = bStandBy ? 100 : 5;
		usleep(iSleepInterval * 1000);

                AddSpeedReading(0);

		iResetCounter+= iSleepInterval;

                //info("iRestCounter is %d",iResetCounter);
                //if (iResetCounter >= 1000 || logtime == 0)
                if (iResetCounter >= 1000 )
		{
                    //FileInfo *pFileInfo;
#if 0
                 if(bArticeDownloadsRunning)
                    {

                    info("$$$$$$$$$$ start update log $$$$$$$$");
                    NZBInfo *pNZBInfo;

                    GroupQueue groupQueue;
                    groupQueue.clear();
                    DownloadQueue* pDownloadQueue = g_pQueueCoordinator->LockQueue();
                    pDownloadQueue->BuildGroups(&groupQueue);
                    g_pQueueCoordinator->UnlockQueue();

                   
                    for (GroupQueue::iterator it = groupQueue.begin(); it != groupQueue.end(); it++)
                    {

                            GroupInfo* pGroupInfo = *it;

                            unsigned long long iFileSizeHi, iFileSizeLo, iFileSize;
                            unsigned long long iRemainingSizeLo, iRemainingSizeHi, iRemainingSize;
                            int ID;
                            ID = pGroupInfo->GetNZBInfo()->GetID();
                            char szNZBNicename[1024];
                            iFileSize = pGroupInfo->GetNZBInfo()->GetSize();
                            iRemainingSize = pGroupInfo->GetRemainingSize();
                            pGroupInfo->GetNZBInfo()->GetNiceNZBName(szNZBNicename, sizeof(szNZBNicename));

                            int iUpTimeSec, iDownloadTimeSec;
                            long long iAllBytes;
                            bool bServerStandBy;
                            g_pQueueCoordinator->CalcStat(&iUpTimeSec, &iDownloadTimeSec, &iAllBytes, &bServerStandBy);

                            if (pGroupInfo->GetNZBInfo()->GetDeleted())
                            {
                               //RemoveIncompleteFiles(szNZBNicename);
                                continue;
                              }
                    // Walf add: update log files

                    //NZBInfo *pNZBInfo,*pNZBInfo2;
                    	char path[256];

                        memset(path, 0, sizeof(path));

                        sprintf(path, "%s/nzb_%d", g_pOptions->GetLogsDir(), ID);

                        //add by gauss for sem
                        int semUse = 1;
                        char sem_name[1024];
                        sprintf(sem_name,"%ssem.nzb_%d",g_pOptions->GetSemsDir(),ID);

                        sema_t sema;
                        if(sema_open(&sema, sem_name, O_RDONLY) == -1){
                                printf("sem open error\n");
                                semUse = 0;
                                }

                        if(semUse)
                            sema_wait(&sema);

                        FILE *fp = fopen(path, "w");
                        if(fp)
                        {
                            // do
                            Log_struc log_s;
                            sprintf(log_s.id, "%d", ID);
                            time(&log_s.begin_t);
                            time(&log_s.now_t);
                            log_s.download_type = NZB;

                            strcpy(log_s.filename, szNZBNicename);
                            log_s.filesize = iFileSize;

                            //add by gauss

                            strcpy(log_s.fullname , szNZBNicename);

                            char info_dest_path[256];
                            memset(info_dest_path,'\0',sizeof(info_dest_path));
                            strcpy(info_dest_path,g_pOptions->GetCompleteDir());

                            char *new_path = cut_path(info_dest_path,4);

                            //strcpy(info_dest_path,new_path);

                            //strcpy(log_s.ifile.inzb.Destination,g_pOptions->GetCompleteDir());
                            strcpy(log_s.ifile.inzb.Destination,new_path);
                            log_s.ifile.inzb.Created_time = log_s.begin_t;
                            log_s.ifile.inzb.Start_Time = log_s.begin_t;
                            int iDownloadRate = (int)(g_pQueueCoordinator->CalcCurrentDownloadSpeed());
                            log_s.ifile.inzb.rate = iDownloadRate;

                            int t_left ;

                            if(iDownloadRate > 0 )
                            {
                                t_left = (int) (iFileSize /1024 / iDownloadRate);
                            }
                            else
                            {
                                strcpy(log_s.ifile.inzb.Time_Left,"");
                            }
                            SecondToDate(t_left,log_s.ifile.inzb.Time_Left);

                            float percent_done = (float) (iFileSize -iRemainingSize ) / iFileSize;
                            log_s.progress = percent_done;


                            //info("remainingParCount file count is %d",pGroupInfo->GetRemainingParCount());
                            //info("ParkedFileCount file count is %d",pGroupInfo->GetNZBInfo()->GetParkedFileCount());

                            int FileCompleteCount = pGroupInfo->GetNZBInfo()->GetCompletedFiles()->size();

                            if ( pGroupInfo->GetNZBInfo()->GetFileCount() == pGroupInfo->GetPausedFileCount() + FileCompleteCount )
                            {
                                 log_s.status = S_PAUSED;
                              }
                             else
                            {
                                log_s.status = S_PROCESSING;
                            }

                            /*
                            if(g_pOptions->GetPauseDownload())
                                 {
                                      log_s.status = S_PAUSED;
                                   }
                            else
                             {
                                log_s.status = S_PROCESSING;
                              }
                            */


                            int count = pGroupInfo->GetLastID() - pGroupInfo->GetFirstID() + 1;


                            //info("file count is %d",pGroupInfo->GetNZBInfo()->GetFileCount());
                            //info("pause file count is %d",pGroupInfo->GetPausedFileCount());
                            //info("complete file is %d",FileCompleteCount);
                            //info("total pause file count is %d",count);

                            log_s.ifile.inzb.fileCount = count;

                            fwrite(&log_s,  LOG_SIZE, 1,fp);

                            /*
                            nzb_file nzb_files[count];
                            //memset(nzb_files,"\0",sizeof(nzb_files));

                            FileQueue t_FileQueue;
			    
                            DownloadQueue* pDownloadQueue = g_pQueueCoordinator->LockQueue();

                            for (FileQueue::iterator it = pDownloadQueue->GetFileQueue()->begin(); it != pDownloadQueue->GetFileQueue()->end(); it++)
                            {
                                    	    FileInfo* pFileInfo = *it;
                                    	    int iFileID = pFileInfo ->GetID();
					 
                                            if(iFileID >= pGroupInfo->GetFirstID() && iFileID <= pGroupInfo->GetLastID() )
                                            {
                                                t_FileQueue.push_back(pFileInfo);
                                            }
                                        
                                }
                            g_pQueueCoordinator->UnlockQueue();

                            for (FileQueue::iterator it = t_FileQueue.begin(); it != t_FileQueue.end(); it++)
                            {
                                int i = 0;
                                FileInfo* tFileInfo = *it;

                                unsigned long long iFileSize,iRemainingSize;
                                double file_percent;
                                char file_name[512];

                                iFileSize = tFileInfo->GetSize();
                                iRemainingSize = tFileInfo->GetRemainingSize();
                                file_percent = (double) ( tFileInfo->GetSize() - tFileInfo->GetRemainingSize() ) / tFileInfo->GetSize();

                                strcpy(nzb_files[i].name,tFileInfo->GetFilename());
                                nzb_files[i].size = iFileSize;
                                //nzb_files[i].total_size = iFileSize;
                                nzb_files[i].percent_done = file_percent;

                                fwrite(&nzb_files[i], sizeof(nzb_file), 1, fp);

                                i++;

                            }
                            */
                            fclose(fp);
                        }


                        if(semUse)
                        {
                            sema_post(&sema);
                            sema_close(&sema);
                        }

                    }
                }
#endif
                  //logtime++;


                       //read log

                        // Walf add: update log files

                        // this code should not be called too often, once per second is OK
                        g_pServerPool->CloseUnusedConnections();
                        ResetHangingDownloads();
                        iResetCounter = 0;
                        AdjustStartTime();
                }
        }


        // waiting for downloads
        debug("QueueCoordinator: waiting for Downloads to complete");
        bool completed = false;
        while (!completed)
        {
                m_mutexDownloadQueue.Lock();
                completed = m_ActiveDownloads.size() == 0;
                m_mutexDownloadQueue.Unlock();
                usleep(100 * 1000);
                ResetHangingDownloads();
        }
        debug("QueueCoordinator: Downloads are completed");

        debug("Exiting QueueCoordinator-loop");



}

void QueueCoordinator::AddNZBFileToQueue(NZBFile* pNZBFile, bool bAddFirst)
{
       debug("Adding NZBFile to queue");

        m_mutexDownloadQueue.Lock();

        FileQueue tmpFileQueue;
        tmpFileQueue.clear();
        FileQueue DupeList;
        DupeList.clear();

        int index1 = 0;
        for (NZBFile::FileInfos::iterator it = pNZBFile->GetFileInfos()->begin(); it != pNZBFile->GetFileInfos()->end(); it++)
        {

                index1++;
                FileInfo* pFileInfo = *it;
        //        printf("NZBfile size: %d\n", pNZBFile->GetFileInfos()->size());
        //        printf("add NZB id: %d\n", pFileInfo->GetID());

                if (g_pOptions->GetDupeCheck())
                {

                        bool dupe = false;
                        if (IsDupe(pFileInfo))
                        {
                                warn("File \"%s\" seems to be duplicate, skipping", pFileInfo->GetFilename());
                                dupe = true;
                        }
                        int index2 = 0;
                        for (NZBFile::FileInfos::iterator it2 = pNZBFile->GetFileInfos()->begin(); it2 != pNZBFile->GetFileInfos()->end(); it2++)
                        {
                                index2++;
                                FileInfo* pFileInfo2 = *it2;
                                if (pFileInfo != pFileInfo2 &&
                                        !strcmp(pFileInfo->GetFilename(), pFileInfo2->GetFilename()) &&
                                        (pFileInfo->GetSize() < pFileInfo2->GetSize() ||
                                         (pFileInfo->GetSize() == pFileInfo2->GetSize() && index2 < index1)))
                                {
                                        warn("File \"%s\" appears twice in nzb-request, adding only the biggest file", pFileInfo->GetFilename());
                                        dupe = true;
                                        break;
                                }
                        }
                        if (dupe)
                        {
                                DupeList.push_back(pFileInfo);
                                continue;
                        }
                }

                if (bAddFirst)
                {
                        tmpFileQueue.push_front(pFileInfo);
                }
                else
                {
                      tmpFileQueue.push_back(pFileInfo);
                }
        }

        for (FileQueue::iterator it = tmpFileQueue.begin(); it != tmpFileQueue.end(); it++)
        {
                if (bAddFirst)
                {
                        m_DownloadQueue.GetFileQueue()->push_front(*it);
                }
                else
                {
                     m_DownloadQueue.GetFileQueue()->push_back(*it);
                }
        }

        for (FileQueue::iterator it = DupeList.begin(); it != DupeList.end(); it++)
        {
                FileInfo* pFileInfo = *it;
                if (g_pOptions->GetSaveQueue() && g_pOptions->GetServerMode())
                {
                        g_pDiskState->DiscardFile(pFileInfo);
                }
                delete pFileInfo;
        }

        m_DownloadQueue.GetNZBInfoList()->Add(pNZBFile->GetNZBInfo());

        pNZBFile->DetachFileInfos();

        Aspect aspect = { eaNZBFileAdded, &m_DownloadQueue, pNZBFile->GetNZBInfo(), NULL };
        Notify(&aspect);

        if (g_pOptions->GetSaveQueue() && g_pOptions->GetServerMode())
        {
              g_pDiskState->SaveDownloadQueue(&m_DownloadQueue);
        }

        m_mutexDownloadQueue.Unlock();
}

/*
 * NOTE: see note to "AddSpeedReading"
 */
float QueueCoordinator::CalcCurrentDownloadSpeed()
{
        if (m_bStandBy)
        {
                return 0;
        }

    int iTimeDiff = (int)time(NULL) - m_iSpeedStartTime * SPEEDMETER_SLOTSIZE;
    if (iTimeDiff == 0)
    {
        return 0;
    }
        float fSpeed = m_iSpeedTotalBytes / 1024.0f / iTimeDiff;
        return fSpeed;
}

/*
 * NOTE: we should use mutex by access to m_iSpeedBytes and m_iSpeedBytesIndex,
 * but this would results in a big performance loss (the function
 * "AddSpeedReading" is called extremly often), so we better agree with calculation
 * errors possible because of simultaneuos access from several threads.
 * The used algorithm is able to recover after few seconds.
 * In any case the calculation errors can not result in fatal system
 * errors (segmentation faults).
 */
void QueueCoordinator::AddSpeedReading(int iBytes)
{
    int iNowSlot = (int)time(NULL) / SPEEDMETER_SLOTSIZE;

    if (iNowSlot > m_iSpeedTime[m_iSpeedBytesIndex])
    {
        //record bytes in next slot
        m_iSpeedBytesIndex++;
        if (m_iSpeedBytesIndex >= SPEEDMETER_SLOTS)
        {
            m_iSpeedBytesIndex = 0;
        }
        //Adjust counters with outging information.
        m_iSpeedTotalBytes -= m_iSpeedBytes[m_iSpeedBytesIndex];

        //Note we should really use the start time of the next slot
        //but its easier to just use the outgoing slot time. This
        //will result in a small error.
        m_iSpeedStartTime = m_iSpeedTime[m_iSpeedBytesIndex];

        //Now reset.
        m_iSpeedBytes[m_iSpeedBytesIndex] = 0;
        m_iSpeedTime[m_iSpeedBytesIndex] = iNowSlot;
    }

    if (m_iSpeedTotalBytes == 0)
    {
        m_iSpeedStartTime = iNowSlot;
    }
    m_iSpeedBytes[m_iSpeedBytesIndex] += iBytes;
    m_iSpeedTotalBytes += iBytes;
        m_iAllBytes += iBytes;
}

void QueueCoordinator::ResetSpeedStat()
{
    m_iSpeedStartTime = (int)time(NULL) / SPEEDMETER_SLOTSIZE;
        for (int i = 0; i < SPEEDMETER_SLOTS; i++)
        {
                m_iSpeedBytes[i] = 0;
        m_iSpeedTime[i] = m_iSpeedStartTime;
        }
        m_iSpeedBytesIndex = 0;
    m_iSpeedTotalBytes = 0;
}

long long QueueCoordinator::CalcRemainingSize()
{
        long long lRemainingSize = 0;

        m_mutexDownloadQueue.Lock();
        for (FileQueue::iterator it = m_DownloadQueue.GetFileQueue()->begin(); it != m_DownloadQueue.GetFileQueue()->end(); it++)
        {
                FileInfo* pFileInfo = *it;
                if (!pFileInfo->GetPaused() && !pFileInfo->GetDeleted())
                {
                        lRemainingSize += pFileInfo->GetRemainingSize();
                }
        }
        m_mutexDownloadQueue.Unlock();

        return lRemainingSize;
}

/*
 * NOTE: DownloadQueue must be locked prior to call of this function
 * Returns True if Entry was deleted from Queue or False if it was scheduled for Deletion.
 * NOTE: "False" does not mean unsuccess; the entry is (or will be) deleted in any case.
 */
bool QueueCoordinator::DeleteQueueEntry(FileInfo* pFileInfo)
{
        pFileInfo->SetDeleted(true);
        bool hasDownloads = false;
        for (ActiveDownloads::iterator it = m_ActiveDownloads.begin(); it != m_ActiveDownloads.end(); it++)
        {
                ArticleDownloader* pArticleDownloader = *it;
                if (pArticleDownloader->GetFileInfo() == pFileInfo)
                {
                        hasDownloads = true;
                        pArticleDownloader->Stop();
                }
        }
        if (!hasDownloads)
        {
                //info("hasDownload");
                Aspect aspect = { eaFileDeleted, &m_DownloadQueue, pFileInfo->GetNZBInfo(), pFileInfo };
                Notify(&aspect);

                DeleteFileInfo(pFileInfo, false);
        }

        return hasDownloads;
}

void QueueCoordinator::Stop()
{
        Thread::Stop();

        debug("Stopping ArticleDownloads");
        m_mutexDownloadQueue.Lock();
        for (ActiveDownloads::iterator it = m_ActiveDownloads.begin(); it != m_ActiveDownloads.end(); it++)
        {
                (*it)->Stop();
        }
        m_mutexDownloadQueue.Unlock();
        debug("ArticleDownloads are notified");
}

bool QueueCoordinator::GetNextArticle(FileInfo* &pFileInfo, ArticleInfo* &pArticleInfo)
{
        //debug("QueueCoordinator::GetNextArticle()");

        for (FileQueue::iterator it = m_DownloadQueue.GetFileQueue()->begin(); it != m_DownloadQueue.GetFileQueue()->end(); it++)
        {
                pFileInfo = *it;
                if (!pFileInfo->GetPaused() && !pFileInfo->GetDeleted())
                {
                        if (pFileInfo->GetArticles()->empty() && g_pOptions->GetSaveQueue() && g_pOptions->GetServerMode())
                        {
                                g_pDiskState->LoadArticles(pFileInfo);
                        }
                        for (FileInfo::Articles::iterator at = pFileInfo->GetArticles()->begin(); at != pFileInfo->GetArticles()->end(); at++)
                        {
                                pArticleInfo = *at;
                                if (pArticleInfo->GetStatus() == 0)
                                {
                                        return true;
                                }
                        }
                }
        }

        return false;
}

void QueueCoordinator::StartArticleDownload(FileInfo* pFileInfo, ArticleInfo* pArticleInfo, NNTPConnection* pConnection)
{
        debug("Starting new ArticleDownloader");

        ArticleDownloader* pArticleDownloader = new ArticleDownloader();
        pArticleDownloader->SetAutoDestroy(true);
        pArticleDownloader->Attach(this);
        pArticleDownloader->SetFileInfo(pFileInfo);
        pArticleDownloader->SetArticleInfo(pArticleInfo);
        pArticleDownloader->SetConnection(pConnection);
        BuildArticleFilename(pArticleDownloader, pFileInfo, pArticleInfo);

        pArticleInfo->SetStatus(ArticleInfo::aiRunning);

        m_ActiveDownloads.push_back(pArticleDownloader);
        pArticleDownloader->Start();
}

void QueueCoordinator::BuildArticleFilename(ArticleDownloader* pArticleDownloader, FileInfo* pFileInfo, ArticleInfo* pArticleInfo)
{
        char name[1024];

        snprintf(name, 1024, "%s%i.%03i", g_pOptions->GetTempDir(), pFileInfo->GetID(), pArticleInfo->GetPartNumber());
        name[1024-1] = '\0';
        pArticleInfo->SetResultFilename(name);

        char tmpname[1024];
        snprintf(tmpname, 1024, "%s.tmp", name);
        tmpname[1024-1] = '\0';
        pArticleDownloader->SetTempFilename(tmpname);

        char szNZBNiceName[1024];
        pFileInfo->GetNZBInfo()->GetNiceNZBName(szNZBNiceName, 1024);

        snprintf(name, 1024, "%s%c%s [%i/%i]", szNZBNiceName, (int)PATH_SEPARATOR, pFileInfo->GetFilename(), pArticleInfo->GetPartNumber(), pFileInfo->GetArticles()->size());
        name[1024-1] = '\0';
        pArticleDownloader->SetInfoName(name);
        if (g_pOptions->GetDirectWrite())
        {
                snprintf(name, 1024, "%s%i.out", g_pOptions->GetTempDir(), pFileInfo->GetID());
                name[1024-1] = '\0';
                pArticleDownloader->SetOutputFilename(name);
        }
}

DownloadQueue* QueueCoordinator::LockQueue()
{
        m_mutexDownloadQueue.Lock();
        return &m_DownloadQueue;
}

void QueueCoordinator::UnlockQueue()
{
        m_mutexDownloadQueue.Unlock();
}

void QueueCoordinator::Update(Subject* Caller, void* Aspect)
{
        debug("Notification from ArticleDownloader received");

        ArticleDownloader* pArticleDownloader = (ArticleDownloader*) Caller;
        if ((pArticleDownloader->GetStatus() == ArticleDownloader::adFinished) ||
                (pArticleDownloader->GetStatus() == ArticleDownloader::adFailed) ||
                (pArticleDownloader->GetStatus() == ArticleDownloader::adRetry))
        {
                ArticleCompleted(pArticleDownloader);
        }
}

void QueueCoordinator::ArticleCompleted(ArticleDownloader* pArticleDownloader)
{
        debug("Article downloaded");

        FileInfo* pFileInfo = pArticleDownloader->GetFileInfo();
        ArticleInfo* pArticleInfo = pArticleDownloader->GetArticleInfo();
        bool bPaused = false;
        bool fileCompleted = false;

        m_mutexDownloadQueue.Lock();

        if (pArticleDownloader->GetStatus() == ArticleDownloader::adFinished)
        {
                pArticleInfo->SetStatus(ArticleInfo::aiFinished);
        }
        else if (pArticleDownloader->GetStatus() == ArticleDownloader::adFailed)
        {
                pArticleInfo->SetStatus(ArticleInfo::aiFailed);
        }
        else if (pArticleDownloader->GetStatus() == ArticleDownloader::adRetry)
        {
                pArticleInfo->SetStatus(ArticleInfo::aiUndefined);
                bPaused = true;
        }

        if (!bPaused)
        {
                pFileInfo->SetRemainingSize(pFileInfo->GetRemainingSize() - pArticleInfo->GetSize());
                pFileInfo->SetCompleted(pFileInfo->GetCompleted() + 1);
                fileCompleted = (int)pFileInfo->GetArticles()->size() == pFileInfo->GetCompleted();
        }

        if (!pFileInfo->GetFilenameConfirmed() &&
                pArticleDownloader->GetStatus() == ArticleDownloader::adFinished &&
                pArticleDownloader->GetArticleFilename())
        {
                pFileInfo->SetFilename(pArticleDownloader->GetArticleFilename());
                pFileInfo->SetFilenameConfirmed(true);
                if (g_pOptions->GetDupeCheck() && pFileInfo->IsDupe(pFileInfo->GetFilename()))
                {
                        warn("File \"%s\" seems to be duplicate, cancelling download and deleting file from queue", pFileInfo->GetFilename());
                        fileCompleted = false;
                        DeleteQueueEntry(pFileInfo);
                }
        }

        bool deleteFileObj = false;

        if (pFileInfo->GetDeleted())
        {
                int cnt = 0;
                for (ActiveDownloads::iterator it = m_ActiveDownloads.begin(); it != m_ActiveDownloads.end(); it++)
                {
                        if ((*it)->GetFileInfo() == pFileInfo)
                        {
                                cnt++;
                        }
                }
                if (cnt == 1)
                {
                        // this was the last Download for a file deleted from queue
                        deleteFileObj = true;
                }
        }

        if (fileCompleted && !IsStopped() && !pFileInfo->GetDeleted())
        {
                // all jobs done
                m_mutexDownloadQueue.Unlock();
                pArticleDownloader->CompleteFileParts();
                m_mutexDownloadQueue.Lock();
                deleteFileObj = true;
        }

        // delete Download from Queue
        for (ActiveDownloads::iterator it = m_ActiveDownloads.begin(); it != m_ActiveDownloads.end(); it++)
        {
                ArticleDownloader* pa = *it;
                if (pa == pArticleDownloader)
                {
                        m_ActiveDownloads.erase(it);
                        break;
                }
        }

        if (deleteFileObj)
        {
                bool fileDeleted = pFileInfo->GetDeleted();
                // delete File from Queue
                pFileInfo->SetDeleted(true);

                Aspect aspect = { fileCompleted && !fileDeleted ? eaFileCompleted : eaFileDeleted, &m_DownloadQueue, pFileInfo->GetNZBInfo(), pFileInfo };
                Notify(&aspect);

                DeleteFileInfo(pFileInfo, fileCompleted);
                printf("deletet form download queue: %d\n", pFileInfo->GetID());
                if (g_pOptions->GetSaveQueue() && g_pOptions->GetServerMode())
                {
                        g_pDiskState->SaveDownloadQueue(&m_DownloadQueue);
                }
        }

        m_mutexDownloadQueue.Unlock();
}

void QueueCoordinator::DeleteFileInfo(FileInfo* pFileInfo, bool bCompleted)
{
        for (FileQueue::iterator it = m_DownloadQueue.GetFileQueue()->begin(); it != m_DownloadQueue.GetFileQueue()->end(); it++)
        {
                FileInfo* pa = *it;
                if (pa == pFileInfo)
                {
                        m_DownloadQueue.GetFileQueue()->erase(it);
                        break;
                }
        }

        if (g_pOptions->GetSaveQueue() && g_pOptions->GetServerMode())
        {
                g_pDiskState->DiscardFile(pFileInfo);
        }

        if (!bCompleted)
        {
                DiscardDiskFile(pFileInfo);
        }

        delete pFileInfo;
}

void QueueCoordinator::DiscardDiskFile(FileInfo* pFileInfo)
{
        // deleting temporary files

        if (!g_pOptions->GetDirectWrite() || g_pOptions->GetContinuePartial())
        {
                for (FileInfo::Articles::iterator it = pFileInfo->GetArticles()->begin(); it != pFileInfo->GetArticles()->end(); it++)
                {
                        ArticleInfo* pa = *it;
                        if (pa->GetResultFilename())
                        {
                                remove(pa->GetResultFilename());
                        }
                }
        }

        if (g_pOptions->GetDirectWrite())
        {
                char name[1024];
                snprintf(name, 1024, "%s%i.out", g_pOptions->GetTempDir(), pFileInfo->GetID());
                name[1024-1] = '\0';
                remove(name);
        }
}

bool QueueCoordinator::IsDupe(FileInfo* pFileInfo)
{
        debug("Checking if the file is already queued");

        // checking on disk
        if (pFileInfo->IsDupe(pFileInfo->GetFilename()))
        {
                return true;
        }

        // checking in queue
        for (FileQueue::iterator it = m_DownloadQueue.GetFileQueue()->begin(); it != m_DownloadQueue.GetFileQueue()->end(); it++)
        {
                FileInfo* pQueueEntry = *it;
                if (!strcmp(pFileInfo->GetNZBInfo()->GetDestDir(), pQueueEntry->GetNZBInfo()->GetDestDir()) &&
                        !strcmp(pFileInfo->GetFilename(), pQueueEntry->GetFilename()) &&
                        pFileInfo != pQueueEntry)
                {
                        return true;
                }
        }

        return false;
}

void QueueCoordinator::LogDebugInfo()
{
        debug("--------------------------------------------");
        debug("Dumping debug info to log");
        debug("--------------------------------------------");

        debug("   QueueCoordinator");
        debug("   ----------------");

        m_mutexDownloadQueue.Lock();
        debug("    Active Downloads: %i", m_ActiveDownloads.size());
        for (ActiveDownloads::iterator it = m_ActiveDownloads.begin(); it != m_ActiveDownloads.end(); it++)
        {
                ArticleDownloader* pArticleDownloader = *it;
                pArticleDownloader->LogDebugInfo();
        }
        m_mutexDownloadQueue.Unlock();

        debug("");

        g_pServerPool->LogDebugInfo();
}

void QueueCoordinator::ResetHangingDownloads()
{
        const int TimeOut = g_pOptions->GetTerminateTimeout();
        if (TimeOut == 0)
        {
                return;
        }

        m_mutexDownloadQueue.Lock();
        time_t tm = ::time(NULL);

        for (ActiveDownloads::iterator it = m_ActiveDownloads.begin(); it != m_ActiveDownloads.end();)
        {
                ArticleDownloader* pArticleDownloader = *it;
            //    printf("ArticleDownloader filemame: %s; remain size: %d\n", pArticleDownloader->GetFileInfo()->GetFilename(), pArticleDownloader->GetFileInfo()->GetRemainingSize());
                if (tm - pArticleDownloader->GetLastUpdateTime() > TimeOut &&
                   pArticleDownloader->GetStatus() == ArticleDownloader::adRunning)
                {
                        ArticleInfo* pArticleInfo = pArticleDownloader->GetArticleInfo();
                        debug("Terminating hanging download %s", pArticleDownloader->GetInfoName());
                        if (pArticleDownloader->Terminate())
                        {
                                error("Terminated hanging download %s", pArticleDownloader->GetInfoName());
                                pArticleInfo->SetStatus(ArticleInfo::aiUndefined);
                        }
                        else
                        {
                                error("Could not terminate hanging download %s", Util::BaseFileName(pArticleInfo->GetResultFilename()));
                        }
                        m_ActiveDownloads.erase(it);
                        // it's not safe to destroy pArticleDownloader, because the state of object is unknown
                        delete pArticleDownloader;
                        it = m_ActiveDownloads.begin();
                        continue;
                }
                it++;
        }

        m_mutexDownloadQueue.Unlock();
}

void QueueCoordinator::EnterLeaveStandBy(bool bEnter)
{
        m_mutexStat.Lock();
        m_bStandBy = bEnter;
        if (bEnter)
        {
                m_tPausedFrom = time(NULL);
        }
        else
        {
                if (m_tStartDownload == 0)
                {
                        m_tStartDownload = time(NULL);
                }
                else
                {
                        m_tStartDownload += time(NULL) - m_tPausedFrom;
                }
                m_tPausedFrom = 0;
                ResetSpeedStat();
        }
        m_mutexStat.Unlock();
}

void QueueCoordinator::CalcStat(int* iUpTimeSec, int* iDnTimeSec, long long* iAllBytes, bool* bStandBy)
{
        m_mutexStat.Lock();
        if (m_tStartServer > 0)
        {
                *iUpTimeSec = (int)(time(NULL) - m_tStartServer);
        }
        else
        {
                *iUpTimeSec = 0;
        }
        *bStandBy = m_bStandBy;
        if (m_bStandBy)
        {
                *iDnTimeSec = (int)(m_tPausedFrom - m_tStartDownload);
        }
        else
        {
                *iDnTimeSec = (int)(time(NULL) - m_tStartDownload);
        }
        *iAllBytes = m_iAllBytes;
        m_mutexStat.Unlock();
}

/*
 * Detects large step changes of system time and adjust statistics.
 */
void QueueCoordinator::AdjustStartTime()
{
        time_t m_tCurTime = time(NULL);
        time_t tDiff = m_tCurTime - m_tLastCheck;
        if (tDiff > 60 || tDiff < 0)
        {
                m_tStartServer += tDiff + 1; // "1" because the method is called once per second
                if (m_tStartDownload != 0)
                {
                        m_tStartDownload += tDiff + 1;
                }
        }
        m_tLastCheck = m_tCurTime;
}

bool QueueCoordinator::SetQueueEntryNZBCategory(NZBInfo* pNZBInfo, const char* szCategory)
{
        if (pNZBInfo->GetPostProcess())
        {
                char szNZBNiceName[1024];
                pNZBInfo->GetNiceNZBName(szNZBNiceName, 1024);
                error("Could not change category for %s. File in post-process-stage", szNZBNiceName);
                return false;
        }

        char szOldDestDir[1024];
        strncpy(szOldDestDir, pNZBInfo->GetDestDir(), 1024);
        szOldDestDir[1024-1] = '\0';

        pNZBInfo->SetCategory(szCategory);
        pNZBInfo->BuildDestDirName();

        bool bDirUnchanged = !strcmp(pNZBInfo->GetDestDir(), szOldDestDir);
        bool bOK = bDirUnchanged || ArticleDownloader::MoveCompletedFiles(pNZBInfo, szOldDestDir);

        return bOK;
}

/*
 * NOTE: DownloadQueue must be locked prior to call of this function
 */
bool QueueCoordinator::MergeQueueEntries(NZBInfo* pDestNZBInfo, NZBInfo* pSrcNZBInfo)
{
        if (pDestNZBInfo->GetPostProcess() || pSrcNZBInfo->GetPostProcess())
        {
                char szDestNZBNiceName[1024];
                pDestNZBInfo->GetNiceNZBName(szDestNZBNiceName, 1024);
                char szSrcNZBNiceName[1024];
                pSrcNZBInfo->GetNiceNZBName(szSrcNZBNiceName, 1024);
                error("Could not merge %s and %s. File in post-process-stage", szDestNZBNiceName, szSrcNZBNiceName);
                return false;
        }

        // set new dest directory, new category and move downloaded files to new dest directory
        pSrcNZBInfo->SetFilename(pSrcNZBInfo->GetFilename());
        SetQueueEntryNZBCategory(pSrcNZBInfo, pDestNZBInfo->GetCategory());

        // reattach file items to new NZBInfo-object
        for (FileQueue::iterator it = m_DownloadQueue.GetFileQueue()->begin(); it != m_DownloadQueue.GetFileQueue()->end(); it++)
        {
                FileInfo* pFileInfo = *it;
                if (pFileInfo->GetNZBInfo() == pSrcNZBInfo)
                {
                        pFileInfo->SetNZBInfo(pDestNZBInfo);
                }
        }

        pDestNZBInfo->SetFileCount(pDestNZBInfo->GetFileCount() + pSrcNZBInfo->GetFileCount());
        pDestNZBInfo->SetSize(pDestNZBInfo->GetSize() + pSrcNZBInfo->GetSize());

        // reattach completed file items to new NZBInfo-object
        for (NZBInfo::Files::iterator it = pSrcNZBInfo->GetCompletedFiles()->begin(); it != pSrcNZBInfo->GetCompletedFiles()->end(); it++)
    {
                char* szFileName = *it;
                pDestNZBInfo->GetCompletedFiles()->push_back(szFileName);
        }
        pSrcNZBInfo->GetCompletedFiles()->clear();

        // concatenate QueuedFilenames using character '|' as separator
        int iLen = strlen(pDestNZBInfo->GetQueuedFilename()) + strlen(pSrcNZBInfo->GetQueuedFilename()) + 1;
        char* szQueuedFilename = (char*)malloc(iLen);
        snprintf(szQueuedFilename, iLen, "%s|%s", pDestNZBInfo->GetQueuedFilename(), pSrcNZBInfo->GetQueuedFilename());
        szQueuedFilename[iLen - 1] = '\0';
        pDestNZBInfo->SetQueuedFilename(szQueuedFilename);
        free(szQueuedFilename);

        return true;
}

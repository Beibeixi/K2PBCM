/*
 *  This file is part of nzbget
 *
 *  Copyright (C) 2007-2009 Andrei Prygounkov <hugbug@users.sourceforge.net>
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
 * $Date: 2011/03/21 01:46:14 $
 *                                                          
 */


#ifndef DISKSTATE_H
#define DISKSTATE_H

#include "DownloadInfo.h"

class DiskState
{
private:
	int					ParseFormatVersion(const char* szFormatSignature);
	bool				SaveFileInfo(FileInfo* pFileInfo, const char* szFilename);
	bool				LoadFileInfo(FileInfo* pFileInfo, const char* szFilename, bool bFileSummary, bool bArticles);
	void				SaveNZBList(DownloadQueue* pDownloadQueue, FILE* outfile);
	bool				LoadNZBList(DownloadQueue* pDownloadQueue, FILE* infile, int iFormatVersion);
	void				SaveFileQueue(DownloadQueue* pDownloadQueue, FileQueue* pFileQueue, FILE* outfile);
	bool				LoadFileQueue(DownloadQueue* pDownloadQueue, FileQueue* pFileQueue, FILE* infile, int iFormatVersion);
	void				SavePostQueue(DownloadQueue* pDownloadQueue, FILE* outfile);
	bool				LoadPostQueue(DownloadQueue* pDownloadQueue, FILE* infile);
	bool				LoadOldPostQueue(DownloadQueue* pDownloadQueue);
	void				SaveHistory(DownloadQueue* pDownloadQueue, FILE* outfile);
	bool				LoadHistory(DownloadQueue* pDownloadQueue, FILE* infile);
	int					FindNZBInfoIndex(DownloadQueue* pDownloadQueue, NZBInfo* pNZBInfo);

public:
	bool				DownloadQueueExists();
	bool				PostQueueExists(bool bCompleted);
	bool				SaveDownloadQueue(DownloadQueue* pDownloadQueue);
	bool				LoadDownloadQueue(DownloadQueue* pDownloadQueue);
	bool				SaveFile(FileInfo* pFileInfo);
	bool				LoadArticles(FileInfo* pFileInfo);
	bool				DiscardDownloadQueue();
	bool				DiscardFile(FileInfo* pFileInfo);
	void				CleanupTempDir(DownloadQueue* pDownloadQueue);
};

#endif

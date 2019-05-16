/*
 *  This file is part of nzbget
 *
 *  Copyright (C) 2005 Bo Cordes Petersen <placebodk@users.sourceforge.net>
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
 * $Date: 2011/03/21 01:46:15 $
 *
 */


#ifndef REMOTECLIENT_H
#define REMOTECLIENT_H

#include "Options.h"
#include "MessageBase.h"
#include "Connection.h"
#include "DownloadInfo.h"

class RemoteClient
{
private:
	Connection* 	m_pConnection;
	NetAddress*		m_pNetAddress;
	bool			m_bVerbose;

	bool			InitConnection();
	void			InitMessageBase(SNZBRequestBase* pMessageBase, int iRequest, int iSize);
	bool			ReceiveBoolResponse();
	void			printf(const char* msg, ...);
	void			perror(const char* msg);

public:
					RemoteClient();
					~RemoteClient();
	void			SetVerbose(bool bVerbose) { m_bVerbose = bVerbose; };
	bool 			RequestServerDownload(const char* szFilename, const char* szCategory, bool bAddFirst);
	bool			RequestServerList(bool bFiles, bool bGroups);
	bool			RequestServerPauseUnpause(bool bPause, eRemotePauseUnpauseAction iAction);
	bool			RequestServerSetDownloadRate(float fRate);
	bool			RequestServerDumpDebug();
	bool 			RequestServerEditQueue(eRemoteEditAction iAction, int iOffset, const char* szText, int* pIDList, int iIDCount, bool bSmartOrder);
	bool			RequestServerLog(int iLines);
	bool			RequestServerShutdown();
	bool			RequestServerVersion();
	bool			RequestPostQueue();
	bool 			RequestWriteLog(int iKind, const char* szText);
	bool			RequestScan();
	bool			RequestHistory();
	void			BuildFileList(SNZBListResponse* pListResponse, const char* pTrailingData, DownloadQueue* pDownloadQueue);
};

#endif

/*
 *  This file is part of nzbget
 *
 *  Copyright (C) 2004  Sven Henkel <sidddy@users.sourceforge.net>
 *  Copyright (C) 2007  Andrei Prygounkov <hugbug@users.sourceforge.net>
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


#ifndef NZBFILE_H
#define NZBFILE_H

#include <vector>

#include "DownloadInfo.h"

class NZBFile
{
public:
	typedef std::vector<FileInfo*>	FileInfos;

private:
	FileInfos			m_FileInfos;
	NZBInfo*			m_pNZBInfo;
	char*				m_szFileName;

						NZBFile(const char* szFileName, const char* szCategory);
	void				AddArticle(FileInfo* pFileInfo, ArticleInfo* pArticleInfo);
	void				AddFileInfo(FileInfo* pFileInfo);
	void				ParseSubject(FileInfo* pFileInfo);
	void				CheckFilenames();
#ifdef WIN32
    bool 				ParseNZB(IUnknown* nzb);
	static void			EncodeURL(const char* szFilename, char* szURL);
#else
    bool 				ParseNZB(void* nzb);
#endif
	static NZBFile*		Create(const char* szFileName, const char* szCategory, const char* szBuffer, int iSize, bool bFromBuffer);

public:
	virtual 			~NZBFile();
	static NZBFile*		CreateFromBuffer(const char* szFileName, const char* szCategory, const char* szBuffer, int iSize);
	static NZBFile*		CreateFromFile(const char* szFileName, const char* szCategory);
	const char* 		GetFileName() const { return m_szFileName; }
	FileInfos*			GetFileInfos() { return &m_FileInfos; }
	NZBInfo*			GetNZBInfo() { return m_pNZBInfo; }
	void				DetachFileInfos();

	void				LogDebugInfo();
};

#endif

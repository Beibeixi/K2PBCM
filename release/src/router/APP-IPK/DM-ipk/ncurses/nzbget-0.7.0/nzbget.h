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
 * $Date: 2011/03/21 01:46:15 $
 *
 */


#ifndef NZBGET_H
#define NZBGET_H

#ifdef WIN32

// WIN32

#define snprintf _snprintf
#ifndef strdup
#define strdup _strdup
#endif
#define fdopen _fdopen
#define ctime_r(timep, buf, bufsize) ctime_s(buf, bufsize, timep)
#define localtime_r(time, tm) localtime_s(tm, time)
#define int32_t __int32
#define mkdir(dir, flags) _mkdir(dir)
#define rmdir _rmdir
#define strcasecmp(a, b) _stricmp(a, b)
#define strncasecmp(a, b, c) _strnicmp(a, b, c)
#define ssize_t SSIZE_T
#define	__S_ISTYPE(mode, mask)	(((mode) & _S_IFMT) == (mask))
#define	S_ISDIR(mode)	 __S_ISTYPE((mode), _S_IFDIR)
#define	S_ISREG(mode)	 __S_ISTYPE((mode), _S_IFREG)
#define	S_DIRMODE NULL
#define usleep(usec) Sleep((usec) / 1000)
#define gettimeofday(tm, ignore) _ftime(tm)
#define socklen_t int
#define SHUT_RDWR 0x02
#define PATH_SEPARATOR '\\'
#define ALT_PATH_SEPARATOR '/'
#define LINE_ENDING "\r\n"
#define pid_t int

#pragma warning(disable:4800) // 'type' : forcing value to bool 'true' or 'false' (performance warning)
#pragma warning(disable:4267) // 'var' : conversion from 'size_t' to 'type', possible loss of data

#else

// POSIX

#define closesocket(sock) close(sock)
#define SOCKET int
#define INVALID_SOCKET (-1)
#define PATH_SEPARATOR '/'
#define ALT_PATH_SEPARATOR '\\'
#define MAX_PATH 1024
#define S_DIRMODE (S_IRWXU | S_IRWXG | S_IRWXO)
#define LINE_ENDING "\n"

#endif

#ifndef SHUT_RDWR
#define SHUT_RDWR 2
#endif

#endif

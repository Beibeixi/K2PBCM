/*
 *  This file if part of nzbget
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WIN32
#include "win32.h"
#endif

#include "Observer.h"
#include "Log.h"

Subject::Subject()
{
	m_Observers.clear();
}

void Subject::Attach(Observer* Observer)
{
	m_Observers.push_back(Observer);
}

void Subject::Detach(Observer* Observer)
{
	m_Observers.remove(Observer);
}

void Subject::Notify(void* Aspect)
{
	debug("Notifying observers");
	
	for (std::list<Observer*>::iterator it = m_Observers.begin(); it != m_Observers.end(); it++)
	{
        Observer* Observer = *it;
		Observer->Update(this, Aspect);
	}
}

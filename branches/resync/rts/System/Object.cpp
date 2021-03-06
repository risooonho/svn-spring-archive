#include "StdAfx.h"
// Object.cpp: implementation of the CObject class.
//
//////////////////////////////////////////////////////////////////////

#include "Object.h"
#include "mmgr.h"

CR_BIND(CObject, )

CR_REG_METADATA(CObject, (
	CR_MEMBER(listening),
	CR_MEMBER(listeners)));

CObject* CObject::syncedObjects;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CObject::CObject(EObjectType synced) : prev(NULL), next(NULL)
{
	/*if (synced == OT_Synced)*/ {
		// Push a synchronized object on front of the list.
		next = syncedObjects;
		syncedObjects = this;
		if (next)
			next->prev = this;
	}
}

CObject::~CObject()
{
	std::multiset<CObject*>::iterator di;
	for(di=listeners.begin();di!=listeners.end();++di){
		(*di)->DependentDied(this);
		(*di)->listening.erase(this);
	}
	for(di=listening.begin();di!=listening.end();++di){
		(*di)->listeners.erase(this);
	}

	// remove from list of synced objects
	if (IsSynchronized()) {
		if (prev)
			prev->next = next;
		else
			syncedObjects = next;
		if (next)
			next->prev = prev;
	}
}

void CObject::DependentDied(CObject* o)
{
//	MessageBox(0,"wrong","ag",0);
}

void CObject::AddDeathDependence(CObject *o)
{
	o->listeners.insert(this);
	listening.insert(o);
}

void CObject::DeleteDeathDependence(CObject *o)
{
	//note that we can be listening to a single object from several different places (like curreclaim in CBuilder and lastAttacker in CUnit, grr) so we should only remove one of them
	std::multiset<CObject*>::iterator oi=listening.find(o);
	if(oi!=listening.end())
		listening.erase(oi);
	std::multiset<CObject*>::iterator oi2=o->listeners.find(this);
	if(oi2!=o->listeners.end())
		o->listeners.erase(oi2);
}

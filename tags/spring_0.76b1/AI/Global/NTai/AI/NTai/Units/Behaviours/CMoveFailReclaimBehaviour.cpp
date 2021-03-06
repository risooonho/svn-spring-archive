#include "../../Core/include.h"

CMoveFailReclaimBehaviour::CMoveFailReclaimBehaviour(Global* GL, int uid){
	//
	G = GL;
	unit = G->GetUnit(uid);
	this->uid = ((CUnit*)unit.get())->GetID();
}

CMoveFailReclaimBehaviour::~CMoveFailReclaimBehaviour(){
	//
}

bool CMoveFailReclaimBehaviour::Init(){
	return true;
}

void CMoveFailReclaimBehaviour::RecieveMessage(CMessage &message){
	if(message.GetType() == string("unitmovefailed")){
		if(message.GetParameter(0) == uid){
			int* f = new int[2];
			int i = G->cb->GetFeatures(f,1,G->GetUnitPos(uid),100.0f);
			delete [] f;
			if(i > 0){
				G->Actions->AreaReclaim(uid,G->GetUnitPos(uid),100);
			}
		}
	}else if(message.GetType() == string("unitdestroyed")){
		if(message.GetParameter(0)== uid){
			End();
		}
	}
}

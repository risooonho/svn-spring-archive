#include "StdAfx.h"
#include "MobileCAI.h"
#include "LineDrawer.h"
#include "ExternalAI/Group.h"
#include "Game/GameHelper.h"
#include "Game/SelectedUnits.h"
#include "Game/Team.h"
#include "Game/UI/CommandColors.h"
#include "Game/UI/CursorIcons.h"
#include "Map/Ground.h"
#include "Rendering/GL/myGL.h"
#include "Sim/MoveTypes/MoveType.h"
#include "Sim/MoveTypes/TAAirMoveType.h"
#include "Sim/Units/UnitDef.h"
#include "Sim/Units/Unit.h"
#include "Sim/Units/UnitHandler.h"
#include "Sim/Weapons/Weapon.h"
#include "System/LogOutput.h"
#include "myMath.h"
#include "mmgr.h"

CMobileCAI::CMobileCAI(CUnit* owner)
: CCommandAI(owner),
//	patrolTime(0),
	goalPos(-1,-1,-1),
	tempOrder(false),
	lastBuggerOffTime(-200),
	buggerOffPos(0,0,0),
	buggerOffRadius(0),
	maxWantedSpeed(0),
	lastIdleCheck(0),
	commandPos1(ZeroVector),
	commandPos2(ZeroVector),
	lastPC(-1),
	cancelDistance(1024),
	slowGuard(false)
{
	lastUserGoal=owner->pos;
	CommandDescription c;

	if (owner->unitDef->canmove) {
		c.id=CMD_MOVE;
		c.action="move";
		c.type=CMDTYPE_ICON_FRONT;
		c.name="Move";
		c.hotkey="m";
		c.tooltip="Move: Order the unit to move to a position";
		c.params.push_back("1000000"); // max distance
		possibleCommands.push_back(c);
		c.params.clear();
	}

	if(owner->unitDef->canPatrol){
		c.id=CMD_PATROL;
		c.action="patrol";
		c.type=CMDTYPE_ICON_MAP;
		c.name="Patrol";
		c.hotkey="p";
		c.tooltip="Patrol: Order the unit to patrol to one or more waypoints";
		possibleCommands.push_back(c);
		c.params.clear();
	}

	if (owner->unitDef->canFight) {
		c.id = CMD_FIGHT;
		c.action="fight";
		c.type = CMDTYPE_ICON_MAP;
		c.name = "Fight";
		c.hotkey = "f";
		c.tooltip = "Fight: Order the unit to take action while moving to a position";
		possibleCommands.push_back(c);
	}

	if(owner->unitDef->canGuard){
		c.id=CMD_GUARD;
		c.action="guard";
		c.type=CMDTYPE_ICON_UNIT;
		c.name="Guard";
		c.hotkey="g";
		c.tooltip="Guard: Order a unit to guard another unit and attack units attacking it";
		possibleCommands.push_back(c);
	}

	if(owner->unitDef->canfly){
		c.params.clear();
		c.id=CMD_AUTOREPAIRLEVEL;
		c.action="autorepairlevel";
		c.type=CMDTYPE_ICON_MODE;
		c.name="Repair level";
		c.params.push_back("1");
		c.params.push_back("LandAt 0");
		c.params.push_back("LandAt 30");
		c.params.push_back("LandAt 50");
		c.tooltip="Repair level: Sets at which health level an aircraft will try to find a repair pad";
		c.hotkey="";
		possibleCommands.push_back(c);
		nonQueingCommands.insert(CMD_AUTOREPAIRLEVEL);
	}

	nonQueingCommands.insert(CMD_SET_WANTED_MAX_SPEED);
}


CMobileCAI::~CMobileCAI()
{

}

void CMobileCAI::GiveCommand(const Command &c)
{
	if (!AllowedCommand(c))
		return;

	if(owner->unitDef->canfly && c.id==CMD_AUTOREPAIRLEVEL){
		if(!dynamic_cast<CTAAirMoveType*>(owner->moveType))
			return;
		if(c.params.empty())
			return;
		switch((int)c.params[0]){
		case 0:
			((CTAAirMoveType*)owner->moveType)->repairBelowHealth=0;
			break;
		case 1:
			((CTAAirMoveType*)owner->moveType)->repairBelowHealth=0.3f;
			break;
		case 2:
			((CTAAirMoveType*)owner->moveType)->repairBelowHealth=0.5f;
			break;
		}
		for(vector<CommandDescription>::iterator cdi=possibleCommands.begin();cdi!=possibleCommands.end();++cdi){
			if(cdi->id==CMD_AUTOREPAIRLEVEL){
				char t[10];
				SNPRINTF(t,10,"%d", (int)c.params[0]);
				cdi->params[0]=t;
				break;
			}
		}
		selectedUnits.PossibleCommandChange(owner);
		return;
	}

	if(!(c.options & SHIFT_KEY) && nonQueingCommands.find(c.id)==nonQueingCommands.end()){
		tempOrder=false;
		StopSlowGuard();
	}
	CCommandAI::GiveAllowedCommand(c);
}

void CMobileCAI::SlowUpdate()
{
	if(owner->unitDef->maxFuel>0 && dynamic_cast<CTAAirMoveType*>(owner->moveType)){
		CTAAirMoveType* myPlane=(CTAAirMoveType*)owner->moveType;
		if(myPlane->reservedPad){
			return;
		} else {
			if(owner->currentFuel <= 0){
				StopMove();
				owner->userAttackGround=false;
				owner->userTarget=0;
				inCommand=false;
				CAirBaseHandler::LandingPad* lp=airBaseHandler->FindAirBase(owner,8000,owner->unitDef->minAirBasePower);
				if(lp){
					myPlane->AddDeathDependence(lp);
					myPlane->reservedPad=lp;
					myPlane->padStatus=0;
					myPlane->oldGoalPos=myPlane->goalPos;
					return;
				}
				float3 landingPos = airBaseHandler->FindClosestAirBasePos(owner,8000,owner->unitDef->minAirBasePower);
				if(landingPos != ZeroVector && owner->pos.distance2D(landingPos) > 300){
					inCommand=false;
					StopMove();
					SetGoal(landingPos,owner->pos);
				} else {
					if(myPlane->aircraftState == CTAAirMoveType::AIRCRAFT_FLYING)
						myPlane->SetState(CTAAirMoveType::AIRCRAFT_LANDING);	
				}
				return;
			}
			if(owner->currentFuel < myPlane->repairBelowHealth*owner->unitDef->maxFuel){
				if(commandQue.empty() || commandQue.front().id==CMD_PATROL){
					CAirBaseHandler::LandingPad* lp=airBaseHandler->FindAirBase(owner,8000,owner->unitDef->minAirBasePower);
					if(lp){
						StopMove();
						owner->userAttackGround=false;
						owner->userTarget=0;
						inCommand=false;
						myPlane->AddDeathDependence(lp);
						myPlane->reservedPad=lp;
						myPlane->padStatus=0;
						myPlane->oldGoalPos=myPlane->goalPos;
						if(myPlane->aircraftState==CTAAirMoveType::AIRCRAFT_LANDED){
							myPlane->SetState(CTAAirMoveType::AIRCRAFT_TAKEOFF);
						}
						return;
					}
				}
			}
		}
	}

	if(!commandQue.empty() && commandQue.front().timeOut < gs->frameNum){
		StopMove();
		FinishCommand();
		return;
	}

	if(commandQue.empty()) {
//		if(!owner->ai || owner->ai->State() != CHasState::Active) {
			IdleCheck();
//		}
		//the attack order could terminate directly and thus cause a loop
		if(commandQue.empty() || commandQue.front().id == CMD_ATTACK) {
			return;
		}
	}

	const float3& curPos=owner->pos;
	
	// treat any following CMD_SET_WANTED_MAX_SPEED commands as options
	// to the current command  (and ignore them when it's their turn
	if (commandQue.size() >= 2 && !slowGuard) {
		deque<Command>::iterator it = commandQue.begin();
		it++;
		const Command& c = *it;
		if ((c.id == CMD_SET_WANTED_MAX_SPEED) && (c.params.size() >= 1)) {
			const float defMaxSpeed = owner->maxSpeed;
			const float newMaxSpeed = min(c.params[0], defMaxSpeed);
			owner->moveType->SetMaxSpeed(newMaxSpeed);
		}
	}

	Execute();
}

/**
* @brief Executes the first command in the commandQue
*/
void CMobileCAI::Execute()
{
	Command& c=commandQue.front();
	switch(c.id){
		case CMD_SET_WANTED_MAX_SPEED: { ExecuteSetWantedMaxSpeed(c); return; }
		case CMD_MOVE:                 { ExecuteMove(c);              return; }
		case CMD_PATROL:               { ExecutePatrol(c);            return; }
		case CMD_FIGHT:                { ExecuteFight(c);             return; }
		case CMD_GUARD:                { ExecuteGuard(c);             return; }
		default:{
			CCommandAI::SlowUpdate();
			return;
		}
	}
}

/**
* @brief executes the set wanted max speed command
*/
void CMobileCAI::ExecuteSetWantedMaxSpeed(Command &c)
{
	if (repeatOrders && (commandQue.size() >= 2) &&
			(commandQue.back().id != CMD_SET_WANTED_MAX_SPEED)) {
		commandQue.push_back(commandQue.front());
	}
	commandQue.pop_front();
	SlowUpdate();
	return;
}

/**
* @brief executes the move command
*/
void CMobileCAI::ExecuteMove(Command &c)
{
	float3 pos(c.params[0], c.params[1], c.params[2]);
	if(!(pos == goalPos)){
		SetGoal(pos, owner->pos);
	}
	if((owner->pos - goalPos).SqLength2D() < cancelDistance ||
			owner->moveType->progressState == CMoveType::Failed){
		FinishCommand();
	}
	return;
}

/**
* @brief Executes the Patrol command c
*/
void CMobileCAI::ExecutePatrol(Command &c)
{
	assert(owner->unitDef->canPatrol);
	assert(c.params.size() >= 3);
	Command temp;
	temp.id = CMD_FIGHT;
	temp.params.push_back(c.params[0]);
	temp.params.push_back(c.params[1]);
	temp.params.push_back(c.params[2]);
	temp.options = c.options | INTERNAL_ORDER;
	commandQue.push_back(c);
	commandQue.pop_front();
	commandQue.push_front(temp);
	if(owner->group){
		owner->group->CommandFinished(owner->id, CMD_PATROL);
	}
	ExecuteFight(temp);
	return;
}

/**
* @brief Executes the Fight command c
*/
void CMobileCAI::ExecuteFight(Command &c)
{
	assert((c.options & INTERNAL_ORDER) || owner->unitDef->canFight);
	if(tempOrder){
		inCommand = true;
		tempOrder = false;
	}
	if(c.params.size()<3){		//this shouldnt happen but anyway ...
		logOutput.Print("Error: got fight cmd with less than 3 params on %s in mobilecai",
			owner->unitDef->humanName.c_str());
		return;
	}
	if(c.params.size() >= 6){
		if(!inCommand){
			commandPos1 = float3(c.params[3],c.params[4],c.params[5]);
		}
	} else {
		// Some hackery to make sure the line (commandPos1,commandPos2) is NOT
		// rotated (only shortened) if we reach this because the previous return
		// fight command finished by the 'if((curPos-pos).SqLength2D()<(64*64)){'
		// condition, but is actually updated correctly if you click somewhere
		// outside the area close to the line (for a new command).
		commandPos1 = ClosestPointOnLine(commandPos1, commandPos2, owner->pos);
		if ((owner->pos - commandPos1).SqLength2D() > (96 * 96)) {
			commandPos1 = owner->pos;
		}
	}
	float3 pos(c.params[0],c.params[1],c.params[2]);
	if(!inCommand){
		inCommand = true;
		commandPos2 = pos;
	}
	if(c.params.size() >= 6){
		pos = ClosestPointOnLine(commandPos1, commandPos2, pos);
	}
	if(pos!=goalPos){
		SetGoal(pos, owner->pos);
	}

	if(owner->unitDef->canAttack && owner->fireState==2){
		float3 curPosOnLine = ClosestPointOnLine(commandPos1, commandPos2, owner->pos);
		CUnit* enemy=helper->GetClosestEnemyUnit(
			curPosOnLine, owner->maxRange + 100 * owner->moveState * owner->moveState,
			owner->allyteam);
		if(enemy && (owner->hasUWWeapons || !enemy->isUnderWater)
				&& !(owner->unitDef->noChaseCategory & enemy->category)
				&& !owner->weapons.empty()){
			Command c2;
			c2.id=CMD_ATTACK;
			c2.options=c.options|INTERNAL_ORDER;
			c2.params.push_back(enemy->id);
			PushOrUpdateReturnFight();
			commandQue.push_front(c2);
			inCommand=false;
			tempOrder=true;
			if(lastPC!=gs->frameNum){	//avoid infinite loops
				lastPC=gs->frameNum;
				SlowUpdate();
			}
			return;
		}
	}
	if((owner->pos - goalPos).SqLength2D() < (64 * 64)
			|| (owner->moveType->progressState == CMoveType::Failed)){
		FinishCommand();
	}
	return;
}

/**
* @brief Executes the guard command c
*/
void CMobileCAI::ExecuteGuard(Command &c)
{
	assert(owner->unitDef->canGuard);
	assert(!c.params.empty());
	if(int(c.params[0]) >= 0 && uh->units[int(c.params[0])] != NULL
			&& UpdateTargetLostTimer(int(c.params[0]))){
		CUnit* guarded = uh->units[int(c.params[0])];
		if(owner->unitDef->canAttack && guarded->lastAttacker
				&& guarded->lastAttack + 40 < gs->frameNum
				&& (owner->hasUWWeapons || !guarded->lastAttacker->isUnderWater)){
			StopSlowGuard();
			Command nc;
			nc.id=CMD_ATTACK;
			nc.params.push_back(guarded->lastAttacker->id);
			nc.options = c.options;
			commandQue.push_front(nc);
			SlowUpdate();
			return;
		} else {
			//float3 dif = guarded->speed * guarded->frontdir;
			float3 dif = guarded->pos - owner->pos;
			dif.Normalize();
			float3 goal = guarded->pos - dif * (guarded->radius + owner->radius + 64);
			if((goalPos - goal).SqLength2D() > 1600
					|| (goalPos - owner->pos).SqLength2D()
						< (owner->maxSpeed*30 + 1 + SQUARE_SIZE*2)
						 * (owner->maxSpeed*30 + 1 + SQUARE_SIZE*2)){
				SetGoal(goal, owner->pos);
			}
			if((goal - owner->pos).SqLength2D() < 6400) {
				StartSlowGuard(guarded->maxSpeed);
				if((goal-owner->pos).SqLength2D() < 1800){
					StopMove();
					NonMoving(); 
				}
			} else {
				StopSlowGuard();
			}
		}
	} else {
		FinishCommand();
	}
	return;
}

/**
* @brief Executes the stop command c
*/
void CMobileCAI::ExecuteStop(Command &c)
{
	StopMove();
	return CCommandAI::ExecuteStop(c);
}

/**
* @brief Executes the DGun command c
*/
void CMobileCAI::ExecuteDGun(Command &c)
{
	if(uh->limitDgun && owner->unitDef->isCommander
			&& owner->pos.distance(gs->Team(owner->team)->startPos)>uh->dgunRadius){
		StopMove();
		return FinishCommand();
	}
	ExecuteAttack(c);
}


/**
* @brief Causes this CMobileCAI to execute the attack order c
*/
void CMobileCAI::ExecuteAttack(Command &c)
{
	assert(owner->unitDef->canAttack);
	if(tempOrder && owner->moveState < 2){		//limit how far away we fly
		if(orderTarget && LinePointDist(commandPos1,commandPos2,orderTarget->pos) > 500 + owner->maxRange){
			StopMove();
			FinishCommand();
			return;
		}
	}
	if(!inCommand){
		if(c.params.size()==1){
			if(uh->units[int(c.params[0])] != 0 && uh->units[int(c.params[0])] != owner){
				float3 fix=uh->units[int(c.params[0])]->pos+owner->posErrorVector*128;
				SetGoal(fix, owner->pos);
				orderTarget=uh->units[int(c.params[0])];
				AddDeathDependence(orderTarget);
				inCommand=true;
			} else {
				StopMove();		//cancel keeppointingto
				FinishCommand();
				return;
			}
		} else {
			float3 pos(c.params[0],c.params[1],c.params[2]);
			SetGoal(pos, owner->pos);
			inCommand=true;
		}
	} else if ((c.params.size() == 3) && (owner->commandShotCount > 0) && (commandQue.size() > 1)) {
		// the trailing CMD_SET_WANTED_MAX_SPEED in a command pair does not count
		if ((commandQue.size() != 2) || (commandQue.back().id != CMD_SET_WANTED_MAX_SPEED)) {
			StopMove();
			FinishCommand();
			return;
		}
	}

	if(targetDied || (c.params.size() == 1 && UpdateTargetLostTimer(int(c.params[0])) == 0)){
		StopMove();		//cancel keeppointingto
		FinishCommand();
		return;
	}
	if(orderTarget){
		//note that we handle aircrafts slightly differently
		if((((owner->AttackUnit(orderTarget, c.id==CMD_DGUN)
				&& owner->weapons.size() > 0 
				&& owner->weapons.front()->range -
					owner->weapons.front()->relWeaponPos.Length() >
					orderTarget->pos.distance(owner->pos))
				|| dynamic_cast<CTAAirMoveType*>(owner->moveType))
				&& (owner->pos-orderTarget->pos).Length2D() <
					owner->maxRange*0.9f)
				|| (owner->pos-orderTarget->pos).SqLength2D()<1024){
			StopMove();
			owner->moveType->KeepPointingTo(orderTarget,
				min((float)(owner->losRadius*SQUARE_SIZE*2),
				owner->maxRange*0.9f), true);
		} else if((orderTarget->pos+owner->posErrorVector*128).distance2D(goalPos) > 10+orderTarget->pos.distance2D(owner->pos)*0.2f){
			float3 fix=orderTarget->pos+owner->posErrorVector*128;
			SetGoal(fix, owner->pos);
		}
	} else {
		float3 pos(c.params[0],c.params[1],c.params[2]);
		if((owner->AttackGround(pos,c.id==CMD_DGUN) && owner->weapons.size() > 0
				&& (owner->pos-pos).Length() < 
					owner->weapons.front()->range -
					owner->weapons.front()->relWeaponPos.Length())
				|| (owner->pos-pos).SqLength2D()<1024){
			StopMove();
			owner->moveType->KeepPointingTo(pos, owner->maxRange*0.9f, true);
		} else if(pos.distance2D(goalPos)>10){
			SetGoal(pos, owner->pos);
		}
	}
}

int CMobileCAI::GetDefaultCmd(CUnit* pointed, CFeature* feature)
{
	if (pointed) {
		if (!gs->Ally(gu->myAllyTeam,pointed->allyteam)) {
			if (owner->unitDef->canAttack) {
				return CMD_ATTACK;
			}
		} else {
			if (owner->unitDef->canGuard) {
				return CMD_GUARD;
			}
		}
	}
	return CMD_MOVE;
}

void CMobileCAI::SetGoal(const float3 &pos, const float3& curPos, float goalRadius)
{
	if(pos == goalPos)
		return;
	goalPos = pos;
	owner->moveType->StartMoving(pos, goalRadius);
}

void CMobileCAI::StopMove()
{
	owner->moveType->StopMoving();
	goalPos=owner->pos;
}

void CMobileCAI::DrawCommands(void)
{
	lineDrawer.StartPath(owner->midPos, cmdColors.start);

	if (owner->selfDCountdown != 0) {
		lineDrawer.DrawIconAtLastPos(CMD_SELFD);
	}

	deque<Command>::iterator ci;
	for(ci=commandQue.begin();ci!=commandQue.end();++ci){
		bool draw=false;
		switch(ci->id){
			case CMD_MOVE:{
				const float3 endPos(ci->params[0],ci->params[1],ci->params[2]);
				lineDrawer.DrawLineAndIcon(ci->id, endPos, cmdColors.move);
				break;
			}
			case CMD_FIGHT:{
				const float3 endPos(ci->params[0],ci->params[1],ci->params[2]);
				lineDrawer.DrawLineAndIcon(ci->id, endPos, cmdColors.fight);
				break;
			}
			case CMD_PATROL:{
				const float3 endPos(ci->params[0],ci->params[1],ci->params[2]);
				lineDrawer.DrawLineAndIcon(ci->id, endPos, cmdColors.patrol);
				break;
			}
			case CMD_ATTACK:
			case CMD_DGUN:{
				if(ci->params.size()==1){
					const CUnit* unit = uh->units[int(ci->params[0])];
					if((unit != NULL) && isTrackable(unit)) {
						const float3 endPos =
							helper->GetUnitErrorPos(unit, owner->allyteam);
						lineDrawer.DrawLineAndIcon(ci->id, endPos, cmdColors.attack);
					}
				} else {
					const float3 endPos(ci->params[0],ci->params[1],ci->params[2]);
					lineDrawer.DrawLineAndIcon(ci->id, endPos, cmdColors.attack);
				}
				break;
			}
			case CMD_GUARD:{
				const CUnit* unit = uh->units[int(ci->params[0])];
				if((unit != NULL) && isTrackable(unit)) {
					const float3 endPos =
						helper->GetUnitErrorPos(unit, owner->allyteam);
					lineDrawer.DrawLineAndIcon(ci->id, endPos, cmdColors.guard);
				}
				break;
			}
			case CMD_WAIT:{
				DrawWaitIcon(*ci);
				break;
			}
			case CMD_SELFD:{
				lineDrawer.DrawIconAtLastPos(ci->id);
				break;
			}
		}
	}
	lineDrawer.FinishPath();
}


void CMobileCAI::BuggerOff(float3 pos, float radius)
{
	lastBuggerOffTime=gs->frameNum;
	buggerOffPos=pos;
	buggerOffRadius=radius+owner->radius;
}

void CMobileCAI::NonMoving(void)
{
	if(lastBuggerOffTime>gs->frameNum-200){
		float3 dif=owner->pos-buggerOffPos;
		dif.y=0;
		float length=dif.Length();
		if(!length) {
			length=0.1f;
			dif = float3(0.1f, 0.0f, 0.0f);
		}
		if(length<buggerOffRadius){
			float3 goalPos = buggerOffPos + dif * ((buggerOffRadius + 128) / length);
			Command c;
			c.id = CMD_MOVE;
			c.options = 0;//INTERNAL_ORDER;
			c.params.push_back(goalPos.x);
			c.params.push_back(goalPos.y);
			c.params.push_back(goalPos.z);
			c.timeOut = gs->frameNum + 40;
			commandQue.push_front(c);
			unimportantMove = true;
		}
	}
}

void CMobileCAI::FinishCommand(void)
{
	if(!(commandQue.front().options & INTERNAL_ORDER)){
		lastUserGoal=owner->pos;
//		logOutput.Print("Reseting user goal");
	}
	StopSlowGuard();
	CCommandAI::FinishCommand();
}

void CMobileCAI::IdleCheck(void)
{
	if(owner->unitDef->canAttack && owner->moveState && owner->fireState && !owner->weapons.empty() && (!owner->haveTarget || owner->weapons[0]->onlyForward)){
		if(owner->lastAttacker && owner->lastAttack+100>gs->frameNum && !(owner->unitDef->noChaseCategory & owner->lastAttacker->category)){
			float3 apos=owner->lastAttacker->pos;
			float dist=apos.distance2D(owner->pos);
			if(dist<owner->maxRange+200*owner->moveState*owner->moveState){
				Command c;
				c.id=CMD_ATTACK;
				c.options=INTERNAL_ORDER;
				c.params.push_back(owner->lastAttacker->id);
				c.timeOut=gs->frameNum+140;
				commandQue.push_front(c);
				return;
			}
		}
	}
	if(owner->unitDef->canAttack && (gs->frameNum!=lastIdleCheck+16) && owner->moveState && owner->fireState==2 && !owner->weapons.empty() && (!owner->haveTarget || owner->weapons[0]->onlyForward)){
		if(!owner->unitDef->noAutoFire){
			CUnit* u=helper->GetClosestEnemyUnit(owner->pos,owner->maxRange+150*owner->moveState*owner->moveState,owner->allyteam);
			if(u && !(owner->unitDef->noChaseCategory & u->category)){
				Command c;
				c.id=CMD_ATTACK;
				c.options=INTERNAL_ORDER;
				c.params.push_back(u->id);
				c.timeOut=gs->frameNum+140;
				commandQue.push_front(c);
				return;
			}
		}
	}
	lastIdleCheck=gs->frameNum;
	if((owner->pos-lastUserGoal).SqLength2D()>10000 && !owner->haveTarget && !dynamic_cast<CTAAirMoveType*>(owner->moveType)){
		Command c;
		c.id=CMD_MOVE;
		c.options=0;		//note that this is not internal order so that we dont keep generating new orders if we cant get to that pos
		c.params.push_back(lastUserGoal.x);
		c.params.push_back(lastUserGoal.y);
		c.params.push_back(lastUserGoal.z);
		commandQue.push_front(c);
		unimportantMove=true;
	} else {
		NonMoving();
	}
}

void CMobileCAI::StopSlowGuard(){
	if(slowGuard){
		slowGuard = false;
		owner->moveType->SetMaxSpeed(owner->maxSpeed);
	}
}

void CMobileCAI::StartSlowGuard(float speed){
	if(!slowGuard){
		slowGuard = true;
		//speed /= 30;
		if(owner->maxSpeed >= speed){
			speed;
			if(!commandQue.empty()){
				Command currCommand = commandQue.front();
				if(commandQue.size() <= 1
						|| commandQue[1].id != CMD_SET_WANTED_MAX_SPEED
						|| commandQue[1].params[0] > speed){
					owner->moveType->SetMaxSpeed(speed);
				}
			}
		}
	}
}

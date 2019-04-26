#ifndef M018585HFUSM_MANAGER_H
#define M018585HFUSM_MANAGER_H

#include "m018585hCommons.h"
#include "FuSM_States/m018585hFuSM_Base.h"
#include "../../PickUpManager.h"
#include <vector>

//FuSM states
#include "FuSM_States/m018585hFuSM_Base.h"
#include "FuSM_States/m018585hFuSM_Attack.h"
#include "FuSM_States/m018585hFuSM_Gather.h"
#include "FuSM_States/m018585hFuSM_Search.h"
#include "FuSM_States/m018585hFuSM_Run.h"
#include "FuSM_States/m018585hFuSM_ObAvoidance.h"

class m018585hTank;

class m018585hFuSM_Manager
{
public:
	m018585hFuSM_Manager();
	~m018585hFuSM_Manager();

	void Update(m018585hTank& myTank, vector<BaseTank*> seenTanks, vector<BaseTank*> heardTanks, float deltaTime);

	m018585hFuSM_Base* GetState(FuSM_STATE stateType);

	void AddState(m018585hFuSM_Base* state);
	void Reset();

private:
	PickUpManager* pickUpManager;
	vector<m018585hFuSM_Base*> mStates;
	vector<m018585hFuSM_Base*> mActivatedStates;
};
#endif
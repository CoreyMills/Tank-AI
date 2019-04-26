#include "m018585hFuSM_Search.h"
#include "../../BaseTank.h"
#include "../m018585hTank.h"
#include "../m018585hSteeringBehaviour.h"

m018585hFuSM_Search::m018585hFuSM_Search() : m018585hFuSM_Base()
{
	stateType = FuSM_SEARCH;
}

m018585hFuSM_Search::~m018585hFuSM_Search()
{
}

float m018585hFuSM_Search::CalculateActivation(m018585hTank & myTank, vector<BaseTank*> seenTanks, vector<BaseTank*> heardTanks)
{
	if (seenTanks.empty() && heardTanks.empty() && 
		!myTank.GetTargetTank())
		mActivationLevels.push_back(1.0f);

	//add level for if no pickups were found

	return m018585hFuSM_Base::CalculateActivation(myTank, seenTanks, heardTanks);
}

void m018585hFuSM_Search::Update(m018585hTank & myTank, float deltaTime)
{
	//multiply by activation level
	//need to reduce rotation amount if wandering
	//need to reduce movement buffer to zero if wandering
	mStateForces.push_back(myTank.GetSteeringBehaviour()->Wander(&myTank, true));

	m018585hFuSM_Base::Update(myTank, deltaTime);
}

void m018585hFuSM_Search::Enter(m018585hSteeringBehaviour& steering)
{
	steering.WanderOn();
}

void m018585hFuSM_Search::Exit(m018585hSteeringBehaviour& steering)
{
	steering.WanderOff();
}
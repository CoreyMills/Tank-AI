#include "m018585hFuSM_Manager.h"
#include "m018585hTank.h"

#include <time.h>

m018585hFuSM_Manager::m018585hFuSM_Manager()
{
	pickUpManager = PickUpManager::Instance();
}

m018585hFuSM_Manager::~m018585hFuSM_Manager()
{
	mStates.clear();
	mActivatedStates.clear();
}

void m018585hFuSM_Manager::Update(m018585hTank& myTank, vector<BaseTank*> seenTanks, vector<BaseTank*> heardTanks, float deltaTime)
{
	if (mStates.size() == 0)
		return;

	vector<m018585hFuSM_Base*> previouslyActiveStates = mActivatedStates;
	mActivatedStates.clear();

	vector<m018585hFuSM_Base*> nonActiveStates;

	for (auto state : mStates)
	{
		if (state->CalculateActivation(myTank, seenTanks, heardTanks) > 0)
		{
			mActivatedStates.push_back(state);
		}
		else
		{
			nonActiveStates.push_back(state);
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	//exit non-active states
	if (previouslyActiveStates.size() != 0 && nonActiveStates.size() != 0)
	{
		for (auto previouslyActiveState : previouslyActiveStates)
		{
			for (auto nonActiveState : nonActiveStates)
			{
				if (previouslyActiveState == nonActiveState)
				{
					nonActiveState->Exit(*myTank.GetSteeringBehaviour());
				}
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	//enter active states
	if (previouslyActiveStates.size() != 0 || mActivatedStates.size() != 0)
	{
		for(auto activeState : mActivatedStates)
		{
			bool callEnterFunc = true;
			for (auto previouslyActiveState : previouslyActiveStates)
			{
				if (previouslyActiveState == activeState)
				{
					callEnterFunc = false;
					break;
				}
			}

			if (callEnterFunc)
			{
				activeState->Enter(*myTank.GetSteeringBehaviour());
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	if (mActivatedStates.size() != 0)
	{
		for (auto state : mActivatedStates)
		{
			state->Update(myTank, deltaTime);
		}
	}
}

m018585hFuSM_Base * m018585hFuSM_Manager::GetState(FuSM_STATE stateType)
{
	for (auto state : mStates)
	{
		if (state->GetStateType() == stateType)
			return state;
	}
	return nullptr;
}

void m018585hFuSM_Manager::AddState(m018585hFuSM_Base * state)
{
	mStates.push_back(state);
}

void m018585hFuSM_Manager::Reset()
{
	//mStates.clear();
	mActivatedStates.clear();
}
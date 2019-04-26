#include "m018585hFuSM_Base.h"
#include "../../BaseTank.h"
#include "../m018585hTank.h"
#include "../m018585hSteeringBehaviour.h"
#include "../../ObstacleManager.h"

m018585hFuSM_Base::m018585hFuSM_Base()
{
	mActivationLevel = 0;
	mTimeWaited = 0;
	mTimeInterval = 2.5f;

	mPath = new vector<Vector2D>;
}

m018585hFuSM_Base::~m018585hFuSM_Base()
{
}

float m018585hFuSM_Base::CalculateActivation(m018585hTank & myTank, vector<BaseTank*> seenTanks, vector<BaseTank*> heardTanks)
{
	mActivationLevel = 0;
	for (auto level : mActivationLevels)
	{
		mActivationLevel += level;
	}
	
	if (mActivationLevel != 0)
	{
		mActivationLevel /= mActivationLevels.size();
	}

	if (mActivationLevel > 1.0f)
	{
		mActivationLevel = 1.0f;
	}
	else if (mActivationLevel < 0.0f)
	{
		mActivationLevel = 0.0f;
	}

	mActivationLevels.clear();
	return mActivationLevel;
}

Vector2D m018585hFuSM_Base::CalculateStateForce()
{
	if (mStateForces.empty())
		return Vector2D();

	Vector2D netForce;
	for (Vector2D force : mStateForces)
	{
		netForce += force;
	}
	netForce *= mActivationLevel;
	mStateForces.clear();
	return netForce;
}

void m018585hFuSM_Base::Update(m018585hTank& myTank, float deltaTime)
{
	myTank.GetSteeringBehaviour()->AddForce(CalculateStateForce());

	if (mPath)   
	{
		mTimeWaited += deltaTime;
		if (mTimeWaited >= mTimeInterval && !mPath->empty())
		{
			mTimeWaited = 0.0f;
			if (!Raycast(myTank.GetCentralPosition(), mPath->at(0), ObstacleManager::Instance()->GetObstacles()))
			{
				cout << "Periodic Clear" << endl;
				mPath->clear();
			}
		}
	}
}

void m018585hFuSM_Base::Enter(m018585hSteeringBehaviour& steering)
{
}

void m018585hFuSM_Base::Exit(m018585hSteeringBehaviour& steering)
{
}

#include "m018585hFuSM_Run.h"
#include "../../BaseTank.h"
#include "../m018585hTank.h"
#include "../m018585hSteeringBehaviour.h"
#include "../../ObstacleManager.h"

m018585hFuSM_Run::m018585hFuSM_Run() : m018585hFuSM_Base()
{
	stateType = FuSM_RUN;
	mTargetTank = nullptr;
	
	mEvading = false;

	dropMine = false;
	mineDropDelay = 8.0f;
	timeSinceDrop = 0.0f;

	mPath = new vector<Vector2D>;
}

m018585hFuSM_Run::~m018585hFuSM_Run()
{
}

float m018585hFuSM_Run::CalculateActivation(m018585hTank & myTank, vector<BaseTank*> seenTanks, vector<BaseTank*> heardTanks)
{
	if (seenTanks.empty() && heardTanks.empty() && !mTargetTank)
		return 0.0f;

	double currentClosestDistSq = MaxDouble;
	float level = 0;

	if (!mTargetTank)
	{
		if (!seenTanks.empty())
		{
			//Tanks that can be seen
			for (BaseTank* enemy : seenTanks)
			{
				double dist = Vec2DDistanceSq(myTank.GetCentralPosition(), enemy->GetCentralPosition());
				if (dist < currentClosestDistSq)
				{
					currentClosestDistSq = dist;
					mTargetTank = enemy;
				}
			}
		}
		else
		{
			//Tanks that can be heard
			for (BaseTank* enemy : heardTanks)
			{
				double dist = Vec2DDistanceSq(myTank.GetCentralPosition(), enemy->GetCentralPosition());
				if (dist < currentClosestDistSq)
				{
					currentClosestDistSq = dist;
					mTargetTank = enemy;
				}
			}
		}
	}
	else
	{
		currentClosestDistSq = Vec2DDistanceSq(myTank.GetCentralPosition(), mTargetTank->GetCentralPosition());
		if (currentClosestDistSq > 10000)
			if (!mPath->empty() || mEvading)
			{
				if (currentClosestDistSq > 16900)
					mTargetTank = nullptr;
			}
			else
				mTargetTank = nullptr;
	}

	level = max((float)min(currentClosestDistSq / 10000.0f, maxActivationLevel), 0.001f);
	mActivationLevels.push_back(level);

	level = 0.5f * seenTanks.size();
	mActivationLevels.push_back(level);

	level = 1.0f - (myTank.GetBullets() / 100.0f);
	mActivationLevels.push_back(level);

	level = 1.0f - (myTank.GetHealth() / 100.0f);
	mActivationLevels.push_back(level);

	if (mTargetTank)
	{
		//cout << "tank" << endl;
		if (!mPath->empty())
		{
			mActivationLevels.push_back(1.0f);
		}

		level = mTargetTank->GetHealth() / 100.0f;
		mActivationLevels.push_back(level);

		level = mTargetTank->GetBullets() / 100.0f;
		mActivationLevels.push_back(level);

		level = 0.2f * mTargetTank->GetMines();
		mActivationLevels.push_back(level);
	}

	if (!mTargetTank)
	{
		myTank.SetTargetPos(myTank.GetCentralPosition());
		mPath->clear();
		mEvading = false;
	}


	return m018585hFuSM_Base::CalculateActivation(myTank, seenTanks, heardTanks);
}

///////////////////////////////////////////////////////////////////////////////////////
//Problem with thrashing between behaviours being used.
//once you have been seen check if the players heading is in your dir. if so keep going
///////////////////////////////////////////////////////////////////////////////////////
void m018585hFuSM_Run::Update(m018585hTank & myTank, float deltaTime)
{
	if (!mTargetTank)
		return;

	//target can see me
	if (mTargetTank->CanSee(&myTank) && !mEvading || mPath->empty())
	{
		if (mPath->empty())
		{
			mEvading = false;
			Vector2D temp = myTank.GetSteeringBehaviour()->Hide(&myTank, mTargetTank, true);

			myTank.GetPathFinder()->FindPath(temp, myTank.GetCentralPosition(), mPath);
		}

		//check if enemy is closer to first path point
		if (!mPath->empty())
		{
			Vector2D target = mPath->at(0);
			if (myTank.GetCentralPosition() == target && mPath->size() >= 2)
				target = mPath->at(1);

			if (!target.isZero())
			{
				if (ClosestPoint(mTargetTank->GetCentralPosition(), myTank.GetCentralPosition(), target))
				{
					//mPath->clear();
					mEvading = true;
					myTank.SetTargetPos(myTank.GetCentralPosition());
				}
			}
		}
	}

	//I know about target
	if (!mEvading)
	{
		//if enemy cant see me
		if (!mTargetTank->CanSee(&myTank))
		{
			//enemy is in front of me
			if(ClosestPoint(myTank.GetCentralPosition() + myTank.GetHeading(), myTank.GetCentralPosition(), mTargetTank->GetCentralPosition()))
			{
				dropMine = false;
				myTank.GetSteeringBehaviour()->Brake(&myTank, 0.9f, true);
			}
		}
		else
		{
			dropMine = true;

			mStateForces.push_back(myTank.GetSteeringBehaviour()->FollowPath(&myTank, *mPath, true));
			if (!mPath->empty())
				myTank.SetTargetPos(mPath->at(0));
		}
	}
	else
	{
		dropMine = true;

		mStateForces.push_back(myTank.GetSteeringBehaviour()->Evade(&myTank, mTargetTank, true));
		
		Vector2D toTarget = mTargetTank->GetCentralPosition() - myTank.GetCentralPosition();
		//myTank.SetTargetPos(toTarget.GetReverse() * myTank.GetVelocity().Length());
	}

	//Drop Mine
	timeSinceDrop += deltaTime;
	if (timeSinceDrop > mineDropDelay && dropMine)
	{
		timeSinceDrop = 0.0f;
		myTank.ChangeState(TANKSTATE_DROPMINE);
	}
	
	dropMine = false;

	m018585hFuSM_Base::Update(myTank, deltaTime);
}

void m018585hFuSM_Run::Enter(m018585hSteeringBehaviour& steering)
{
	steering.HideOn();
	steering.EvadeOn();
	steering.FollowPathOn();
}

void m018585hFuSM_Run::Exit(m018585hSteeringBehaviour& steering)
{
	if (!mPath->empty())
		mPath->clear();

	steering.HideOff();
	steering.EvadeOff();
	steering.FollowPathOff();

	mEvading = false;
	mTargetTank = false;
}
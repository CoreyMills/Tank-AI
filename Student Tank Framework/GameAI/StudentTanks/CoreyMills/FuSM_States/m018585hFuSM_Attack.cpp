#include "m018585hFuSM_Attack.h"
#include "../../BaseTank.h"
#include "../m018585hTank.h"
#include "../m018585hSteeringBehaviour.h"

m018585hFuSM_Attack::m018585hFuSM_Attack() : m018585hFuSM_Base()
{
	stateType = FuSM_ATTACK;
	mTargetTank = nullptr;

	mPathResetDelay = 2.0f;
	mPathResetTimeWaited = 0.0f;

	mPath = new vector<Vector2D>;
}

m018585hFuSM_Attack::~m018585hFuSM_Attack()
{
}

float m018585hFuSM_Attack::CalculateActivation(m018585hTank & myTank, vector<BaseTank*> seenTanks, vector<BaseTank*> heardTanks)
{
	if (seenTanks.empty() && heardTanks.empty() && !mTargetTank)
		return 0;

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
	}

	level = max(1.0f - (float)min(currentClosestDistSq / 10000, maxActivationLevel), 0.001f);
	mActivationLevels.push_back(level);

	if (!seenTanks.empty())
	{
		level = 1.0f / seenTanks.size();
		mActivationLevels.push_back(level);
	}

	level = myTank.GetBullets() / 100.0f;
	mActivationLevels.push_back(level);

	level = myTank.GetHealth() / 100.0f;
	mActivationLevels.push_back(level);

	if (mTargetTank)
	{
		level = max(1.0f - (mTargetTank->GetHealth() / 100.0f), 0.1f);
		mActivationLevels.push_back(level);
	}

	//size of tank
	//level = mTargetTank.
	return m018585hFuSM_Base::CalculateActivation(myTank, seenTanks, heardTanks);
}

void m018585hFuSM_Attack::Update(m018585hTank & myTank, float deltaTime)
{
	//Target Died
	if (mTargetTank->GetHealth() <= 0)
		mTargetTank = nullptr;

	//exit if no target
	if (!mTargetTank)
	{
		myTank.ChangeState(TANKSTATE_IDLE);
		return;
	}


	double dist = Vec2DDistanceSq(myTank.GetCentralPosition(), mTargetTank->GetCentralPosition());

	if (Raycast(myTank.GetCentralPosition(), mTargetTank->GetCentralPosition(), ObstacleManager::Instance()->GetObstacles()))
	{
		if (!mPath->empty())
			mPath->clear();

		mStateForces.push_back(myTank.GetSteeringBehaviour()->Pursuit(&myTank, mTargetTank, true));
		
		if (!myTank.GetTargetTank())
			myTank.SetTargetTank(mTargetTank);
	}
	else
	{
		cout << (int)dist << endl;
		if (dist < 80000)
		{
			mPathResetTimeWaited += deltaTime;
			if (mPathResetTimeWaited >= mPathResetDelay)
			{
				mPathResetTimeWaited = 0.0f;

				if (!mPath->empty())
					mPath->clear();
			}

			if (mPath->empty())
				myTank.GetPathFinder()->FindPath(mTargetTank->GetCentralPosition(), myTank.GetCentralPosition(), mPath);
			
			mStateForces.push_back(myTank.GetSteeringBehaviour()->FollowPath(&myTank, *mPath, true));
			if (!mPath->empty())
			{
				myTank.SetTargetTank(nullptr);
				myTank.SetTargetPos(mPath->at(0));
			}
		}
		else
		{
			if (!mPath->empty())
				mPath->clear();
			
			mTargetTank = nullptr;
			myTank.SetTargetTank(nullptr);
		}
	}

	if (dist < 19600)
		myTank.ChangeState(TANKSTATE_MANFIRE);
	else
		myTank.ChangeState(TANKSTATE_IDLE);

	if (mTargetTank)
	{
		Vector2D toTarget = mTargetTank->GetCentralPosition() - myTank.GetCentralPosition();
		toTarget.Normalize();

		float angle = (float)asin((myTank.Cross(toTarget, myTank.GetHeading()) /
			toTarget.Length() * myTank.GetHeading().Length()));

		if (dist < 10000)
		{
			float buffer = 0.10472f;
			if (angle <= buffer && angle >= -buffer)
			{
				myTank.ChangeState(TANKSTATE_CANNONFIRE);
				myTank.GetSteeringBehaviour()->Brake(&myTank, 0.97f, true);
			}
		}
	
		angle = (float)asin((myTank.Cross(myTank.GetManFireDir(), toTarget) /
			myTank.GetManFireDir().Length() * toTarget.Length()));

		myTank.RotateManByRadian(angle, deltaTime);
	}

	m018585hFuSM_Base::Update(myTank, deltaTime);
}

void m018585hFuSM_Attack::Enter(m018585hSteeringBehaviour& steering)
{
	steering.FollowPathOn();
	steering.PursuitOn();
}

void m018585hFuSM_Attack::Exit(m018585hSteeringBehaviour& steering)
{
	if (!mPath->empty())
		mPath->clear();

	steering.FollowPathOff();
	steering.PursuitOff();
}
#include "m018585hFuSM_Gather.h"
//#include "../../BaseTank.h"
#include "../m018585hTank.h"
#include "../m018585hSteeringBehaviour.h"
#include "../../PickUpManager.h"
#include "../../ObstacleManager.h"

m018585hFuSM_Gather::m018585hFuSM_Gather() : m018585hFuSM_Base()
{
	stateType = FuSM_GATHER;
	mTargetPickUp = nullptr;
	mTargetTank = nullptr;

	mPathNeeded = false;

	mPreviousNumOfPickups = 0;

	mPath = new vector<Vector2D>;
}

m018585hFuSM_Gather::~m018585hFuSM_Gather()
{
}

float m018585hFuSM_Gather::CalculateActivation(m018585hTank & myTank, vector<BaseTank*> seenTanks, vector<BaseTank*> heardTanks)
{
	if (PickUpManager::Instance()->GetAllPickUps().empty())
		return 0.0f;
	else
		mActivationLevels.push_back(1.0f);

	if (!seenTanks.empty() || !heardTanks.empty() && mTargetTank)
		return 0.0f;

	if (!seenTanks.empty() || !heardTanks.empty())
	{
		if (seenTanks.size() + heardTanks.size() > 1)
			mActivationLevels.push_back(0.0f);

		mActivationLevels.push_back(0.0f);
	}

	if (!mTargetTank)
	{
		double currentClosestDistSq = MaxDouble;

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
			if (!heardTanks.empty())
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
	}

	float level = 0;

	if (mTargetTank)
	{
		if (!mPath->empty())
		{
			mActivationLevels.push_back(1.0f);
		}

		level = min(mTargetTank->GetHealth() / 100.0f, 1.0f);
		mActivationLevels.push_back(level);

		level = min(mTargetTank->GetBullets() / 100.0f, 1.0f);
		mActivationLevels.push_back(level);

		level = min(0.2f * mTargetTank->GetMines(), 1.0f);
		mActivationLevels.push_back(level);
	}

	if (kTilemapPath != "XML Data Files/Arena.xml" && mTargetPickUp)
	{
		cout << "Type" << endl;
		switch (mTargetPickUp->GetGameObjectType())
		{
		case GAMEOBJECT_PICKUP_HEALTH:
			level = max(1 - myTank.GetHealth() / 100.0f, 0.3f);
			mActivationLevels.push_back(level);
			break;
		case GAMEOBJECT_PICKUP_BULLETS:
			level = max(1 - myTank.GetBullets() / 100.0f, 0.2f);
			mActivationLevels.push_back(level);
			break;
		case GAMEOBJECT_PICKUP_ROCKETS:
			level = max(1 - myTank.GetBullets() / 100.0f, 0.2f);
			mActivationLevels.push_back(level);
			break;
		case GAMEOBJECT_PICKUP_MINES:
			level = max(1 - myTank.GetMines() / 5.0f, 0.4f);
			mActivationLevels.push_back(level);
			break;
		case GAMEOBJECT_PICKUP_FUEL:
			level = max(1 - myTank.GetFuel() / 100.0f, 0.4f);
			mActivationLevels.push_back(level);
			break;
		}
	}
   
	return m018585hFuSM_Base::CalculateActivation(myTank, seenTanks, heardTanks);
}

void m018585hFuSM_Gather::Update(m018585hTank & myTank, float deltaTime)
{
	unsigned int numOfPickups = 0;
	for (auto pickup : PickUpManager::Instance()->GetAllPickUps())
	{
		if (pickup == pickup)
			numOfPickups++;
	}

	if (mPath->empty() && mPathNeeded)
		mTargetPickUp = nullptr;

	if (!mTargetPickUp || mPreviousNumOfPickups > numOfPickups)
	{
		cout << numOfPickups << endl;

		myTank.SetTargetPos(myTank.GetCentralPosition());
		mTargetPickUp = nullptr;

		//find best pickup
		if (!PickUpManager::Instance()->GetAllPickUps().empty())
		{
			mPreviousNumOfPickups = PickUpManager::Instance()->GetAllPickUps().size();

			vector<GameObject*> pickups = PickUpManager::Instance()->GetAllPickUps();
			vector<GameObject*> orderedPickups;

			while (!pickups.empty())
			{
				double currentClosestDistSq = MaxFloat;
				GameObject* closestPickup = nullptr;
				int closestPickupPos = -1;

				for (unsigned int i = 0; i < pickups.size(); i++)
				{
					double dist = Vec2DDistanceSq(myTank.GetCentralPosition(), pickups.at(i)->GetCentralPosition());
					if (dist < currentClosestDistSq)
					{
						currentClosestDistSq = dist;
						closestPickup = pickups.at(i);
						closestPickupPos = i;
					}
				}

				orderedPickups.push_back(closestPickup);
				pickups.erase(pickups.begin() + closestPickupPos);
			}

			for (GameObject* pickup : orderedPickups)
			{
				if (Raycast(myTank.GetCentralPosition(), pickup->GetCentralPosition(), ObstacleManager::Instance()->GetObstacles()))
				{
					mTargetPickUp = pickup;
					mPathNeeded = false;
					break;
				}
				else if (!mTargetPickUp)
				{
					mTargetPickUp = pickup;
					mPathNeeded = true;
				}
			}

			if (mPathNeeded)
			{
				if (!mPath->empty())
					mPath->clear();
				myTank.GetPathFinder()->FindPath(mTargetPickUp->GetCentralPosition(), myTank.GetCentralPosition(), mPath);
			}
		}
	}

	if (mTargetPickUp)
	{
		if (mPathNeeded)
		{
			if(!mPath->empty())
				mStateForces.push_back(myTank.GetSteeringBehaviour()->FollowPath(&myTank, *mPath, true));

			if (!mPath->empty())
			{
				if (Vec2DDistanceSq(myTank.GetTargetPos(), myTank.GetCentralPosition()) < 900)
					myTank.SetTargetPos(mPath->at(0));
			}
		}
		else
		{
			mStateForces.push_back(myTank.GetSteeringBehaviour()->Arrive(&myTank, mTargetPickUp->GetCentralPosition(), 20.0f, 0.8f, Deceleration::fast, true));

			if (myTank.GetTargetPos() == myTank.GetCentralPosition())
				myTank.SetTargetPos(mTargetPickUp->GetCentralPosition());
		}
	}

	m018585hFuSM_Base::Update(myTank, deltaTime);
}

void m018585hFuSM_Gather::Enter(m018585hSteeringBehaviour& steering)
{
	steering.ArriveOn();
	steering.FollowPathOn();
}

void m018585hFuSM_Gather::Exit(m018585hSteeringBehaviour& steering)
{
	if (!mPath->empty())
		mPath->clear();

	steering.ArriveOff();
	steering.FollowPathOff();

	mTargetPickUp = nullptr;
}
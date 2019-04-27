#include "m018585hFuSM_ObAvoidance.h"
#include "../../BaseTank.h"
#include "../m018585hTank.h"
#include "../m018585hSteeringBehaviour.h"


m018585hFuSM_ObAvoidance::m018585hFuSM_ObAvoidance() : m018585hFuSM_Base()
{
	stateType = FuSM_AVOID;
}

m018585hFuSM_ObAvoidance::~m018585hFuSM_ObAvoidance()
{
	m018585hFuSM_Base::~m018585hFuSM_Base();
}

float m018585hFuSM_ObAvoidance::CalculateActivation(m018585hTank & myTank, vector<BaseTank*> seenTanks, vector<BaseTank*> heardTanks)
{
	//double checkRadiusSq = 25600.0f;

	//for (auto obstacle : _obManager->GetObstacles())
	//{
	//	double dist = Vec2DDistanceSq(myTank.GetCentralPosition(), obstacle->GetCentralPosition());
	//	if (dist < checkRadiusSq)
	//	{
	//		return 1.0f;

	//		//mActivationLevels.push_back(1.0f);
	//		//break;
	//	}
	//}
	//return 0.0f;

	mActivationLevels.push_back(1.0f);
	return m018585hFuSM_Base::CalculateActivation(myTank, seenTanks, heardTanks);
}

void m018585hFuSM_ObAvoidance::Update(m018585hTank& myTank, float deltaTime)
{
	//multiply by activation level
	Vector2D obForce = myTank.GetSteeringBehaviour()->ObstacleAvoidance(&myTank, true);
	
	if (!obForce.isZero())
	{
		Vector2D tempForce = Vec2DNormalize(obForce);

		float angle = (float)asin((myTank.Cross(tempForce, myTank.GetHeading()) /
			obForce.Length() * myTank.GetHeading().Length()));

		Vector2D centralPos = myTank.GetCentralPosition();
		if (mOldPos == centralPos)
		{
			myTank.SetObRotated(true);
			myTank.RotateHeadingByRadian(angle * 10, deltaTime);

			tempForce = obForce;
			tempForce.Truncate(myTank.GetMaxForce());
			myTank.GetVelocity() = tempForce * 1.5f;
		}

		mStateForces.push_back(obForce);
	}

	mOldPos = myTank.GetCentralPosition();
	m018585hFuSM_Base::Update(myTank, deltaTime);
}

void m018585hFuSM_ObAvoidance::Enter(m018585hSteeringBehaviour& steering)
{
	steering.ObstacleAvoidanceOn();
}

void m018585hFuSM_ObAvoidance::Exit(m018585hSteeringBehaviour& steering)
{
	steering.ObstacleAvoidanceOff();
}
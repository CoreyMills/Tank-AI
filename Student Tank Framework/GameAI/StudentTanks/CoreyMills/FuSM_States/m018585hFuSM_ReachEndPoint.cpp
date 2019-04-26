#include "m018585hFuSM_ReachEndPoint.h"
#include "../../BaseTank.h"
#include "../m018585hTank.h"
#include "../m018585hSteeringBehaviour.h"
#include "../../PickUpManager.h"
#include "../../ObstacleManager.h"

m018585hFuSM_ReachEndPoint::m018585hFuSM_ReachEndPoint(Vector2D endPoint) : m018585hFuSM_Base()
{
	stateType = FuSM_ENDPOINT;
	mEndPoint = endPoint;

	mPath = new vector<Vector2D>;
}

m018585hFuSM_ReachEndPoint::~m018585hFuSM_ReachEndPoint()
{
}

float m018585hFuSM_ReachEndPoint::CalculateActivation(m018585hTank & myTank, vector<BaseTank*> seenTanks, vector<BaseTank*> heardTanks)
{
	double dist = Vec2DDistance(myTank.GetCentralPosition(), mEndPoint);

	if (PickUpManager::Instance()->GetAllPickUps().empty() && dist > 10)
		mActivationLevels.push_back(1.0f);

	mActivationLevels.push_back(0.0f);

	return m018585hFuSM_Base::CalculateActivation(myTank, seenTanks, heardTanks);

}

void m018585hFuSM_ReachEndPoint::Update(m018585hTank & myTank, float deltaTime)
{
	if (mPath->empty())
		myTank.GetPathFinder()->FindPath(mEndPoint, myTank.GetCentralPosition(), mPath);
	
	mStateForces.push_back(myTank.GetSteeringBehaviour()->FollowPath(&myTank, *mPath, true));

	if (!mPath->empty())
		myTank.SetTargetPos(mPath->at(0));
	else
		cout << "Winner!!!" << '\n' << "Health: " << myTank.GetHealth() << endl;

	m018585hFuSM_Base::Update(myTank, deltaTime);
}

void m018585hFuSM_ReachEndPoint::Enter(m018585hSteeringBehaviour& steering)
{
	steering.FollowPathOn();
}

void m018585hFuSM_ReachEndPoint::Exit(m018585hSteeringBehaviour& steering)
{
	if (!mPath->empty())
		mPath->clear();

	steering.FollowPathOff();
}
#include "m018585hSteeringBehaviour.h"
#include "m018585hTank.h"
#include "../../C2DMatrix.h"

m018585hSteeringBehaviour::m018585hSteeringBehaviour()
{
	//steering bools
	mBrakeOn = mSeekOn = mFleeOn = false;
	mRadiusFleeOn = mArriveOn = mPursuitOn = false;
	mEvadeOn = mWanderOn = mObstacleAvoidanceOn = false;
	mInterposeOn = mHideOn = mFollowPathOn = false;

	//Wander Setup
	mWanderTarget = { 0, 0 };
	mWanderRadius = 30;
	mWanderDistance = 25;
	mWanderJitter = 15;

	//Follow path setup
	mChaseLengthSq = 900;

	//Feelers
	_LargeFeelerLength = 20.0f;
	_smallFeelerLength = 15.0f;
}

m018585hSteeringBehaviour::~m018585hSteeringBehaviour()
{
}

Vector2D m018585hSteeringBehaviour::Calculate(m018585hTank* myTank)
{
	Vector2D steeringForce;
	for (unsigned int i = 0; i < mAllForces.size(); i++)
	{ 
		double magUsed = steeringForce.Length();

		double magRemaining = myTank->GetMaxForce() - magUsed;
		if (magRemaining <= 0.0)
			break;

		double magToAdd = mAllForces.at(i).Length();

		if (magToAdd < magRemaining)
			steeringForce += mAllForces.at(i);
		else 
			steeringForce += (Vec2DNormalize(mAllForces.at(i)) * magRemaining);
	}
	mAllForces.clear();

	if (steeringForce.isZero())
		Brake(myTank, 0.95f, true);

	return steeringForce;
}

//velocity multiplied by percentage param
void m018585hSteeringBehaviour::Brake(m018585hTank* myTank, float percentage, bool bypass)
{
	if (mBrakeOn || bypass)
		myTank->SetVelocity(myTank->GetVelocity() * percentage);
}

Vector2D m018585hSteeringBehaviour::Seek(m018585hTank* myTank, Vector2D targetPos, bool bypass)
{
	if (mSeekOn || bypass)
	{
		Vector2D DesiredVelocity = Vec2DNormalize(targetPos - myTank->GetCentralPosition()) * myTank->GetMaxSpeed();
		return (DesiredVelocity - myTank->GetVelocity());
	}
	return { 0, 0 };
}

Vector2D m018585hSteeringBehaviour::Flee(m018585hTank* myTank, Vector2D targetPos, bool bypass)
{
	if (mFleeOn || bypass)
	{
		Vector2D DesiredVelocity = Vec2DNormalize(myTank->GetCentralPosition() - targetPos) * myTank->GetMaxSpeed();
		return (DesiredVelocity - myTank->GetVelocity());
	}
	return { 0, 0 };
}

Vector2D m018585hSteeringBehaviour::RadiusFlee(m018585hTank* myTank, Vector2D targetPos, float radius, bool bypass)
{
	if (mRadiusFleeOn || bypass)
	{
		if (myTank->GetCentralPosition().DistanceSq(targetPos) > radius)
		{
			return Vector2D(0, 0);
		}

		Vector2D DesiredVelocity = Vec2DNormalize(myTank->GetCentralPosition() - targetPos) * myTank->GetMaxSpeed();
		return (DesiredVelocity - myTank->GetVelocity());
	}
	return { 0, 0 };
}

Vector2D m018585hSteeringBehaviour::Arrive(m018585hTank* myTank, Vector2D targetPos, float distToBrake, 
											float brakePercentage, Deceleration deceleration, bool bypass)
{
	if (mArriveOn || bypass)
	{
		const double decelerationTweaker = 1.0f;
		Vector2D toTarget = targetPos - myTank->GetCentralPosition();
		double dist = toTarget.Length();

		//cout << dist << endl;

		if (dist > 0)
		{
			double speed = dist / ((double)deceleration * decelerationTweaker);
			speed = min(speed, myTank->GetMaxSpeed());

			if (dist > distToBrake)
			{
				Vector2D desiredVelocity = toTarget * speed / dist;
				return desiredVelocity - myTank->GetVelocity();
				//Vector2D desiredVelocity = toTarget * speed / dist;
				//return desiredVelocity - (myTank->GetVelocity() * 2);
			}
			else
			{
				Brake(myTank, brakePercentage, true);
			}
		}
	}
	return { 0, 0 };
}

Vector2D m018585hSteeringBehaviour::Pursuit(m018585hTank* myTank, BaseTank* target, bool byPass)
{
	if (mPursuitOn && target || byPass)
	{ 
		Vector2D toTarget = target->GetCentralPosition() - myTank->GetCentralPosition();
		//double relativeHeading = myTank->GetForward().Dot(target->GetHeading() * -1); 
		double relativeHeading = myTank->GetHeading().Dot(target->GetHeading());
		
		if (toTarget.Dot(myTank->GetHeading()) < 0 && 
			relativeHeading < -0.45 || relativeHeading > 0.45) //acos(0.45) = 63 degs 
		{
			return Seek(myTank, target->GetCentralPosition(), true);
		}

		double lookAheadTime = toTarget.Length() / myTank->GetMaxSpeed() + abs(target->GetCurrentSpeed());
		//lookAheadTime += TurnAroundTime(myTank, target->GetCentralPosition());

		return Seek(myTank, target->GetCentralPosition() + target->GetVelocity() * lookAheadTime, true);
	}
	return { 0, 0 };
}

Vector2D m018585hSteeringBehaviour::Evade(m018585hTank* myTank, BaseTank* pursuer, bool bypass)
{
	if (mEvadeOn || bypass)
	{
		Vector2D toPursuer = pursuer->GetCentralPosition() - myTank->GetCentralPosition();
		double lookAheadTime = toPursuer.Length() / (myTank->GetMaxSpeed() + pursuer->GetCurrentSpeed());

		//now flee away from predicted future position of the pursuer 
		return Flee(myTank, pursuer->GetCentralPosition() + pursuer->GetVelocity() * lookAheadTime, true);
	}
	return { 0, 0 };
}

Vector2D m018585hSteeringBehaviour::Wander(m018585hTank* myTank, bool bypass)
{
	if (mWanderOn || bypass)
	{
		mWanderTarget += Vector2D(RandomClamped() * mWanderJitter, RandomClamped() * mWanderJitter);
		mWanderTarget.Normalize();
		
		mWanderTarget *= mWanderRadius;

		Vector2D target = myTank->GetCentralPosition() - mWanderTarget - Vector2D(0, mWanderDistance);
		return target - myTank->GetCentralPosition();
	}
	return { 0, 0 };
}

Vector2D m018585hSteeringBehaviour::ObstacleAvoidance(m018585hTank* myTank, const std::vector<GameObject*> &obstacles, bool bypass)
{
	if (mObstacleAvoidanceOn || bypass)
	{
		Vector2D totalForce;
		mDebugVectors.clear();
		int count = 0;
		for (auto feeler : mDetectionFeelers)
		{
			count++; 
			Vector2D projection = myTank->GetCentralPosition() + feeler.feeler;

			for (GameObject* object : obstacles)
			{
				vector<Vector2D> objBox = object->GetAdjustedBoundingBox();

				if (InsideRegion(projection, objBox.at(0), objBox.at(2)))
				{
					/*
					Vector2D tankToObj = (myTank->GetCentralPosition() - object->GetCentralPosition()) / 2;
					Vector2D projToObj = projection - object->GetCentralPosition();
					totalForce += projToObj + tankToObj;

					mDebugVectors.push_back(projToObj);
					mDebugVectors.push_back(tankToObj);
					*/

					if (feeler.insideFeeler)
						Brake(myTank, 0.8f, true);

					Vector2D projToTank = myTank->GetCentralPosition() - projection;
					totalForce += (Vector2D(round(projToTank.x * 1000.0f), round(projToTank.y * 1000.0f)) / 1000.0f) * feeler.strengthMultiplier;
					mDebugVectors.push_back(projToTank);
				}
			}
		}
		return totalForce;
	}

	return Vector2D();
}

Vector2D m018585hSteeringBehaviour::Interpose(m018585hTank* myTank, BaseTank*  agentA, BaseTank* agentB, bool bypass)
{
	if (mInterposeOn || bypass)
	{
		Vector2D MidPoint = (agentA->GetCentralPosition() + agentB->GetCentralPosition()) / 2.0;
		double TimeToReachMidPoint = Vec2DDistance(myTank->GetCentralPosition(), MidPoint) / myTank->GetMaxSpeed();
		
		Vector2D APos = agentA->GetCentralPosition() + agentA->GetVelocity() * TimeToReachMidPoint;
		Vector2D BPos = agentB->GetCentralPosition() + agentB->GetVelocity() * TimeToReachMidPoint;

		MidPoint = (APos + BPos) / 2.0;

		return Arrive(myTank, MidPoint, 5.0f, 0.9f, fast, true);
	}
	return Vector2D{ 0, 0 };
}

//returns bestHidingSpot, use arrive or pathfinding to reach the point 
Vector2D m018585hSteeringBehaviour::Hide(m018585hTank* myTank, BaseTank* target, vector<GameObject*>& obstacles, bool bypass)
{
	if (mHideOn || bypass)
	{
		double distToClosest = MaxDouble;
		Vector2D bestHidingSpot;
		std::vector<GameObject*>::iterator curOb = obstacles.begin();
		while (curOb != obstacles.end()) 
		{ 
			//calculate the position of the hiding spot for this obstacle 
			Vector2D hidingSpot = GetHidingPosition((*curOb)->GetCentralPosition(), 
													(*curOb)->GetCollisionRadius(), target->GetCentralPosition());
			//work in distance-squared space to find the closest hiding 
			//spot to the agent 
			double dist = Vec2DDistanceSq(hidingSpot, myTank->GetCentralPosition());
			if (dist < distToClosest) 
			{
				distToClosest = dist;
				bestHidingSpot = hidingSpot;
			}
			++curOb;
		}

		//if no suitable obstacles found then evade the target 
		if (distToClosest == MaxDouble) 
		{ 
			return Evade(myTank, target, true); 
		}
		//else use Arrive on the hiding spot 
		//return Arrive(myTank, BestHidingSpot, 5.0f, 0.6f, fast, true);
		return bestHidingSpot;
	}
	return Vector2D{ 0, 0 };
}

Vector2D m018585hSteeringBehaviour::GetHidingPosition(const Vector2D& posOb, const double radiusOb, const Vector2D& posTarget)
{
	//calculate how far away the agent is to be from the chosen obstacle’s 
	//bounding radius 
	const double DistanceFromBoundary = 50.0;
	double DistAway = radiusOb + DistanceFromBoundary;
	//calculate the heading toward the object from the target 
	Vector2D ToOb = Vec2DNormalize(posOb - posTarget);
	//scale it to size and add to the obstacle's position to get 
	//the hiding spot. 
	return (ToOb * DistAway) + posOb;
}

Vector2D m018585hSteeringBehaviour::FollowPath(m018585hTank* myTank, vector<Vector2D>& path, bool bypass)
{
	if (mFollowPathOn && !path.empty() || bypass && !path.empty())
	{
		if (path.size() >= 1)
		{
			float distCurrentSq = (float)Vec2DDistanceSq(myTank->GetCentralPosition(), path.at(0));
		
			if (distCurrentSq < mChaseLengthSq)
				path.erase(path.begin());
			else if (path.size() >= 2)
			{
				float distNextSq = (float)Vec2DDistanceSq(myTank->GetCentralPosition(), path.at(1));;
				
				if (distCurrentSq > distNextSq)
					path.erase(path.begin());
			}
		}

		if (!path.empty())
		{
			if (path.size() == 1)
			{
				return Arrive(myTank, path.at(0), 5.0f, 0.9f, normal, true);
			}
			else 
			{
				//return Seek(myTank, path.at(0), true);
				return Arrive(myTank, path.at(0), 15.0f, 0.99f, normal, true);
			}
		}
	}
	return Vector2D();
}

void m018585hSteeringBehaviour::CalculateFeelers(m018585hTank * myTank)
{
	mDetectionFeelers.clear();
	
	//Calculate North East Feeler
	C2DMatrix rotMatrix1;
	rotMatrix1.Rotate(0.785398);
	Vector2D neVec = myTank->GetHeading();
	rotMatrix1.TransformVector2Ds(neVec);

	//Calculate North West Feeler
	C2DMatrix rotMatrix2;
	rotMatrix2.Rotate(-0.785398);
	Vector2D nwVec = myTank->GetHeading();
	rotMatrix2.TransformVector2Ds(nwVec);

	mDetectionFeelers.push_back(Feeler(Vec2DNormalize(Vector2D(-neVec.x, -neVec.y)), false, 0.25f));							 //South-East
	mDetectionFeelers.push_back(Feeler(Vec2DNormalize(Vector2D(-nwVec.x, -nwVec.y)), false, 0.25f));							 //South-West
	mDetectionFeelers.push_back(Feeler(Vec2DNormalize(Vector2D(myTank->GetHeading().x, myTank->GetHeading().y)), false, 0.25f)); //North
	mDetectionFeelers.push_back(Feeler(Vec2DNormalize(Vector2D(neVec.x, neVec.y)), false, 0.25f));							     //North-East
	mDetectionFeelers.push_back(Feeler(Vec2DNormalize(Vector2D(nwVec.x, nwVec.y)), false, 0.25f));							     //North-West
	mDetectionFeelers.push_back(Feeler(Vec2DNormalize(Vector2D(myTank->GetSide().x, myTank->GetSide().y)), false, 0.25f));	     //East
	mDetectionFeelers.push_back(Feeler(Vec2DNormalize(Vector2D(-myTank->GetSide().x, -myTank->GetSide().y)), false, 0.25f));	 //West

	mDetectionFeelers.push_back(Feeler(Vec2DNormalize(Vector2D(-myTank->GetHeading().x, -myTank->GetHeading().y)), true, 1.0f)); //South - Short
	mDetectionFeelers.push_back(Feeler(Vec2DNormalize(Vector2D(myTank->GetHeading().x, myTank->GetHeading().y)), true, 1.0f)); //North
	mDetectionFeelers.push_back(Feeler(Vec2DNormalize(Vector2D(neVec.x, neVec.y)), true, 1.0f));							     //North-East - Short
	mDetectionFeelers.push_back(Feeler(Vec2DNormalize(Vector2D(nwVec.x, nwVec.y)), true, 1.0f));							     //North-West - Short
	mDetectionFeelers.push_back(Feeler(Vec2DNormalize(Vector2D(myTank->GetSide().x, myTank->GetSide().y)), true, 1.0f));	     //East - Short
	mDetectionFeelers.push_back(Feeler(Vec2DNormalize(Vector2D(-myTank->GetSide().x, -myTank->GetSide().y)), true, 1.0f));    //West - Short


	for (unsigned int i = 0; i < mDetectionFeelers.size(); i++)
	{
		if (!mDetectionFeelers.at(i).insideFeeler)
		{
			mDetectionFeelers.at(i).feeler = Vec2DNormalize(mDetectionFeelers.at(i).feeler) * (_LargeFeelerLength + (myTank->GetVelocity().Length() / 2));
		}
		else
		{
			mDetectionFeelers.at(i).feeler = Vec2DNormalize(mDetectionFeelers.at(i).feeler) * (_smallFeelerLength + (myTank->GetVelocity().Length() / 4));
		}
	}
}

double m018585hSteeringBehaviour::TurnAroundTime(BaseTank* agent, Vector2D targetPos)
{
	Vector2D toTarget = Vec2DNormalize(targetPos - agent->GetPosition());

	double dot = agent->GetHeading().Dot(toTarget);

	//change this value to get the desired behavior. The higher the max turn 
	//rate of the vehicle, the higher this value should be. If the vehicle is 
	//heading in the opposite direction to its target position then a value 
	//of 0.5 means that this function will return a time of 1 second for the 
	//vehicle to turn around. 

	const double coefficient = 0.5f;

	//the dot product gives a value of 1 if the target is directly ahead and -1 
	//if it is directly behind. Subtracting 1 and multiplying by the negative of 
	//the coefficient gives a positive value proportional to the rotational 
	//displacement of the vehicle and target. 
	return (dot - 1.0f) * -coefficient;
}

float m018585hSteeringBehaviour::RandomClamped()
{
	float temp = (float)(rand() % 200 - 100);
	return temp /= 100.0f;
}
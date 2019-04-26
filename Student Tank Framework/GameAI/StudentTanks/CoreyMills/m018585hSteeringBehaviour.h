#ifndef M018585HSTEERINGBEHAVIOUR_H
#define M018585HSTEERINGBEHAVIOUR_H

#include "m018585hCommons.h"
#include "../../BaseTank.h"

struct Feeler
{
	Vector2D feeler;
	bool insideFeeler;
	float strengthMultiplier;

	Feeler()
	{
		this->insideFeeler = false;
		this->strengthMultiplier = 1.0f;
	}

	Feeler(Vector2D feelerVec, bool boolVal, float multiplier)
	{
		this->feeler = feelerVec;
		this->insideFeeler = boolVal;
		this->strengthMultiplier = multiplier;
	}
};

class m018585hTank;

class m018585hSteeringBehaviour
{
public:
	m018585hSteeringBehaviour();
	~m018585hSteeringBehaviour();

	//void Update();

	void AddForce(Vector2D force){ mAllForces.push_back(force); }
	Vector2D Calculate(m018585hTank* myTank);

	//acts directly upon the tanks velocity
	void Brake(m018585hTank* myTank, float percentage, bool bypass);

	Vector2D Seek(m018585hTank* myTank, Vector2D targetPos, bool bypass);
	Vector2D Flee(m018585hTank* myTank, Vector2D targetPos, bool bypass);
	Vector2D RadiusFlee(m018585hTank* myTank, Vector2D targetPos, float radius, bool bypass);
	Vector2D Arrive(m018585hTank* myTank, Vector2D targetPos, float distToBrake, float brakePercentage, Deceleration deceleration, bool bypass);

	Vector2D Pursuit(m018585hTank* myTank, BaseTank* evader, bool byPass);
	Vector2D Evade(m018585hTank* myTank, BaseTank* pursuer, bool bypass);

	Vector2D Wander(m018585hTank* myTank, bool bypass);
	Vector2D ObstacleAvoidance(m018585hTank* myTank, const std::vector<GameObject*> &obstacles, bool bypass);
	Vector2D Interpose(m018585hTank* myTank, BaseTank* agentA, BaseTank* agentB, bool bypass);
	Vector2D Hide(m018585hTank* myTank, BaseTank* target, vector<GameObject*>& obstacles, bool bypass);
	Vector2D FollowPath(m018585hTank* myTank, vector<Vector2D>& path, bool bypass);

	void CalculateFeelers(m018585hTank* myTank);

	void Reset() {mBrakeOn = mSeekOn = mFleeOn = 
					mRadiusFleeOn = mArriveOn = 
					mPursuitOn = mEvadeOn = mWanderOn = 
					mObstacleAvoidanceOn = mInterposeOn = 
					mHideOn = mFollowPathOn = false;}

	void BrakeOn() { mBrakeOn = true; }
	void BrakeOff() { mBrakeOn = false; }
	bool CheckBrake() { return mBrakeOn; }

	void SeekOn() { mSeekOn = true; }
	void SeekOff() { mSeekOn = false; }
	bool CheckSeek() { return mSeekOn; }

	void FleeOn() { mFleeOn = true; }
	void FleeOff() { mFleeOn = false; }
	bool CheckFlee() { return mFleeOn; }

	void RadiusFleeOn() { mRadiusFleeOn = true; }
	void RadiusFleeOff() { mRadiusFleeOn = false; }
	bool CheckRadiusFlee() { return mRadiusFleeOn; }

	void ArriveOn() { mArriveOn = true; }
	void ArriveOff() { mArriveOn = false; }
	bool CheckArrive() { return mArriveOn; }

	void PursuitOn() { mPursuitOn = true; }
	void PursuitOff() { mPursuitOn = false; }
	bool CheckPursuit() { return mPursuitOn; }

	void EvadeOn() { mEvadeOn = true; }
	void EvadeOff() { mEvadeOn = false; }
	bool CheckEvade() { return mEvadeOn; }

	void WanderOn() { mWanderOn = true; }
	void WanderOff() { mWanderOn = false; }
	bool CheckWander() { return mWanderOn; }

	void ObstacleAvoidanceOn() { mObstacleAvoidanceOn = true; }
	void ObstacleAvoidanceOff() { mObstacleAvoidanceOn = false; }
	bool CheckObstacleAvoidance() { return mObstacleAvoidanceOn; }

	void InterposeOn() { mInterposeOn = true; }
	void InterposeOff() { mInterposeOn = false; }
	bool CheckInterpose() { return mInterposeOn; }

	void HideOn() { mHideOn = true; }
	void HideOff() { mHideOn = false; }
	bool CheckHide() { return mHideOn; }

	void FollowPathOn() { mFollowPathOn = true; }
	void FollowPathOff() { mFollowPathOn = false; }
	bool CheckFollowPath() { return mFollowPathOn; }

	vector<Feeler> GetFeelers() { return mDetectionFeelers; }

	vector<Vector2D> GetDebugVectors() { return mDebugVectors; }

private:
	//random number clamped between - 1 and 1
	float RandomClamped();
	double TurnAroundTime(BaseTank* pAgent, Vector2D targetPos);
	Vector2D GetHidingPosition(const Vector2D& posOb, const double radiusOb, const Vector2D& posTarget);

	vector<Vector2D> mAllForces;

	//wander vars
	Vector2D mWanderTarget;
	float mWanderRadius, mWanderDistance, mWanderJitter;

	//obstacle avoidance vars
	vector<Feeler> mDetectionFeelers;
	float _LargeFeelerLength, _smallFeelerLength;

	bool mBrakeOn, mSeekOn, mFleeOn;
	bool mRadiusFleeOn, mArriveOn, mPursuitOn;
	bool mEvadeOn, mWanderOn, mObstacleAvoidanceOn;
	bool mInterposeOn, mHideOn, mFollowPathOn;

	//followPath vars
	float mChaseLengthSq;

	//debugging
	vector<Vector2D> mDebugVectors;
};

#endif //M018585HSTEERINGBEHAVIOUR_H
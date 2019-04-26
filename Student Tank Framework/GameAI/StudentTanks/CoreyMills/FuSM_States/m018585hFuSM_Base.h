#ifndef M018585HFUSM_BASE_H
#define M018585HFUSM_BASE_H

#include "../m018585hCommons.h"
#include <vector>

enum FuSM_STATE
{
	FuSM_IDLE = 0,
	FuSM_ATTACK,
	FuSM_SEARCH,
	FuSM_RUN,
	FuSM_AVOID,
	FuSM_GATHER,
	FuSM_ENDPOINT,
	FuSM_MAX
};

class BaseTank;
class m018585hTank;
class m018585hSteeringBehaviour;

class m018585hFuSM_Base
{
public:
	m018585hFuSM_Base();
	virtual ~m018585hFuSM_Base();

	virtual float CalculateActivation(m018585hTank& myTank, vector<BaseTank*> seenTanks, vector<BaseTank*> heardTanks);

	virtual void Update(m018585hTank& myTank, float deltaTime);

	virtual void Enter(m018585hSteeringBehaviour& steering) = 0;
	virtual void Exit(m018585hSteeringBehaviour& steering) = 0;

	bool IsActivated() { return (mActivationLevel > 0); }

	FuSM_STATE GetStateType() { return stateType; }

protected:
	Vector2D CalculateStateForce();
	vector<Vector2D> mStateForces;
	
	vector<Vector2D>* mPath;
	float mTimeWaited, mTimeInterval;

	vector<float> mActivationLevels;

	FuSM_STATE stateType;

	float mActivationLevel;
	float maxActivationLevel = 1.0f;
	float minActivationLevel = 0.0f;
};
#endif // !M018585HFUSM_BASE_H
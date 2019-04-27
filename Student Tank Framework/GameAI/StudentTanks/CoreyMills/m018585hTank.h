#ifndef M018585HTANK_H
#define M018585HTANK_H

#include <SDL.h>

#include "m018585hCommons.h"
#include "../../BaseTank.h"
#include "../../ObstacleManager.h"
#include "m018585hSteeringBehaviour.h"
#include "m018585hFuSM_Manager.h"
#include "m018585h_DRS.h"

//FuSM states
#include "FuSM_States/m018585hFuSM_Base.h"
#include "FuSM_States/m018585hFuSM_Attack.h"
#include "FuSM_States/m018585hFuSM_Gather.h"
#include "FuSM_States/m018585hFuSM_Search.h"
#include "FuSM_States/m018585hFuSM_Run.h"
#include "FuSM_States/m018585hFuSM_ObAvoidance.h"
#include "FuSM_States/m018585hFuSM_ReachEndPoint.h"

//---------------------------------------------------------------

enum STEERING_STATES
{
	SS_IDLE = 0,
	SS_SEEK,
	SS_FLEE,
	SS_ARRIVE,
	SS_INTRPOSE,
	SS_OBSTACLEAVOIDANCE,
	SS_PATHFOLLOWING,
	SS_WANDER,
	SS_PURSUIT,
	SS_EVADE,
	SS_HIDE,
	SS_BRAKE,
	SS_MAX
};

//---------------------------------------------------------------

class m018585hTank : public BaseTank
{
public:
	m018585hTank(SDL_Renderer* renderer, TankSetupDetails details);
	~m018585hTank();

	void InitFuSMStates(SDL_Renderer* renderer);

	void Update(float deltaTime, SDL_Event e);
	void Render();
	 
	m018585hSteeringBehaviour* GetSteeringBehaviour() { return mSteering; }
	m018585h_DRS* GetPathFinder() { return mPathFinder; }

	void HandleInput(float deltaTime, SDL_Event e);
	void ChangeSteeringState(STEERING_STATES newState);

	void ChangeTankState(BASE_TANK_STATE newState);

	void RotateHeadingByRadian(double radian, float deltaTime);

	void RotateManByRadian(double radian, float deltaTime);

	void FindEnemyTank();

	void AttackEnemyTank(float deltaTime);

	void SetVelocity(Vector2D newVel) { mVelocity = newVel; }
	
	void MoveInHeadingDirection(float deltaTime);

	inline double Cross(const Vector2D& v1, const Vector2D& v2)
	{
		return v1.x * v2.y - v2.x * v1.y;
	}

	BaseTank* GetTargetTank() { return mTargetTank; }
	void SetTargetTank(BaseTank* newTank) { mTargetTank = newTank; }

	Vector2D GetTargetPos() { return mTargetPos; }
	void SetTargetPos(Vector2D newPos) { mTargetPos = newPos; }

	void ChangeRotationStats(float rotBuffer1, float rotAmount1) 
			{ mRotationBuffer = rotBuffer1; mRotationAmount = rotAmount1;}

	void ChangeMovementStats(float moveBuffer) { mMovementBuffer = moveBuffer; }

	Vector2D GetManFireDir() { return mManFireDirection; }

	void SetObRotated(bool newVal) { mObRotated = newVal; }
	void SetRotateBlocked(bool newVal) { mRotateBlocked = newVal; }

	void SetEnemyLookAhead(Vector2D v1) { mEnemyLookAhead = v1; }

private:
	m018585hSteeringBehaviour* mSteering;
	m018585hFuSM_Manager* mFuSM_Manager;
	m018585h_DRS* mPathFinder;
	vector<Vector2D> mPath;

	STEERING_STATES mSteeringState;

	Vector2D mMousePos;

	BaseTank* mTargetTank;
	Vector2D mTargetPos;

	//for pursuit
	Vector2D mEnemyLookAhead;

	Vector2D mOldPos;

	bool mInitialBrake;

	bool mFuSMActivated, mSpaceKeyDown;

	bool mNormalBufferStats;

	float mRotationBuffer, mMovementBuffer;
	float mRotationAmount;
	bool mRotating, mMoving, mObRotated, mRotateBlocked;
};
#endif //M018585HTANK_H
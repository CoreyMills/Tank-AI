#include "m018585hTank.h"
#include "../../TankManager.h"
#include "../../C2DMatrix.h"

#include <iostream>

//--------------------------------------------------------------------------------------------------

m018585hTank::m018585hTank(SDL_Renderer* renderer, TankSetupDetails details): BaseTank(renderer, details)
{
	mSteering = new m018585hSteeringBehaviour();
	mPathFinder = new m018585h_DRS(renderer, mCollisionMap);
	mFuSM_Manager = new m018585hFuSM_Manager();

	mSteeringState = SS_IDLE;

	mInitialBrake = false;

	mTargetPos = mMousePos = GetCentralPosition();

	mTargetTank = nullptr;

	mFuSMActivated = true;
	mSpaceKeyDown = false;
	mMoving = true;
	mObRotated = false;

	//10 degrees
	//_rotationBuffer1 = 0.174533f;
	//5 degrees
	//_rotationBuffer1 = 0.0872665f;
	//3 degrees
	//_rotationBuffer1 = 0.0523599f;
	//2 degrees
	mRotationBuffer = 0.0349066f;
	//1 degrees
	//_rotationBuffer1 = 0.0174533f;
	//0.5 degrees
	//_rotationBuffer1 = 0.00872665f;

	mMovementBuffer = 0.01f;
	mRotationAmount = 0.008f;

	mRotating = false;

	mNormalBufferStats = true;

	InitFuSMStates(renderer);
}

//--------------------------------------------------------------------------------------------------

m018585hTank::~m018585hTank()
{
	mSteering = nullptr;
	delete mSteering;

	mPathFinder = nullptr;
	delete mPathFinder;

	mFuSM_Manager = nullptr;
	delete mFuSM_Manager;

	mTargetTank = nullptr;
	delete mTargetTank;
}

void m018585hTank::InitFuSMStates(SDL_Renderer* renderer)
{
	if (kTilemapPath == "XML Data Files/Arena.xml")
	{
		m018585hFuSM_ObAvoidance* state1 = new m018585hFuSM_ObAvoidance();
		mFuSM_Manager->AddState(state1);

		m018585hFuSM_Run* state2 = new m018585hFuSM_Run();
		mFuSM_Manager->AddState(state2);

		m018585hFuSM_Gather* state3 = new m018585hFuSM_Gather();
		mFuSM_Manager->AddState(state3);

		m018585hFuSM_ReachEndPoint* state4 = new m018585hFuSM_ReachEndPoint(Vector2D(900.0f, 575.0f));
		mFuSM_Manager->AddState(state4);
	}
	else if (kTilemapPath == "XML Data Files/BattleGround.xml")
	{
		m018585hFuSM_ObAvoidance* state1 = new m018585hFuSM_ObAvoidance();
		mFuSM_Manager->AddState(state1);

		m018585hFuSM_Attack* state2 = new m018585hFuSM_Attack();
		mFuSM_Manager->AddState(state2);

		m018585hFuSM_Run* state3 = new m018585hFuSM_Run();
		mFuSM_Manager->AddState(state3);

		m018585hFuSM_Gather* state4 = new m018585hFuSM_Gather();
		mFuSM_Manager->AddState(state4);

		m018585hFuSM_Search* state5 = new m018585hFuSM_Search();
		mFuSM_Manager->AddState(state5);
	}
}

//--------------------------------------------------------------------------------------------------

void m018585hTank::ChangeTankState(BASE_TANK_STATE newState)
{
	BaseTank::ChangeState(newState);
}

//--------------------------------------------------------------------------------------------------

void m018585hTank::Update(float deltaTime, SDL_Event e)
{
	mSteering->CalculateFeelers(this);

	//change between FSM and FuSM
	if (GetAsyncKeyState(VK_SPACE))
	{
		if (!mSpaceKeyDown)
		{
			mSpaceKeyDown = true;
			mFuSMActivated = !mFuSMActivated;
			mSteering->Reset();
			mSteeringState = SS_IDLE;
			mPath.clear();

			if (mFuSMActivated)
				cout << "FuSM" << endl;
			else
			{
				cout << "FSM" << endl;
				mFuSM_Manager->Reset();
			}
		}
	}
	else
	{
		mSpaceKeyDown = false;
	}

	if (mFuSMActivated)
	{
		mFuSM_Manager->Update(*this, mTanksICanSee, mTanksICanHear, deltaTime);
	}
	else
	{
		HandleInput(deltaTime, e);

		///////////////////////////////////////////////////
		Vector2D obForce = mSteering->ObstacleAvoidance(this, true);

		Vector2D newPos = GetCentralPosition();
		if (!obForce.isZero())
		{
			Vector2D tempForce = Vec2DNormalize(obForce);

			float angle = (float)asin((Cross(tempForce, mHeading) /
				obForce.Length() * mHeading.Length()));

			if (mOldPos == newPos)
			{
				mObRotated = true;
				RotateHeadingByRadian(angle * 10, deltaTime);
			}
			mSteering->AddForce(obForce);
		}
		mOldPos = newPos;
		///////////////////////////////////////////////////

		//Follow Path
		mSteering->AddForce(mSteering->FollowPath(this, mPath, false));
		if (!mPath.empty() && mSteering->CheckFollowPath())
		{
			mTargetPos = mPath.at(0);
		}

		//Seek
		mSteering->AddForce(mSteering->Seek(this, mTargetPos, false));

		//Arrive
		mSteering->AddForce(mSteering->Arrive(this, mTargetPos, 1.0f, 0.9f, Deceleration::slow, false));

		//Pursuit
		mSteering->AddForce(mSteering->Pursuit(this, mTargetTank, false));
		if (mSteering->CheckPursuit())
		{
			if (!mTargetTank)
				FindEnemyTank();
			else
			{
				//Target Died
				if (mTargetTank->GetHealth() <= 0)
					mTargetTank = nullptr;
			}
			AttackEnemyTank(deltaTime);
			//cout << "Health: " << mTargetTank->GetHealth() << endl;
		}

		//Wander
		mSteering->AddForce(mSteering->Wander(this, false));
	}

	if (!mSteering->CheckPursuit())
	{
		float angle = (float)asin((Cross(mManFireDirection, mHeading) /
			mManFireDirection.Length() * mHeading.Length()));

		RotateManByRadian(angle, deltaTime);
	}

	if (mSteering->CheckWander())
	{
		mTargetPos = GetCentralPosition() + mVelocity; 
		mVelocity.Truncate(GetMaxSpeed() / 1.05f);

		ChangeRotationStats(0.00872665f, 0.001f);
	}
	else
	{
		ChangeRotationStats(0.0349066f, 0.01f);
	}

	if (mSteering->CheckEvade() || mSteering->CheckWander())
		ChangeMovementStats(0.0f);
	else if (!mSteering->CheckEvade() && !mSteering->CheckWander())
		ChangeMovementStats(0.01f);

	if (mTargetTank)
	{
		if (mTargetTank->GetHealth() <= 0)
			mTargetTank = nullptr;
	}

	//Rotate to face the target
	//////////////////////////////////////////////////////////////////////
	Vector2D toTarget;
	if (mTargetTank)
		toTarget = mTargetTank->GetCentralPosition() - GetCentralPosition();
	else
	{
		if (mTargetPos.isZero())
			mTargetPos = GetCentralPosition();

		toTarget = mTargetPos - GetCentralPosition();
	}

	if (toTarget.LengthSq() > 10 || mSteering->CheckWander() || mSteering->CheckEvade() || !mFuSMActivated)
	{
		if (mVelocity.LengthSq() < 10 && !mFuSMActivated)
			mRotateBlocked = true;

		toTarget = mVelocity;

		if (!mEnemyLookAhead.isZero())
		{
			toTarget = mEnemyLookAhead;
			mEnemyLookAhead.Zero();
		}

		toTarget.Normalize();

		float angle = (float)asin((Cross(toTarget, mHeading) /
			toTarget.Length() * mHeading.Length()));

		if (mCurrentState != TANKSTATE_IDLE)
		{
			mRotationBuffer /= 2;
		}

		if (mSteering->CheckEvade() ||
			ClosestPoint(GetCentralPosition() + mHeading, GetCentralPosition(), toTarget + GetCentralPosition()))
		{
			mMoving = true;
		}

		if (!mRotateBlocked)
		{
			//right
			if (angle >= mRotationBuffer)
			{
				RotateHeadingByRadian(-mRotationAmount, deltaTime);
			}
			//left
			else if (angle <= -mRotationBuffer)
			{
				RotateHeadingByRadian(mRotationAmount, deltaTime);
			}
			else
			{
				Vector2D projForward = GetCentralPosition() + mHeading;
				Vector2D projToTarget = (toTarget + GetCentralPosition()) - projForward;

				if (projToTarget.LengthSq() > toTarget.LengthSq())
					RotateHeadingByRadian(1.0f, deltaTime);
			}
		}
		mRotateBlocked = false;
	}

	//Call parent update.
	BaseTank::Update(deltaTime, e);
}

//--------------------------------------------------------------------------------------------------

void m018585hTank::Render()
{
	if(!mTargetPos.isZero())
		DrawDebugCircle(mTargetPos, 10, 255, 255, 0);

	if (mTargetTank)
		DrawDebugCircle(mTargetPos, 10, 255, 255, 0);

	//debug
	if (!mPath.empty())
	{
		for (unsigned int i = 0; i < mPath.size() - 1; i++)
		{
			DrawDebugLine(mPath.at(i), mPath.at(i + 1), 255, 0, 0);
			DrawDebugCircle(mPath.at(i), 6, 20, 20, 255);
		}
		DrawDebugCircle(mPath.at(mPath.size() - 1), 6, 20, 20, 255);
	}

	vector<Vector2D>* tempPath;
	m018585hFuSM_Run* state1 = (m018585hFuSM_Run*)mFuSM_Manager->GetState(FuSM_RUN);
	if (state1)
	{
		if (state1->IsActivated())
		{
			tempPath = state1->GetPath();
			if (!tempPath->empty())
			{
				for (unsigned int i = 0; i < tempPath->size() - 1; i++)
				{
					DrawDebugLine(tempPath->at(i), tempPath->at(i + 1), 255, 0, 0);
					DrawDebugCircle(tempPath->at(i), 6, 20, 20, 255);
				}
				DrawDebugCircle(tempPath->at(tempPath->size() - 1), 6, 20, 20, 255);
			}
		}
	}

	m018585hFuSM_Attack* state2 = (m018585hFuSM_Attack*)mFuSM_Manager->GetState(FuSM_ATTACK);
	if (state2)
	{
		if (state2->IsActivated())
		{
			tempPath = state2->GetPath();
			if (!tempPath->empty())
			{
				for (unsigned int i = 0; i < tempPath->size() - 1; i++)
				{
					DrawDebugLine(tempPath->at(i), tempPath->at(i + 1), 255, 0, 0);
					DrawDebugCircle(tempPath->at(i), 6, 20, 20, 255);
				}
				DrawDebugCircle(tempPath->at(tempPath->size() - 1), 6, 20, 20, 255);
			}
		}
	}

	m018585hFuSM_Gather* state3 = (m018585hFuSM_Gather*)mFuSM_Manager->GetState(FuSM_GATHER);
	if (state3)
	{
		if (state3->IsActivated())
		{
			tempPath = state3->GetPath();
			if (!tempPath->empty())
			{
				for (unsigned int i = 0; i < tempPath->size() - 1; i++)
				{
					DrawDebugLine(tempPath->at(i), tempPath->at(i + 1), 255, 0, 0);
					DrawDebugCircle(tempPath->at(i), 6, 20, 20, 255);
				}
				DrawDebugCircle(tempPath->at(tempPath->size() - 1), 6, 20, 20, 255);
			}
		}
	}

	m018585hFuSM_ReachEndPoint* state4 = (m018585hFuSM_ReachEndPoint*)mFuSM_Manager->GetState(FuSM_ENDPOINT);
	if (state4)
	{
		if (state4->IsActivated())
		{
			tempPath = state4->GetPath();
			if (!tempPath->empty())
			{
				for (unsigned int i = 0; i < tempPath->size() - 1; i++)
				{
					DrawDebugLine(tempPath->at(i), tempPath->at(i + 1), 255, 0, 0);
					DrawDebugCircle(tempPath->at(i), 6, 20, 20, 255);
				}
				DrawDebugCircle(tempPath->at(tempPath->size() - 1), 6, 20, 20, 255);
			}
		}
	}

	state1 = nullptr;
	delete state1;

	state2 = nullptr;
	delete state2;

	state3 = nullptr;
	delete state3;

	state4 = nullptr;
	delete state4;

	if (!mSteering->GetFeelers().empty())
	{
		for (auto feeler : mSteering->GetFeelers())
		{
			DrawDebugCircle(GetCentralPosition() + feeler.feeler, 6, 100, 100, 255);
		}
	}

	if (!mSteering->GetDebugVectors().empty())
	{
		for (auto vec : mSteering->GetDebugVectors())
		{
			DrawDebugCircle(GetCentralPosition() + vec, 6, 10, 255, 50);
		}
	}

	if (!ObstacleManager::Instance()->GetObstacles().empty())
	{
		for (auto obstacle : ObstacleManager::Instance()->GetObstacles())
		{
			DrawDebugCircle( obstacle->GetCentralPosition(), 5, 255, 50, 255);
		}
	}

	BaseTank::Render();
}

//--------------------------------------------------------------------------------------------------

void m018585hTank::HandleInput(float deltaTime, SDL_Event e)
{
	//player input
	switch (e.type)
	{
		//Deal with mouse click input.
	case SDL_MOUSEBUTTONUP:
		switch (e.button.button)
		{
		case SDL_BUTTON_LEFT:
			//get mouse position
			if (mPath.empty())
			{
				mTargetPos = mMousePos = Vector2D(e.button.x, e.button.y);
				mPathFinder->FindPath(mMousePos, GetCentralPosition(), &mPath);
			}
			else
			{
				mTargetPos = Vector2D();
				mPath.clear();
			}

			break;

		case SDL_BUTTON_RIGHT:
			mTargetPos = mMousePos = GetCentralPosition();
			mSteering->Brake(this, 0.0f, true);
			break;

		default:
			break;
		}
		break;

	case SDL_KEYDOWN:
		switch (e.key.keysym.sym)
		{
			//Fire Cannon
		case SDLK_SPACE:
			ChangeTankState(TANKSTATE_MANFIRE);
			break;
			//Fire Rockets
		case SDLK_f:
			if (mCurrentState != TANKSTATE_CANNONFIRE)
				ChangeTankState(TANKSTATE_CANNONFIRE);
			break;

		case SDLK_1:
			ChangeSteeringState(SS_SEEK);
			break;
		case SDLK_2:
			ChangeSteeringState(SS_ARRIVE);
			break;
		case SDLK_3:
			ChangeSteeringState(SS_PURSUIT);
			break;
		case SDLK_4:
			ChangeSteeringState(SS_PATHFOLLOWING);
			break;
		case SDLK_5:
			ChangeSteeringState(SS_WANDER);
			break;
		case SDLK_6:
			ChangeSteeringState(SS_FLEE);
			break;
		case SDLK_7:
			ChangeSteeringState(SS_EVADE);
			break;

		default:
			break;
		}
		break;

	case SDL_KEYUP:
		ChangeTankState(TANKSTATE_IDLE);
		break;

	default:
		break;
	}
}

//--------------------------------------------------------------------------------------------------

//Most states also turn on obstacle avoidance
void m018585hTank::ChangeSteeringState(STEERING_STATES newState)
{
	//newState is not valid
	if (newState >= SS_MAX)
		return;

	//change steering behaviour
	if (newState == mSteeringState)
		return;

	//alwasy turn these behaviours off
	mSteering->ObstacleAvoidanceOff();
	mSteering->BrakeOff();

	//Exit
	switch (mSteeringState)
	{
	case SS_SEEK:
		cout << "SeekOff" << endl;
		mSteering->SeekOff();
		break;
	case SS_ARRIVE:
		cout << "ArriveOff" << endl;
		mSteering->ArriveOff();
		break; 
	case SS_PURSUIT:
		cout << "PursuitOff" << endl;
		mSteering->PursuitOff();
		break; 
	case SS_PATHFOLLOWING:
		cout << "FollowPathOff" << endl;
		mSteering->FollowPathOff();
		break;
	case SS_WANDER:
		cout << "WanderOff" << endl;
		mSteering->WanderOff();
		break;
	case SS_FLEE:
		cout << "FleeOff" << endl;
		mSteering->FleeOff();
		break;
	case SS_EVADE:
		cout << "EvadeOff" << endl;
		mSteering->EvadeOff();
		break;
	default:
		mSteering->Reset();
		break;
	}

	//Switch
	mSteeringState = newState;

	//Enter
	switch (mSteeringState)
	{
	case SS_SEEK:
		cout << "SeekOn" << endl;
		mSteering->SeekOn();
		mSteering->ObstacleAvoidanceOn();
		break;
	case SS_ARRIVE:
		cout << "ArriveOn" << endl;
		mSteering->ArriveOn();
		mSteering->ObstacleAvoidanceOn();
		mSteering->BrakeOn();
		break;
	case SS_PURSUIT:
		cout << "PursuitOn" << endl;
		mSteering->PursuitOn();
		mSteering->ObstacleAvoidanceOn();
		mSteering->BrakeOn();
		FindEnemyTank();
		break;
	case SS_PATHFOLLOWING:
		cout << "FollowPathOn" << endl;
		//mPathFinderDRS->FindPath(GetCentralPosition(), mTargetPos);
		mSteering->FollowPathOn();
		mSteering->ObstacleAvoidanceOn();
		mSteering->BrakeOn();
		break;
	case SS_WANDER:
		cout << "WanderOn" << endl;
		mSteering->WanderOn();
		mSteering->ObstacleAvoidanceOn();
		break;
	case SS_FLEE:
		cout << "FleeOn" << endl;
		mSteering->FleeOn();
		mSteering->ObstacleAvoidanceOn();
		break;
	case SS_EVADE:
		cout << "EvadeOn" << endl;
		mSteering->EvadeOn();
		mSteering->ObstacleAvoidanceOn();
		mSteering->BrakeOn();
			break;
	}
}

//--------------------------------------------------------------------------------------------------

void m018585hTank::FindEnemyTank()
{
	double currentClosestDistSq = MaxDouble;
	mTargetTank = nullptr;

	//Tanks that can be seen
	for (BaseTank* enemy : mTanksICanSee)
	{
		double dist = Vec2DDistanceSq(GetCentralPosition(), enemy->GetCentralPosition());
		if (dist < currentClosestDistSq)
		{
			currentClosestDistSq = dist;
			mTargetTank = enemy;
		}
	}

	if (!mTargetTank)
	{
		//Tanks that be heard
		for (BaseTank* enemy : mTanksICanHear)
		{
			double dist = Vec2DDistanceSq(GetCentralPosition(), enemy->GetCentralPosition());
			if (dist < currentClosestDistSq)
			{
				currentClosestDistSq = dist;
				mTargetTank = enemy;
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------

void m018585hTank::AttackEnemyTank(float deltaTime)
{
	//exit if no target
	if (!mTargetTank)
	{
		ChangeState(TANKSTATE_IDLE);
		return;
	}

	mMoving = true;

	float dist = (float)Vec2DDistanceSq(GetCentralPosition(), mTargetTank->GetCentralPosition());
	
	Vector2D toTarget = mTargetTank->GetCentralPosition() - GetCentralPosition();
	toTarget.Normalize();

	//Move man to look ahead of enemy
	Vector2D toTLookAhead = (mTargetTank->GetCentralPosition() + (mTargetTank->GetVelocity() * 0.7f)) - GetCentralPosition();
	toTLookAhead.Normalize();

	float angle = (float)asin((Cross(GetManFireDir(), toTLookAhead) /
		GetManFireDir().Length() * toTLookAhead.Length()));

	RotateManByRadian(angle, deltaTime);

	//160^2
	if (dist < 25600)
		ChangeState(TANKSTATE_MANFIRE);

	if (dist < 18225)
	{
		float buffer = 0.10472f;
		
		angle = (float)asin((Cross(toTarget, mHeading) /
			toTarget.Length() * mHeading.Length()));

		if (angle <= buffer && angle >= -buffer)
		{
			ChangeState(TANKSTATE_CANNONFIRE);

			mEnemyLookAhead = toTLookAhead;

			if (mSteering->CheckBrake() && dist < 5625)
				mSteering->Brake(this, 0.97f, false);
		}
	}
}

//--------------------------------------------------------------------------------------------------

void m018585hTank::MoveInHeadingDirection(float deltaTime)
{
	Vector2D newPos = GetPosition();
	Vector2D acceleration = mSteering->Calculate(this) / GetMass();

	mVelocity += acceleration * deltaTime;
	mVelocity.Truncate(GetMaxSpeed());

	//cout << mVelocity.Length() << endl;

	if (mVelocity != mVelocity)
		mVelocity.Zero();

	if (mRotating)
	{
		mRotating = false;

		if (!mMoving)
			mVelocity.Truncate(GetMaxSpeed() / 6);

		//if (mFuSMActivated)
			mVelocity = mVelocity * 0.99f;
	}

	if (mMoving/* || !mFuSMActivated*/)
	{
		mMoving = false;
	
		Vector2D moveBy = mHeading * mVelocity.Length();
		newPos += moveBy * deltaTime;

		//newPos += mVelocity * deltaTime;

		double distToNewPos = Vec2DDistance(GetPosition(), newPos);
		if (distToNewPos > mMovementBuffer)
			SetPosition(newPos);
	}
}

//--------------------------------------------------------------------------------------------------

void m018585hTank::RotateHeadingByRadian(double radian, float deltaTime)
{
	if (mObRotated)
	{
		mObRotated = false;
		return;
	}

	mRotating = true;

	//Clamp the amount to turn to the max turn rate.
	if (radian > mMaxTurnRate)
		radian = mMaxTurnRate;
	else if (radian < -mMaxTurnRate)
		radian = -mMaxTurnRate;

	//IncrementTankRotationAngle(RadsToDegs(radian));
	mRotationAngle += RadsToDegs(radian);

	//Usee a rotation matrix to rotate the player's heading
	C2DMatrix rotMatrix;

	//Calculate the direction of rotation.
	rotMatrix.Rotate(radian);
	//Get the new heading.

	rotMatrix.TransformVector2Ds(mHeading);

	//Get the new velocity.
	rotMatrix.TransformVector2Ds(mVelocity);

	//Side vector must always be perpendicular to the heading.
	mSide = mHeading.Perp();
}

//--------------------------------------------------------------------------------------------------

void m018585hTank::RotateManByRadian(double radian, float deltaTime)
{
	//Clamp the amount to turn to the max turn rate.
	if (radian > mMaxTurnRate)
		radian = mMaxTurnRate;
	else if (radian < -mMaxTurnRate)
		radian = -mMaxTurnRate;

	IncrementManRotationAngle(RadsToDegs(radian));

	//Usee a rotation matrix to rotate the player's heading
	C2DMatrix RotationMatrix;

	//Calculate the direction of rotation.
	RotationMatrix.Rotate(radian);

	//Get the new fire direction.
	RotationMatrix.TransformVector2Ds(mManFireDirection);
}

//--------------------------------------------------------------------------------------------------
#ifndef M018585HFUSM_RUN_H
#define M018585HFUSM_RUN_H

#include "m018585hFuSM_Base.h"
#include "../m018585h_DRS.h"

class m018585hFuSM_Run : public m018585hFuSM_Base
{
public:
	m018585hFuSM_Run();
	~m018585hFuSM_Run();

	float CalculateActivation(m018585hTank& myTank, vector<BaseTank*> seenTanks, vector<BaseTank*> heardTanks) override;

	void Update(m018585hTank& myTank, float deltaTime);

	void Enter(m018585hSteeringBehaviour& steering);
	void Exit(m018585hSteeringBehaviour& steering);

	vector<Vector2D>* GetPath() { return mPath; }

private:
	BaseTank* mTargetTank;
	bool mEvading, dropMine;

	float mineDropDelay, timeSinceDrop;
};
#endif
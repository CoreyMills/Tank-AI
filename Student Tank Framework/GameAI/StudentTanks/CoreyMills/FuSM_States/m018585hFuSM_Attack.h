#ifndef M018585HFUSM_ATTACK_H
#define M018585HFUSM_ATTACK_H

#include "m018585hFuSM_Base.h"
#include "../m018585h_DRS.h"

class m018585hFuSM_Attack : public m018585hFuSM_Base
{
public:
	m018585hFuSM_Attack();
	~m018585hFuSM_Attack();

	float CalculateActivation(m018585hTank& myTank, vector<BaseTank*> seenTanks, vector<BaseTank*> heardTanks) override;
	//float CalculateActivation(m018585hTank& myTank, vector<BaseTank*> seenTanks);

	void Update(m018585hTank& myTank, float deltaTime);

	void Enter(m018585hSteeringBehaviour& steering);
	void Exit(m018585hSteeringBehaviour& steering);

	vector<Vector2D>* GetPath() { return mPath; }

private:
	BaseTank* mTargetTank;

	float mPathResetDelay, mPathResetTimeWaited;
};
#endif
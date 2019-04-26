#ifndef M018585HFUSM_OBAVOIDANCE_H
#define M018585HFUSM_OBAVOIDANCE_H

#include "m018585hFuSM_Base.h"
#include "../../ObstacleManager.h"

class m018585hFuSM_ObAvoidance : public m018585hFuSM_Base
{
public:
	m018585hFuSM_ObAvoidance();
	~m018585hFuSM_ObAvoidance() override;

	float CalculateActivation(m018585hTank& myTank, vector<BaseTank*> seenTanks, vector<BaseTank*> heardTanks) override;

	void Update(m018585hTank& myTank, float deltaTime);

	void Enter(m018585hSteeringBehaviour& steering);
	void Exit(m018585hSteeringBehaviour& steering);

private:
	Vector2D mOldPos;
};
#endif
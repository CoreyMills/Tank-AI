#ifndef M018585HFUSM_SEARCH_H
#define M018585HFUSM_SEARCH_H

#include "m018585hFuSM_Base.h"

class m018585hFuSM_Search : public m018585hFuSM_Base
{
public:
	m018585hFuSM_Search();
	~m018585hFuSM_Search();

	float CalculateActivation(m018585hTank& myTank, vector<BaseTank*> seenTanks, vector<BaseTank*> heardTanks) override;

	void Update(m018585hTank& myTank, float deltaTime);

	void Enter(m018585hSteeringBehaviour& steering);
	void Exit(m018585hSteeringBehaviour& steering);

private:

};
#endif
#include<iostream>
#include<vector>
#include"define.h"

using namespace std;

void joinbs(UE* u, BS* targetbs, double T)
{
	u->connecting_BS = targetbs;
	targetbs->connectingUE.push_back(u);
	targetbs->lambda += u->lambdai;
	targetbs->systemT = T;
}

void findbs_minT(UE *u, vector <BS> *bslist)
{
	double minT=0;
	BS *targetbs = NULL;
	for (int i = 0; i < bslist->size(); i++)
	{
		int CQI = calc_CQI(u, &bslist->at(i));
		if (CQI == 0)
			continue;
		double T = predictT(u, &bslist->at(i));
		if (T == -1)
			continue;
		if (targetbs == NULL)
		{
			targetbs = &bslist->at(i);
			minT = T;
		}
		else
		{
			if (T < minT)
			{
				targetbs = &bslist->at(i);
				minT = T;
			}
			else
			{
				if (T == minT)
				{
					double capacity = getCapacity(u, &bslist->at(i));
					double capacity_targetbs = getCapacity(u, targetbs);
					if (capacity > capacity_targetbs)
					{
						targetbs = &bslist->at(i);
						minT = T;
					}
				}
			}
		}
	}
	if (targetbs != NULL)
		joinbs(u, targetbs, minT);
}
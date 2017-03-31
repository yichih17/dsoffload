#include<iostream>
#include<vector>
#include"define.h"

using namespace std;

void joinbs(UE* u, BS* targetbs, double T)
{
	u->connecting_BS = targetbs;
	u->CQI = get_CQI(u, targetbs);
	targetbs->connectingUE.push_back(u);
	targetbs->lambda += u->lambdai;
	targetbs->systemT = T;
}

void findbs_minT(UE *u, vector <BS> *bslist)
{
	double minT = 0;
	int targetBS_CQI = 0;
	BS *targetBS = NULL;
	for (int i = 0; i < bslist->size(); i++)
	{
		int CQI = get_CQI(u, &bslist->at(i));
		if (CQI == 0)
			continue;
		double T = predict_T(u, &bslist->at(i), CQI);
		if (T == -1)
			continue;
		if (targetBS == NULL)
		{
			targetBS = &bslist->at(i);
			minT = T;
			targetBS_CQI = CQI;
		}
		else
		{
			if (T < minT)
			{
				targetBS = &bslist->at(i);
				minT = T;
				targetBS_CQI = CQI;
			}
			else
			{
				if (T == minT)
				{
					double capacity = predict_C(u, &bslist->at(i), CQI);
					double capacity_targetbs = predict_C(u, targetBS, targetBS_CQI);
					if (capacity > capacity_targetbs)
					{
						targetBS = &bslist->at(i);
						minT = T;
						targetBS_CQI = CQI;
					}
					else
					{
						if (capacity == capacity_targetbs)
						{
							double distance = get_distance(u, &bslist->at(i));
							double distance_targetBS = get_distance(u, targetBS);
							if (distance < distance_targetBS)
							{
								targetBS = &bslist->at(i);
								minT = T;
								targetBS_CQI = CQI;
							}
						}
					}
				}
			}
		}
	}
	if (targetBS != NULL)
		joinbs(u, targetBS, minT);
}
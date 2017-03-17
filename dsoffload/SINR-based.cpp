#include"define.h"
#include<iostream>
#include<vector>
#include<math.h>

using namespace std;

void joinbs(UE* u, BS* targetbs);
double calc_T(BS* b);

void findbs_sinr(UE *u, vector <BS> *bslist)
{
	BS *targetbs = NULL;
	double minPL = 0;
	for (int i = 0; i < bslist->size(); i++)
	{
		double distance = calc_distance(u, &bslist->at(i)) / 1000;
		double pathloss;
		if (bslist->at(i).type == macro)
		{
			if (distance > range_macro[0] / 1000.0)
				continue;
			pathloss = 128.1 + 37.6 * log10(distance);
		}			
		else
		{
			if (distance > range_ap[0] / 1000.0)
				continue;
			pathloss = 140.1 + 36.7 * log10(distance);
		}
			
		if (targetbs == NULL)
		{
			minPL = pathloss;
			targetbs = &bslist->at(i);
		}			
		else
		{
			if (pathloss < minPL)
			{
				minPL = pathloss;
				targetbs = &bslist->at(i);
			}
		}
	}
	joinbs(u, targetbs);
	return;
}

void joinbs(UE* u, BS* targetbs)
{
	u->connecting_BS = targetbs;
	targetbs->connectingUE.push_back(u);
	targetbs->lambda += u->lambdai;
	targetbs->systemT = calc_T(targetbs);
}

double calc_T(BS* b)
{
	double T = 0;
	double Xj = 0, Xj2 = 0;
	for (int i = 0; i < b->connectingUE.size(); i++)
	{
		Xj += b->connectingUE.at(i)->packet_size / getCapacity(b->connectingUE.at(i)) * (b->connectingUE.at(i)->lambdai / b->lambda);
		Xj2 += pow(b->connectingUE.at(i)->packet_size / getCapacity(b->connectingUE.at(i)), 2) * (b->connectingUE.at(i)->lambdai / b->lambda);
	}		
	double rho = b->lambda * Xj;
	if (rho > 0.95)
		T = Xj + b->lambda * Xj2 / (1 - 0.95);
	else
		T = Xj + b->lambda * Xj2 / (1 - rho);
	return T;
}
#include"define.h"
#include<iostream>
#include<vector>
#include<math.h>
#define T_upper_bound 3000

using namespace std;

void joinbs(UE* u, BS* targetbs);
double calc_T(BS* b);

void findbs_sinr(UE *u, vector <BS> *bslist)
{
	BS *targetbs = NULL;
	double max_received_power = 0;
	for (int i = 0; i < bslist->size(); i++)
	{
		double distance = get_distance(u, &bslist->at(i)) / 1000;
		double receiced_power;
		if (bslist->at(i).type == macro)
		{
			if (distance > range_macro[0] / 1000.0)
				continue;
			receiced_power = power_macro - (128.1 + 37.6 * log10(distance));
		}			
		else
		{
			if (distance > range_ap[0] / 1000.0)
				continue;
			receiced_power = power_ap - (140.1 + 36.7 * log10(distance));
		}
			
		if (targetbs == NULL)
		{
			max_received_power = receiced_power;
			targetbs = &bslist->at(i);
		}			
		else
		{
			if (receiced_power > max_received_power)
			{
				max_received_power = receiced_power;
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
	u->CQI = get_CQI(u, targetbs);
	targetbs->connectingUE.push_back(u);
	targetbs->lambda += u->lambdai;
	targetbs->systemT = calc_T(targetbs);
}

double calc_T(BS* b)
{
	if (b->systemT == T_upper_bound)
		return T_upper_bound;
	double T = 0;
	double Xj = 0, Xj2 = 0;
	for (int i = 0; i < b->connectingUE.size(); i++)
	{
		Xj += b->connectingUE.at(i)->packet_size / get_C(b->connectingUE.at(i)) * (b->connectingUE.at(i)->lambdai / b->lambda);
		Xj2 += pow(b->connectingUE.at(i)->packet_size / get_C(b->connectingUE.at(i)), 2) * (b->connectingUE.at(i)->lambdai / b->lambda);
	}		
	double rho = b->lambda * Xj;
	if (rho > 0.999)
		T = T_upper_bound;
	else
		T = Xj + b->lambda * Xj2 / (1 - rho);
	return T;
}
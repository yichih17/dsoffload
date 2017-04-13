#include"define.h"
#include<iostream>
#include<vector>
#include<math.h>
#include<algorithm>

#define T_upper_bound 3000

using namespace std;

int max_power(vector <double> power);
void joinBS_simple(UE* u, BS* targetbs, double T);
double calc_T(BS* b);

void findbs_sinr(UE *u, vector <BS> *bslist)
{
	vector <BS*> availbs;
	vector <double> P_received;
	for (int i = 0; i < bslist->size(); i++)
	{
		double distance = get_distance(u, &bslist->at(i)) / 1000;
		double receiced_power;
		if (bslist->at(i).type == macro)
		{
			if (distance > range_macro[0] / 1000.0)
				continue;
			availbs.push_back(&bslist->at(i));
			receiced_power = power_macro - (128.1 + 37.6 * log10(distance));
			P_received.push_back(receiced_power);
		}
		else
		{
			if (distance > range_ap[0] / 1000.0)
				continue;
			availbs.push_back(&bslist->at(i));
			receiced_power = power_ap - (140.1 + 36.7 * log10(distance));
			P_received.push_back(receiced_power);
		}
	}

	while(P_received.size()!=0)
	{
		int max_index = max_power(P_received);
		double T = predict_T(u, availbs[max_index]);
		//T method
		if (T != -1)
		{
			joinBS_simple(u, availbs[max_index], T);
			break;
		}
		else
		{
			P_received.erase(P_received.begin() + max_index);
			availbs.erase(availbs.begin() + max_index);
		}		
	}
	return;
}

int max_power(vector <double> power)
{
	int max_index = 0;
	double max_power = power[0];
	for (int i = 1; i < power.size(); i++)
	{
		if (power[i] > max_power)
		{
			max_index = i;
			max_power = power[i];
		}
	}
	return max_index;
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
	if (rho > rho_max)
		T = T_upper_bound;
	else
		T = Xj + b->lambda * Xj2 / (1 - rho);
	return T;
}
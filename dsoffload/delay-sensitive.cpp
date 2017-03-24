#include<iostream>
#include<vector>
#include"define.h"

using namespace std;


double predict_T(UE* u, BS* b);

bool findbs_dso_test(UE* u, connection_status* cs, int depth)
{
	vector <BS*> influence_bs;
	vector <BS*> no_influence_bs;
	vector <BS*> saturated_bs;
	vector <double> influence_bs_T;
	vector <double> no_influence_bs_T;
	vector <double> saturated_bs_T;

	for (int i = 0; i < cs->bslist.size(); i++)
	{
		double T = predict_T(u, &cs->bslist.at(i));
		if (T == 0)
			continue;
		else
		{
			u->availBS.push_back(&cs->bslist.at(i));
			if (T == -1)
			{
				saturated_bs.push_back(&cs->bslist.at(i));
				saturated_bs_T.push_back(T);
			}
			else
			{
				
			}
		}
	}
	return true;
}

double predict_T(UE* u, BS* b)
{
	int CQI = calc_CQI(u, b);
	if (CQI == 0)
		return 0;
	//試算u加入b後的lambda
	double lambda = b->lambda + u->lambdai;
	//計算u加入b後的Xj
	double Xj = 0;
	double Xj2 = 0;
	for (int i = 0; i < b->connectingUE.size(); i++)	//原本在b的UE的Xij加起來
	{
		double pktsize_i = b->connectingUE[i]->packet_size;
		double capacity_i = getCapacity(b->connectingUE[i], b) / (b->connectingUE.size() + 1);
		double weight = b->connectingUE[i]->lambdai / lambda;
		Xj += pktsize_i / capacity_i * weight;
		Xj2 += pow(pktsize_i / capacity_i, 2) * weight;
	}
	Xj += u->packet_size / (getCapacity(u, b) / (b->connectingUE.size() + 1)) * (u->lambdai / lambda);
	//When Rho > 1, return -1;
	double rho = Xj * lambda;
	if (rho >= 0.999)
		return -1;
	Xj2 += pow(u->packet_size / (getCapacity(u, b) / (b->connectingUE.size() + 1)), 2) * (u->lambdai / lambda);
	//用M/G/1公式算T
	return Xj + lambda * Xj2 / (1 - lambda * Xj);
}
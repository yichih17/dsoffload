#include<iostream>
#include "define.h"

int macro_range[] = { 1732, 1511, 1325, 1162, 1019, 894, 784, 688, 623, 565, 512, 449, 407, 357, 303 };
int ap_range[] = { 82, 68, 60, 50, 39, 30, 28, 26 };
double macro_eff[15] = { 0.1523, 0.2344, 0.377, 0.6016, 0.8770, 1.1758, 1.4766, 1.9141, 2.4063, 2.7305, 3.3223, 3.9023, 4.5234, 5.1152, 5.5546 };
double ap_capacity[8] = { 6500, 13000, 19500, 26000, 39000, 52000, 58500, 65000 };

double getdis(UE* u, BS* b)
{
	return sqrt(pow((u->coor_X - b->coor_X), 2) + pow((u->coor_Y - b->coor_Y), 2));
}

int getCQI2(UE* u, BS* b)
{
	double dis = getdis(u, b);
	int CQI;
	if (b->type == macro)	//計算LTE的CQI
		for (int i = 0; i < 15; i++)
		{
			if (dis < macro_range[i])
				CQI = i + 1;
			else
				return CQI;
		}
	if (b->type == ap)		//計算Wifi的CQI
		for (int i = 0; i < 8; i++)
		{
			if (dis < ap_range[i])
				CQI = i + 1;
			else
				return CQI;
		}
	return 16;				//出錯
}

/*試算Capacity*/
double predictCapa(UE* u, BS* b)
{
	int CQI = getCQI2(u, b);
	if (b->type == macro)
		return ((resource_element * macro_eff[CQI] * total_RBG) / (b->connectingUE.size() + 1));
	if (b->type == ap)
		return ap_capacity[CQI] / (b->connectingUE.size() + 1);
	return 0;				//出錯
}

/*已連接BS的UE的Capacity*/
double getCapa(UE* u)
{
	int CQI = getCQI2(u, u->connecting_BS);
	if (u->connecting_BS->type == macro)
		return ((resource_element * macro_eff[CQI] * total_RBG) / u->connecting_BS->connectingUE.size());
	if (u->connecting_BS->type == ap)
		return ap_capacity[CQI] / u->connecting_BS->connectingUE.size();
}

/*試算T*/
double predictT(UE* u, BS* b)
{
	double Xj = 0;
	for (int i = 0; i < b->connectingUE.size(); i++)
		Xj += (b->connectingUE[i]->psize / predictCapa(b->connectingUE[i], b) * (b->connectingUE[i]->lambdai / (b->lambda + u->lambdai)));
	Xj += (u->psize / predictCapa(u, b) * (u->lambdai / (b->lambda + u->lambdai)));
	double Xj2 = 0;
	for (int i = 0; i < b->connectingUE.size(); i++)
		Xj2 += pow((b->connectingUE[i]->psize / predictCapa(b->connectingUE[i], b)), 2) * (b->connectingUE[i]->lambdai / (b->lambda + u->lambdai));
	Xj2 += pow((u->psize / predictCapa(u, b)), 2) * (u->lambdai / (b->lambda + u->lambdai));
	double T = 0;
	T = Xj + (b->lambda + u->lambdai)*Xj2 / (1 - (b->lambda + u->lambdai)*Xj);
	return T;
}

/*BS的T*/
double getT(BS* b)
{
	double Xj = 0;
	for (int i = 0; i < b->connectingUE.size(); i++)
		Xj += (b->connectingUE[i]->psize / getCapa(b->connectingUE[i]) * (b->connectingUE[i]->lambdai / b->lambda));
	double Xj2 = 0;
	for (int i = 0; i < b->connectingUE.size(); i++)
		Xj2 += pow((b->connectingUE[i]->psize / getCapa(b->connectingUE[i])), 2) * (b->connectingUE[i]->lambdai / b->lambda);
	double T = 0;
	T = Xj + (b->lambda * Xj2) / (1 - b->lambda * Xj);
	return T;
}

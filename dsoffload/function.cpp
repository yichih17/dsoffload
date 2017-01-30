#include<iostream>
#include "define.h"

using namespace std;

int macro_range[] = { 1732, 1511, 1325, 1162, 1019, 894, 784, 688, 623, 565, 512, 449, 407, 357, 303 };
int ap_range[] = { 82, 68, 60, 50, 39, 30, 28, 26 };
double macro_eff[15] = { 0.1523, 0.2344, 0.377, 0.6016, 0.8770, 1.1758, 1.4766, 1.9141, 2.4063, 2.7305, 3.3223, 3.9023, 4.5234, 5.1152, 5.5546 };
double ap_capacity[8] = { 6500, 13000, 19500, 26000, 39000, 52000, 58500, 65000 };

double getdis(UE* u, BS* b)
{
	return sqrt(pow((u->coor_X - b->coor_X), 2) + pow((u->coor_Y - b->coor_Y), 2));
}

int getCQI(UE* u, BS* b)
{
	double dis = getdis(u, b);
	int CQI = -1;
	if (b->type == macro)	//�p��LTE��CQI
	{
		for (int i = 0; i < 15; i++)
		{
			if (dis <= macro_range[i])
				CQI = i + 1;
			else
				return CQI;
		}
		return CQI;
	}		
	if (b->type == ap)		//�p��Wifi��CQI
	{
		for (int i = 0; i < 8; i++)
		{
			if (dis <= ap_range[i])
				CQI = i + 1;
			else
				return CQI;
		}
		return CQI;
	}		
	return 16;				//�X��
}

/*�պ�Capacity*/
double predictCapa(UE* u, BS* b)
{
	int CQI = getCQI(u, b);
	double capa;
	if (b->type == macro)
		capa = resource_element * macro_eff[CQI-1] * (total_RBG / (b->connectingUE.size() + 1));
	if (b->type == ap)
		capa = ap_capacity[CQI-1] / (b->connectingUE.size() + 1);
	return capa;
}

/*��CQI���`Capacity*/
double getCapa(UE* u, BS* b)
{
	int CQI = getCQI(u, b);
	double capa;
	if (b->type == macro)
		capa = resource_element * macro_eff[CQI-1] * total_RBG;
	if (b->type == ap)
		capa = ap_capacity[CQI-1];
	return capa;
}

/*�պ�T*/
double predictT(UE* u, BS* b)
{
//�p��u�[�Jb�᪺lambda
	double newlambda = b->lambda + u->lambdai;
//�p��u�[�Jb�᪺Xj
	double Xj = 0;
	for (int i = 0; i < b->connectingUE.size(); i++)	//�쥻�bb��UE��Xij�[�_��
	{
		double psize = b->connectingUE[i]->psize;
		double capacity = getCapa(b->connectingUE[i], b) / (b->connectingUE.size() + 1);  //�`capacity �� UE��
		double Xij = psize/capacity ;
		Xj += Xij * b->connectingUE[i]->lambdai / newlambda;
	}
	Xj += u->psize / (getCapa(u, b) / (b->connectingUE.size() + 1)) * (u->lambdai / newlambda);
//�p��u�[�Jb�᪺Xj^2
	double Xj2 = 0;
	for (int i = 0; i < b->connectingUE.size(); i++)	//�쥻�bb��UE��Xij2�[�_��
	{
		double psize = b->connectingUE[i]->psize;
		double capacity = getCapa(b->connectingUE[i], b) / (b->connectingUE.size() + 1);  //�`capacity �� UE��
		double Xij2 = pow(psize / capacity, 2);
		Xj2 += Xij2*b->connectingUE[i]->lambdai / newlambda;
	}
	Xj2 += pow((u->psize / (getCapa(u, b) / (b->connectingUE.size() + 1))), 2) * (u->lambdai / newlambda);
//��M/G/1������T
	double T;
	T = Xj + newlambda*Xj2 / (1 - newlambda*Xj);
	return T;
}

//UE�M��X�A��BS
BS* findbs(UE* u)
{
	for (int i = 0; i < vbslist.size(); i++)
	{
		int CQI = getCQI(u, &vbslist[i]);
		switch (CQI)
		{
		case 16:
			cout << "CQI error, BS type is neither macro nor ap." << endl;
			break;
		case -1:
			break;
		case 0:
			cout << "UE" << u->UEnum << " to BS" << vbslist[i].BSnum << " CQI is 0" << endl;
			break;
		default:
			u->availBS.push_back(&vbslist[i]);
			break;
		}
	}
	BS* minTbs = u->availBS[0];
	double T_minTbs = predictT(u, u->availBS[0]); 
	double C_minTbs = predictCapa(u, u->availBS[0]);

	for (int i = 1; i < u->availBS.size(); i++)
	{
		double T, C;
		T = predictT(u, u->availBS[i]);
		C = predictCapa(u, u->availBS[i]);
		//T����p
		if (T < T_minTbs)
		{
			T_minTbs = T;
			C_minTbs = C;
			minTbs = u->availBS[i];
		}
		//T�ۦP�A��C
		if (T == T_minTbs && C < C_minTbs)
		{
			T_minTbs = T;
			C_minTbs = C;
			minTbs = u->availBS[i];
		}
	}
	return minTbs;
}

void refresh()
{
	for (int i = 0; i < vbslist.size(); i++)
	{
		double lambda = 0;
		for (int j = 0; j < vbslist[i].connectingUE.size(); j++)
			lambda += vbslist[i].connectingUE[j]->lambdai;
	}
}
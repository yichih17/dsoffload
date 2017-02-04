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
double predictCapacity(UE* u, BS* b)
{
	if (b->type == macro)
		return resource_element * macro_eff[getCQI(u, b) - 1] * total_RBG / (b->connectingUE.size() + 1);
	if (b->type == ap)
		return ap_capacity[getCQI(u, b) - 1] / (b->connectingUE.size() + 1);
	return 0;
}

/*�p��UE�P�ثe�s��BS��System Capacity(UE�֦��Ҧ�RB)*/
double getCapacity(UE* u, BS* b)
{
	if (b->type == macro)
		return resource_element * macro_eff[getCQI(u, b) - 1] * total_RBG;
	if (b->type == ap)
		return ap_capacity[getCQI(u, b) - 1];
	return 0;
}

/* �p��UE�P�ثe�s��BS��Capacity */
double getCapacity(UE* u)
{
	if (u->connecting_BS = NULL)
		cout << "Error, UE" << u->UEnum << " no connecting BS.\n";
	else
	{
		if (u->connecting_BS->type == macro)
			return resource_element * macro_eff[getCQI(u, u->connecting_BS) - 1] * total_RBG / u->connecting_BS->connectingUE.size();
		if (u->connecting_BS->type == ap)
			return ap_capacity[getCQI(u, u->connecting_BS) - 1] / u->connecting_BS->connectingUE.size();
	}
	return 0;
}

/* �պ�UE�[�JBS��ABS��Avg. system time(T*) */
double predictT(UE* u, BS* b)
{
	//�p��u�[�Jb�᪺lambda
	double lambda = b->lambda + u->lambdai;
	//�p��u�[�Jb�᪺Xj
	double Xj = 0;
	for (int i = 0; i < b->connectingUE.size(); i++)	//�쥻�bb��UE��Xij�[�_��
	{
		double packetsize = b->connectingUE[i]->packet_size;
		double capacity = getCapacity(b->connectingUE[i], b) / (b->connectingUE.size() + 1);
		double weight = b->connectingUE[i]->lambdai / lambda;
		Xj += packetsize / capacity * weight;
	}
//	for (int i = 0; i < b->connectingUE.size(); i++)	//�쥻�bb��UE��Xij�[�_��
//		Xj += b->connectingUE[i]->psize / (getCapacity(b->connectingUE[i], b) / (b->connectingUE.size() + 1)) * b->connectingUE[i]->lambdai / lambda;
	double packetsize_u = u->packet_size;
	double capacity_u = predictCapacity(u, b);
	double capacity_test = getCapacity(u, b) / (b->connectingUE.size() + 1);
	double weight = u->lambdai / lambda;
	Xj += packetsize_u / capacity_u * weight;
//	Xj += u->psize / predictCapacity(u, b) * (u->lambdai / lambda);
	//�p��u�[�Jb�᪺Xj^2
	double Xj2 = 0;
	for (int i = 0; i < b->connectingUE.size(); i++)	//�쥻�bb��UE��Xij2�[�_��
		Xj2 += pow(b->connectingUE[i]->packet_size / (getCapacity(b->connectingUE[i], b) / (b->connectingUE.size() + 1)), 2) * b->connectingUE[i]->lambdai / lambda;
	Xj2 += pow(u->packet_size / predictCapacity(u, b), 2) * (u->lambdai / lambda);
	//��M/G/1������T
	return Xj + lambda * Xj2 / (1 - lambda*Xj);
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

	//�w�]�̨�BS��macro eNB
	BS* minTbs = u->availBS[0];
	double T_minTbs = predictT(u, u->availBS[0]);

	double T;
	for (int i = 1; i < u->availBS.size(); i++)
	{
		T = predictT(u, u->availBS[i]);
		//T����p
		if (T < T_minTbs)
		{
			T_minTbs = T;
			minTbs = u->availBS[i];
		}
		//T�ۦP�A��C
		if (T == T_minTbs)
		{
			double C_minTbs = predictCapacity(u, minTbs);
			double C = predictCapacity(u, u->availBS[i]);
			if (C < C_minTbs)
			{
				T_minTbs = T;
				minTbs = u->availBS[i];
			}			
		}
	}
	return minTbs;
}

void BSsystemTupdate(BS* b)
{
	b->lambda = 0;
	for (int i = 0; i < b->connectingUE.size(); i++)
		b->lambda += b->connectingUE[i]->lambdai;
	double Xj = 0;
	for (int i = 0; i < b->connectingUE.size(); i++)
	{
		//Xij = packet_size / capacity
		//Xj = sigma( Xij*(lambdai/lambda) )
		Xj += b->connectingUE[i]->packet_size / (getCapacity(b->connectingUE[i], b) / b->connectingUE.size()) * (b->connectingUE[i]->lambdai / b->lambda);
	}

}

void BSbaddUEu(UE* u, BS* b)
{
	if (u->connecting_BS != NULL)
	{
		cout << "UE���XBS" << u->connecting_BS->BSnum << "�e:\nBS��" << u->connecting_BS->connectingUE.size() << "��UE" << endl;
		u->connecting_BS->lambda -= u->lambdai;
		int i;
		for (i = 0; i < u->connecting_BS->connectingUE.size(); i++)
		{
			if (u->connecting_BS->connectingUE[i]->UEnum == u->UEnum)
				break;
		}
		u->connecting_BS->connectingUE.erase(u->connecting_BS->connectingUE.begin() + i - 1);
		cout << "UE���XBS" << u->connecting_BS->BSnum << "��:\nBS��" << u->connecting_BS->connectingUE.size() << "��UE" << endl;
	}
	u->connecting_BS = b;
	b->lambda += u->lambdai;
	b->connectingUE.push_back(u);
	//BSsystemTupdate(b);
}
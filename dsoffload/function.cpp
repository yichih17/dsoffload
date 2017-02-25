#include <iostream>
#include <math.h>
#include <algorithm>
#include "define.h"

using namespace std;

int range_macro[] = { 1732, 1511, 1325, 1162, 1019, 894, 784, 688, 623, 565, 512, 449, 407, 357, 303 };
int range_ap[] = { 185, 152, 133, 109, 84, 64, 60, 56 };
double macro_eff[15] = { 0.1523, 0.2344, 0.377, 0.6016, 0.8770, 1.1758, 1.4766, 1.9141, 2.4063, 2.7305, 3.3223, 3.9023, 4.5234, 5.1152, 5.5546 };
double ap_capacity[8] = { 6500, 13000, 19500, 26000, 39000, 52000, 58500, 65000 };	//65Mbps = 65000000bps = 65000 bits/ms

//�p��AP���T���d��
//void countAPrange()
//{
//	int SINR_AP[8] = { 4, 7, 9, 12, 16, 20, 21, 22 };
//	for (int i = 0; i < 8; i++)
//	{
//		//Path loss: 140.7 + 36.7 log(D), D in km.
//		double distance = pow(10, (-(SINR_AP[i] - 78 - power_ap) - 122.7) / 35.1) * 1000;
//		range_ap[i] = (int)distance;
//	}
//	//Result:185, 152, 133, 109, 84, 64, 60, 56
//}

//�p��UE�PBS�����Z��
double getDistance(UE* u, BS* b)
{
	return sqrt(pow((u->coor_X - b->coor_X), 2) + pow((u->coor_Y - b->coor_Y), 2));
}

//�p��UE�PBS��Channel Quality
int getCQI(UE* u, BS* b)
{
	double dis = getDistance(u, b);
	int CQI = -1;
	if (b->type == macro)	//�p��LTE��CQI
	{
		for (int i = 0; i < 15; i++)
		{
			if (dis <= range_macro[i])
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
			if (dis <= range_ap[i])
				CQI = i + 1;
			else
				return CQI;
		}
		return CQI;
	}		
	return 16;				//�X��
}

//��sUE�i�s����BS�M��
void availbs(UE* u, vector<BS> *bslist)
{
	//�p�GUE��availbs�w�g����ơA�M���ç�s
	if (u->availBS.size() != 0)
		u->availBS.clear();
	//�p�GUE�w�s��BS�Aavailbs�N���ӥX�{�w�s����BS
	if (u->connecting_BS != NULL)
	{
		for (int i = 0; i < bslist->size(); i++)
		{
			if (u->connecting_BS->num == bslist->at(i).num)
				continue;
			int CQI = getCQI(u, &bslist->at(i));
			switch (CQI)
			{
			case 16:
				cout << "CQI error, BS type is neither macro nor ap." << endl;
				break;
			case -1:
				break;
			default:
				u->availBS.push_back(&bslist->at(i));
				break;
			}
		}
	}
	else
	{
		for (int i = 0; i < bslist->size(); i++)
		{
			int CQI = getCQI(u, &bslist->at(i));
			switch (CQI)
			{
			case 16:
				cout << "CQI error, BS type is neither macro nor ap." << endl;
				break;
			case -1:
				break;
			default:
				u->availBS.push_back(&bslist->at(i));
				break;
			}
		}
	}
}

//UE�[�JBS�e�A����savailbs	*newbs���Y�N�n�[�J��BS
void availbs_update(UE* u, BS* newb)
{
	//�n���ثe��BS�F�A�ҥH�ثe��BS�|�ܦ�availbs
	if (u->connecting_BS != NULL)
		u->availBS.push_back(u->connecting_BS);
	int delete_bs;		//newbs�bavailbs������m
	for (delete_bs = 0; delete_bs < u->availBS.size(); delete_bs++)
		if (u->availBS[delete_bs]->num == newb->num)
			break;
	u->availBS.erase(u->availBS.begin() + delete_bs);
}

//�պ�UE�[�JBS�᪺Capacity
double predict_Capacity(UE* u, BS* b)
{
	if (b->type == macro)
		return resource_element * macro_eff[getCQI(u, b) - 1] * total_RBG / (b->connectingUE.size() + 1);
	if (b->type == ap)
		return ap_capacity[getCQI(u, b) - 1] / (b->connectingUE.size() + 1);
	return -1;
}

//�p��UE�PBS��System Capacity
double getCapacity(UE* u, BS* b)
{
	if (b->type == macro)
		return resource_element * macro_eff[getCQI(u, b) - 1] * total_RBG;
	if (b->type == ap)
		return ap_capacity[getCQI(u, b) - 1];
	return -1;
}

//�p��UE�P�ثe�s��BS��Capacity
double getCapacity(UE* u)
{
	if (u->connecting_BS == NULL)
		cout << "Error, UE" << u->num << " no connecting BS.\n";
	else
	{
		if (u->connecting_BS->type == macro)
			return resource_element * macro_eff[getCQI(u, u->connecting_BS) - 1] * total_RBG / u->connecting_BS->connectingUE.size();
		if (u->connecting_BS->type == ap)
			return ap_capacity[getCQI(u, u->connecting_BS) - 1] / u->connecting_BS->connectingUE.size();
	}
	return -1;
}

/* �պ�UE�[�JBS��ABS��Avg. system time(T*) */
double predictT(UE* u, BS* b)
{
	//�պ�u�[�Jb�᪺lambda
	double lambda = b->lambda + u->lambdai;
	//�p��u�[�Jb�᪺Xj
	double Xj = 0;

/*	for (int i = 0; i < b->connectingUE.size(); i++)	//�쥻�bb��UE��Xij�[�_��
	{
		double packetsize = b->connectingUE[i]->packet_size;
		double capacity = getCapacity(b->connectingUE[i], b) / (b->connectingUE.size() + 1);
		double weight = b->connectingUE[i]->lambdai / lambda;
		Xj += packetsize / capacity * weight;
	}
	double packetsize_u = u->packet_size;
	double capacity_u = predict_Capacity(u, b);
	double weight = u->lambdai / lambda;
	Xj += packetsize_u / capacity_u * weight;*/

	for (int i = 0; i < b->connectingUE.size(); i++)	//�쥻�bb��UE��Xij�[�_��
		Xj += b->connectingUE[i]->packet_size / (getCapacity(b->connectingUE[i], b) / (b->connectingUE.size() + 1)) * b->connectingUE[i]->lambdai / lambda;
	Xj += u->packet_size / (getCapacity(u, b) / (b->connectingUE.size() + 1)) * (u->lambdai / lambda);
	//if (Xj * lambda > 1)								//When Rho > 1, return -1;
	//	return -1;
	//�p��u�[�Jb�᪺Xj^2
	double Xj2 = 0;
	for (int i = 0; i < b->connectingUE.size(); i++)	//�쥻�bb��UE��Xij2�[�_��
		Xj2 += pow(b->connectingUE[i]->packet_size / (getCapacity(b->connectingUE[i], b) / (b->connectingUE.size() + 1)), 2) * b->connectingUE[i]->lambdai / lambda;
	Xj2 += pow(u->packet_size / (getCapacity(u, b) / (b->connectingUE.size() + 1)), 2) * (u->lambdai / lambda);

	//��M/G/1������T
	double T = Xj + lambda * Xj2 / (1 - lambda * Xj);
	if (Xj * lambda > 1)								//When Rho > 1, return -1;
		return -1;
	return Xj + lambda * Xj2 / (1 - lambda * Xj);
}

//�p��BS��avg system time(delay)
double getT(BS* b)
{
	double Xj = 0;		//�bBS���U��UE��avg service time
	for (int i = 0; i < b->connectingUE.size(); i++)
		Xj += b->connectingUE[i]->packet_size / (getCapacity(b->connectingUE[i], b) / b->connectingUE.size()) * b->connectingUE[i]->lambdai / b->lambda;
	double Xj2 = 0;		//�bBS���U��UE��avg Xj2
	for (int i = 0; i < b->connectingUE.size(); i++)	
		Xj2 += pow(b->connectingUE[i]->packet_size / (getCapacity(b->connectingUE[i], b) / b->connectingUE.size()), 2) * b->connectingUE[i]->lambdai / b->lambda;
	return Xj + b->lambda * Xj2 / (1 - b->lambda * Xj);
}

//UE�M��X�A��BS
BS* findbs_minT(UE *u, vector <BS> *bslist)
{
	availbs(u, bslist);
	//�w�]�̨�BS��macro eNB
	BS* minTbs = u->availBS[0];
	double T_minTbs = predictT(u, u->availBS[0]);
	if (T_minTbs <= 0)
	{
		if (u->availBS.size() <= 1)
			return NULL;
		else
		{
			T_minTbs = 999999;
			minTbs = NULL;
		}			
	}
	double T;
	for (int i = 1; i < u->availBS.size(); i++)
	{
		T = predictT(u, u->availBS[i]);
		if (T <= 0)
			return NULL;
		else
		{
			//T����p
			if (T < T_minTbs)
			{
				T_minTbs = T;
				minTbs = u->availBS[i];
			}
			//T�ۦP�A��C
			else if (T == T_minTbs)
			{
				double C_minTbs = predict_Capacity(u, minTbs);
				double C = predict_Capacity(u, u->availBS[i]);
				if (C < C_minTbs)
				{
					T_minTbs = T;
					minTbs = u->availBS[i];
				}
			}
		}
		
	}
	return minTbs;
}

//for algorithm dso
BS* findbs_minT(UE* u, vector<BS*> *bslist)
{
	//��l�Ƴ̤pT��BS
	BS* minbs = NULL;
	double minbs_T = 0;

	//�p��UE�[�JBS�᪺T�A�ä���̤p���X��
	for (int i = 0; i < bslist->size(); i++)
	{
		double T = predictT(u, bslist->at(i));
		//�p�GT == -1�A�N��L�k�[�J		*�p�G�᳣̫����[�J�N�|return NULL
		if (T == -1)
			continue;
		//�p�Gminbs == NULL�A�N��ثe�٨S��minbs���Կ�H�A�N��{�b�o�ӳ]���Կ�H
		if (minbs == NULL)
		{
			minbs = bslist->at(i);
			minbs_T = T;
			continue;
		}
		//�p�G�{�b�o�Ӫ�T��minbs��T�٤p�A�N��o�ӧאּ�Կ�H
		if (minbs_T > T)
		{
			minbs = bslist->at(i);
			minbs_T = T;
			continue;
		}
		//�p�G�{�b�o�Ӫ�T��minbs��T�@�ˡA�N��Capacity
		if (minbs_T == T)
		{
			double minbs_capacity = predict_Capacity(u, minbs);
			double capacity = predict_Capacity(u, bslist->at(i));
			//�p�G�{�b�o�Ӫ�Capacity��minbs��Capacity�٤j�A�N��o�ӧאּ�Կ�H
			if (minbs_capacity < capacity)
			{
				minbs = bslist->at(i);
				minbs_T = T;
			}
		}
	}
	//�p�G�i��ܪ�BS��T�̤p����쥻��BS�٤j�A���N�S���noffload�F
	if (u->connecting_BS != NULL)
		if (minbs_T > u->connecting_BS->systemT)
			return NULL;
	return minbs;
}

bool calc_influence(UE *u)
{
	for (int i = 0; i < u->availBS.size(); i++)
	{
		int influence = 0;
		double T = predictT(u, u->availBS[i]);
		for (int j = 0; j < u->availBS[i]->connectingUE.size(); j++)
		{
			if (T > u->availBS[i]->connectingUE[j]->delay_budget)
			{
				influence++;
				break;
			}		
		}
		if (influence == 0)
			return false;
		else
			return true;
	}
}

bool ue_sort_cp(UE* a, UE* b) { return a->lambdai/getCapacity(a) > b->lambdai/getCapacity(b); }

bool is_all_ue_be_satisify(BS* b)
{
	int unsatisfy = 0;
	for (int i = 0; i < b->connectingUE.size(); i++)
		if (b->systemT > b->connectingUE[i]->delay_budget)
			unsatisfy++;
	return unsatisfy > 0 ? true : false;
}

//��UE�[�JBS
void ue_join_bs(UE* u, BS* b)
{
	if (u->connecting_BS != NULL)
	{
		u->connecting_BS->lambda -= u->lambdai;
		int delete_ue;
		for (delete_ue = 0; delete_ue < u->connecting_BS->connectingUE.size(); delete_ue++)
			if (u->connecting_BS->connectingUE[delete_ue]->num == u->num)
				break;
		u->connecting_BS->connectingUE.erase(u->connecting_BS->connectingUE.begin() + delete_ue);
		u->connecting_BS->systemT = getT(u->connecting_BS);
	}
	availbs_update(u, b);
	u->connecting_BS = b;
	b->lambda += u->lambdai;
	b->connectingUE.push_back(u);
	b->systemT = getT(b);
}

////��UE�[�JBS
//void ue_join_bs(UE* u, BS* b, connection_status* cs)
//{
//	if (b == NULL)
//		return;
//	//���X��BS
//	if (u->connecting_BS != NULL)
//	{
//		cs->bslist.at(u->connecting_BS->num).lambda -= u->lambdai;																			//��slambda
//		int delete_ue;
//		for (delete_ue = 0; delete_ue < cs->bslist.at(u->connecting_BS->num).connectingUE.size(); delete_ue++)
//			if (cs->bslist.at(u->connecting_BS->num).connectingUE.at(delete_ue)->num == u->num)
//				break;
//		cs->bslist.at(u->connecting_BS->num).connectingUE.erase(cs->bslist.at(u->connecting_BS->num).connectingUE.begin() + delete_ue);		//��UE�q�M�椤�R��
//		cs->bslist.at(u->connecting_BS->num).systemT = getT(&cs->bslist.at(u->connecting_BS->num));											//��sT
//	}
//
//	//�[�J�sBS
//	u->connecting_BS = &cs->bslist.at(b->num);			//��sUE�s����BS
//	availbs(u, &cs->bslist);							//��sUE��availbs
//	cs->bslist.at(b->num).lambda += u->lambdai;			//��slambda
//	cs->bslist.at(b->num).connectingUE.push_back(u);	//��UE�s�W��M��
//	cs->bslist.at(b->num).systemT = getT(&cs->bslist.at(b->num));			//��sT
//	cs->influence++;
//}

bool check_satisfy(BS* b, double T)
{
	for (int i = 0; i < b->connectingUE.size(); i++)
		if (b->connectingUE[i]->delay_budget < T)
			return false;
	return true;
}

void findbs_dso(UE* u, connection_status* cs, int k)
{
	//�M��or��savailbs		*�|��o�̪�UE�����:1.New UE 2.�Qoffload��UE �o��ؤ��|�S��availbs
	availbs(u, &cs->bslist);
	
	//availbs����: �̷Ӧ��L�v�T�������v�TBS(influence_bs) �P �����v�TBS(no_influence_bs)
	vector <BS*> no_influence_bs;		//�����v�TBS
	vector <BS*> influence_bs;			//���v�TBS
	for (int i = 0; i < u->availBS.size(); i++)
	{
		int influence_ue_number = 0;			//�պ�[�JBS��v�T��UE�ƶq
		double T = predictT(u, u->availBS.at(i));	//�պ�[�JBS�᪺T
		//�{�b�L�k�[�J��BS�]��@influence�Aoffload UE�X�h����]�\�N��[�J
		if (T == -1)
		{
			influence_bs.push_back(u->availBS[i]);
			continue;
		}
		for (int j = 0; j < u->availBS[i]->connectingUE.size(); j++)
			if (T > u->availBS[i]->connectingUE[j]->delay_budget)
			{
				influence_ue_number++;
				influence_bs.push_back(u->availBS[i]);
				break;
			}
		if (influence_ue_number == 0)
			no_influence_bs.push_back(u->availBS[i]);
	}

	//�p�G�������v�TBS�N��T�̤p��		*targetBS���|�ONULL�A�O����BS�N���|�Ono_influence_bs
	if (no_influence_bs.size() > 0)
	{
		BS *targetBS = findbs_minT(u, &no_influence_bs);	//�i�[�J��BS��T�̤p��
		ue_join_bs(u, targetBS);
		return;
	}
	//�S�������v�TBS����
	else
	{
		//�p�G�w��t��k�̤j�`�סA��T�̤p���[
		if (k == MAX_DEPTH)
		{
			BS *targetBS = findbs_minT(u, &influence_bs);	//�i�[�J��BS��T�̤p��
			if (targetBS != NULL)
			{
				ue_join_bs(u, targetBS);
				return;
			}
			else
			{
				//�Qoffload��UE�B�S����n����ܤ~�|��o�A�����쥻���w�s��BS�A�ҥH�����@outage UE
				if (u->connecting_BS == NULL)
					cs->outage_dso++;
				return;
			}
		}
		//�p�G�٨S��t��k�̤j�`�� -> offload
		else
		{
			connection_status min_influence_cs;		//�̤p���v�T��cs
			min_influence_cs.influence = -1;		//�w�]�ȡA�ΥH�Х��٦b��l�ƪ��A
			BS* min_influence_bs = NULL;

			for (int j = 0; j < influence_bs.size(); j++)	//for all influence bs
			{
				double T = predictT(u, influence_bs[j]);	//UE u�p�G�[�J���v�TBS j�᪺T
				
				//UE����:��XDB���Q������UE�A�M�����:�|�v�TUE(influence_ue)�P���|�v�TUE(no_influence_ue)
				vector <UE*> influence_ue;		//�|�v�T��LBS��UE
				vector <UE*> no_influence_ue;	//���|�v�T��LBS��UE
				//�p�GBS�w���M�AT�N�|�O-1�A���ɭn��offload UE�X�h�A�ݯण��[�J�A�]��T��BS�[�JUE�e��system time
				if (T == -1)
				{				
					for (int k = 0; k < influence_bs.at(j)->connectingUE.size(); k++)
					{
						if (influence_bs.at(j)->connectingUE[k]->delay_budget < influence_bs.at(j)->systemT)	//�p�GUE��DB�p��T(���Q����)
						{
							if (influence_bs.at(j)->connectingUE[k]->availBS.size() == 0)						//�p�GUE�S��LBS�i�H��ܴN���L
								continue;
							if (calc_influence(influence_bs.at(j)->connectingUE[k]) == 1)						//�p�Goffload UE�|�A�v�T��LBS�N�Oinfluence_ue
								influence_ue.push_back(influence_bs.at(j)->connectingUE[k]);
							else
								no_influence_ue.push_back(influence_bs.at(j)->connectingUE[k]);
						}
					}
				}
				//�p�GBS�����M�AT�N�|�O���ȡA���]UE�[�J��n�⨺��UE offload�X�h
				else
				{
					//UE����:��XDB���Q������UE�A�M�����:�|�v�TUE(influence_ue)�P���|�v�TUE(no_influence_ue)
					for (int k = 0; k < influence_bs.at(j)->connectingUE.size(); k++)
					{
						if (influence_bs.at(j)->connectingUE[k]->delay_budget < T)			//�p�GUE��DB�p��T(���Q����)
						{
							if (influence_bs.at(j)->connectingUE[k]->availBS.size() == 0)	//�p�GUE�S��LBS�i�H��ܴN���L
								continue;
							if (calc_influence(influence_bs.at(j)->connectingUE[k]) == 1)	//�p�Goffload UE�|�A�v�T��LBS�N�Oinfluence_ue
								influence_ue.push_back(influence_bs.at(j)->connectingUE[k]);
							else
								no_influence_ue.push_back(influence_bs.at(j)->connectingUE[k]);
						}
					}
				}

				//UE�Ƨ�:�M�wUE�Qoffload�X�h������
				vector <UE*> ue_sorted;		//�ƧǹL�᪺UE�M��
				if (no_influence_ue.size() > 0)
				{
					sort(no_influence_ue.begin(), no_influence_ue.end(), ue_sort_cp);		//�j��p�Ƨ�
					ue_sorted = no_influence_ue;
					if (influence_ue.size() > 0)
					{
						sort(no_influence_ue.begin(), no_influence_ue.end(), ue_sort_cp);
						ue_sorted.insert(ue_sorted.end(), influence_ue.begin(), influence_ue.end());
					}
				}
				else
				{
					if (influence_ue.size() > 0)
					{
						sort(influence_ue.begin(), influence_ue.end(), ue_sort_cp);
						ue_sorted = influence_ue;
					}
				}
				if (ue_sorted.size() == 0)
					break;
				//Offload UE�A����BS���Ҧ�UE���Q���� �� �Ҧ���offload�����v�TUE��offload
				connection_status cs_temp = *cs;
				bool join = false;		//�P�_�n���noffload UE��index
				for (int k = 0; k < ue_sorted.size(); k++)
				{
					findbs_dso(&cs_temp.uelist[ue_sorted.at(k)->num], &cs_temp, k + 1);
					T = predictT(&cs_temp.uelist[u->num], &cs_temp.bslist[influence_bs[j]->num]);
					if (check_satisfy(&cs_temp.bslist[influence_bs[j]->num], T))	//offload���@��UE��A��������Ҧ�DB�F
					{
						join = true;
						break;
					}
				}
				if (T != -1)		//offload�Ҧ�UE��S��k�����Ҧ�DB�A���i�[�J
					join = true;
				//�p�G�Ҧ��ioffload��UE��offload�F�ABS�٬O���M�Ajoin�N�אּfalse�A�N������v�T�j�p�A�[���i�h���m?
				if (join == false)			//���L�o��influence_bs
					break;
				ue_join_bs(&cs_temp.uelist[u->num], &cs_temp.bslist[influence_bs[j]->num]);		//�w�g��UE���X�Ŧ�F�A�M��N��UE�[�i��

				//����v�T(���F����UE���ʤF�h��UE)
				if (min_influence_cs.influence == -1)
				{
					min_influence_cs = cs_temp;
					min_influence_bs = influence_bs[j];
				}
				else
					if (cs_temp.influence < min_influence_cs.influence)
					{
						min_influence_cs = cs_temp;
						min_influence_bs = influence_bs[j];
					}
			}
			if (min_influence_bs == NULL)
			{
				findbs_minT(u, &influence_bs);
				return;
			}				
			*cs = min_influence_cs;
			return;
		}
	}		
}

void add_UE_to_BS(UE* u, BS* b)
{
	if (u->connecting_BS != NULL)
	{
		cout << "UE���XBS" << u->connecting_BS->num << "�e:\nBS��" << u->connecting_BS->connectingUE.size() << "��UE" << endl;
		u->connecting_BS->lambda -= u->lambdai;
		int i;
		for (i = 0; i < u->connecting_BS->connectingUE.size(); i++)
		{
			if (u->connecting_BS->connectingUE[i]->num == u->num)
				break;
		}
		u->connecting_BS->connectingUE.erase(u->connecting_BS->connectingUE.begin() + i - 1);
		cout << "UE���XBS" << u->connecting_BS->num << "��:\nBS��" << u->connecting_BS->connectingUE.size() << "��UE" << endl;
	}
	u->connecting_BS = b;
	b->lambda += u->lambdai;
	b->connectingUE.push_back(u);
	b->systemT = getT(b);
}
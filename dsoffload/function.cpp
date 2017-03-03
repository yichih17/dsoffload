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
//void countaprange()
//{
//	int sinr_ap[8] = { 4, 7, 9, 12, 16, 20, 21, 22 };
//	for (int i = 0; i < 8; i++)
//	{
//		//path loss: 140.7 + 36.7 log(d), d in km.
//		double distance = pow(10, (-(sinr_ap[i] - 78 - power_ap) - 122.7) / 35.1) * 1000;
//		range_ap[i] = (int)distance;
//	}
//	//result:185, 152, 133, 109, 84, 64, 60, 56
//}

//�p��UE�PBS�����Z��
double calc_distance(UE* u, BS* b)
{
	return sqrt(pow((u->coor_X - b->coor_X), 2) + pow((u->coor_Y - b->coor_Y), 2));
}

//�p��UE�PBS��CQI
int calc_CQI(UE* u, BS* b)
{
	double distance = calc_distance(u, b);
	int CQI = 0;
	if (b->type == macro)	//�p��LTE��CQI
	{
		for (int i = 0; i < 15; i++)
		{
			if (distance <= range_macro[i])
				CQI++;
			else
				break;
		}
	}
	if (b->type == ap)		//�p��Wifi��CQI
	{
		for (int i = 0; i < 8; i++)
		{
			if (distance <= range_ap[i])
				CQI++;
			else
				break;
		}
	}
	return CQI;
}

//��sUE�i�s����BS�M��
void availbs(UE* u, vector<BS> *bslist)
{
	//�p�GUE��availbs�w�g����ơA�M���ç�s
	if (u->availBS.size() != 0)
		u->availBS.clear();
	if (u->connecting_BS != NULL)
	{
		for (int i = 0; i < bslist->size(); i++)
		{
			//�p�GUE�w�s��BS�Aavailbs�N���ӥX�{�w�s����BS
			if (u->connecting_BS->num == bslist->at(i).num)
				continue;
			if (calc_CQI(u, &bslist->at(i)) == 0)
				continue;
			else
				u->availBS.push_back(&bslist->at(i));
		}
	}
	else
	{
		for (int i = 0; i < bslist->size(); i++)
		{
			if (calc_CQI(u, &bslist->at(i)) == 0)
				continue;
			else
				u->availBS.push_back(&bslist->at(i));
		}
	}
}

//UE�[�JBS�e�A����savailbs	*newbs���Y�N�n�[�J��BS
//void availbs_update(UE* u, BS* newb)
//{
//	//�n���ثe��BS�F�A�ҥH�ثe��BS�|�ܦ�availbs
//	if (u->connecting_BS != NULL)
//		u->availBS.push_back(u->connecting_BS);
//	int delete_bs;		//newbs�bavailbs������m
//	for (delete_bs = 0; delete_bs < u->availBS.size(); delete_bs++)
//		if (u->availBS[delete_bs]->num == newb->num)
//			break;
//	u->availBS.erase(u->availBS.begin() + delete_bs);
//}

//�պ�UE�[�JBS�᪺Capacity
double predict_Capacity(UE* u, BS* b)
{
	if (b->type == macro)
		return resource_element * macro_eff[calc_CQI(u, b) - 1] * total_RBG / (b->connectingUE.size() + 1);
	if (b->type == ap)
		return ap_capacity[calc_CQI(u, b) - 1] / (b->connectingUE.size() + 1);
}

//�p��UE�PBS��System Capacity
double getCapacity(UE* u, BS* b)
{
	if (b->type == macro)
		return resource_element * macro_eff[calc_CQI(u, b) - 1] * total_RBG;
	if (b->type == ap)
		return ap_capacity[calc_CQI(u, b) - 1];
}

//�p��UE�P�ثe�s��BS��Capacity
double getCapacity(UE* u)
{
	if (u->connecting_BS->type == macro)
		return resource_element * macro_eff[calc_CQI(u, u->connecting_BS) - 1] * total_RBG / u->connecting_BS->connectingUE.size();
	if (u->connecting_BS->type == ap)
		return ap_capacity[calc_CQI(u, u->connecting_BS) - 1] / u->connecting_BS->connectingUE.size();
}

/* �պ�UE�[�JBS��ABS��Avg. system time(T*) */
double predictT(UE* u, BS* b)
{
	//�պ�u�[�Jb�᪺lambda
	double lambda = b->lambda + u->lambdai;
	//�p��u�[�Jb�᪺Xj
	double Xj = 0;
	double Xj2 = 0;
	for (int i = 0; i < b->connectingUE.size(); i++)	//�쥻�bb��UE��Xij�[�_��
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
	//if (rho == 1)
	//	cout << "stop" << endl;
	if (rho >= 0.999999)
		return -1;
	Xj2 += pow(u->packet_size / (getCapacity(u, b) / (b->connectingUE.size() + 1)), 2) * (u->lambdai / lambda);
	//��M/G/1������T
	return Xj + lambda * Xj2 / (1 - lambda * Xj);
}

//�p��BS��avg system time(delay)
double getT(BS* b)
{
	double Xj = 0;		//�bBS���U��UE��avg service time
	double Xj2 = 0;		//�bBS���U��UE��avg Xj2
	for (int i = 0; i < b->connectingUE.size(); i++)
	{
		double capacity_i = getCapacity(b->connectingUE[i]);
		double weight_i = b->connectingUE[i]->lambdai / b->lambda;
		double pktsize_i = b->connectingUE[i]->packet_size;
		Xj += pktsize_i / capacity_i * weight_i;
		Xj2 += pow(pktsize_i / capacity_i, 2) * weight_i;
	}
	return Xj + b->lambda * Xj2 / (1 - b->lambda * Xj);
}

double getrho(BS* b)
{
	double Xj = 0;		//�bBS���U��UE��avg service time
	for (int i = 0; i < b->connectingUE.size(); i++)
	{
		Xj += b->connectingUE[i]->packet_size / getCapacity(b->connectingUE[i]) * (b->connectingUE[i]->lambdai / b->lambda);
	}
	return b->lambda * Xj;
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
				continue;
			}
			//�p�G�{�b�o�Ӫ�Capacity��minbs��Capacity�@�ˡA�N��Z��
			if (minbs_capacity == capacity)
			{
				double minbs_distance = calc_distance(u, minbs);
				double distance = calc_distance(u, bslist->at(i));
				if (minbs_distance > distance)
				{
					minbs = bslist->at(i);
					minbs_T = T;
				}
			}
		}
	}
	if (minbs == NULL)
		return NULL;
	else
	{
		//�p�G�i��ܪ�BS��T�̤p����쥻��BS�٤j�A���N�S���noffload�F
		if (u->connecting_BS != NULL)
			if (minbs_T > u->connecting_BS->systemT)
				return NULL;
		return minbs;
	}
}

//�ˬdoffload UE�|���|���LBS�y���v�T
int is_influence_ue(UE *u)
{
	bool no_bs_to_offload = true;					//���S����LBS�i�H�[
	bool has_no_influence_bs = false;				//���S��offload�ᤣ�|�Q�v�T��BS
	for (int i = 0; i < u->availBS.size(); i++)
	{
		double T = predictT(u, u->availBS[i]);		//�[�Javailbs[i]�᪺T
		if (T == -1)
			continue;								//���L
		no_bs_to_offload = false;
		bool influence = false;
		for (int j = 0; j < u->availBS[i]->connectingUE.size(); j++)
		{
			if (T > u->availBS[i]->connectingUE[j]->delay_budget)
			{
				influence = true;
				break;
			}
		}
		if (influence == false)
		{
			has_no_influence_bs = true;
			break;
		}			
	}
	if (no_bs_to_offload)
		return -1;
	if (has_no_influence_bs)
		return 0;
	return 1;
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
		u->availBS.push_back(u->connecting_BS);		//UE�ثe�s����BS�|�ܦ��L���Ӫ�availbs
	}
	//��newbs�qavailbs���R���A�]���L�|�ܦ�connecting_bs�A�ҥH���Ӧbavailbs
	for (int delete_bs = 0; delete_bs < u->availBS.size(); delete_bs++)
	{
		if (u->availBS[delete_bs]->num == b->num)
		{
			u->availBS.erase(u->availBS.begin() + delete_bs);
			break;
		}
	}
	u->connecting_BS = b;
	b->lambda += u->lambdai;
	b->connectingUE.push_back(u);
	b->systemT = getT(b);
	if (b->systemT < 0)
		cout << "T < 0" << endl;
}

bool check_satisfy(BS* b, double T)
{
	for (int i = 0; i < b->connectingUE.size(); i++)
		if (b->connectingUE[i]->delay_budget < T)
			return false;
	return true;
}

bool findbs_dso(UE* u, connection_status* cs, int depth)
{
	//�M��availbs
	if (u->availBS.size() == 0)
		availbs(u, &cs->bslist);
	
	//availbs����: �̷Ӧ��L�v�T�������v�TBS(influence_bs) �P �����v�TBS(no_influence_bs)
	vector <BS*> no_influence_bs;		//�����v�TBS
	vector <BS*> influence_bs;			//���v�TBS
	vector <BS*> saturated_bs;			//�w���MBS
	for (int i = 0; i < u->availBS.size(); i++)
	{
		bool influence = false;				//�պ�[�JBS��v�T��UE�ƶq
		double T = predictT(u, u->availBS.at(i));	//�պ�[�JBS�᪺T
		//�{�b�L�k�[�J��BS�]��@influence�Aoffload UE�X�h����]�\�N��[�J
		if (T == -1)
			saturated_bs.push_back(u->availBS[i]);
		else
		{
			//�p��availbs[i]�UUE��DB���L�Q����
			//��:no_influence_bs; �L:influence_bs
			for (int j = 0; j < u->availBS[i]->connectingUE.size(); j++)
			{
				if (T > u->availBS[i]->connectingUE[j]->delay_budget)
				{
					influence = true;
					break;
				}
			}
			if (influence)
				influence_bs.push_back(u->availBS[i]);
			else
				no_influence_bs.push_back(u->availBS[i]);
		}
	}

	//�p�G�������v�TBS�N��T�̤p��	
	if (no_influence_bs.size() > 0)
	{
		BS *targetBS = findbs_minT(u, &no_influence_bs);	//�i�[�J��BS��T�̤p��
		if (targetBS != NULL)
		{
			ue_join_bs(u, targetBS);
			return true;
		}
		else
			return false;
	}
	//�S�������v�TBS����
	else
	{
		//�p�G�w��t��k�̤j�`�סA��T�̤p���[
		if (depth == MAX_DEPTH)
		{
			BS *targetBS = findbs_minT(u, &influence_bs);	//�i�[�J��BS��T�̤p��
			if (targetBS != NULL)
			{
				ue_join_bs(u, targetBS);
				return true;
			}
			else
				return false;
		}
		//�p�G�٨S��t��k�̤j�`�� -> offload
		else
		{
			//influence_bs�Msaturated_bs����offload�ݬݤ�v�T�j�p
			influence_bs.insert(influence_bs.end(), saturated_bs.begin(), saturated_bs.end());

			connection_status cs_origin = *cs;		//cs����l���A
			
			//min_influence_cs.influence = -1;		//�w�]�ȡA�ΥH�Х��٦b��l�ƪ��A

			BS* bs_min_T = NULL;			//offload�L�᪺BS���AT�̤p��
			double min_T;					//offload�L�᪺BS���AT���̤p��
			connection_status cs_min_T;		//offload�L�᪺BS���AT�̤p��cs
			for (int i = 0; i < influence_bs.size(); i++)	//for all influence bs
			{
				*cs = cs_origin;
				double T = predictT(u, influence_bs.at(i));	//UE u�p�G�[�J���v�TBS j�᪺T
				
				//UE����:��XDB���Q������UE�A�M�����:�|�v�TUE(influence_ue)�P���|�v�TUE(no_influence_ue)
				vector <UE*> influence_ue;		//�|�v�T��LBS��UE
				vector <UE*> no_influence_ue;	//���|�v�T��LBS��UE
				//�p�GBS�w���M�AT�N�|�O-1�A���ɭn��offload UE�X�h�A�ݯण��[�J�A�]��T��BS�[�JUE�e��system time
				if (T == -1)
				{
					for (int j = 0; j < influence_bs.at(i)->connectingUE.size(); j++)
					{
						if (influence_bs.at(i)->connectingUE.at(j)->delay_budget < influence_bs.at(i)->systemT)		//���Q������UE
						{
							if (influence_bs.at(i)->connectingUE.at(j)->availBS.size() > 0)							//����LBS�i�Hoffload
							{
								switch (is_influence_ue(influence_bs.at(i)->connectingUE[j]))
								{
								case 0:
									no_influence_ue.push_back(influence_bs.at(i)->connectingUE[j]);
									break;
								case 1:
									influence_ue.push_back(influence_bs.at(i)->connectingUE[j]);
									break;
								default:
									break;
								}							
							}								
						}							
					}
				}
				else
				{
					for (int j = 0; j < influence_bs.at(i)->connectingUE.size(); j++)
					{
						if (influence_bs.at(i)->connectingUE.at(j)->delay_budget < T)			//���Q������UE
						{
							if (influence_bs.at(i)->connectingUE.at(j)->availBS.size() > 0)		//����LBS�i�Hoffload
							{
								switch (is_influence_ue(influence_bs.at(i)->connectingUE[j]))
								{
								case 0:
									no_influence_ue.push_back(influence_bs.at(i)->connectingUE[j]);
									break;
								case 1:
									influence_ue.push_back(influence_bs.at(i)->connectingUE[j]);
									break;
								default:
									break;
								}
							}
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
					else
						continue;	//�S��UE�i�Hoffload�A���L�o��influence_bs
				}

				//Offload UE�A����BS���Ҧ�UE���Q���� �� �Ҧ���offload�����v�TUE��offload
				for (int j = 0; j < ue_sorted.size(); j++)
				{
					if (findbs_dso(ue_sorted.at(j), cs, depth + 1))
					{
						cs->influence++;					//���\offload�@��UE�A�v�T+1
						T = predictT(u, influence_bs.at(i));	//��sT�A���ٻݤ��ݭn�~��offload
						if (T != -1 && check_satisfy(influence_bs.at(i), T))	//offload���@��UE��AUE����[�J�B��������Ҧ�DB�F
							break;
					}
				}
				if (T == -1)		//offload�Ҧ�UE���٬O�L�k�[�JUE
					continue;
				//���
				if (bs_min_T == NULL)
				{
					bs_min_T = influence_bs.at(i);
					min_T = T;
					cs_min_T = *cs;
				}
				else
				{
					if (T < min_T)		//�p�GT����p
					{
						bs_min_T = influence_bs.at(i);
						min_T = T;
						cs_min_T = *cs;
					}
					else
					{
						if (T == min_T)		//�p�GT�@��
						{
							if (cs->influence < cs_min_T.influence)		//��v�T�j�p
							{
								bs_min_T = influence_bs.at(i);
								min_T = T;
								cs_min_T = *cs;
							}
						}
					}
				}
			}
			if (bs_min_T == NULL)		//�S����k��UE offload UE �X�h
			{
				*cs = cs_origin;
				return false;
			}
			else
			{
				ue_join_bs(u, bs_min_T);
				*cs = cs_min_T;
				return true;
			}
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
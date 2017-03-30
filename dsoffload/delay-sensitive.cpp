#include<iostream>
#include<vector>
#include<algorithm>
#include"define.h"

using namespace std;

double get_distance(UE* u, BS* b);
int get_CQI(UE* u, BS* b);
double predict_C(UE* u);
double predict_C(UE* u, BS* b);
double predict_C(UE* u, BS* b, int CQI);
double get_C(UE* u);
double predict_T(UE* u, BS* b);
double predict_T(UE* u, BS* b, int CQI);
double update_T(BS* b);
bool influence(BS* b, double T);
int influence(UE *u);
bool join_minT_bs(UE* u, vector <BS*> *list, vector <double> *list_T);
void joinBS(UE* u, BS* b, double T);
bool ue_cp(UE* a, UE* b);
bool all_ue_satisfy(BS* b, double T);

bool findbs_ex(UE* u, connection_status* cs, int depth)
{
	vector <int> availBS_CQI;
	if (u->availBS.size() == 0)
	{
		for (int i = 0; i < cs->bslist.size(); i++)
		{
			int CQI = get_CQI(u, &cs->bslist.at(i));
			if (CQI == 0)
				continue;
			u->availBS.push_back(&cs->bslist.at(i));
			availBS_CQI.push_back(CQI);
		}
	}
	else
	{
		for (int i = 0; i < u->availBS.size(); i++)
		{
			int CQI = get_CQI(u, u->availBS.at(i));
			availBS_CQI.push_back(CQI);
		}
	}

	vector <BS*> influence_bs;
	vector <BS*> no_influence_bs;
	vector <BS*> saturated_bs;
	vector <double> influence_bs_T;
	vector <double> no_influence_bs_T;
	vector <double> saturated_bs_T;

	for (int i = 0; i < u->availBS.size(); i++)
	{
		double T = predict_T(u, u->availBS.at(i), availBS_CQI.at(i));
		if (T == -1)
		{
			saturated_bs.push_back(u->availBS.at(i));
			saturated_bs_T.push_back(T);
		}
		else
		{
			if (influence(&cs->bslist.at(i), T))
			{
				influence_bs.push_back(u->availBS.at(i));
				influence_bs_T.push_back(T);
			}
			else
			{
				no_influence_bs.push_back(u->availBS.at(i));
				no_influence_bs_T.push_back(T);
			}
		}
	}

	if (no_influence_bs.size() != 0)
	{
		if (join_minT_bs(u, &no_influence_bs, &no_influence_bs_T))
			return true;
		else
			return false;
	}
	else
	{
		if (depth == max_depth)
		{
			if (influence_bs.size() == 0)
				return false;
			if (join_minT_bs(u, &influence_bs, &influence_bs_T))
				return true;
			else
				return false;
		}
		else
		{
			vector <BS*> offload_bs = influence_bs;
			offload_bs.insert(offload_bs.end(), saturated_bs.begin(), saturated_bs.end());
			vector <double> offload_bs_T = influence_bs_T;
			offload_bs_T.insert(offload_bs_T.end(), saturated_bs_T.begin(), saturated_bs_T.end());

			connection_status cs_origin = *cs;

			BS* bs_min_T = NULL;
			double min_T;
			connection_status cs_min_T;

			for (int i = 0; i < offload_bs.size(); i++)
			{
				*cs = cs_origin;
				vector <UE*> influence_ue;
				vector <UE*> no_influence_ue;

				double T = offload_bs_T.at(i);

				if (T == -1)
				{
					for (int j = 0; j < offload_bs.at(i)->connectingUE.size(); j++)
					{
						if (offload_bs.at(i)->connectingUE.at(j)->availBS.size() > 0)
						{
							//if (offload_bs.at(i)->connectingUE.at(j)->delay_budget < offload_bs.at(i)->systemT)
							//{
								switch (influence(offload_bs.at(i)->connectingUE.at(j)))
								{
								case 0:
									no_influence_ue.push_back(offload_bs.at(i)->connectingUE.at(j));
									break;
								case 1:
									influence_ue.push_back(offload_bs.at(i)->connectingUE.at(j));
								default:
									break;
								}
							//}
						}
					}
				}
				else
				{
					for (int j = 0; j < offload_bs.at(i)->connectingUE.size(); j++)
					{
						if (offload_bs.at(i)->connectingUE.at(j)->availBS.size() > 0)
						{
							//if (offload_bs.at(i)->connectingUE.at(j)->delay_budget < T)
							//{
								switch (influence(offload_bs.at(i)->connectingUE.at(j)))
								{
								case 0:
									no_influence_ue.push_back(offload_bs.at(i)->connectingUE.at(j));
									break;
								case 1:
									influence_ue.push_back(offload_bs.at(i)->connectingUE.at(j));
								default:
									break;
								//}
							}
						}
					}
				}

				vector <UE*> ue_sorted;
				if (no_influence_ue.size() > 0)
				{
					sort(no_influence_ue.begin(), no_influence_ue.end(), ue_cp);		//大到小排序
					ue_sorted = no_influence_ue;
					if (influence_ue.size() > 0)
					{
						sort(no_influence_ue.begin(), no_influence_ue.end(), ue_cp);
						ue_sorted.insert(ue_sorted.end(), influence_ue.begin(), influence_ue.end());
					}
				}
				else
				{
					if (influence_ue.size() > 0)
					{
						sort(influence_ue.begin(), influence_ue.end(), ue_cp);
						ue_sorted = influence_ue;
					}
				}

				for (int j = 0; j < ue_sorted.size(); j++)
				{
					if (findbs_ex(ue_sorted.at(j), cs, depth + 1))
					{
						cs->influence++;
						T = predict_T(u, offload_bs.at(i));
						//if (T != -1 && all_ue_satisfy(offload_bs.at(i), T))
						//	break;
					}
				}
				if (T == -1)
					continue;

				if (bs_min_T == NULL)
				{
					bs_min_T = offload_bs.at(i);
					min_T = T;
					cs_min_T = *cs;
				}
				else
				{
					if (T < min_T)		//如果T比較小
					{
						bs_min_T = offload_bs.at(i);
						min_T = T;
						cs_min_T = *cs;
					}
					else
					{
						if (T == min_T)		//如果T一樣
						{
							if (cs->influence < cs_min_T.influence)		//比影響大小
							{
								bs_min_T = offload_bs.at(i);
								min_T = T;
								cs_min_T = *cs;
							}
						}
					}
				}
			}
			if (bs_min_T == NULL)		//沒有辦法為UE offload UE 出去
			{
				*cs = cs_origin;
				return false;
			}
			else
			{
				*cs = cs_min_T;
				joinBS(u, bs_min_T, min_T);
			}
		}
	}
	return true;
}

double get_distance(UE* u, BS* b)
{
	return sqrt(pow((u->coor_X - b->coor_X), 2) + pow((u->coor_Y - b->coor_Y), 2));
}

int get_CQI(BS* b, double distance)
{
	int CQI = 0;
	if (b->type == macro)	//計算LTE的CQI
	{
		for (int i = 0; i < 15; i++)
		{
			if (distance <= range_macro[i])
				CQI++;
			else
				break;
		}
	}
	if (b->type == ap)		//計算Wifi的CQI
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

int get_CQI(UE* u, BS* b)
{
	double distance = get_distance(u, b);
	int CQI = 0;
	if (b->type == macro)	//計算LTE的CQI
	{
		for (int i = 0; i < 15; i++)
		{
			if (distance <= range_macro[i])
				CQI++;
			else
				break;
		}
	}
	if (b->type == ap)		//計算Wifi的CQI
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

double predict_C(UE* u)
{
	if (u->connecting_BS->type == macro)
		return resource_element * macro_eff[u->CQI - 1] * total_RBG / (u->connecting_BS->connectingUE.size() + 1);
	if (u->connecting_BS->type == ap)
		return ap_capacity[u->CQI - 1] / (u->connecting_BS->connectingUE.size() + 1);
}

double predict_C(UE* u, BS* b)
{
	if (b->type == macro)
		return resource_element * macro_eff[get_CQI(u, b) - 1] * total_RBG / (b->connectingUE.size() + 1);
	if (b->type == ap)
		return ap_capacity[get_CQI(u, b) - 1] / (b->connectingUE.size() + 1);
}

double predict_C(UE* u, BS* b, int CQI)
{
	if (b->type == macro)
		return resource_element * macro_eff[CQI - 1] * total_RBG / (b->connectingUE.size() + 1);
	if (b->type == ap)
		return ap_capacity[CQI - 1] / (b->connectingUE.size() + 1);
}

double get_C(UE* u)
{
	if (u->connecting_BS->type == macro)
		return resource_element * macro_eff[u->CQI - 1] * total_RBG / u->connecting_BS->connectingUE.size();
	if (u->connecting_BS->type == ap)
		return ap_capacity[u->CQI - 1] / u->connecting_BS->connectingUE.size();
}

double predict_T(UE* u, BS* b)
{
	int CQI = get_CQI(u, b);
	//試算u加入b後的lambda
	double lambda = b->lambda + u->lambdai;
	//計算u加入b後的Xj
	double Xj = 0;
	double Xj2 = 0;
	for (int i = 0; i < b->connectingUE.size(); i++)	//原本在b的UE的Xij加起來
	{
		double pktsize_i = b->connectingUE.at(i)->packet_size;
		double capacity_i = predict_C(b->connectingUE.at(i));
		double weight_i = b->connectingUE.at(i)->lambdai / lambda;
		double Xij = pktsize_i / capacity_i;
		Xj += Xij * weight_i;
		Xj2 += pow(Xij, 2) * weight_i;
	}
	double Xuj = u->packet_size / predict_C(u, b, CQI);
	Xj += Xuj * (u->lambdai / lambda);
	double rho = Xj * lambda;
	if (rho >= 0.999)		//Saturated
		return -1;
	Xj2 += pow(Xuj, 2) * (u->lambdai / lambda);
	//用M/G/1公式算T
	return Xj + lambda * Xj2 / (1 - lambda * Xj);
}

double predict_T(UE* u, BS* b, int CQI)
{
	//試算u加入b後的lambda
	double lambda = b->lambda + u->lambdai;
	//計算u加入b後的Xj
	double Xj = 0;
	double Xj2 = 0;
	for (int i = 0; i < b->connectingUE.size(); i++)	//原本在b的UE的Xij加起來
	{
		double pktsize_i = b->connectingUE.at(i)->packet_size;
		double capacity_i = predict_C(b->connectingUE.at(i));
		double weight_i = b->connectingUE.at(i)->lambdai / lambda;
		double Xij = pktsize_i / capacity_i;
		Xj += Xij * weight_i;
		Xj2 += pow(Xij, 2) * weight_i;
	}
	double Xuj = u->packet_size / predict_C(u, b, CQI);
	Xj += Xuj * (u->lambdai / lambda);
	double rho = Xj * lambda;
	if (rho >= 0.999)		//Saturated
		return -1;
	Xj2 += pow(Xuj, 2) * (u->lambdai / lambda);
	//用M/G/1公式算T
	return Xj + lambda * Xj2 / (1 - lambda * Xj);
}

double update_T(BS* b)
{
	double Xj = 0;
	double Xj2 = 0;
	for (int i = 0; i < b->connectingUE.size(); i++)	//原本在b的UE的Xij加起來
	{
		double weight_i = b->connectingUE.at(i)->lambdai / b->lambda;
		double Xij = b->connectingUE.at(i)->packet_size / get_C(b->connectingUE.at(i));
		Xj += Xij * weight_i;
		Xj2 += pow(Xij, 2) * weight_i;
	}
	return Xj + b->lambda * Xj2 / (1 - b->lambda * Xj);
}

bool influence(BS* b, double T)
{
	for (int i = 0; i < b->connectingUE.size(); i++)
	{
		if (T > b->connectingUE.at(i)->delay_budget)
			return true;
	}
	return false;
}

bool join_minT_bs(UE* u, vector <BS*> *list, vector <double> *list_T)
{
	BS* targetBS = NULL;
	double minT = 0;
	for (int i = 0; i < list->size(); i++)
	{
		if (targetBS == NULL)
		{
			targetBS = list->at(i);
			minT = list_T->at(i);
		}
		else
		{
			if (list_T->at(i) < minT)
			{
				targetBS = list->at(i);
				minT = list_T->at(i);
			}
			else
			{
				if (list_T->at(i) == minT)
				{
					double capacity = predict_C(u, list->at(i));
					double capacity_targetBS = predict_C(u, targetBS);
					if (capacity > capacity_targetBS)
					{
						targetBS = list->at(i);
						minT = list_T->at(i);
					}
					else
					{
						if (capacity == capacity_targetBS)
						{
							double distance = get_distance(u, list->at(i));
							double distance_targetBS = get_distance(u, targetBS);
							if (distance < distance_targetBS)
							{
								targetBS = list->at(i);
								minT = list_T->at(i);
							}
						}
					}
				}
			}
		}
	}

	if (u->connecting_BS != NULL)
	{
		if (minT > u->connecting_BS->systemT)
			targetBS = NULL;
	}

	if (targetBS == NULL)
		return false;
	else
		joinBS(u, targetBS, minT);

	return true;
}

void joinBS(UE* u, BS* targetBS, double T)
{
	if (u->connecting_BS != NULL)
	{
		u->connecting_BS->lambda -= u->lambdai;
		int delete_ue;
		for (delete_ue = 0; delete_ue < u->connecting_BS->connectingUE.size(); delete_ue++)
		{
			if (u->connecting_BS->connectingUE.at(delete_ue)->num == u->num)
			{
				u->connecting_BS->connectingUE.erase(u->connecting_BS->connectingUE.begin() + delete_ue);
				break;
			}
		}
		u->connecting_BS->systemT = update_T(u->connecting_BS);
		u->availBS.push_back(u->connecting_BS);
	}
	for (int delete_bs = 0; delete_bs < u->availBS.size(); delete_bs++)
	{
		if (u->availBS[delete_bs]->num == targetBS->num)
		{
			u->availBS.erase(u->availBS.begin() + delete_bs);
			break;
		}
	}
	u->connecting_BS = targetBS;
	u->CQI = get_CQI(u, targetBS);
	targetBS->lambda += u->lambdai;
	targetBS->connectingUE.push_back(u);
	targetBS->systemT = T;
}

//檢查UE是否為influence_ue
int influence(UE *u)
{
	bool no_bs_to_offload = true;
	bool has_no_influence_bs = false;
	for (int i = 0; i < u->availBS.size(); i++)
	{
		double T = predict_T(u, u->availBS.at(i));
		if (T == -1)
			continue;
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

bool ue_cp(UE* a, UE* b) { return a->lambdai / get_C(a) > b->lambdai / get_C(b); }

bool all_ue_satisfy(BS* b, double T)
{
	for (int i = 0; i < b->connectingUE.size(); i++)
		if (b->connectingUE[i]->delay_budget < T)
			return false;
	return true;
}
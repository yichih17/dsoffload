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
double get_C(UE* u, BS *b);
double get_C(UE* u, BS *b, int CQI);
double predict_T(UE* u, BS* b);
double predict_T(UE* u, BS* b, int CQI);
double update_T(BS* b);
bool influence(BS* b, double T);
int influence(UE *u);
bool join_minT_bs(UE* u, vector <BS*> *list, vector <double> *list_T, int DB_th);
void joinBS_simple(UE* u, BS* targetbs, double T);
void joinBS(UE* u, BS* b, double T, int DB_th);
bool ue_cp(UE* a, UE* b);
bool all_ue_satisfy(BS* b, double T);
template <class T> int max_index(vector <T> v);
double get_T_constraint(BS *b, int DB_th);
double predict_constraint(BS *b, UE *u, int DB_th);

int range_macro[] = { 1732, 1511, 1325, 1162, 1019, 894, 784, 688, 623, 565, 512, 449, 407, 357, 303 };
int range_ap[] = { 185, 152, 133, 109, 84, 64, 60, 56 };
double macro_eff[15] = { 0.1523, 0.2344, 0.377, 0.6016, 0.8770, 1.1758, 1.4766, 1.9141, 2.4063, 2.7305, 3.3223, 3.9023, 4.5234, 5.1152, 5.5546 };
double ap_capacity[8] = { 6500, 13000, 19500, 26000, 39000, 52000, 58500, 65000 };	//65Mbps = 65000000bps = 65000 bits/ms

bool findbs_dso(UE* u, connection_status* cs, int depth, int depth_max, int DB_th)
{
	//計算可連接基地台
	vector <BS*> uninfluence_BS;
	vector <double> uninfluence_BST;
	vector <BS*> offloading_BS;
	vector <double> offloading_BST;

	if (u->availBS.size() == 0)
	{
		//UE_u是New UE
		for (int i = 0; i < cs->bslist.size(); i++)					//尋找可連接基地台
		{
			int CQI = get_CQI(u, &cs->bslist[i]);						//計算CQI
			if (CQI == 0)
				continue;
			u->availBS.push_back(&cs->bslist.at(i));				//BS_i可連接
			double T = predict_T(u, &cs->bslist[i], CQI);			//試算T
			if (T == -1)											//Load飽和
			{
				offloading_BS.push_back(&cs->bslist[i]);
				offloading_BST.push_back(T);
			}
			else
			{
				//看有沒有影響原本在BS_i底下的UE
				if (influence(&cs->bslist[i], T))
				{
					//有
					offloading_BS.push_back(&cs->bslist[i]);
					offloading_BST.push_back(T);
				}
				else
				{
					//沒有
					uninfluence_BS.push_back(&cs->bslist[i]);
					uninfluence_BST.push_back(T);
				}
			}
		}
	}
	else
	{
		//UE_u是offload UE
		for (int i = 0; i < u->availBS.size(); i++)
		{
			if (u->connecting_BS->num == u->availBS[i]->num)
				continue;
			double T = predict_T(u, u->availBS[i]);						//試算T
			if (T == -1)												//Load飽和
			{
				offloading_BS.push_back(u->availBS[i]);
				offloading_BST.push_back(T);
			}
			else
			{
				//看有沒有影響原本在BS_i底下的UE
				if (influence(u->availBS[i], T))
				{
					//有
					offloading_BS.push_back(u->availBS[i]);
					offloading_BST.push_back(T);
				}
				else
				{
					//沒有
					uninfluence_BS.push_back(u->availBS[i]);
					uninfluence_BST.push_back(T);
				}
			}
		}
	}

	if (uninfluence_BS.size() != 0)						//優先加入不受影響基地台
	{
		if (join_minT_bs(u, &uninfluence_BS, &uninfluence_BST, DB_th))
			return true;
		else
			return false;
	}
	else
	{
		if (depth == depth_max)								//從受影響基地台擇一
		{
			if (join_minT_bs(u, &offloading_BS, &offloading_BST, DB_th))
				return true;
			else
				return false;
		}
		else
		{
			//嘗試offload UE出去看看
			connection_status cs_origin = *cs;

			BS* bs_min_T = NULL;
			double min_T;
			connection_status cs_min_T;

			for (int i = 0; i < offloading_BS.size(); i++)
			{
				*cs = cs_origin;
				vector <UE*> influence_ue;
				vector <UE*> no_influence_ue;

				double T = offloading_BST[i];
				for (int j = 0; j < offloading_BS[i]->connectingUE.size(); j++)
				{
					if (offloading_BS[i]->connectingUE[j]->availBS.size() < 2)
						continue;
					if (offloading_BS[i]->connectingUE[j]->delay_budget > offloading_BS[i]->systemT)
						continue;
					switch (influence(offloading_BS[i]->connectingUE[j]))
					{
					case 0:
						no_influence_ue.push_back(offloading_BS[i]->connectingUE[j]);
						break;
					case 1:
						influence_ue.push_back(offloading_BS[i]->connectingUE[j]);
					default:
						break;
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

				int offloaedd_ue_number = 0;
				for (int j = 0; j < ue_sorted.size(); j++)
				{
//					double T_before = offload_bs.at(i)->systemT;
					if (findbs_dso(ue_sorted.at(j), cs, depth + 1, depth_max, DB_th))
					{
						offloaedd_ue_number++;
						cs->influence++;
						T = predict_T(u, offloading_BS[i]);
						if (T == -1)
							continue;
						if (T > predict_constraint(offloading_BS[i], u, DB_th))
							continue;
						if (all_ue_satisfy(offloading_BS[i], T))
							break;
					}
				}

				if (T == -1)
					continue;
				if (T > predict_constraint(offloading_BS[i], u, DB_th))
					continue;

				if (bs_min_T == NULL)
				{
					bs_min_T = offloading_BS[i];
					min_T = T;
					cs_min_T = *cs;
				}
				else
				{
					if (T < min_T)		//如果T比較小
					{
						bs_min_T = offloading_BS[i];
						min_T = T;
						cs_min_T = *cs;
					}
					else
					{
						if (T == min_T)		//如果T一樣
						{
							if (cs->influence < cs_min_T.influence)		//比影響大小
							{
								bs_min_T = offloading_BS[i];
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
				if(depth == 0)
					cs->Offloaded_UE_Number += cs->influence;
				joinBS(u, bs_min_T, min_T, DB_th);
			}
		}
	}
	return true;
}

void findbs_minT(UE *u, vector <BS> *bslist)
{
	double minT = 0;
	int targetBS_CQI = 0;
	BS *targetBS = NULL;
	for (int i = 0; i < bslist->size(); i++)
	{
		int CQI = get_CQI(u, &bslist->at(i));
		if (CQI == 0)
			continue;
		double T = predict_T(u, &bslist->at(i), CQI);
		if (T == -1)
			continue;
		if (targetBS == NULL)
		{
			targetBS = &bslist->at(i);
			minT = T;
			targetBS_CQI = CQI;
		}
		else
		{
			if (T < minT)
			{
				targetBS = &bslist->at(i);
				minT = T;
				targetBS_CQI = CQI;
			}
			else
			{
				if (T == minT)
				{
					double capacity = get_C(u, &bslist->at(i), CQI);
					double capacity_targetbs = get_C(u, targetBS, targetBS_CQI);
					if (capacity > capacity_targetbs)
					{
						targetBS = &bslist->at(i);
						minT = T;
						targetBS_CQI = CQI;
					}
					else
					{
						if (capacity == capacity_targetbs)
						{
							double distance = get_distance(u, &bslist->at(i));
							double distance_targetBS = get_distance(u, targetBS);
							if (distance < distance_targetBS)
							{
								targetBS = &bslist->at(i);
								minT = T;
								targetBS_CQI = CQI;
							}
						}
					}
				}
			}
		}
	}
	if (targetBS != NULL)
		joinBS_simple(u, targetBS, minT);
}

double get_capa(UE *u, BS *b, int CQI)
{
	double capacity;
	if (b->type == macro)
		capacity = (resource_element * macro_eff[CQI - 1] * total_RBG) / b->connectingUE.size() + 1;
	if (b->type == ap)
		capacity = ap_capacity[CQI - 1] / b->connectingUE.size() + 1;
	return capacity;
}

void findbs_capa(UE *u, vector <BS> *bslist)
{
	vector <double> capacity;
	vector <BS*> availbs;
	for (int i = 0; i < bslist->size(); i++)
	{
		int CQI = get_CQI(u, &bslist->at(i));
		if (CQI == 0)
			continue;
		availbs.push_back(&bslist->at(i));
		capacity.push_back(get_capa(u, &bslist->at(i), CQI));
	}

	while (availbs.size()!=0)
	{
		int max = max_index(capacity);
		double T = predict_T(u, availbs[max]);
		if (T == -1)
		{
			availbs.erase(availbs.begin() + max);
			capacity.erase(capacity.begin() + max);
		}
		else
		{
			joinBS_simple(u, availbs[max], T);
			break;
		}
	}
	return;
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
			{
				CQI++;
			}	
			else
			{
				break;
			}	
		}
	}
	if (b->type == ap)		//計算Wifi的CQI
	{
		for (int i = 0; i < 8; i++)
		{
			if (distance <= range_ap[i])
			{
				CQI++;
			}	
			else
			{
				break;
			}	
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
			{
				CQI++;
			}	
			else
			{
				break;
			}	
		}
	}
	if (b->type == ap)		//計算Wifi的CQI
	{
		for (int i = 0; i < 8; i++)
		{
			if (distance <= range_ap[i])
			{
				CQI++;
			}	
			else
			{
				break;
			}	
		}
	}
	return CQI;
}

double predict_C(UE* u)
{
	if (u->connecting_BS->type == macro)
		return resource_element * macro_eff[u->CQI - 1] * total_RBG;
	if (u->connecting_BS->type == ap)
		return ap_capacity[u->CQI - 1];
}

double predict_C(UE* u, BS* b)
{
	if (b->type == macro)
		return resource_element * macro_eff[get_CQI(u, b) - 1] * total_RBG;
	if (b->type == ap)
		return ap_capacity[get_CQI(u, b) - 1];
}

double predict_C(UE* u, BS* b, int CQI)
{
	if (b->type == macro)
		return resource_element * macro_eff[CQI - 1] * total_RBG;
	if (b->type == ap)
		return ap_capacity[CQI - 1];
}

double get_C(UE* u)
{
	double capacity;
	if (u->connecting_BS->type == macro)
		capacity = resource_element * macro_eff[u->CQI - 1] * total_RBG;
	if (u->connecting_BS->type == ap)
		capacity = ap_capacity[u->CQI - 1];
	return capacity;
}

double get_C(UE* u, BS *b)
{
	int CQI = get_CQI(u, b);
	double capacity;
	if (b->type == macro)
		capacity = resource_element * macro_eff[CQI - 1] * total_RBG;
	if (b->type == ap)
		capacity = ap_capacity[CQI - 1];
	return capacity;
}

double get_C(UE* u, BS *b, int CQI)
{
	double capacity;
	if (b->type == macro)
		capacity = resource_element * macro_eff[CQI - 1] * total_RBG;
	if (b->type == ap)
		capacity = ap_capacity[CQI - 1];
	return capacity;
}

double predict_T(UE* u, BS* b)
{
	//試算u加入b後的lambda
	double lambda = b->lambda + u->lambdai;

	//計算u加入b後的Xj
	double Xj = 0;
	double Xj2 = 0;
	for (int i = 0; i < b->connectingUE.size(); i++)
	{
		double weight_i = b->connectingUE.at(i)->lambdai / lambda;
		double Xij = b->connectingUE.at(i)->packet_size / get_C(b->connectingUE.at(i));
		Xj += (Xij * weight_i);
		Xj2 += (pow(Xij, 2) * weight_i);
	}
	double Xuj = u->packet_size / get_C(u, b);
	Xj += (Xuj * (u->lambdai / lambda));
	double rho = Xj * lambda;

	if (rho >= rho_max)		//Saturated
		return -1;
	Xj2 += (pow(Xuj, 2) * (u->lambdai / lambda));
	return Xj + lambda * Xj2 / (1 - lambda * Xj);
}

double predict_T(UE* u, BS* b, int CQI)
{
	//試算u加入b後的lambda
	double lambda = b->lambda + u->lambdai;

	//計算u加入b後的Xj
	double Xj = 0;
	double Xj2 = 0;
	for (int i = 0; i < b->connectingUE.size(); i++)
	{
		double weight_i = b->connectingUE.at(i)->lambdai / lambda;
		double Xij = b->connectingUE.at(i)->packet_size / get_C(b->connectingUE.at(i));
		Xj += (Xij * weight_i);
		Xj2 += (pow(Xij, 2) * weight_i);
	}
	double Xuj = u->packet_size / get_C(u, b, CQI);
	Xj += (Xuj * (u->lambdai / lambda));
	double rho = Xj * lambda;

	if (rho >= rho_max)		//Saturated
		return -1;
	Xj2 += (pow(Xuj, 2) * (u->lambdai / lambda));
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
		Xj += (Xij * weight_i);
		Xj2 += (pow(Xij, 2) * weight_i);
	}
	return Xj + b->lambda * Xj2 / (1 - b->lambda * Xj);
}

bool influence(BS* b, double T)
{
	if (T <= 50)
		return false;
	if (b->db50 > 0)
		return true;

	for (int i = 0; i < b->connectingUE.size(); i++)
	{
		if (T > b->connectingUE.at(i)->delay_budget)
			return true;
	}
	return false;
}

bool join_minT_bs(UE* u, vector <BS*> *list, vector <double> *list_T, int DB_th)
{
	BS* targetBS = NULL;
	double minT = 0;
	for (int i = 0; i < list->size(); i++)
	{
		if (list_T->at(i) == -1)
			continue;

		if (u->connecting_BS != NULL)
			if (u->connecting_BS->systemT < list_T->at(i))
				continue;

		if (targetBS == NULL)
		{
			if (list_T->at(i) > 50)
				if (list_T->at(i) > predict_constraint(list->at(i), u, DB_th))
					continue;
			targetBS = list->at(i);
			minT = list_T->at(i);
		}
		else
		{
			if (list_T->at(i) < minT)
			{
				if (list_T->at(i) > 50)
					if (list_T->at(i) > predict_constraint(list->at(i), u, DB_th))
						continue;
				targetBS = list->at(i);
				minT = list_T->at(i);
			}
			else
			{
				if (list_T->at(i) == minT)
				{
					double capacity = get_C(u, list->at(i));
					double capacity_targetBS = get_C(u, targetBS);
					if (capacity > capacity_targetBS)
					{
						if (list_T->at(i) > 50)
							if (list_T->at(i) > predict_constraint(list->at(i), u, DB_th))
								continue;
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
								if (list_T->at(i) > 50)
									if (list_T->at(i) > predict_constraint(list->at(i), u, DB_th))
										continue;
								targetBS = list->at(i);
								minT = list_T->at(i);
							}
						}
					}
				}
			}
		}
	}

	if (targetBS == NULL)
		return false;
	else
		joinBS(u, targetBS, minT, DB_th);

	return true;
}

void joinBS_simple(UE* u, BS* targetbs, double T)
{
	u->connecting_BS = targetbs;
	u->CQI = get_CQI(u, targetbs);
	targetbs->connectingUE.push_back(u);
	targetbs->lambda += u->lambdai;
	targetbs->systemT = T;
}

void joinBS(UE* u, BS* targetBS, double T, int DB_th)
{
	if (u->connecting_BS != NULL)
	{
		u->connecting_BS->lambda -= u->lambdai;
		if (u->delay_budget == 50)
			u->connecting_BS->db50--;
		else
		{
			if (u->delay_budget == 100)
				u->connecting_BS->db100--;
			else
				u->connecting_BS->db300--;
		}
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
		u->connecting_BS->T_max = get_T_constraint(u->connecting_BS, DB_th);
	}

//	int predict = predict_constraint(targetBS, u);
	u->connecting_BS = targetBS;
	u->CQI = get_CQI(u, targetBS);
	targetBS->lambda += u->lambdai;
	targetBS->connectingUE.push_back(u);
	if (u->delay_budget == 50)
		targetBS->db50++;
	else
	{
		if (u->delay_budget == 100)
			targetBS->db100++;
		else
			targetBS->db300++;
	}
	targetBS->systemT = T;
//	int old_constrain = targetBS->T_max;
	targetBS->T_max = get_T_constraint(targetBS, DB_th);
}

//檢查UE是否為influence_ue
int influence(UE *u)
{
	bool no_bs_to_offload = true;
	bool has_no_influence_bs = false;
	for (int i = 0; i < u->availBS.size(); i++)
	{
		if (u->availBS[i] == u->connecting_BS)
			continue;
		no_bs_to_offload = false;
		double T = predict_T(u, u->availBS.at(i));
		if (T == -1)
			continue;
		if (T < 50)
			return 0;

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
	if (T < 50)
		return true;
	for (int i = 0; i < b->connectingUE.size(); i++)
		if (b->connectingUE[i]->delay_budget < T)
			return false;
	return true;
}

template <class T>
int max_index(vector <T> v)
{
	int index = 0;
	T max = v[0];
	for (int i = 1; i < v.size(); i++)
	{
		if (v[i] > max)
		{
			index = i;
			max = v[i];
		}
	}
	return index;
}

double get_T_constraint(BS *b, int DB_th)
{
	if (DB_th == 100)
		return 50;
	if (DB_th == 0)
		return 1000;

	double threshold = (double)DB_th / (double)100;
	double HaveToSatisfyUENumber = b->connectingUE.size() * threshold;
	if (b->db300 >= HaveToSatisfyUENumber)
		return 300;
	else
	{
		if ((b->db300 + b->db100) >= HaveToSatisfyUENumber)
			return 100;
		else
		{
			return 50;
		}	
	}
}

double predict_constraint(BS *b, UE *u, int DB_th)
{
	if (DB_th == 100)
		return 50;
	if (DB_th == 0)
		return 1000;

	double threshold = (double)DB_th / (double)100;
	int type_conut[3] = { 0 };
	type_conut[0] = b->db50;
	type_conut[1] = b->db100;
	type_conut[2] = b->db300;

	if(u->delay_budget == 50)
		type_conut[0]++;
	else
	{
		if (u->delay_budget == 100)
			type_conut[1]++;
		else
		{
			type_conut[2]++;
		}	
	}
	double HaveToSatisfyUENumber = (b->connectingUE.size() + 1) * threshold;
	if (type_conut[2] >= HaveToSatisfyUENumber)
		return 300;
	else
	{
		if ((type_conut[2] + type_conut[1]) >= HaveToSatisfyUENumber)
			return 100;
		else
		{
			return 50;
		}	
	}
}
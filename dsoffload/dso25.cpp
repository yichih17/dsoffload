#include<iostream>
#include<vector>
#include<algorithm>
#include"define.h"

using namespace std;

double get_T_constraint25(BS *b);
double predict_constraint25(BS *b, UE *u);

void joinBS25(UE* u, BS* targetBS, double T)
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
		u->connecting_BS->systemT_constraint = get_T_constraint25(u->connecting_BS);
	}
	for (int delete_bs = 0; delete_bs < u->availBS.size(); delete_bs++)
	{
		if (u->availBS[delete_bs]->num == targetBS->num)
		{
			u->availBS.erase(u->availBS.begin() + delete_bs);
			break;
		}
	}
	//	int predict = predict_constraint(targetBS, u);
	u->connecting_BS = targetBS;
	u->CQI = get_CQI(u, targetBS);
	targetBS->lambda += u->lambdai;
	targetBS->connectingUE.push_back(u);
	targetBS->systemT = T;
	//	int old_constrain = targetBS->systemT_constraint;
	targetBS->systemT_constraint = get_T_constraint25(targetBS);
}

bool join_minT_bs25(UE* u, vector <BS*> *list, vector <double> *list_T)
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
	{
		joinBS25(u, targetBS, minT);
	}

	return true;
}

bool findbs_dso25(UE* u, connection_status* cs, int depth, int depth_max)
{
	//計算可連接基地台
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

	//基地台分類
	vector <BS*> influence_bs;
	vector <BS*> no_influence_bs;
	vector <BS*> saturated_bs;
	vector <double> influence_bs_T;
	vector <double> no_influence_bs_T;
	vector <double> saturated_bs_T;

	for (int i = 0; i < u->availBS.size(); i++)
	{
		double T = predict_T(u, u->availBS.at(i), availBS_CQI.at(i));
		if (T == -1)										//load飽和
		{
			saturated_bs.push_back(u->availBS.at(i));
			saturated_bs_T.push_back(T);
		}
		else
		{
			if (T > predict_constraint25(u->availBS.at(i), u))	//T上限飽和
			{
				saturated_bs.push_back(u->availBS.at(i));
				saturated_bs_T.push_back(T);
			}
			else
			{
				if (influence(&cs->bslist.at(i), T))		//受影響
				{
					influence_bs.push_back(u->availBS.at(i));
					influence_bs_T.push_back(T);
				}
				else
				{											//不受影響
					no_influence_bs.push_back(u->availBS.at(i));
					no_influence_bs_T.push_back(T);
				}
			}
		}
	}

	if (no_influence_bs.size() != 0)						//優先加入不受影響基地台
	{
		if (join_minT_bs25(u, &no_influence_bs, &no_influence_bs_T))
			return true;
		else
			return false;
	}
	else
	{
		if (depth == depth_max)								//從受影響基地台擇一
		{
			if (influence_bs.size() == 0)
				return false;
			if (join_minT_bs25(u, &influence_bs, &influence_bs_T))
				return true;
			else
				return false;
		}
		else
		{													//嘗試offload UE出去看看
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
							if (offload_bs.at(i)->connectingUE.at(j)->delay_budget < offload_bs.at(i)->systemT)
							{
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
							}
						}
					}
				}
				else
				{
					for (int j = 0; j < offload_bs.at(i)->connectingUE.size(); j++)
					{
						if (offload_bs.at(i)->connectingUE.at(j)->availBS.size() > 0)
						{
							if (offload_bs.at(i)->connectingUE.at(j)->delay_budget < T)
							{
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

				int offloaedd_ue_number = 0;
				for (int j = 0; j < ue_sorted.size(); j++)
				{
					//					double T_before = offload_bs.at(i)->systemT;
					if (findbs_dso(ue_sorted.at(j), cs, depth + 1, depth_max))
					{
						offloaedd_ue_number++;
						cs->influence++;
						T = predict_T(u, offload_bs.at(i));
						if (T < predict_constraint25(offload_bs.at(i), u) && all_ue_satisfy(offload_bs.at(i), T))
							break;
					}
				}
				if (T == -1)
					continue;
				if (T > predict_constraint25(offload_bs.at(i), u))
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
				joinBS25(u, bs_min_T, min_T);
			}
		}
	}
	return true;
}

double get_T_constraint25(BS *b)
{
	int type_conut[3] = { 0 };
	for (int i = 0; i < b->connectingUE.size(); i++)
	{
		if (b->connectingUE.at(i)->delay_budget == 50)
			type_conut[0]++;
		else
		{
			if (b->connectingUE.at(i)->delay_budget == 100)
				type_conut[1]++;
			else
				type_conut[2]++;
		}
	}

	if (type_conut[2] >= b->connectingUE.size() * 0.25)
		return 300;
	else
	{
		if (type_conut[2] + type_conut[1] >= b->connectingUE.size() * 0.25)
			return 100;
		else
			return 50;
	}
}

double predict_constraint25(BS *b, UE *u)
{
	int type_conut[3] = { 0 };
	for (int i = 0; i < b->connectingUE.size(); i++)
	{
		if (b->connectingUE.at(i)->delay_budget == 50)
			type_conut[0]++;
		else
		{
			if (b->connectingUE.at(i)->delay_budget == 100)
				type_conut[1]++;
			else
				type_conut[2]++;
		}
	}
	if (u->delay_budget == 50)
		type_conut[0]++;
	else
	{
		if (u->delay_budget == 100)
			type_conut[1]++;
		else
			type_conut[2]++;
	}

	if (type_conut[2] >= (b->connectingUE.size() + 1) * 0.25)
		return 300;
	else
	{
		if (type_conut[2] + type_conut[1] >= (b->connectingUE.size() + 1) * 0.25)
			return 100;
		else
			return 50;
	}
}
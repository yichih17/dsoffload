#include <iostream>
#include <math.h>
#include <algorithm>
#include "define.h"

using namespace std;

int range_macro[] = { 1732, 1511, 1325, 1162, 1019, 894, 784, 688, 623, 565, 512, 449, 407, 357, 303 };
int range_ap[] = { 185, 152, 133, 109, 84, 64, 60, 56 };
double macro_eff[15] = { 0.1523, 0.2344, 0.377, 0.6016, 0.8770, 1.1758, 1.4766, 1.9141, 2.4063, 2.7305, 3.3223, 3.9023, 4.5234, 5.1152, 5.5546 };
double ap_capacity[8] = { 6500, 13000, 19500, 26000, 39000, 52000, 58500, 65000 };	//65Mbps = 65000000bps = 65000 bits/ms

//計算AP的訊號範圍
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

//計算UE與BS間的距離
double calc_distance(UE* u, BS* b)
{
	return sqrt(pow((u->coor_X - b->coor_X), 2) + pow((u->coor_Y - b->coor_Y), 2));
}

//計算UE與BS的CQI
int calc_CQI(UE* u, BS* b)
{
	double distance = calc_distance(u, b);
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

//更新UE可連接的BS清單
void availbs(UE* u, vector<BS> *bslist)
{
	//如果UE的availbs已經有資料，清除並更新
	if (u->availBS.size() != 0)
		u->availBS.clear();
	if (u->connecting_BS != NULL)
	{
		for (int i = 0; i < bslist->size(); i++)
		{
			//如果UE已連接BS，availbs就不該出現已連接的BS
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

//UE加入BS前，先更新availbs	*newbs為即將要加入的BS
//void availbs_update(UE* u, BS* newb)
//{
//	//要離目前的BS了，所以目前的BS會變成availbs
//	if (u->connecting_BS != NULL)
//		u->availBS.push_back(u->connecting_BS);
//	int delete_bs;		//newbs在availbs中的位置
//	for (delete_bs = 0; delete_bs < u->availBS.size(); delete_bs++)
//		if (u->availBS[delete_bs]->num == newb->num)
//			break;
//	u->availBS.erase(u->availBS.begin() + delete_bs);
//}

//試算UE加入BS後的Capacity
double predict_Capacity(UE* u, BS* b)
{
	if (b->type == macro)
		return resource_element * macro_eff[calc_CQI(u, b) - 1] * total_RBG / (b->connectingUE.size() + 1);
	if (b->type == ap)
		return ap_capacity[calc_CQI(u, b) - 1] / (b->connectingUE.size() + 1);
}

//計算UE與BS的System Capacity
double getCapacity(UE* u, BS* b)
{
	if (b->type == macro)
		return resource_element * macro_eff[calc_CQI(u, b) - 1] * total_RBG;
	if (b->type == ap)
		return ap_capacity[calc_CQI(u, b) - 1];
}

//計算UE與目前連接BS的Capacity
double getCapacity(UE* u)
{
	if (u->connecting_BS->type == macro)
		return resource_element * macro_eff[calc_CQI(u, u->connecting_BS) - 1] * total_RBG / u->connecting_BS->connectingUE.size();
	if (u->connecting_BS->type == ap)
		return ap_capacity[calc_CQI(u, u->connecting_BS) - 1] / u->connecting_BS->connectingUE.size();
}

/* 試算UE加入BS後，BS的Avg. system time(T*) */
double predictT(UE* u, BS* b)
{
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
	//if (rho == 1)
	//	cout << "stop" << endl;
	if (rho >= 0.999999)
		return -1;
	Xj2 += pow(u->packet_size / (getCapacity(u, b) / (b->connectingUE.size() + 1)), 2) * (u->lambdai / lambda);
	//用M/G/1公式算T
	return Xj + lambda * Xj2 / (1 - lambda * Xj);
}

//計算BS的avg system time(delay)
double getT(BS* b)
{
	double Xj = 0;		//在BS底下的UE的avg service time
	double Xj2 = 0;		//在BS底下的UE的avg Xj2
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
	double Xj = 0;		//在BS底下的UE的avg service time
	for (int i = 0; i < b->connectingUE.size(); i++)
	{
		Xj += b->connectingUE[i]->packet_size / getCapacity(b->connectingUE[i]) * (b->connectingUE[i]->lambdai / b->lambda);
	}
	return b->lambda * Xj;
}

//UE尋找合適的BS
BS* findbs_minT(UE *u, vector <BS> *bslist)
{
	availbs(u, bslist);
	//預設最佳BS為macro eNB
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
			//T比較小
			if (T < T_minTbs)
			{
				T_minTbs = T;
				minTbs = u->availBS[i];
			}
			//T相同，比C
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
	//初始化最小T的BS
	BS* minbs = NULL;
	double minbs_T = 0;

	//計算UE加入BS後的T，並比較最小的出來
	for (int i = 0; i < bslist->size(); i++)
	{
		double T = predictT(u, bslist->at(i));
		//如果T == -1，代表無法加入		*如果最後都不能加入就會return NULL
		if (T == -1)
			continue;
		//如果minbs == NULL，代表目前還沒有minbs的候選人，就把現在這個設成候選人
		if (minbs == NULL)
		{
			minbs = bslist->at(i);
			minbs_T = T;
			continue;
		}
		//如果現在這個的T比minbs的T還小，就把這個改為候選人
		if (minbs_T > T)
		{
			minbs = bslist->at(i);
			minbs_T = T;
			continue;
		}
		//如果現在這個的T跟minbs的T一樣，就比Capacity
		if (minbs_T == T)
		{
			double minbs_capacity = predict_Capacity(u, minbs);
			double capacity = predict_Capacity(u, bslist->at(i));
			//如果現在這個的Capacity比minbs的Capacity還大，就把這個改為候選人
			if (minbs_capacity < capacity)
			{
				minbs = bslist->at(i);
				minbs_T = T;
				continue;
			}
			//如果現在這個的Capacity比minbs的Capacity一樣，就比距離
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
		//如果可選擇的BS中T最小的比原本的BS還大，那就沒必要offload了
		if (u->connecting_BS != NULL)
			if (minbs_T > u->connecting_BS->systemT)
				return NULL;
		return minbs;
	}
}

//檢查offload UE會不會對其他BS造成影響
int is_influence_ue(UE *u)
{
	bool no_bs_to_offload = true;					//有沒有其他BS可以加
	bool has_no_influence_bs = false;				//有沒有offload後不會被影響的BS
	for (int i = 0; i < u->availBS.size(); i++)
	{
		double T = predictT(u, u->availBS[i]);		//加入availbs[i]後的T
		if (T == -1)
			continue;								//跳過
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

//把UE加入BS
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
		u->availBS.push_back(u->connecting_BS);		//UE目前連接的BS會變成他未來的availbs
	}
	//把newbs從availbs中刪除，因為他會變成connecting_bs，所以不該在availbs
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
	//尋找availbs
	if (u->availBS.size() == 0)
		availbs(u, &cs->bslist);
	
	//availbs分類: 依照有無影響分成受影響BS(influence_bs) 與 不受影響BS(no_influence_bs)
	vector <BS*> no_influence_bs;		//不受影響BS
	vector <BS*> influence_bs;			//受影響BS
	vector <BS*> saturated_bs;			//已飽和BS
	for (int i = 0; i < u->availBS.size(); i++)
	{
		bool influence = false;				//試算加入BS後影響的UE數量
		double T = predictT(u, u->availBS.at(i));	//試算加入BS後的T
		//現在無法加入的BS也當作influence，offload UE出去之後也許就能加入
		if (T == -1)
			saturated_bs.push_back(u->availBS[i]);
		else
		{
			//計算availbs[i]下UE的DB有無被滿足
			//有:no_influence_bs; 無:influence_bs
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

	//如果有不受影響BS就選T最小的	
	if (no_influence_bs.size() > 0)
	{
		BS *targetBS = findbs_minT(u, &no_influence_bs);	//可加入的BS中T最小的
		if (targetBS != NULL)
		{
			ue_join_bs(u, targetBS);
			return true;
		}
		else
			return false;
	}
	//沒有不受影響BS的話
	else
	{
		//如果已到演算法最大深度，選T最小的加
		if (depth == MAX_DEPTH)
		{
			BS *targetBS = findbs_minT(u, &influence_bs);	//可加入的BS中T最小的
			if (targetBS != NULL)
			{
				ue_join_bs(u, targetBS);
				return true;
			}
			else
				return false;
		}
		//如果還沒到演算法最大深度 -> offload
		else
		{
			//influence_bs和saturated_bs都該offload看看比影響大小
			influence_bs.insert(influence_bs.end(), saturated_bs.begin(), saturated_bs.end());

			connection_status cs_origin = *cs;		//cs的初始狀態
			
			//min_influence_cs.influence = -1;		//預設值，用以標示還在初始化狀態

			BS* bs_min_T = NULL;			//offload過後的BS中，T最小的
			double min_T;					//offload過後的BS中，T的最小值
			connection_status cs_min_T;		//offload過後的BS中，T最小的cs
			for (int i = 0; i < influence_bs.size(); i++)	//for all influence bs
			{
				*cs = cs_origin;
				double T = predictT(u, influence_bs.at(i));	//UE u如果加入受影響BS j後的T
				
				//UE分類:選出DB不被滿足的UE，然後分為:會影響UE(influence_ue)與不會影響UE(no_influence_ue)
				vector <UE*> influence_ue;		//會影響其他BS的UE
				vector <UE*> no_influence_ue;	//不會影響其他BS的UE
				//如果BS已飽和，T就會是-1，此時要先offload UE出去再看能不能加入，因此T為BS加入UE前的system time
				if (T == -1)
				{
					for (int j = 0; j < influence_bs.at(i)->connectingUE.size(); j++)
					{
						if (influence_bs.at(i)->connectingUE.at(j)->delay_budget < influence_bs.at(i)->systemT)		//不被滿足的UE
						{
							if (influence_bs.at(i)->connectingUE.at(j)->availBS.size() > 0)							//有其他BS可以offload
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
						if (influence_bs.at(i)->connectingUE.at(j)->delay_budget < T)			//不被滿足的UE
						{
							if (influence_bs.at(i)->connectingUE.at(j)->availBS.size() > 0)		//有其他BS可以offload
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

				//UE排序:決定UE被offload出去的順序
				vector <UE*> ue_sorted;		//排序過後的UE清單
				if (no_influence_ue.size() > 0)
				{
					sort(no_influence_ue.begin(), no_influence_ue.end(), ue_sort_cp);		//大到小排序
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
						continue;	//沒有UE可以offload，跳過這個influence_bs
				}

				//Offload UE，直到BS的所有UE都被滿足 或 所有能offload的不影響UE都offload
				for (int j = 0; j < ue_sorted.size(); j++)
				{
					if (findbs_dso(ue_sorted.at(j), cs, depth + 1))
					{
						cs->influence++;					//成功offload一個UE，影響+1
						T = predictT(u, influence_bs.at(i));	//更新T，看還需不需要繼續offload
						if (T != -1 && check_satisfy(influence_bs.at(i), T))	//offload掉一些UE後，UE能夠加入且能夠滿足所有DB了
							break;
					}
				}
				if (T == -1)		//offload所有UE後還是無法加入UE
					continue;
				//比較
				if (bs_min_T == NULL)
				{
					bs_min_T = influence_bs.at(i);
					min_T = T;
					cs_min_T = *cs;
				}
				else
				{
					if (T < min_T)		//如果T比較小
					{
						bs_min_T = influence_bs.at(i);
						min_T = T;
						cs_min_T = *cs;
					}
					else
					{
						if (T == min_T)		//如果T一樣
						{
							if (cs->influence < cs_min_T.influence)		//比影響大小
							{
								bs_min_T = influence_bs.at(i);
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
		cout << "UE移出BS" << u->connecting_BS->num << "前:\nBS有" << u->connecting_BS->connectingUE.size() << "個UE" << endl;
		u->connecting_BS->lambda -= u->lambdai;
		int i;
		for (i = 0; i < u->connecting_BS->connectingUE.size(); i++)
		{
			if (u->connecting_BS->connectingUE[i]->num == u->num)
				break;
		}
		u->connecting_BS->connectingUE.erase(u->connecting_BS->connectingUE.begin() + i - 1);
		cout << "UE移出BS" << u->connecting_BS->num << "後:\nBS有" << u->connecting_BS->connectingUE.size() << "個UE" << endl;
	}
	u->connecting_BS = b;
	b->lambda += u->lambdai;
	b->connectingUE.push_back(u);
	b->systemT = getT(b);
}
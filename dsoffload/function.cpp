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

//計算UE與BS間的距離
double getDistance(UE* u, BS* b)
{
	return sqrt(pow((u->coor_X - b->coor_X), 2) + pow((u->coor_Y - b->coor_Y), 2));
}

//計算UE與BS的Channel Quality
int getCQI(UE* u, BS* b)
{
	double dis = getDistance(u, b);
	int CQI = -1;
	if (b->type == macro)	//計算LTE的CQI
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
	if (b->type == ap)		//計算Wifi的CQI
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
	return 16;				//出錯
}

//更新UE可連接的BS清單
void availbs(UE* u, vector<BS> *bslist)
{
	//如果UE的availbs已經有資料，清除並更新
	if (u->availBS.size() != 0)
		u->availBS.clear();
	//如果UE已連接BS，availbs就不該出現已連接的BS
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

//UE加入BS前，先更新availbs	*newbs為即將要加入的BS
void availbs_update(UE* u, BS* newb)
{
	//要離目前的BS了，所以目前的BS會變成availbs
	if (u->connecting_BS != NULL)
		u->availBS.push_back(u->connecting_BS);
	int delete_bs;		//newbs在availbs中的位置
	for (delete_bs = 0; delete_bs < u->availBS.size(); delete_bs++)
		if (u->availBS[delete_bs]->num == newb->num)
			break;
	u->availBS.erase(u->availBS.begin() + delete_bs);
}

//試算UE加入BS後的Capacity
double predict_Capacity(UE* u, BS* b)
{
	if (b->type == macro)
		return resource_element * macro_eff[getCQI(u, b) - 1] * total_RBG / (b->connectingUE.size() + 1);
	if (b->type == ap)
		return ap_capacity[getCQI(u, b) - 1] / (b->connectingUE.size() + 1);
	return -1;
}

//計算UE與BS的System Capacity
double getCapacity(UE* u, BS* b)
{
	if (b->type == macro)
		return resource_element * macro_eff[getCQI(u, b) - 1] * total_RBG;
	if (b->type == ap)
		return ap_capacity[getCQI(u, b) - 1];
	return -1;
}

//計算UE與目前連接BS的Capacity
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

/* 試算UE加入BS後，BS的Avg. system time(T*) */
double predictT(UE* u, BS* b)
{
	//試算u加入b後的lambda
	double lambda = b->lambda + u->lambdai;
	//計算u加入b後的Xj
	double Xj = 0;

/*	for (int i = 0; i < b->connectingUE.size(); i++)	//原本在b的UE的Xij加起來
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

	for (int i = 0; i < b->connectingUE.size(); i++)	//原本在b的UE的Xij加起來
		Xj += b->connectingUE[i]->packet_size / (getCapacity(b->connectingUE[i], b) / (b->connectingUE.size() + 1)) * b->connectingUE[i]->lambdai / lambda;
	Xj += u->packet_size / (getCapacity(u, b) / (b->connectingUE.size() + 1)) * (u->lambdai / lambda);
	//if (Xj * lambda > 1)								//When Rho > 1, return -1;
	//	return -1;
	//計算u加入b後的Xj^2
	double Xj2 = 0;
	for (int i = 0; i < b->connectingUE.size(); i++)	//原本在b的UE的Xij2加起來
		Xj2 += pow(b->connectingUE[i]->packet_size / (getCapacity(b->connectingUE[i], b) / (b->connectingUE.size() + 1)), 2) * b->connectingUE[i]->lambdai / lambda;
	Xj2 += pow(u->packet_size / (getCapacity(u, b) / (b->connectingUE.size() + 1)), 2) * (u->lambdai / lambda);

	//用M/G/1公式算T
	double T = Xj + lambda * Xj2 / (1 - lambda * Xj);
	if (Xj * lambda > 1)								//When Rho > 1, return -1;
		return -1;
	return Xj + lambda * Xj2 / (1 - lambda * Xj);
}

//計算BS的avg system time(delay)
double getT(BS* b)
{
	double Xj = 0;		//在BS底下的UE的avg service time
	for (int i = 0; i < b->connectingUE.size(); i++)
		Xj += b->connectingUE[i]->packet_size / (getCapacity(b->connectingUE[i], b) / b->connectingUE.size()) * b->connectingUE[i]->lambdai / b->lambda;
	double Xj2 = 0;		//在BS底下的UE的avg Xj2
	for (int i = 0; i < b->connectingUE.size(); i++)	
		Xj2 += pow(b->connectingUE[i]->packet_size / (getCapacity(b->connectingUE[i], b) / b->connectingUE.size()), 2) * b->connectingUE[i]->lambdai / b->lambda;
	return Xj + b->lambda * Xj2 / (1 - b->lambda * Xj);
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
			}
		}
	}
	//如果可選擇的BS中T最小的比原本的BS還大，那就沒必要offload了
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
	}
	availbs_update(u, b);
	u->connecting_BS = b;
	b->lambda += u->lambdai;
	b->connectingUE.push_back(u);
	b->systemT = getT(b);
}

////把UE加入BS
//void ue_join_bs(UE* u, BS* b, connection_status* cs)
//{
//	if (b == NULL)
//		return;
//	//移出原BS
//	if (u->connecting_BS != NULL)
//	{
//		cs->bslist.at(u->connecting_BS->num).lambda -= u->lambdai;																			//更新lambda
//		int delete_ue;
//		for (delete_ue = 0; delete_ue < cs->bslist.at(u->connecting_BS->num).connectingUE.size(); delete_ue++)
//			if (cs->bslist.at(u->connecting_BS->num).connectingUE.at(delete_ue)->num == u->num)
//				break;
//		cs->bslist.at(u->connecting_BS->num).connectingUE.erase(cs->bslist.at(u->connecting_BS->num).connectingUE.begin() + delete_ue);		//把UE從清單中刪除
//		cs->bslist.at(u->connecting_BS->num).systemT = getT(&cs->bslist.at(u->connecting_BS->num));											//更新T
//	}
//
//	//加入新BS
//	u->connecting_BS = &cs->bslist.at(b->num);			//更新UE連接的BS
//	availbs(u, &cs->bslist);							//更新UE的availbs
//	cs->bslist.at(b->num).lambda += u->lambdai;			//更新lambda
//	cs->bslist.at(b->num).connectingUE.push_back(u);	//把UE新增到清單
//	cs->bslist.at(b->num).systemT = getT(&cs->bslist.at(b->num));			//更新T
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
	//尋找or更新availbs		*會到這裡的UE有兩種:1.New UE 2.被offload的UE 這兩種不會沒有availbs
	availbs(u, &cs->bslist);
	
	//availbs分類: 依照有無影響分成受影響BS(influence_bs) 與 不受影響BS(no_influence_bs)
	vector <BS*> no_influence_bs;		//不受影響BS
	vector <BS*> influence_bs;			//受影響BS
	for (int i = 0; i < u->availBS.size(); i++)
	{
		int influence_ue_number = 0;			//試算加入BS後影響的UE數量
		double T = predictT(u, u->availBS.at(i));	//試算加入BS後的T
		//現在無法加入的BS也當作influence，offload UE出去之後也許就能加入
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

	//如果有不受影響BS就選T最小的		*targetBS不會是NULL，是的話BS就不會是no_influence_bs
	if (no_influence_bs.size() > 0)
	{
		BS *targetBS = findbs_minT(u, &no_influence_bs);	//可加入的BS中T最小的
		ue_join_bs(u, targetBS);
		return;
	}
	//沒有不受影響BS的話
	else
	{
		//如果已到演算法最大深度，選T最小的加
		if (k == MAX_DEPTH)
		{
			BS *targetBS = findbs_minT(u, &influence_bs);	//可加入的BS中T最小的
			if (targetBS != NULL)
			{
				ue_join_bs(u, targetBS);
				return;
			}
			else
			{
				//被offload的UE且沒有更好的選擇才會到這，但它原本有已連接BS，所以不能當作outage UE
				if (u->connecting_BS == NULL)
					cs->outage_dso++;
				return;
			}
		}
		//如果還沒到演算法最大深度 -> offload
		else
		{
			connection_status min_influence_cs;		//最小的影響的cs
			min_influence_cs.influence = -1;		//預設值，用以標示還在初始化狀態
			BS* min_influence_bs = NULL;

			for (int j = 0; j < influence_bs.size(); j++)	//for all influence bs
			{
				double T = predictT(u, influence_bs[j]);	//UE u如果加入受影響BS j後的T
				
				//UE分類:選出DB不被滿足的UE，然後分為:會影響UE(influence_ue)與不會影響UE(no_influence_ue)
				vector <UE*> influence_ue;		//會影響其他BS的UE
				vector <UE*> no_influence_ue;	//不會影響其他BS的UE
				//如果BS已飽和，T就會是-1，此時要先offload UE出去再看能不能加入，因此T為BS加入UE前的system time
				if (T == -1)
				{				
					for (int k = 0; k < influence_bs.at(j)->connectingUE.size(); k++)
					{
						if (influence_bs.at(j)->connectingUE[k]->delay_budget < influence_bs.at(j)->systemT)	//如果UE的DB小於T(不被滿足)
						{
							if (influence_bs.at(j)->connectingUE[k]->availBS.size() == 0)						//如果UE沒其他BS可以選擇就跳過
								continue;
							if (calc_influence(influence_bs.at(j)->connectingUE[k]) == 1)						//如果offload UE會再影響其他BS就是influence_ue
								influence_ue.push_back(influence_bs.at(j)->connectingUE[k]);
							else
								no_influence_ue.push_back(influence_bs.at(j)->connectingUE[k]);
						}
					}
				}
				//如果BS未飽和，T就會是正值，假設UE加入後要把那些UE offload出去
				else
				{
					//UE分類:選出DB不被滿足的UE，然後分為:會影響UE(influence_ue)與不會影響UE(no_influence_ue)
					for (int k = 0; k < influence_bs.at(j)->connectingUE.size(); k++)
					{
						if (influence_bs.at(j)->connectingUE[k]->delay_budget < T)			//如果UE的DB小於T(不被滿足)
						{
							if (influence_bs.at(j)->connectingUE[k]->availBS.size() == 0)	//如果UE沒其他BS可以選擇就跳過
								continue;
							if (calc_influence(influence_bs.at(j)->connectingUE[k]) == 1)	//如果offload UE會再影響其他BS就是influence_ue
								influence_ue.push_back(influence_bs.at(j)->connectingUE[k]);
							else
								no_influence_ue.push_back(influence_bs.at(j)->connectingUE[k]);
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
				}
				if (ue_sorted.size() == 0)
					break;
				//Offload UE，直到BS的所有UE都被滿足 或 所有能offload的不影響UE都offload
				connection_status cs_temp = *cs;
				bool join = false;		//判斷要不要offload UE的index
				for (int k = 0; k < ue_sorted.size(); k++)
				{
					findbs_dso(&cs_temp.uelist[ue_sorted.at(k)->num], &cs_temp, k + 1);
					T = predictT(&cs_temp.uelist[u->num], &cs_temp.bslist[influence_bs[j]->num]);
					if (check_satisfy(&cs_temp.bslist[influence_bs[j]->num], T))	//offload掉一些UE後，能夠滿足所有DB了
					{
						join = true;
						break;
					}
				}
				if (T != -1)		//offload所有UE後沒辦法滿足所有DB，但可加入
					join = true;
				//如果所有可offload的UE都offload了，BS還是飽和，join就改為false，就不比較影響大小，加不進去比洨?
				if (join == false)			//跳過這個influence_bs
					break;
				ue_join_bs(&cs_temp.uelist[u->num], &cs_temp.bslist[influence_bs[j]->num]);		//已經為UE挪出空位了，然後就把UE加進來

				//比較影響(為了那個UE移動了多少UE)
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
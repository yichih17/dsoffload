#include <iostream>
#include <math.h>
#include <algorithm>
#include "define.h"

using namespace std;

int range_macro[] = { 1732, 1511, 1325, 1162, 1019, 894, 784, 688, 623, 565, 512, 449, 407, 357, 303 };
int range_ap[8];
double macro_eff[15] = { 0.1523, 0.2344, 0.377, 0.6016, 0.8770, 1.1758, 1.4766, 1.9141, 2.4063, 2.7305, 3.3223, 3.9023, 4.5234, 5.1152, 5.5546 };
double ap_capacity[8] = { 6500, 13000, 19500, 26000, 39000, 52000, 58500, 65000 };	//65Mbps = 65000000bps = 65000 bits/ms

/* 計算AP*/
void countAPrange()
{
	int SINR_AP[8] = { 4, 7, 9, 12, 16, 20, 21, 22 };
	for (int i = 0; i < 8; i++)
	{
		//Path loss: 140.7 + 36.7 log(D), D in km.
		double distance = pow(10, (-(SINR_AP[i] - 78 - power_ap) - 122.7) / 35.1) * 1000;
		range_ap[i] = (int)distance;
	}
}

double getDistance(UE* u, BS* b)
{
	return sqrt(pow((u->coor_X - b->coor_X), 2) + pow((u->coor_Y - b->coor_Y), 2));
}

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

/*試算Capacity*/
double predict_Capacity(UE* u, BS* b)
{
	if (b->type == macro)
		return resource_element * macro_eff[getCQI(u, b) - 1] * total_RBG / (b->connectingUE.size() + 1);
	if (b->type == ap)
		return ap_capacity[getCQI(u, b) - 1] / (b->connectingUE.size() + 1);
	return 0;
}

/*計算UE與目前連接BS的System Capacity(UE擁有所有RB)*/
double getCapacity(UE* u, BS* b)
{
	if (b->type == macro)
		return resource_element * macro_eff[getCQI(u, b) - 1] * total_RBG;
	if (b->type == ap)
		return ap_capacity[getCQI(u, b) - 1];
	return 0;
}

/* 計算UE與目前連接BS的Capacity */
double getCapacity(UE* u)
{
	if (u->connecting_BS = NULL)
		cout << "Error, UE" << u->num << " no connecting BS.\n";
	else
	{
		if (u->connecting_BS->type == macro)
			return resource_element * macro_eff[getCQI(u, u->connecting_BS) - 1] * total_RBG / u->connecting_BS->connectingUE.size();
		if (u->connecting_BS->type == ap)
			return ap_capacity[getCQI(u, u->connecting_BS) - 1] / u->connecting_BS->connectingUE.size();
	}
	return 0;
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
	if (Xj * lambda > 1)								//When Rho > 1, return 0;
		return -1;
	//計算u加入b後的Xj^2
	double Xj2 = 0;
	for (int i = 0; i < b->connectingUE.size(); i++)	//原本在b的UE的Xij2加起來
		Xj2 += pow(b->connectingUE[i]->packet_size / (getCapacity(b->connectingUE[i], b) / (b->connectingUE.size() + 1)), 2) * b->connectingUE[i]->lambdai / lambda;
	Xj2 += pow(u->packet_size / (getCapacity(u, b) / (b->connectingUE.size() + 1)), 2) * (u->lambdai / lambda);
	//用M/G/1公式算T
	return Xj + lambda * Xj2 / (1 - lambda * Xj);
}

double getT(BS* b)
{
	double Xj = 0;
	for (int i = 0; i < b->connectingUE.size(); i++)	//在b的UE的Xij加起來
		Xj += b->connectingUE[i]->packet_size / (getCapacity(b->connectingUE[i], b) / b->connectingUE.size()) * b->connectingUE[i]->lambdai / b->lambda;
	double Xj2 = 0;
	for (int i = 0; i < b->connectingUE.size(); i++)	//在b的UE的Xij2加起來
		Xj2 += pow(b->connectingUE[i]->packet_size / (getCapacity(b->connectingUE[i], b) / b->connectingUE.size()), 2) * b->connectingUE[i]->lambdai / b->lambda;
	return Xj + b->lambda * Xj2 / (1 - b->lambda * Xj);
}

//UE尋找合適的BS
BS* findbs_minT(UE *u, vector <BS> *bslist)
{
	if (u->availBS.size() == 0)
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
			case 0:
				cout << "UE" << u->num << " to BS" << bslist->at(i).num << " CQI is 0" << endl;
				break;
			default:
				u->availBS.push_back(&bslist->at(i));
				break;
			}
		}
	}	

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

int calc_influence(UE *u, vector <BS> *bslist)
{
	u->availBS.clear();
	for (int i = 0; i < bslist->size(); i++)
	{
		if (&bslist[i].at(i) == u->connecting_BS)
			continue;
		int CQI = getCQI(u, &bslist->at(i));
		switch (CQI)
		{
		case 16:
			cout << "CQI error, BS type is neither macro nor ap." << endl;
			break;
		case -1:
			break;
		case 0:
			cout << "UE" << u->num << " to BS" << bslist->at(i).num << " CQI is 0" << endl;
			break;
		default:
			u->availBS.push_back(&bslist->at(i));
			break;
		}
	}
	for (int i = 0; i < u->availBS.size(); i++)
	{
		int influence = 0;
		double T = predictT(u, u->availBS[i]);
		for (int j = 0; j < u->availBS[i]->connectingUE.size(); j++)
		{
			if (T > u->availBS[i]->connectingUE[j]->delay_budget)
				influence++;
		}
		if (influence == 0)
			return 0;
	}
	return -1;
}

bool uecompare(UE a, UE b) { return (a.lambdai/getCapacity(&a)) < (b.lambdai/getCapacity(&b)); }

bool is_all_ue_be_satisify(BS* b)
{
	int unsatisfy = 0;
	for (int i = 0; i < b->connectingUE.size(); i++)
		if (b->systemT > b->connectingUE[i]->delay_budget)
			unsatisfy++;
	return unsatisfy > 0 ? true : false;
}

connection_status findbs_dso(UE u, connection_status cs, int k)
{
	if (u.availBS.size() == 0)
	{
		//計算UE的availBS
		for (int i = 0; i < cs.bslist.size(); i++)
		{
			if (&cs.bslist.at(i) == u.connecting_BS)		//可連接的BS要扣掉正在連接的BS
				continue;
			int CQI = getCQI(&u, &cs.bslist.at(i));
			switch (CQI)
			{
			case 16:
				cout << "CQI error, BS type is neither macro nor ap." << endl;
				break;
			case -1:
				break;
			case 0:
				cout << "UE" << u.num << " to BS" << cs.bslist.at(i).num << " CQI is 0" << endl;
				break;
			default:
				u.availBS.push_back(&cs.bslist.at(i));
				break;
			}
		}
	}
	
	vector <BS> no_influence_bs;
	vector <BS> influence_bs;
	//availBS分類
	for (int i = 0; i < u.availBS.size(); i++)
	{
		double T_after = predictT(&u, u.availBS[i]);
		int influence_ue_number = 0;
		for (int j = 0; j < u.availBS[i]->connectingUE.size(); j++)
			if (T_after > u.availBS[i]->connectingUE[j]->delay_budget)
				influence_ue_number++;
		if (influence_ue_number != 0)
			influence_bs.push_back(*u.availBS[i]);
		else
			no_influence_bs.push_back(*u.availBS[i]);
	}
	//有不受影響BS就選T最小的加入
	if (no_influence_bs.size() > 0)
	{
		BS *targetBS = findbs_minT(&u, &no_influence_bs);
		if (targetBS == NULL)
		{
			cs.outage_dso++;
			return cs;
		}
		cs.uelist[u.num].connecting_BS = targetBS;
		cs.bslist[targetBS->num].connectingUE.push_back(&cs.uelist[u.num]);
		cs.bslist[targetBS->num].lambda += cs.uelist[u.num].lambdai;
		cs.bslist[targetBS->num].systemT = getT(&cs.bslist[targetBS->num]);
		cs.influence++;
		return cs;
	}
	//沒有不受影響BS的話
	else
	{
		//如果已到演算法最大深度，選T最小的加
		if (k == MAX_DEPTH)
		{
			BS *targetBS = findbs_minT(&u, &influence_bs);
			if (targetBS == NULL)
			{
				cs.outage_dso++;
				return cs;
			}
			cs.uelist[u.num].connecting_BS = targetBS;
			cs.bslist[targetBS->num].connectingUE.push_back(&cs.uelist[u.num]);
			cs.bslist[targetBS->num].lambda += cs.uelist[u.num].lambdai;
			cs.bslist[targetBS->num].systemT = getT(&cs.bslist[targetBS->num]);
			cs.influence++;
			return cs;
		}
		//如果還沒到演算法最大深度 -> offload
		else
		{
			connection_status min_influence_cs;		//最小的影響的cs
			min_influence_cs.influence = -1;

			for (int j = 0; j < influence_bs.size(); j++)	//for all influence bs
			{
				connection_status cs_temp = cs;				//offloading tree的cs，用來與上層tree的cs做分隔
				double T = predictT(&u, &influence_bs[j]);	//UE u如果加入受影響BS j後的T

				//UE分類:選出DB不被滿足的UE，然後分為:會影響UE(influence_ue)與不會影響UE(no_influence_ue)
				vector <UE> influence_ue;		//會影響其他BS的UE
				vector <UE> no_influence_ue;	//不會影響其他BS的UE
				for (int k = 0; k < influence_bs[j].connectingUE.size(); k++)
				{
					if (influence_bs[j].connectingUE[k]->delay_budget < T)							//如果UE的DB小於T(不被滿足)
					{
						if (influence_bs[j].connectingUE[k]->availBS.size() <= 1)					//如果UE沒其他BS可以選擇就跳過
							continue;
						if (calc_influence(influence_bs[j].connectingUE[k], &cs_temp.bslist) != 0)	//如果offload UE會再影響其他BS
							influence_ue.push_back(*influence_bs[j].connectingUE[k]);
						else
							no_influence_ue.push_back(*influence_bs[j].connectingUE[k]);
					}
				}
				
				//UE排序:決定UE被offload出去的順序
				vector <UE> ue_sorted;			//排序過後的UE清單
				if (no_influence_bs.size() > 0)
				{
					sort(no_influence_ue.begin(), no_influence_ue.end(), uecompare);
					ue_sorted = no_influence_ue;
					if (influence_ue.size() > 0)
					{
						sort(no_influence_ue.begin(), no_influence_ue.end(), uecompare);
						ue_sorted.insert(ue_sorted.end(), influence_ue.begin(), influence_ue.end());
					}
				}
				else
				{
					if (influence_ue.size() > 0)
					{
						sort(influence_ue.begin(), influence_ue.end(), uecompare);
						ue_sorted = influence_ue;
					}
				}
				
				//Offload 不影響UE:優先offload不會影響其他BS的UE，直到BS的所有UE都被滿足 或 所有能offload的不影響UE都offload
				if (no_influence_ue.size() != 0)
				{
					for (int k = 0; k < no_influence_ue.size(); k++)
					{
						//不會影響UE加入T最小的BS
						BS *targetBS = findbs_minT(&no_influence_ue[k], &cs_temp.bslist);

						//移出原BS
						cs_temp.bslist[influence_bs[j].num].lambda -= no_influence_ue[k].lambdai;
						vector <UE*>::iterator delete_ue_num = cs_temp.bslist[influence_bs[j].num].connectingUE.begin();	//計算UE在原BS.connectingUE的位置
						for (int l = 0; l < cs_temp.bslist[influence_bs[j].num].connectingUE.size(); l++, delete_ue_num++)
							if (cs_temp.bslist[influence_bs[j].num].connectingUE[l]->num == no_influence_ue[k].num)
								break;
						cs_temp.bslist[influence_bs[j].num].connectingUE.erase(delete_ue_num);								//把UE從原BS.connectingUE去除
						cs_temp.bslist[influence_bs[j].num].systemT = getT(&cs_temp.bslist[influence_bs[j].num]);			//更新原BS的system time

						//加入新BS
						cs_temp.uelist[no_influence_ue[k].num].connecting_BS = targetBS;									//更改UE的連接BS
						cs_temp.bslist[targetBS->num].connectingUE.push_back(&cs_temp.uelist[no_influence_ue[k].num]);		//新BS的連接UE清單新增
						cs_temp.bslist[targetBS->num].lambda += cs_temp.uelist[no_influence_ue[k].num].lambdai;				//更新lambda
						cs_temp.bslist[targetBS->num].systemT = getT(&cs_temp.bslist[targetBS->num]);						//更新system time
						cs_temp.influence++;
					}
				}
				else
					for (int k = 0; k < influence_ue.size(); k++)
						cs_temp = findbs_dso(influence_ue[k], cs_temp, k + 1);
				//比較影響
				if (min_influence_cs.influence == -1)
					min_influence_cs = cs_temp;
				else
					if (cs_temp.influence<min_influence_cs.influence)
						min_influence_cs = cs_temp;
			}
			return min_influence_cs;
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
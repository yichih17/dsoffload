#include"define.h"
#include<iostream>
#include<iomanip>
#include<fstream>
#include<string>
#include<vector>

using namespace std;

vector <UE> vuelist;
vector <BS> vbslist;

void SINR_based(vector <UE> uelist, vector <BS> bslist);
void minT_algorithm(vector <UE> uelist, vector <BS> bslist);
void proposed_algorithm(vector <UE> uelist, vector <BS> bslist);

/*Outage: 當UE可連接的基地台皆超出負荷，就會沒有基地台可連接，因此定義為outage */
//int outage_minT = 0;			//min T algorithm的outage UE數量
int outage_sinr = 0;			//sinr algorithm的outage UE數量	**未完成
int outage_proposed = 0;		//proposed algorithm的outage UE數量	**未完成
int max_depth;

/*環境初始設定*/
void initialconfig()
{
	//Define Macro eNB
	BS macro;
	macro.num = 0;
	macro.type = type_bs::macro;
	macro.coor_X = 0;
	macro.coor_Y = 0;
	macro.connectingUE.clear();
	macro.lambda = 0;
	macro.systemT = 0;
	vbslist.push_back(macro);
}

/*讀取UE分布(座標)*/
void readUE()
{
	ifstream freadUE;
	freadUE.open("UE_dis.txt", ios::in);
	if (freadUE.fail())						//抓計事本內的UE座標
	{
		cout << "UE分布不存在\n";
	}
	else
	{
		int num = 0;						//num = UE編號
		string bufferx, buffery;
		while (freadUE >> bufferx)
		{
			freadUE >> buffery;				//讀取y座標
			UE temp;
			temp.num = num++;			//assign編號，然後index++
			temp.coor_X = stof(bufferx);//String to double
			temp.coor_Y = stof(buffery);//把座標給UE
			//initialize
			temp.connecting_BS = NULL;
			temp.bit_rate = 10;
			temp.packet_size = 800;
			temp.delay_budget = 100;
			temp.lambdai = temp.bit_rate / temp.packet_size;
			vuelist.push_back(temp);
		}
	}
}

/*讀取AP分布(座標)*/
void readAP(vector <BS> &bslist)
{
	ifstream freadAP;
	freadAP.open("AP_dis.txt", ios::in);
	if (freadAP.fail())						//抓計事本內的UE座標
	{
		cout << "AP分布不存在\n";
	}
	else
	{
		int i = 1;
		string bufferx, buffery;
		while (freadAP >> bufferx)
		{
			freadAP >> buffery;				//讀取y座標	
			BS temp;
			temp.num = i++;
			temp.type = ap;				//設定基地台類型(AP)
			temp.coor_X = stof(bufferx);//String to double
			temp.coor_Y = stof(buffery);//把座標給AP
			//initialize
			temp.connectingUE.clear();
			temp.lambda = 0;
			temp.systemT = 0;
			vbslist.push_back(temp);
		}
	}
}

int main()
{
	for (int times = 1; times <101; times++)
	{
		for (int number = 1; number < 16; number++)
		{
			int number_ap = 200;
			int number_ue = number * 1000;
			
			distribution(number_ap, number_ue);	//產生AP、UE分布

			vbslist.clear();					//vbslist, vuelist初始化
			vuelist.clear();

			initialconfig();					//macro eNB初始化
			readAP(vbslist);					//讀入BS
			readUE();							//讀入UE
			//printf("times: %d, UE number: %d\n", times, number_ue);
//			countAPrange();						//計算AP可傳送資料的範圍大小
//			packet_arrival(number);					//產生packet arrival
			for (int depth = 0; depth < 3; depth++)
			{
				printf("times: %d, UE number: %d, depth: %d\n", times, number_ue, depth);
				max_depth = depth;
				proposed_algorithm(vuelist, vbslist);
			}
			minT_algorithm(vuelist, vbslist);
			SINR_based(vuelist, vbslist);
		}
	}
	return 0;
}

void SINR_based(vector<UE> uelist, vector<BS> bslist)
{
	for (int i = 0; i < uelist.size(); i++)
		findbs_sinr(&uelist.at(i), &bslist);
	result_output(&bslist, &uelist, "SINR");
}

void minT_algorithm(vector<UE> uelist, vector<BS> bslist)
{
	int outage_minT = 0;
	for (int i = 0; i < uelist.size(); i++)
	{
		BS *target_bs = findbs_minT(&uelist[i], &bslist);
		if (target_bs == NULL)
			outage_minT++;
		else
			add_UE_to_BS(&uelist[i], target_bs);
	}
	result_output(&bslist, &uelist, "minT");
}

void proposed_algorithm(vector <UE> uelist, vector <BS> bslist)
{
	int outage_proposed = 0;
	connection_status cs;
	cs.bslist.assign(bslist.begin(), bslist.end());
	cs.uelist.assign(uelist.begin(), uelist.end());
	cs.outage_dso = 0;
	for (int i = 0; i < cs.uelist.size(); i++)
	{
		cs.influence = 0;
		findbs_dso(&cs.uelist[i], &cs, 0);
	}
	
	uelist.assign(cs.uelist.begin(), cs.uelist.end());
	bslist.assign(cs.bslist.begin(), cs.bslist.end());
	char filename[50];
	sprintf_s(filename, "dso_%d", max_depth);
	result_output(&bslist, &uelist, filename);
}
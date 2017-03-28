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
void proposed_algorithm_new(vector <UE> uelist, vector <BS> bslist);

int max_depth;

int calc_dis_count = 0;
int calc_cqi_count = 0;
int availbs_count = 0;
int predict_capacity_count = 0;
int getcapacity1_count = 0;
int getcapacity2_count = 0;
int predictT_count = 0;
int getT_count = 0;
int is_influence_ue_count = 0;
int is_all_ue_be_satisify_count = 0;
int ue_join_bs_count = 0;
int check_satisfy_count = 0;
int outage_proposed = 0;

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
			temp.num = num++;				//assign編號，然後index++
			temp.coor_X = stof(bufferx);	//String to double
			temp.coor_Y = stof(buffery);	//把座標給UE
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
void readAP()
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
			temp.type = ap;					//設定基地台類型(AP)
			temp.coor_X = stof(bufferx);	//String to double
			temp.coor_Y = stof(buffery);	//把座標給AP
			//initialize
			temp.connectingUE.clear();
			temp.lambda = 0;
			temp.systemT = 0;
			vbslist.push_back(temp);
		}
	}
}

void initialUE()
{
	for (int i = 0; i < vuelist.size(); i++)
	{
		vuelist.at(i).num = i;
		vuelist.at(i).connecting_BS = NULL;
		vuelist.at(i).bit_rate = 10;
		vuelist.at(i).packet_size = 800;
		vuelist.at(i).delay_budget = 100;
		vuelist.at(i).lambdai = vuelist.at(i).bit_rate / vuelist.at(i).packet_size;
	}
}

void initialAP()
{
	for (int i = 1; i < vbslist.size(); i++)
	{
		vbslist.at(i).num = i;
		vbslist.at(i).type = ap;
		vbslist.at(i).connectingUE.clear();
		vbslist.at(i).lambda = 0;
		vbslist.at(i).systemT = 0;
	}
}

int main()
{
	for (int times = 1; times < 101; times++)
	{
		double start_time = 0, end_time = 0;
		start_time = clock();
		for (int number = 1; number < 16; number++)
		{
			int number_ap = 200;
			int number_ue = number * 1000;
			printf("times: %d, UE number: %d\n", times, number_ue);
			vbslist.clear();					//vbslist清空
			vuelist.clear();					//vuelist清空

			initialconfig();					//macro eNB初始化
			distribution(number_ap, number_ue);	//產生AP、UE分布
			initialUE();						//UE參數初始化
			initialAP();						//AP參數初始化
//			readAP();							//讀入BS
//			readUE();							//讀入UE
//			countAPrange();						//計算AP可傳送資料的範圍大小
//			packet_arrival(number);				//產生packet arrival
			
			for (int depth = 0; depth < 3; depth++)
			{
				max_depth = depth;
				proposed_algorithm(vuelist, vbslist);
				//proposed_algorithm_new(vuelist, vbslist);
			}
			minT_algorithm(vuelist, vbslist);
			SINR_based(vuelist, vbslist);
		}
		end_time = clock();
		cout << "一輪執行時間 : " << (end_time - start_time) / 1000 << " s\n\n";
	}
	return 0;
}

void SINR_based(vector<UE> uelist, vector<BS> bslist)
{
	double start_time = 0, end_time = 0;
	start_time = clock();

	for (int i = 0; i < uelist.size(); i++)
		findbs_sinr(&uelist.at(i), &bslist);
	end_time = clock();
	cout << "SINR, run time: " << (end_time - start_time) / 1000 << " s" << endl;
	result_output(&bslist, &uelist, "SINR");
}

void minT_algorithm(vector<UE> uelist, vector<BS> bslist)
{
	double start_time = 0, end_time = 0;
	start_time = clock();

	for (int i = 0; i < uelist.size(); i++)
		findbs_minT(&uelist.at(i), &bslist);

	end_time = clock();
	cout << "minT, run time: " << (end_time - start_time) / 1000 << " s" << endl;

	result_output(&bslist, &uelist, "minT");
}

void proposed_algorithm(vector <UE> uelist, vector <BS> bslist)
{
	double start_time = 0, end_time = 0;
	start_time = clock();

	connection_status cs;
	cs.bslist.assign(bslist.begin(), bslist.end());
	cs.uelist.assign(uelist.begin(), uelist.end());
	cs.outage_dso = 0;
	for (int i = 0; i < cs.uelist.size(); i++)
	{
		cs.influence = 0;
		findbs_dso(&cs.uelist[i], &cs, 0);
	}
	end_time = clock();
	cout << "dso" << max_depth << ", run time: " << (end_time - start_time) / 1000 << " s" << endl;
	uelist.assign(cs.uelist.begin(), cs.uelist.end());
	bslist.assign(cs.bslist.begin(), cs.bslist.end());
	char filename[50];
	sprintf_s(filename, "dso_%d", max_depth);
	result_output(&bslist, &uelist, filename);
}

void proposed_algorithm_new(vector <UE> uelist, vector <BS> bslist)
{
	//double start_time = 0, end_time = 0;
	//start_time = clock();
	calc_dis_count = 0;
	calc_cqi_count = 0;
	availbs_count = 0;
	predict_capacity_count = 0;
	getcapacity1_count = 0;
	getcapacity2_count = 0;
	predictT_count = 0;
	getT_count = 0;
	is_influence_ue_count = 0;
	is_all_ue_be_satisify_count = 0;
	ue_join_bs_count = 0;
	check_satisfy_count = 0;
	outage_proposed = 0;
	connection_status cs;
	cs.bslist.assign(bslist.begin(), bslist.end());
	cs.uelist.assign(uelist.begin(), uelist.end());
	cs.outage_dso = 0;
	for (int i = 0; i < cs.uelist.size(); i++)
	{
		cs.influence = 0;
		findbs_dso_test(&cs.uelist[i], &cs, 0);
	}
	//end_time = clock();
	//cout << (end_time - start_time) - CLOCKS_PER_SEC << endl;
	uelist.assign(cs.uelist.begin(), cs.uelist.end());
	bslist.assign(cs.bslist.begin(), cs.bslist.end());
	char filename[50];
	sprintf_s(filename, "dso_new_%d", max_depth);
	result_output(&bslist, &uelist, filename);
}
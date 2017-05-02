#include"define.h"
#include<iostream>
#include<iomanip>
#include<fstream>
#include<string>
#include<vector>
#include<thread>

using namespace std;

vector <UE> vuelist;
vector <BS> vbslist;

void SINR_based(vector <UE> uelist, vector <BS> bslist);
void minT_algorithm(vector <UE> uelist, vector <BS> bslist);
void capacity_based(vector <UE> uelist, vector <BS> bslist);
void proposed_algorithm(vector <UE> uelist, vector <BS> bslist, int depth_max);
void proposed_algorithm_ex(vector <UE> uelist, vector <BS> bslist, int depth_max);

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
	macro.systemT_constraint = 1000;
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
			temp.systemT_constraint = 1000;
			vbslist.push_back(temp);
		}
	}
}

void initialUE()
{
	int type_count[UE_type_number] = { 0 };
	int type_max = vuelist.size() / UE_type_number + 1;
	srand((unsigned)time(NULL));			//給定亂數種子

	for (int i = 0; i < vuelist.size(); i++)
	{
		int type;
		do
		{
			type = rand() % 3;
		} while (type_count[type] == type_max);

		type_count[type]++;
		vuelist.at(i).type = (type_ue)type;

		switch (vuelist.at(i).type)
		{
		case 0:
			vuelist.at(i).bit_rate = 10;
			vuelist.at(i).packet_size = 800;
			vuelist.at(i).delay_budget = 50;
			break;
		case 1:
			vuelist.at(i).bit_rate = 10;
			vuelist.at(i).packet_size = 800;
			vuelist.at(i).delay_budget = 100;
			break;
		case 2:
			vuelist.at(i).bit_rate = 10;
			vuelist.at(i).packet_size = 800;
			vuelist.at(i).delay_budget = 300;
			break;
		default:
			break;
		}

		vuelist.at(i).num = i;
		vuelist.at(i).connecting_BS = NULL;
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
		vbslist.at(i).systemT_constraint = 1000;
	}
}

int main()
{
	for (int times = 1; times <= 10; times++)
	{
		double start_time = 0, end_time = 0;
		start_time = clock();
		for (int number = 1; number <= 5; number++)
		{
			int number_ap = 200;
			int number_ue = number * 1000;
			printf("\ntimes: %d, UE number: %d, distribution mode: %d\n", times, number_ue, UE_dis_type);
			vbslist.clear();					//vbslist清空
			vuelist.clear();					//vuelist清空

			initialconfig();					//macro eNB初始化
			distribution(number_ap, number_ue);	//產生AP、UE分布
			if (read_mode == 1)
			{
				readAP();							//讀入BS
				readUE();							//讀入UE
			}
			initialUE();						//UE參數初始化
			initialAP();						//AP參數初始化

//			countAPrange();						//計算AP可傳送資料的範圍大小
//			packet_arrival(number);				//產生packet arrival
			
			//thread dso0(proposed_algorithm, vuelist, vbslist, 0);
			//thread dso1(proposed_algorithm, vuelist, vbslist, 1);
			//thread dso2(proposed_algorithm, vuelist, vbslist, 2);
			//thread sinr_thread(SINR_based, vuelist, vbslist);
			//thread capa_thread(capacity_based, vuelist, vbslist);

			//thread dso0_ex(proposed_algorithm_ex, vuelist, vbslist, 0);
			//thread dso1_ex(proposed_algorithm_ex, vuelist, vbslist, 1);
			//thread dso2_ex(proposed_algorithm_ex, vuelist, vbslist, 2);

			//dso0.join();
			//dso1.join();
			//dso2.join();
			//sinr_thread.join();
			//capa_thread.join();
			
			//dso0_ex.join();
			//dso1_ex.join();
			//dso2_ex.join();

			SINR_based(vuelist, vbslist);
			capacity_based(vuelist, vbslist);
			for (int depth = 0; depth < 3; depth++)
			{
				proposed_algorithm(vuelist, vbslist, depth);
				//proposed_algorithm_ex(vuelist, vbslist, depth);
			}
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
	printf("SINR, run time: %f\n", (end_time - start_time) / 1000);
	result_output(&bslist, &uelist, "SINR");
}

void capacity_based(vector<UE> uelist, vector<BS> bslist)
{
	double start_time = 0, end_time = 0;
	start_time = clock();
	for (int i = 0; i < uelist.size(); i++)
		findbs_capa(&uelist.at(i), &bslist);
	end_time = clock();
	printf("Capa, run time: %f\n", (end_time - start_time) / 1000);
	result_output(&bslist, &uelist, "Capa");
}

void minT_algorithm(vector<UE> uelist, vector<BS> bslist)
{
	double start_time = 0, end_time = 0;
	start_time = clock();
	for (int i = 0; i < uelist.size(); i++)
		findbs_minT(&uelist.at(i), &bslist);

	end_time = clock();
	printf("minT, run time: %f\n", (end_time - start_time) / 1000);
	result_output(&bslist, &uelist, "minT");
}

void proposed_algorithm(vector <UE> uelist, vector <BS> bslist, int depth_max)
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
		findbs_dso(&cs.uelist[i], &cs, 0, depth_max);
	}
	end_time = clock();
	printf("dso%d, run time: %f\n", depth_max, (end_time - start_time) / 1000);
	uelist.assign(cs.uelist.begin(), cs.uelist.end());
	bslist.assign(cs.bslist.begin(), cs.bslist.end());
	char filename[50];
	sprintf_s(filename, "dso_%d", depth_max);
	result_output(&bslist, &uelist, filename);
}


void proposed_algorithm_ex(vector <UE> uelist, vector <BS> bslist, int depth_max)
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
		findbs_ex(&cs.uelist[i], &cs, 0, depth_max);
	}
	end_time = clock();
	printf("dso_ex_%d, run time: %f\n", depth_max, (end_time - start_time) / 1000);
	uelist.assign(cs.uelist.begin(), cs.uelist.end());
	bslist.assign(cs.bslist.begin(), cs.bslist.end());
	char filename[50];
	sprintf_s(filename, "dso_ex_%d", depth_max);
	result_output(&bslist, &uelist, filename);
}
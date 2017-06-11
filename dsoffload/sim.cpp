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
void proposed_algorithm(vector <UE> uelist, vector <BS> bslist, int depth_max, int DB_th);

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

/*���Ҫ�l�]�w*/
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
	macro.T_max = 1000;
	vbslist.push_back(macro);
}

/*Ū��UE����(�y��)*/
void readUE()
{
	ifstream freadUE;
	freadUE.open("UE_dis.txt", ios::in);
	if (freadUE.fail())						//��p�ƥ�����UE�y��
	{
		cout << "UE�������s�b\n";
	}
	else
	{
		int num = 0;						//num = UE�s��
		string bufferx, buffery;
		while (freadUE >> bufferx)
		{
			freadUE >> buffery;				//Ū��y�y��
			UE temp;
			temp.num = num++;				//assign�s���A�M��index++
			temp.coor_X = stof(bufferx);	//String to double
			temp.coor_Y = stof(buffery);	//��y�е�UE
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

/*Ū��AP����(�y��)*/
void readAP()
{
	ifstream freadAP;
	freadAP.open("AP_dis.txt", ios::in);
	if (freadAP.fail())						//��p�ƥ�����UE�y��
	{
		cout << "AP�������s�b\n";
	}
	else
	{
		int i = 1;
		string bufferx, buffery;
		while (freadAP >> bufferx)
		{
			freadAP >> buffery;				//Ū��y�y��	
			BS temp;
			temp.num = i++;
			temp.type = ap;					//�]�w��a�x����(AP)
			temp.coor_X = stof(bufferx);	//String to double
			temp.coor_Y = stof(buffery);	//��y�е�AP
			//initialize
			temp.connectingUE.clear();
			temp.lambda = 0;
			temp.systemT = 0;
			temp.T_max = 1000;
			vbslist.push_back(temp);
		}
	}
}

void initialUE()
{
	int type_count[UE_type_number] = { 0 };
	int type_max = vuelist.size() / UE_type_number + 1;
	srand((unsigned)time(NULL));			//���w�üƺؤl

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
			vuelist.at(i).bit_rate = 300;
			vuelist.at(i).packet_size = 8000;
			vuelist.at(i).delay_budget = 50;
			break;
		case 1:
			vuelist.at(i).bit_rate = 300;
			vuelist.at(i).packet_size = 8000;
			vuelist.at(i).delay_budget = 100;
			break;
		case 2:
			vuelist.at(i).bit_rate = 300;
			vuelist.at(i).packet_size = 8000;
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
		vbslist.at(i).T_max = 1000;
	}
}

int main()
{
	for (int times = 1; times <= 20; times++)
	{
		double start_time = 0, end_time = 0;
		start_time = clock();
		for (int number = 5; number <= 9; number++)
		{
			int number_ap = 200;
			int number_ue = number * 1000;
			printf("\ntimes: %d, UE number: %d, distribution mode: %d\n", times, number_ue, UE_dis_type);
			vbslist.clear();					//vbslist�M��
			vuelist.clear();					//vuelist�M��

			initialconfig();					//macro eNB��l��
			distribution(number_ap, number_ue);	//����AP�BUE����
			if (read_mode == 1)
			{
				readAP();							//Ū�JBS
				readUE();							//Ū�JUE
			}
			initialUE();						//UE�Ѽƪ�l��
			initialAP();						//AP�Ѽƪ�l��

			thread dso_2_0(proposed_algorithm, vuelist, vbslist, 2, 0);
			thread dso_2_25(proposed_algorithm, vuelist, vbslist, 2, 25);
			thread dso_2_50(proposed_algorithm, vuelist, vbslist, 2, 50);
			thread dso_2_75(proposed_algorithm, vuelist, vbslist, 2, 75);
			thread dso_2_100(proposed_algorithm, vuelist, vbslist, 2, 100);
			dso_2_100.join();
			thread dso_1_0(proposed_algorithm, vuelist, vbslist, 1, 0);
			dso_2_75.join();
			thread dso_1_25(proposed_algorithm, vuelist, vbslist, 1, 25);
			dso_1_25.join();
			thread dso_1_50(proposed_algorithm, vuelist, vbslist, 1, 50);
			dso_1_50.join();
			thread dso_1_75(proposed_algorithm, vuelist, vbslist, 1, 75);
			dso_1_75.join();
			thread dso_1_100(proposed_algorithm, vuelist, vbslist, 1, 100);
			dso_1_100.join();
			dso_1_0.join();			

			thread dso_0_0(proposed_algorithm, vuelist, vbslist, 0, 0);
			dso_0_0.join();
			thread dso_0_25(proposed_algorithm, vuelist, vbslist, 0, 25);
			dso_0_25.join();
			thread dso_0_50(proposed_algorithm, vuelist, vbslist, 0, 50);
			dso_0_50.join();
			thread dso_0_75(proposed_algorithm, vuelist, vbslist, 0, 75);
			dso_0_75.join();
			thread dso_0_100(proposed_algorithm, vuelist, vbslist, 0, 100);
			dso_0_100.join();

			thread sinr_thread(SINR_based, vuelist, vbslist);
			sinr_thread.join();
			thread capa_thread(capacity_based, vuelist, vbslist);
			capa_thread.join();


			dso_2_50.join();
			dso_2_25.join();
			dso_2_0.join();
		}
		end_time = clock();
		cout << "�@������ɶ� : " << (end_time - start_time) / 1000 << " s\n\n";
	}
	return 0;
}

void SINR_based(vector<UE> uelist, vector<BS> bslist)
{
	double start_time = 0, end_time = 0;
	start_time = clock();
	connection_status cs;
	cs.bslist = bslist;
	cs.uelist = uelist;
	for (int i = 0; i < uelist.size(); i++)
		findbs_sinr(&cs.uelist.at(i), &cs.bslist);
	end_time = clock();
	printf("SINR, run time: %f\n", (end_time - start_time) / 1000);
	result_output(&cs, "SINR");
}

void capacity_based(vector<UE> uelist, vector<BS> bslist)
{
	double start_time = 0, end_time = 0;
	start_time = clock();
	connection_status cs;
	cs.bslist = bslist;
	cs.uelist = uelist;
	for (int i = 0; i < uelist.size(); i++)
		findbs_capa(&cs.uelist.at(i), &cs.bslist);
	end_time = clock();
	printf("Capa, run time: %f\n", (end_time - start_time) / 1000);
	result_output(&cs, "Capa");
}

void minT_algorithm(vector<UE> uelist, vector<BS> bslist)
{
	double start_time = 0, end_time = 0;
	start_time = clock();
	connection_status cs;
	cs.bslist = bslist;
	cs.uelist = uelist;
	for (int i = 0; i < uelist.size(); i++)
		findbs_minT(&cs.uelist.at(i), &cs.bslist);
	end_time = clock();
	printf("minT, run time: %f\n", (end_time - start_time) / 1000);
	result_output(&cs, "minT");
}

void proposed_algorithm(vector <UE> uelist, vector <BS> bslist, int depth_max, int DB_th)
{
	double start_time = 0, end_time = 0;
	start_time = clock();

	connection_status cs;
	cs.bslist.assign(bslist.begin(), bslist.end());
	cs.uelist.assign(uelist.begin(), uelist.end());
	cs.Offloaded_UE_Number = 0;
	for (int i = 0; i < cs.uelist.size(); i++)
	{
		//if (i % 1000 == 0)
		//	cout << i << endl;
		cs.influence = 0;
		findbs_dso(&cs.uelist[i], &cs, 0, depth_max, DB_th);
	}
	end_time = clock();
	printf("DSO%d_%d, run time: %f\n", depth_max, DB_th, (end_time - start_time) / 1000);
	uelist.assign(cs.uelist.begin(), cs.uelist.end());
	bslist.assign(cs.bslist.begin(), cs.bslist.end());
	char filename[50];
	sprintf_s(filename, "DSO_%d_%d", depth_max, DB_th);
	result_output(&cs, filename);
}
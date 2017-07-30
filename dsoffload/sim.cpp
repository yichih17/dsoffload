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
double eNB_capacity[15] = { 0 };

/*���Ҫ�l�]�w*/
void initialconfig()
{
	//Calculate eNB Channel Capatity
	for (int i = 0; i < 15; i++)
		eNB_capacity[i] = resource_element * macro_eff[i] * total_RBG;

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
	macro.db50 = 0;
	macro.db100 = 0;
	macro.db300 = 0;
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
		int type_count[UE_type_number] = { 0 };					//�UType UE�ƶq�έp
		int type_max = vuelist.size() / UE_type_number + 1;		//�UType UE�ƶq�W�� ( 1:1:1 )
		srand((unsigned)time(NULL));							//�üƺؤl

		int num = 0;											//UE�s����l��
		string bufferx;											//�ȦsX�y��
		string buffery;											//�ȦsY�y��

		while (freadUE >> bufferx)
		{
			freadUE >> buffery;
			UE temp;
			temp.num = num++;									//���s��
			temp.coor_X = stof(bufferx);						//���y��
			temp.coor_Y = stof(buffery);

			int type;											
			do
			{
				type = rand() % 3;
			} while (type_count[type] == type_max);
			temp.type = (type_ue)type;							//��Type
			type_count[type]++;

			switch (temp.type)									//��Type�Ѽ�
			{
			case type_ue::type1:
				temp.bit_rate = UE_type1_bit_rate;
				temp.packet_size = UE_type1_pkt_size;
				temp.delay_budget = UE_type1_delay_budget;
				break;
			case type_ue::type2:
				temp.bit_rate = UE_type2_bit_rate;
				temp.packet_size = UE_type2_pkt_size;
				temp.delay_budget = UE_type2_delay_budget;
				break;
			case type_ue::type3:
				temp.bit_rate = UE_type3_bit_rate;
				temp.packet_size = UE_type3_pkt_size;
				temp.delay_budget = UE_type3_delay_budget;
				break;
			default:
				break;
			}
			
			//initialize
			temp.connecting_BS = NULL;
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
			temp.T_max = BS_T_max;
			vbslist.push_back(temp);
		}
	}
}

void initialUE()
{
	int type_count[UE_type_number] = { 0 };							//�UType UE�ƶq�έp
	int type_max = vuelist.size() / UE_type_number + 1;				//�UType UE�ƶq�W�� ( 1:1:1 )
	srand((unsigned)time(NULL));									//�üƺؤl

	for (int i = 0; i < vuelist.size(); i++)
	{
		int type;
		do
		{
			type = rand() % 3;
		} while (type_count[type] == type_max);
		vuelist.at(i).type = (type_ue)type;							//��Type
		type_count[type]++;

		switch (vuelist.at(i).type)									//��Type�Ѽ�
		{
		case type_ue::type1:
			vuelist.at(i).bit_rate = UE_type1_bit_rate;
			vuelist.at(i).packet_size = UE_type1_pkt_size;
			vuelist.at(i).delay_budget = UE_type1_delay_budget;
			break;
		case type_ue::type2:
			vuelist.at(i).bit_rate = UE_type2_bit_rate;
			vuelist.at(i).packet_size = UE_type2_pkt_size;
			vuelist.at(i).delay_budget = UE_type2_delay_budget;
			break;
		case type_ue::type3:
			vuelist.at(i).bit_rate = UE_type3_bit_rate;
			vuelist.at(i).packet_size = UE_type3_pkt_size;
			vuelist.at(i).delay_budget = UE_type3_delay_budget;
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
		vbslist.at(i).T_max = BS_T_max;
		vbslist.at(i).db50 = 0;
		vbslist.at(i).db100 = 0;
		vbslist.at(i).db300 = 0;
	}
}

int main()
{
	for (int times = 1; times <= 100; times++)
	{
		double start_time = 0, end_time = 0;			//�p��{������ɶ�(s)
		start_time = clock();
		for (int number = 1; number <= 12; number++)
		{
			int number_ap = 200;
			int number_ue = number * 1000;
			printf("\ntimes: %d, UE number: %d, distribution mode: %d\n", times, number_ue, UE_dis_type);
			vbslist.clear();					//vbslist�M��
			vuelist.clear();					//vuelist�M��

			initialconfig();					//macro eNB��l��
			distribution(number_ap, number_ue);	//����AP�BUE����

			if (read_mode == 1)					//�q�~��.txt��Ū���y��
			{
				readAP();						//Ū�JBS
				readUE();						//Ū�JUE
			}

			initialUE();						//UE�Ѽƪ�l��
			initialAP();						//AP�Ѽƪ�l��

			if (ThreadExeMode == 0)				//���Ӻt��k�W�ٶ��ǰ���
			{
				thread dso_2_0(proposed_algorithm, vuelist, vbslist, 2, 0);
				thread dso_2_25(proposed_algorithm, vuelist, vbslist, 2, 25);
				thread dso_2_50(proposed_algorithm, vuelist, vbslist, 2, 50);
				thread dso_2_75(proposed_algorithm, vuelist, vbslist, 2, 75);
				thread dso_2_100(proposed_algorithm, vuelist, vbslist, 2, 100);

				dso_2_0.join();
				dso_2_25.join();
				dso_2_50.join();
				dso_2_75.join();
				dso_2_100.join();

				thread dso_1_0(proposed_algorithm, vuelist, vbslist, 1, 0);
				thread dso_1_25(proposed_algorithm, vuelist, vbslist, 1, 25);
				thread dso_1_50(proposed_algorithm, vuelist, vbslist, 1, 50);
				thread dso_1_75(proposed_algorithm, vuelist, vbslist, 1, 75);
				thread dso_1_100(proposed_algorithm, vuelist, vbslist, 1, 100);
				dso_1_0.join();
				dso_1_25.join();
				dso_1_50.join();
				dso_1_75.join();
				dso_1_100.join();

				thread dso_0_0(proposed_algorithm, vuelist, vbslist, 0, 0);
				thread dso_0_25(proposed_algorithm, vuelist, vbslist, 0, 25);
				thread dso_0_50(proposed_algorithm, vuelist, vbslist, 0, 50);
				thread dso_0_75(proposed_algorithm, vuelist, vbslist, 0, 75);
				thread dso_0_100(proposed_algorithm, vuelist, vbslist, 0, 100);
				dso_0_0.join();
				dso_0_25.join();
				dso_0_50.join();
				dso_0_75.join();
				dso_0_100.join();

				thread sinr_thread(SINR_based, vuelist, vbslist);
				thread capa_thread(capacity_based, vuelist, vbslist);
				sinr_thread.join();
				capa_thread.join();
			}
			else
			{	//���Ӻt��k����ɶ� (�Q��CPU�����ɺ�O���t��k)
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
		//if (i % 1000 == 0)			//�{������i��
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
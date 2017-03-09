#include"define.h"
#include<iostream>
#include<iomanip>
#include<fstream>
#include<string>
#include<vector>

using namespace std;

vector <UE> vuelist;
vector <BS> vbslist;

result minT_algorithm(vector <UE> uelist, vector <BS> bslist);
result proposed_algorithm(vector <UE> uelist, vector <BS> bslist);

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
	for (int times = 1; times < 101; times++)
	{
		for (int number = 1; number < 16; number++)
		{
			int number_ap = 200;
			int number_ue = number * 1000;
			distribution(ap, number_ap);	//產生AP分布
			distribution(ue, number_ue);	//產生UE分布

			vbslist.clear();					//vbslist, vuelist初始化
			vuelist.clear();

			initialconfig();					//macro eNB初始化
			readAP(vbslist);					//讀入BS
			readUE();							//讀入UE
//			cout << "Number of BS :" << vbslist.size() << "\n";
//			cout << "Number of UE :" << vuelist.size() << "\n";
//			countAPrange();						//計算AP可傳送資料的範圍大小
//			packet_arrival(number);					//產生packet arrival
			for (int depth = 0; depth < 3; depth++)
			{
				printf("times: %d, UE number: %d, depth: %d\n", times, number_ue, depth);
				max_depth = depth;
				proposed_algorithm(vuelist, vbslist);

			}
			minT_algorithm(vuelist, vbslist);
		}		
	}
	return 0;
}

result minT_algorithm(vector<UE> uelist, vector<BS> bslist)
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

	result result_minT;
	result_minT.outage_number = outage_minT;

	//cout << "============Result for minT algorithm============" << endl;
	//每個UE可連接的BS數量
	//for (int i = 0; i < number_ue; i++)
	//{
	//	int count = 0;
	//	if (cs.uelist[i].connecting_BS != NULL)
	//		count++;
	//	count += cs.uelist[i].availBS.size();
	//	cout << count << ", ";
	//}

	char result_total_filename[50];
	char result_separate_filename[50];
	char result_detail_filename[50];
	char result_extra_filename[50];
	sprintf_s(result_total_filename, "minT_UE%d_result_total.txt", uelist.size());
	sprintf_s(result_separate_filename, "minT_UE%d_result_separate.txt", uelist.size());
	sprintf_s(result_detail_filename, "minT_UE%d_result_detail.txt", uelist.size());
	sprintf_s(result_extra_filename, "minT_UE%d_result_extra.txt", uelist.size());

	fstream result_total;
	result_total.open(result_total_filename, ios::out | ios::app);
	fstream result_separate;
	result_separate.open(result_separate_filename, ios::out | ios::app);
	fstream result_detail;
	result_detail.open(result_detail_filename, ios::out | ios::app);
	fstream result_extra;
	result_extra.open(result_extra_filename, ios::out | ios::app);
	if (result_separate.fail())
		cout << "檔案開啟失敗" << endl;
	else
	{
		double outage_UE_number = 0;
		double t_total = 0, rho_total = 0, UE_number_total = 0, capacity_total = 0, t_UE_total = 0;

		double t_LTE = 0;																								//BS_T
		double rho_LTE = 0;																								//BS_rho
		double UE_number_LTE = 0;																						//BS_UE Number
		double capacity_LTEUE_avg = 0, capacity_LTEUE_max = -1, capacity_LTEUE_min = -1, capacity_LTEUE_stdev = 0;		//UE_Capacity
		double t_LTEUE = 0;

		//LTE
		double Xj = 0;
		vector <double> capacity_LTEUE;
		for (int i = 0; i < bslist.at(0).connectingUE.size(); i++)
		{
			double capacity = getCapacity(bslist.at(0).connectingUE.at(i));
			capacity_LTEUE.push_back(capacity);
			capacity_total += capacity;
			Xj += bslist.at(0).connectingUE.at(i)->packet_size / capacity * (bslist.at(0).connectingUE.at(i)->lambdai / bslist.at(0).lambda);
			capacity_LTEUE_avg += capacity / bslist.at(0).connectingUE.size();
			if (capacity_LTEUE_max == -1)
				capacity_LTEUE_max = capacity;
			else
				if (capacity > capacity_LTEUE_max)
					capacity_LTEUE_max = capacity;
			if (capacity_LTEUE_min == -1)
				capacity_LTEUE_min = capacity;
			else
				if (capacity < capacity_LTEUE_min)
					capacity_LTEUE_min = capacity;
			t_UE_total += bslist.at(0).systemT;
		}
		t_LTE = bslist.at(0).systemT;
		rho_LTE = Xj * bslist.at(0).lambda;
		UE_number_LTE = bslist.at(0).connectingUE.size();
		t_LTEUE = bslist.at(0).systemT;

		for (int i = 0; i < bslist.at(0).connectingUE.size(); i++)
			capacity_LTEUE_stdev += pow(capacity_LTEUE.at(i) - capacity_LTEUE_avg, 2);
		capacity_LTEUE_stdev /= bslist.at(0).connectingUE.size();
		capacity_LTEUE_stdev = sqrt(capacity_LTEUE_stdev);


		double t_avg_WIFI = 0, t_max_WIFI = -1, t_min_WIFI = -1, t_stdev_WIFI = 0;										//BS_T
		double rho_avg_WIFI = 0, rho_max_WIFI = -1, rho_min_WIFI = -1, rho_stdev_WIFI = 0;								//BS_rho
		double UE_number_avg_WIFI = 0, UE_number_max_WIFI = -1, UE_number_min_WIFI = -1, UE_number_stdev_WIFI = 0;		//BS_UE Number
		double capacity_WIFIUE_avg = 0, capacity_WIFIUE_max = -1, capacity_WIFIUE_min = -1, capacity_WIFIUE_stdev = 0;	//UE_Capacity
		double t_WIFIUE_avg = 0, t_WIFIUE_max = -1, t_WIFIUE_min = -1, t_WIFIUE_stdev = 0;								//UE_T

																														//WIFI
		int UE_number_WIFI = 0;
		t_total += bslist.at(0).systemT / bslist.size();
		rho_total += rho_LTE / bslist.size();
		vector <double> rho;
		vector <double> capacity_WIFIUE;
		vector <double> t_WIFIUE;
		for (int i = 1; i < bslist.size(); i++)
		{
			double Xj = 0;
			for (int j = 0; j < bslist.at(i).connectingUE.size(); j++)
			{
				double capacity = getCapacity(bslist.at(i).connectingUE.at(j));
				Xj += bslist.at(i).connectingUE.at(j)->packet_size / capacity * (bslist.at(i).connectingUE.at(j)->lambdai / bslist.at(i).lambda);
				capacity_WIFIUE_avg += capacity;
				capacity_WIFIUE.push_back(capacity);
				capacity_total += capacity;
				UE_number_WIFI++;
				if (capacity_WIFIUE_max == -1)
					capacity_WIFIUE_max = capacity;
				else
					if (capacity > capacity_WIFIUE_max)
						capacity_WIFIUE_max = capacity;
				if (capacity_WIFIUE_min == -1)
					capacity_WIFIUE_min = capacity;
				else
					if (capacity < capacity_WIFIUE_min)
						capacity_WIFIUE_min = capacity;
				t_WIFIUE_avg += bslist.at(i).systemT;
				t_WIFIUE.push_back(bslist.at(i).systemT);
				t_UE_total += bslist.at(i).systemT;
			}
			double rho_i = Xj * bslist.at(i).lambda;
			rho.push_back(rho_i);
			rho_total += rho_i / bslist.size();
			rho_avg_WIFI += rho_i / (bslist.size() - 1);
			if (rho_max_WIFI == -1)
				rho_max_WIFI = rho_i;
			else
				if (rho_max_WIFI < rho_i)
					rho_max_WIFI = rho_i;
			if (rho_min_WIFI == -1)
				rho_min_WIFI = rho_i;
			else
				if (rho_min_WIFI > rho_i)
					rho_min_WIFI = rho_i;

			t_avg_WIFI += bslist.at(i).systemT / (bslist.size() - 1);
			t_total += bslist.at(i).systemT / bslist.size();

			if (t_max_WIFI == -1)
				t_max_WIFI = bslist.at(i).systemT;
			else
				if (t_max_WIFI < bslist.at(i).systemT)
					t_max_WIFI = bslist.at(i).systemT;
			if (t_min_WIFI == -1)
				t_min_WIFI = bslist.at(i).systemT;
			else
				if (t_min_WIFI > bslist.at(i).systemT)
					t_min_WIFI = bslist.at(i).systemT;

			if (UE_number_max_WIFI == -1)
				UE_number_max_WIFI = bslist.at(i).connectingUE.size();
			else
				if (UE_number_max_WIFI < bslist.at(i).connectingUE.size())
					UE_number_max_WIFI = bslist.at(i).connectingUE.size();
			if (UE_number_min_WIFI == -1)
				UE_number_min_WIFI = bslist.at(i).connectingUE.size();
			else
				if (UE_number_min_WIFI > bslist.at(i).connectingUE.size())
					UE_number_min_WIFI = bslist.at(i).connectingUE.size();
			result_detail << "BS " << bslist[i].num << " has " << bslist[i].connectingUE.size() << "UE, T :" << bslist[i].systemT << ", rho : " << rho_i << endl;
		}
		UE_number_total = (UE_number_LTE + UE_number_WIFI) / bslist.size();
		capacity_total /= (UE_number_LTE + UE_number_WIFI);
		t_UE_total /= (UE_number_LTE + UE_number_WIFI);
		capacity_WIFIUE_avg /= UE_number_WIFI;
		UE_number_avg_WIFI = UE_number_WIFI / (bslist.size() - 1);
		for (int i = 1; i < bslist.size(); i++)
		{
			t_stdev_WIFI += pow(bslist.at(i).systemT - t_avg_WIFI, 2);
			rho_stdev_WIFI += pow(rho.at(i - 1) - rho_avg_WIFI, 2);
			UE_number_stdev_WIFI += pow(bslist.at(i).connectingUE.size() - UE_number_avg_WIFI, 2);
		}
		t_stdev_WIFI /= (bslist.size() - 1);
		t_stdev_WIFI = sqrt(t_stdev_WIFI);
		rho_stdev_WIFI /= (bslist.size() - 1);
		rho_stdev_WIFI = sqrt(rho_stdev_WIFI);
		UE_number_stdev_WIFI /= (bslist.size() - 1);
		UE_number_stdev_WIFI = sqrt(UE_number_stdev_WIFI);
		t_WIFIUE_avg /= UE_number_WIFI;
		t_WIFIUE_max = t_max_WIFI;
		t_WIFIUE_min = t_min_WIFI;

		for (int i = 0; i < UE_number_WIFI; i++)
		{
			capacity_WIFIUE_stdev += pow(capacity_WIFIUE.at(i) - capacity_WIFIUE_avg, 2);
			t_WIFIUE_stdev += pow(t_WIFIUE.at(i) - t_WIFIUE_avg, 2);
		}
		capacity_WIFIUE_stdev /= UE_number_WIFI;
		capacity_WIFIUE_stdev = sqrt(capacity_WIFIUE_stdev);
		t_WIFIUE_stdev /= UE_number_WIFI;
		t_WIFIUE_stdev = sqrt(t_WIFIUE_stdev);

		result_total << uelist.size() - UE_number_LTE - UE_number_WIFI << " " << t_total << " " << rho_total << " " << UE_number_total << " " << capacity_total << " " << t_UE_total << endl;

		result_separate << t_LTE << " " << t_avg_WIFI << " " << t_stdev_WIFI << " ";
		result_separate << rho_LTE << " " << rho_avg_WIFI << " " << rho_stdev_WIFI << " ";
		result_separate << UE_number_LTE << " " << UE_number_avg_WIFI << " " << UE_number_stdev_WIFI << " ";
		result_separate << capacity_LTEUE_avg << " " << capacity_LTEUE_stdev << " " << capacity_WIFIUE_avg << " " << capacity_WIFIUE_stdev << " ";
		result_separate << t_LTEUE << " " << t_WIFIUE_avg << " " << t_WIFIUE_stdev << endl;

		result_extra << t_max_WIFI << " " << t_min_WIFI << " ";
		result_extra << rho_max_WIFI << " " << rho_min_WIFI << " ";
		result_extra << UE_number_max_WIFI << " " << UE_number_min_WIFI << " ";
		result_extra << capacity_LTEUE_max << " " << capacity_LTEUE_min << " ";
		result_extra << capacity_WIFIUE_max << " " << capacity_WIFIUE_min << " ";
		result_extra << t_WIFIUE_max << " " << t_WIFIUE_min << endl;
	}

	return result_minT;
}

result proposed_algorithm(vector <UE> uelist, vector <BS> bslist)
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

	result result_proposed;
	result_proposed.outage_number = cs.outage_dso;

//	cout << "============Result for proposed algorithm============" << endl;
	//每個UE可連接的BS數量
	//for (int i = 0; i < number_ue; i++)
	//{
	//	int count = 0;
	//	if (cs.uelist[i].connecting_BS != NULL)
	//		count++;
	//	count += cs.uelist[i].availBS.size();
	//	cout << count << ", ";
	//}

	char result_total_filename[50];
	char result_separate_filename[50];
	char result_detail_filename[50];
	char result_extra_filename[50];
	sprintf_s(result_total_filename, "dso_depth%d_UE%d_result_total.txt", max_depth, uelist.size());
	sprintf_s(result_separate_filename, "dso_depth%d_UE%d_result_separate.txt", max_depth, uelist.size());
	sprintf_s(result_detail_filename, "dso_depth%d_UE%d_result_detail.txt", max_depth, uelist.size());
	sprintf_s(result_extra_filename, "dso_depth%d_UE%d_result_extra.txt", max_depth, uelist.size());
	
	fstream result_total;
	result_total.open(result_total_filename, ios::out | ios::app);
	fstream result_separate;
	result_separate.open(result_separate_filename, ios::out | ios::app);
	fstream result_detail;
	result_detail.open(result_detail_filename, ios::out | ios::app);
	fstream result_extra;
	result_extra.open(result_extra_filename, ios::out | ios::app);
	if (result_separate.fail())
		cout << "檔案開啟失敗" << endl;
	else
	{
		double outage_UE_number = 0;
		double t_total = 0, rho_total = 0, UE_number_total = 0, capacity_total = 0, t_UE_total = 0;

		double t_LTE = 0;																								//BS_T
		double rho_LTE = 0;																								//BS_rho
		double UE_number_LTE = 0;																						//BS_UE Number
		double capacity_LTEUE_avg = 0, capacity_LTEUE_max = -1, capacity_LTEUE_min = -1, capacity_LTEUE_stdev = 0;		//UE_Capacity
		double t_LTEUE = 0;
		
		//LTE
		double Xj = 0;
		vector <double> capacity_LTEUE;
		for (int i = 0; i < bslist.at(0).connectingUE.size(); i++)
		{
			double capacity = getCapacity(bslist.at(0).connectingUE.at(i));
			capacity_LTEUE.push_back(capacity);
			capacity_total += capacity;
			Xj += bslist.at(0).connectingUE.at(i)->packet_size / capacity * (bslist.at(0).connectingUE.at(i)->lambdai / bslist.at(0).lambda);
			capacity_LTEUE_avg += capacity / bslist.at(0).connectingUE.size();
			if (capacity_LTEUE_max == -1)
				capacity_LTEUE_max = capacity;
			else
				if(capacity > capacity_LTEUE_max)
					capacity_LTEUE_max = capacity;
			if (capacity_LTEUE_min == -1)
				capacity_LTEUE_min = capacity;
			else
				if (capacity < capacity_LTEUE_min)
					capacity_LTEUE_min = capacity;
			t_UE_total += bslist.at(0).systemT;
		}
		t_LTE = bslist.at(0).systemT;
		rho_LTE = Xj * bslist.at(0).lambda;
		UE_number_LTE = bslist.at(0).connectingUE.size();
		t_LTEUE = bslist.at(0).systemT;

		for (int i = 0; i < bslist.at(0).connectingUE.size(); i++)
			capacity_LTEUE_stdev += pow(capacity_LTEUE.at(i) - capacity_LTEUE_avg, 2);
		capacity_LTEUE_stdev /= bslist.at(0).connectingUE.size();
		capacity_LTEUE_stdev = sqrt(capacity_LTEUE_stdev);


		double t_avg_WIFI = 0, t_max_WIFI = -1, t_min_WIFI = -1, t_stdev_WIFI = 0;										//BS_T
		double rho_avg_WIFI = 0, rho_max_WIFI = -1, rho_min_WIFI = -1, rho_stdev_WIFI = 0;								//BS_rho
		double UE_number_avg_WIFI = 0, UE_number_max_WIFI = -1, UE_number_min_WIFI = -1, UE_number_stdev_WIFI = 0;		//BS_UE Number
		double capacity_WIFIUE_avg = 0, capacity_WIFIUE_max = -1, capacity_WIFIUE_min = -1, capacity_WIFIUE_stdev = 0;	//UE_Capacity
		double t_WIFIUE_avg = 0, t_WIFIUE_max = -1, t_WIFIUE_min = -1, t_WIFIUE_stdev = 0;								//UE_T

		//WIFI
		int UE_number_WIFI = 0;
		t_total += bslist.at(0).systemT / bslist.size();
		rho_total += rho_LTE / bslist.size();
		vector <double> rho;
		vector <double> capacity_WIFIUE;
		vector <double> t_WIFIUE;
		for (int i = 1; i < bslist.size(); i++)
		{
			double Xj = 0;
			for (int j = 0; j < bslist.at(i).connectingUE.size(); j++)
			{
				double capacity = getCapacity(bslist.at(i).connectingUE.at(j));
				Xj += bslist.at(i).connectingUE.at(j)->packet_size / capacity * (bslist.at(i).connectingUE.at(j)->lambdai / bslist.at(i).lambda);
				capacity_WIFIUE_avg += capacity;
				capacity_WIFIUE.push_back(capacity);
				capacity_total += capacity;
				UE_number_WIFI++;
				if (capacity_WIFIUE_max == -1)
					capacity_WIFIUE_max = capacity;
				else
					if(capacity > capacity_WIFIUE_max)
						capacity_WIFIUE_max = capacity;
				if (capacity_WIFIUE_min == -1)
					capacity_WIFIUE_min = capacity;
				else
					if (capacity < capacity_WIFIUE_min)
						capacity_WIFIUE_min = capacity;
				t_WIFIUE_avg += bslist.at(i).systemT;
				t_WIFIUE.push_back(bslist.at(i).systemT);
				t_UE_total += bslist.at(i).systemT;
			}
			double rho_i = Xj * bslist.at(i).lambda;
			rho.push_back(rho_i);
			rho_total += rho_i / bslist.size();
			rho_avg_WIFI += rho_i / (bslist.size() - 1);
			if (rho_max_WIFI == -1)
				rho_max_WIFI = rho_i;
			else
				if(rho_max_WIFI < rho_i)
					rho_max_WIFI = rho_i;
			if (rho_min_WIFI == -1)
				rho_min_WIFI = rho_i;
			else
				if (rho_min_WIFI > rho_i)
					rho_min_WIFI = rho_i;

			t_avg_WIFI += bslist.at(i).systemT / (bslist.size() - 1);
			t_total += bslist.at(i).systemT / bslist.size();

			if (t_max_WIFI == -1)
				t_max_WIFI = bslist.at(i).systemT;
			else
				if(t_max_WIFI < bslist.at(i).systemT)
					t_max_WIFI = bslist.at(i).systemT;
			if (t_min_WIFI == -1)
				t_min_WIFI = bslist.at(i).systemT;
			else
				if (t_min_WIFI > bslist.at(i).systemT)
					t_min_WIFI = bslist.at(i).systemT;

			if(UE_number_max_WIFI ==-1)
				UE_number_max_WIFI = bslist.at(i).connectingUE.size();
			else
				if(UE_number_max_WIFI < bslist.at(i).connectingUE.size())
					UE_number_max_WIFI = bslist.at(i).connectingUE.size();
			if (UE_number_min_WIFI == -1)
				UE_number_min_WIFI = bslist.at(i).connectingUE.size();
			else
				if (UE_number_min_WIFI > bslist.at(i).connectingUE.size())
					UE_number_min_WIFI = bslist.at(i).connectingUE.size();
			result_detail << "BS " << bslist[i].num << " has " << bslist[i].connectingUE.size() << "UE, T :" << bslist[i].systemT << ", rho : " << rho_i << endl;
		}
		UE_number_total = (UE_number_LTE + UE_number_WIFI) / bslist.size();
		capacity_total /= (UE_number_LTE + UE_number_WIFI);
		t_UE_total /= (UE_number_LTE + UE_number_WIFI);
		capacity_WIFIUE_avg /= UE_number_WIFI;
		UE_number_avg_WIFI = UE_number_WIFI / (bslist.size() - 1);
		for (int i = 1; i < bslist.size(); i++)
		{
			t_stdev_WIFI += pow(bslist.at(i).systemT - t_avg_WIFI, 2);
			rho_stdev_WIFI += pow(rho.at(i - 1) - rho_avg_WIFI, 2);
			UE_number_stdev_WIFI += pow(bslist.at(i).connectingUE.size() - UE_number_avg_WIFI, 2);
		}
		t_stdev_WIFI /= (bslist.size() - 1);
		t_stdev_WIFI = sqrt(t_stdev_WIFI);
		rho_stdev_WIFI /= (bslist.size() - 1);
		rho_stdev_WIFI = sqrt(rho_stdev_WIFI);
		UE_number_stdev_WIFI /= (bslist.size() - 1);
		UE_number_stdev_WIFI = sqrt(UE_number_stdev_WIFI);
		t_WIFIUE_avg /= UE_number_WIFI;
		t_WIFIUE_max = t_max_WIFI;
		t_WIFIUE_min = t_min_WIFI;		

		for (int i = 0; i < UE_number_WIFI; i++)
		{
			capacity_WIFIUE_stdev += pow(capacity_WIFIUE.at(i) - capacity_WIFIUE_avg, 2);
			t_WIFIUE_stdev += pow(t_WIFIUE.at(i) - t_WIFIUE_avg, 2);
		}
		capacity_WIFIUE_stdev /= UE_number_WIFI;
		capacity_WIFIUE_stdev = sqrt(capacity_WIFIUE_stdev);
		t_WIFIUE_stdev /= UE_number_WIFI;
		t_WIFIUE_stdev = sqrt(t_WIFIUE_stdev);

		result_total << uelist.size() - UE_number_LTE - UE_number_WIFI << " " << t_total << " " << rho_total << " " << UE_number_total << " " << capacity_total << " " << t_UE_total << endl;

		result_separate << t_LTE << " " << t_avg_WIFI << " " << t_stdev_WIFI << " ";
		result_separate << rho_LTE << " " << rho_avg_WIFI << " " << rho_stdev_WIFI << " ";
		result_separate << UE_number_LTE << " " << UE_number_avg_WIFI << " " << UE_number_stdev_WIFI << " ";
		result_separate << capacity_LTEUE_avg << " " << capacity_LTEUE_stdev << " " << capacity_WIFIUE_avg << " " << capacity_WIFIUE_stdev << " ";
		result_separate << t_LTEUE << " " << t_WIFIUE_avg << " " << t_WIFIUE_stdev << endl;

		result_extra << t_max_WIFI << " " << t_min_WIFI << " ";
		result_extra << rho_max_WIFI << " " << rho_min_WIFI << " ";
		result_extra << UE_number_max_WIFI << " " << UE_number_min_WIFI << " ";
		result_extra << capacity_LTEUE_max << " " << capacity_LTEUE_min << " ";
		result_extra << capacity_WIFIUE_max << " " << capacity_WIFIUE_min << " ";
		result_extra << t_WIFIUE_max << " " << t_WIFIUE_min << endl;
	}
//	cout << "Number of outage UE is:" << outage << endl;
//	cout << "avg T: " << avgt << ", avg rho: " << avgrho << ", avg ue num: " << avguenum << endl;
//	cout << "=========================End=========================" << endl;

	return result_proposed;
}
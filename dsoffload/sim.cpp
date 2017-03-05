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

/*環境初始設定*/
void initialconfig(vector <BS> &bslist)
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
	for (double j = 0; j < 10; j++)	//實驗次數
	{
		for (int i = 1; i < 16; i++)	//UE數量
		{
			cout << "UE number = " << i << ", " << j << endl;				//實驗進度
			distribution(ue, i * 1000);		//產生UE分布
			distribution(ap, i * 1000);		//產生AP分布

			vbslist.clear();				//bslist, uelist初始化
			vuelist.clear();

			initialconfig(vbslist);			//macro eNB初始化
			readAP(vbslist);				//讀入BS
			readUE();						//讀入UE
			
//			countAPrange();					//計算AP可傳送資料的範圍大小
//			packet_arrival();				//產生packet arrival

//			cout << "Number of BS :" << vbslist.size() << "\n";
//			cout << "Number of UE :" << vuelist.size() << "\n";
			
//			minT_algorithm(vuelist, vbslist);
			proposed_algorithm(vuelist, vbslist);
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

	cout << "============Result for minT algorithm============" << endl;
	//每個UE可連接的BS數量
	//for (int i = 0; i < number_ue; i++)
	//{
	//	int count = 0;
	//	if (cs.uelist[i].connecting_BS != NULL)
	//		count++;
	//	count += cs.uelist[i].availBS.size();
	//	cout << count << ", ";
	//}

	double Ti, rhoi, uenumi;
	double avgt = 0, avgrho = 0, avguenum = 0;
	//每個BS的連接UE數量與T
	for (int i = 0; i < bslist.size(); i++)
	{
		Ti = bslist[i].systemT;
		rhoi = getrho(&bslist[i]);
		uenumi = bslist[i].connectingUE.size();
		avgt += Ti / bslist.size();
		avgrho += rhoi / bslist.size();
		avguenum += uenumi / bslist.size();
		printf("BS%3d has %2zd UE, T : %12lf, rho : %lf\n", bslist[i].num, bslist[i].connectingUE.size(), bslist[i].systemT, getrho(&bslist[i]));
		//cout << bslist[i].systemT << ", rho is " << getrho(&bslist[i]) << "\n";
	}

	//最後無法連接BS的UE數量
	int outage = 0;
	for (int i = 0; i < uelist.size(); i++)
	{
		if (uelist[i].connecting_BS == NULL)
			outage++;
	}
	cout << "Number of outage UE is:" << outage << endl;
	cout << "avg T: " << avgt << ", avg rho: " << avgrho << ", avg ue num: " << avguenum << endl;
	cout << "=========================End=========================" << endl;

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

	char result_filename[50];
	char result_detail_filename[50];
	sprintf_s(result_filename, "dso_depth%d_UE%d_result.txt", MAX_DEPTH, uelist.size());
	sprintf_s(result_detail_filename, "dso_depth%d_UE%d_result_detail.txt", MAX_DEPTH, uelist.size());

	fstream result;
	result.open(result_filename, ios::out | ios::app);
	fstream result2;
	result2.open(result_detail_filename, ios::out | ios::app);
	if (result.fail())
		cout << "檔案無法開啟" << endl;
	else
	{
		double t_bs = 0;							//System time of BS
		double rho_bs = 0;							//Rho of BS
		double uenum_bs = 0;						//# of UE of BS
		double avg_capacity_of_ue_under_bs = 0;		//Average capacity of UE under the BS

		double avg_t = 0;							//Average T of all BS in the system
		double avg_rho = 0;							//Average Rho of all BS in the system
		double avg_uenum = 0;						//Average UE number of all BS in the system

		double avg_capacity_ue = 0;					//Average capacity of UE in the system
		double avg_delay_ue = 0;					//Average delay of UE in the system
		double avg_availbs = 0;
		vector <double> ue_capacity, ue_delay;
		int non_outage_ue_num = 0;
		for (int i = 0; i < bslist.size(); i++)
		{
			t_bs = bslist[i].systemT;
			uenum_bs = bslist[i].connectingUE.size();
			double Xj = 0;
			avg_capacity_of_ue_under_bs = 0;
			for (int j = 0; j < bslist[i].connectingUE.size(); j++)
			{
				double capacityj = getCapacity(bslist[i].connectingUE[j]);
				Xj += bslist[i].connectingUE[j]->packet_size / capacityj * (bslist[i].connectingUE[j]->lambdai / bslist[i].lambda);
				avg_capacity_of_ue_under_bs += capacityj / bslist[i].connectingUE.size();
				avg_capacity_ue += capacityj / uelist.size();
				avg_delay_ue += bslist[i].systemT / uelist.size();
				non_outage_ue_num++;
				avg_availbs += bslist[i].connectingUE[j]->availBS.size() / uelist.size();
			}
			rho_bs = Xj * bslist[i].lambda;;
			avg_t += t_bs / bslist.size();
			avg_rho += rho_bs / bslist.size();
			avg_uenum += uenum_bs / bslist.size();

			result2 << "BS " << bslist[i].num << " has " << bslist[i].connectingUE.size() << "UE, T :" << bslist[i].systemT << ", rho : " << rho_bs << ", avg UE capacity : " << avg_capacity_of_ue_under_bs << endl;
			//printf("BS%3d has %2zd UE, T : %12lf, rho : %lf\n", bslist[i].num, bslist[i].connectingUE.size(), bslist[i].systemT, getrho(&bslist[i]));
		}
//		cout << uelist.size() - non_outage_ue_num << " " << avg_t << " " << avg_rho << " " << avg_uenum << " " <<  avg_capacity_ue << " " << avg_delay_ue << " " << avg_availbs << endl;
		result << uelist.size() - non_outage_ue_num << " " << avg_t << " " << avg_rho << " " << avg_uenum << " " <<  avg_capacity_ue << " " << avg_delay_ue << " " << avg_availbs << endl;
		result2 << uelist.size() - non_outage_ue_num <<  " " << avg_t << " " << avg_rho << " " << avg_uenum << " " << avg_capacity_ue << " " << avg_delay_ue << " " << avg_availbs << endl;
	}
//	cout << "Number of outage UE is:" << outage << endl;
//	cout << "avg T: " << avgt << ", avg rho: " << avgrho << ", avg ue num: " << avguenum << endl;
//	cout << "=========================End=========================" << endl;

	return result_proposed;
}
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
		cout << "UE分布不存在\n產生新分布" << endl;
		distribution(ue);
		readUE();
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
		cout << "AP分布不存在\n產生新分布" << endl;
		distribution(ap);
		readAP(bslist);
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
	for (double i = 1; i < 1000; i++)
	{
		cout << i << endl;
		initialconfig(vbslist);
		vbslist.clear();
		vuelist.clear();
		distribution(ue);
		//countAPrange();
		//UE and AP location initial
		readAP(vbslist);		//讀入BS
//		cout << "Number of BS :" << vbslist.size() << "\n";
		readUE();		//讀入UE
//		cout << "Number of UE :" << vuelist.size() << "\n";

		//packet_arrival();
		/*	// Show UE coordinates
			for (int i = 0; i < vuelist.size(); i++)
				cout << "UE " << i << ": X=" << vuelist[i].coor_X << ", Y=" << vuelist[i].coor_Y << "; DB=" << vuelist[i].delaybg << "\n";
			for (int i = 0; i < vbslist.size(); i++)
				cout << "BS " << i << ": X=" << vbslist[i].coor_X << ", Y=" << vbslist[i].coor_Y << "; type=" << vbslist[i].type << "\n";
		*/

		//minT_algorithm(vuelist, vbslist);

		proposed_algorithm(vuelist, vbslist);
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

	fstream result;
	result.open("dso_run_result0302.txt", ios::out | ios::app);
	fstream result2;
	result2.open("dso_run_result03022.txt", ios::out | ios::app);
	if (result.fail())
		cout << "檔案無法開啟" << endl;
	else
	{
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
			result << "BS " << bslist[i].num << " has " << bslist[i].connectingUE.size() << "UE, T :" << bslist[i].systemT << ", rho : " << getrho(&bslist[i]) << endl;
			//printf("BS%3d has %2zd UE, T : %12lf, rho : %lf\n", bslist[i].num, bslist[i].connectingUE.size(), bslist[i].systemT, getrho(&bslist[i]));
		}

		//最後無法連接BS的UE數量
		int outage = 0;
		for (int i = 0; i < uelist.size(); i++)
		{
			if (uelist[i].connecting_BS == NULL)
				outage++;
		}
	

		result << outage << " " << avgt << " " << avgrho << " " << avguenum << endl;
		result2 << outage << " " << avgt << " " << avgrho << " " << avguenum << endl;
	}
//	cout << "Number of outage UE is:" << outage << endl;
//	cout << "avg T: " << avgt << ", avg rho: " << avgrho << ", avg ue num: " << avguenum << endl;
//	cout << "=========================End=========================" << endl;

	return result_proposed;
}
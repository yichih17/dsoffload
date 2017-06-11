#include<iostream>
#include<vector>
#include<fstream>
#include"define.h"

using namespace std;

void result_output(connection_status *cs, char algorithm_name[])
{
	int non_outage_UE = 0;	//Τ硈钡BSUE计秖
	int outage_UE = 0;		//礚硈钡BSUE计秖
	int number_eNB = 0;		//LTE eNB计秖
	int number_AP = 0;		//WIFI AP计秖
	int number_UE_LTE = 0;	//LTE eNB UE计秖
	int number_UE_WIFI = 0;	//WIFI AP UE计秖
	int number_UE_DB_50 = 0;
	int number_UE_DB_100 = 0;
	int number_UE_DB_300 = 0;
	int UE_DB[201][3] = { 0 };

	double avg_T = 0;		//┮ΤTキА
	double avg_T_LTE = 0;	//┮ΤLTE eNB TキА
	double avg_T_WIFI = 0;	//┮ΤWIFI AP TキА
	double avg_UE_number = 0;			//┮ΤBSUE计秖キА
	double avg_UE_number_LTE = 0;		//┮ΤLTE eNBUE计秖キА
	double avg_UE_number_WIFI = 0;		//┮ΤWIFI APUE计秖キА
	vector <double> capacity_LTEUE;
	vector <double> capacity_WIFIUE;
	double avg_capacity_UE = 0;
	double avg_capacity_LTEUE = 0;
	double avg_capacity_WIFIUE = 0;
	vector <double> T_LTEUE;
	vector <double> T_WIFIUE;
	double avg_T_UE = 0;
	double avg_T_UE_LTE = 0;
	double avg_T_UE_WIFI = 0;
	int DB_satisfied = 0;
	int DB50_satisfied = 0;
	int DB100_satisfied = 0;
	int DB300_satisfied = 0;
	double throughput = 0;		//虫:Kbps

	for (int i = 0; i < cs->bslist.size(); i++)
	{
		if (cs->bslist.at(i).type == macro)
		{
			number_eNB++;
			avg_T_LTE += cs->bslist.at(i).systemT;
			//cout << i << " :" << bslist->at(i).systemT << " ; " << bslist->at(i).T_max << endl;
			avg_UE_number_LTE += cs->bslist.at(i).connectingUE.size();
			for (int j = 0; j < cs->bslist.at(i).connectingUE.size(); j++)
			{
				number_UE_LTE++;
				if (cs->bslist.at(i).connectingUE.at(j)->delay_budget == 50)
				{
					number_UE_DB_50++;
					UE_DB[i][0]++;
					if (cs->bslist.at(i).systemT < 50)
						DB50_satisfied++;
				}					
				else
				{
					if (cs->bslist.at(i).connectingUE.at(j)->delay_budget == 100)
					{
						number_UE_DB_100++;
						UE_DB[i][1]++;
						if (cs->bslist.at(i).systemT < 100)
							DB100_satisfied++;
					}
						
					else
					{
						number_UE_DB_300++;
						UE_DB[i][2]++;
						if (cs->bslist.at(i).systemT < 300)
							DB300_satisfied++;
					}
				}
				double capacity = get_C(cs->bslist.at(i).connectingUE.at(j)) / cs->bslist.at(i).connectingUE.size();
				capacity_LTEUE.push_back(capacity);
				avg_capacity_LTEUE += capacity;
				if (capacity > cs->bslist.at(i).connectingUE.at(j)->bit_rate)
					throughput += cs->bslist.at(i).connectingUE.at(j)->bit_rate;
				else
					throughput += capacity;
				T_LTEUE.push_back(cs->bslist.at(i).systemT);
				avg_T_UE_LTE += cs->bslist.at(i).systemT;
			}
		}	
		else
		{
			number_AP++;
			avg_T_WIFI += cs->bslist.at(i).systemT;
			//cout << i << " :" << bslist->at(i).systemT << " ; " << bslist->at(i).T_max << endl;
			avg_UE_number_WIFI += cs->bslist.at(i).connectingUE.size();
			for (int j = 0; j < cs->bslist.at(i).connectingUE.size(); j++)
			{
				number_UE_WIFI++;
				if (cs->bslist.at(i).connectingUE.at(j)->delay_budget == 50)
				{
					number_UE_DB_50++;
					UE_DB[i][0]++;
					if (cs->bslist.at(i).systemT < 50)
						DB50_satisfied++;
				}
				else
				{
					if (cs->bslist.at(i).connectingUE.at(j)->delay_budget == 100)
					{
						number_UE_DB_100++;
						UE_DB[i][1]++;
						if (cs->bslist.at(i).systemT < 100)
							DB100_satisfied++;
					}

					else
					{
						number_UE_DB_300++;
						UE_DB[i][2]++;
						if (cs->bslist.at(i).systemT < 300)
							DB300_satisfied++;
					}
				}
				double capacity = get_C(cs->bslist.at(i).connectingUE.at(j)) / cs->bslist.at(i).connectingUE.size();
				capacity_WIFIUE.push_back(capacity);
				avg_capacity_WIFIUE += capacity;
				if (capacity > cs->bslist.at(i).connectingUE.at(j)->bit_rate)
					throughput += cs->bslist.at(i).connectingUE.at(j)->bit_rate;
				else
					throughput += capacity;
				T_WIFIUE.push_back(cs->bslist.at(i).systemT);
				avg_T_UE_WIFI += cs->bslist.at(i).systemT;
			}
		}

		avg_T += cs->bslist.at(i).systemT;
		avg_UE_number += cs->bslist.at(i).connectingUE.size();
	}

	non_outage_UE = number_UE_LTE + number_UE_WIFI;
	outage_UE = cs->uelist.size() - non_outage_UE;
	DB_satisfied = DB50_satisfied + DB100_satisfied + DB300_satisfied;

	avg_T /= cs->bslist.size();
	avg_T_LTE /= (double)number_eNB;
	avg_T_WIFI /= (double)number_AP;
	
	avg_UE_number /= cs->bslist.size();
	avg_UE_number_LTE /= (double)number_eNB;
	avg_UE_number_WIFI /= (double)number_AP;
	
	avg_capacity_UE = (avg_capacity_LTEUE + avg_capacity_WIFIUE) / (double)non_outage_UE;
	avg_capacity_LTEUE /= (double)number_UE_LTE;
	avg_capacity_WIFIUE /= (double)number_UE_WIFI;

	avg_T_UE = (avg_T_UE_LTE + avg_T_UE_WIFI) / (double)non_outage_UE;
	avg_T_UE_LTE /= (double)number_UE_LTE;
	avg_T_UE_WIFI /= (double)number_UE_WIFI;

	double stdev_T = 0;			//┮ΤTキА
	double stdev_T_LTE = 0;		//┮ΤLTE eNB TキА
	double stdev_T_WIFI = 0;	//┮ΤWIFI AP TキА
	double stdev_UE_number = 0;		//┮ΤBSUE计秖夹非畉
	double stdev_UE_number_LTE = 0;	//┮ΤLTE eNBUE计秖夹非畉
	double stdev_UE_number_WIFI = 0;//┮ΤWIFI APUE计秖夹非畉

	for (int i = 0; i < cs->bslist.size(); i++)
	{
		if (cs->bslist.at(i).type == macro)
		{
			stdev_T_LTE += pow(cs->bslist.at(i).systemT - avg_T_LTE, 2);
			stdev_UE_number_LTE += pow(cs->bslist.at(i).connectingUE.size() - avg_UE_number_LTE, 2);
		}
		else
		{
			stdev_T_WIFI += pow(cs->bslist.at(i).systemT - avg_T_WIFI, 2);
			stdev_UE_number_WIFI += pow(cs->bslist.at(i).connectingUE.size() - avg_UE_number_WIFI, 2);
		}

		stdev_T += pow(cs->bslist.at(i).systemT - avg_T, 2);
		stdev_UE_number += pow(cs->bslist.at(i).connectingUE.size() - avg_UE_number, 2);
	}

	stdev_T /= cs->bslist.size();
	stdev_T = sqrt(stdev_T);
	stdev_T_LTE /= (double)number_eNB;
	stdev_T_LTE = sqrt(stdev_T_LTE);
	stdev_T_WIFI /= (double)number_AP;
	stdev_T_WIFI = sqrt(stdev_T_WIFI);

	stdev_UE_number /= cs->bslist.size();
	stdev_UE_number = sqrt(stdev_UE_number);
	stdev_UE_number_LTE /= (double)number_eNB;
	stdev_UE_number_LTE = sqrt(stdev_UE_number_LTE);
	stdev_UE_number_WIFI /= (double)number_AP;
	stdev_UE_number_WIFI = sqrt(stdev_UE_number_WIFI);

	double stdev_capacity_UE = 0;		//┮ΤUECapacity夹非畉
	double stdev_capacity_UE_LTE = 0;	//┮ΤLTE UECapacity夹非畉
	double stdev_capacity_UE_WIFI = 0;	//┮ΤWIFIUECapacity夹非畉
	double stdev_T_UE = 0;					//┮ΤUET夹非畉
	double stdev_T_UE_LTE = 0;				//┮ΤLTE UET夹非畉
	double stdev_T_UE_WIFI = 0;				//┮ΤWIFI UET夹非畉

	for (int i = 0; i < capacity_LTEUE.size(); i++)
	{
		stdev_capacity_UE_LTE += pow(capacity_LTEUE.at(i) - avg_capacity_LTEUE, 2);
		stdev_T_UE_LTE += pow(T_LTEUE.at(i) - avg_T_UE_LTE, 2);
	}		
	for (int i = 0; i < capacity_WIFIUE.size(); i++)
	{
		stdev_capacity_UE_WIFI += pow(capacity_WIFIUE.at(i) - avg_capacity_WIFIUE, 2);
		stdev_T_UE_WIFI += pow(T_WIFIUE.at(i) - avg_T_UE_WIFI, 2);
	}

	stdev_capacity_UE = (stdev_capacity_UE_LTE + stdev_capacity_UE_WIFI) / (double)non_outage_UE;
	stdev_capacity_UE = sqrt(stdev_capacity_UE);
	stdev_capacity_UE_LTE /= (double)number_UE_LTE;
	stdev_capacity_UE_LTE = sqrt(stdev_capacity_UE_LTE);
	stdev_capacity_UE_WIFI /= (double)number_UE_WIFI;
	stdev_capacity_UE_WIFI = sqrt(stdev_capacity_UE_WIFI);
	stdev_T_UE = (stdev_T_UE_LTE + stdev_T_UE_WIFI) / (double)non_outage_UE;
	stdev_T_UE = sqrt(stdev_T_UE);
	stdev_T_UE_LTE /= (double)number_UE_LTE;
	stdev_T_UE_LTE = sqrt(stdev_T_UE_LTE);
	stdev_T_UE_WIFI /= (double)number_UE_WIFI;
	stdev_T_UE_WIFI = sqrt(stdev_T_UE_WIFI);

	fstream output_result;
	char filename_result[50];
	if (UE_dis_type == uniform)
		sprintf_s(filename_result, "hs_UE%d_%s_result.csv", cs->uelist.size(), algorithm_name);
	if (UE_dis_type == hotspot)
		sprintf_s(filename_result, "hs_UE%d_%s_result.csv", cs->uelist.size(), algorithm_name);
	output_result.open(filename_result, ios::out | ios::app);

	//fstream output_extra;
	//char filename_extra[50];
	//sprintf_s(filename_result, "%s_UE%d_extra.csv", algorithm_name, uelist->size());
	//output_extra.open(filename_extra, ios::out | ios::app);
	
	if (output_mode == 0)
	{
		output_result << outage_UE << "," << avg_T << "," << stdev_T << "," << avg_UE_number << "," << stdev_UE_number << "," << avg_capacity_UE << "," << stdev_capacity_UE << "," << avg_T_UE << "," << stdev_T_UE << ","
			<< avg_T_LTE << "," << avg_T_WIFI << "," << stdev_T_WIFI << ","
			<< avg_UE_number_LTE << "," << avg_UE_number_WIFI << "," << stdev_UE_number_WIFI << ","
			<< avg_capacity_LTEUE << "," << stdev_capacity_UE_LTE << "," << avg_capacity_WIFIUE << "," << stdev_capacity_UE_WIFI << ","
			<< avg_T_UE_LTE << "," << avg_T_UE_WIFI << "," << stdev_T_UE_WIFI << "," 
			<< DB_satisfied << "," << (double)throughput/(double)1000 << "," << (double)DB_satisfied / (double)non_outage_UE << "," << (double)DB50_satisfied / (double)number_UE_DB_50 << "," << (double)DB100_satisfied / (double)number_UE_DB_100 << "," << (double)DB300_satisfied / (double)number_UE_DB_300 << ","
			<< (double)cs->Offloaded_UE_Number / (double)non_outage_UE << "," << cs->uelist.at(0).coor_X << "," << cs->uelist.at(0).coor_Y << endl;
	}
	else
	{
		cout << outage_UE << "," << avg_T << "," << stdev_T << "," << avg_UE_number << "," << stdev_UE_number << "," << avg_capacity_UE << "," << stdev_capacity_UE << "," << avg_T_UE << "," << stdev_T_UE << ","
			<< avg_T_LTE << "," << avg_T_WIFI << "," << stdev_T_WIFI << ","
			<< avg_UE_number_LTE << "," << avg_UE_number_WIFI << "," << stdev_UE_number_WIFI << ","
			<< avg_capacity_LTEUE << "," << stdev_capacity_UE_LTE << "," << avg_capacity_WIFIUE << "," << stdev_capacity_UE_WIFI << ","
			<< avg_T_UE_LTE << "," << avg_T_UE_WIFI << "," << stdev_T_UE_WIFI << ","
			<< DB_satisfied << "," << (double)throughput / (double)1000 << "," << (double)DB_satisfied / (double)non_outage_UE << "," << (double)DB50_satisfied / (double)number_UE_DB_50 << "," << (double)DB100_satisfied / (double)number_UE_DB_100 << "," << (double)DB300_satisfied / (double)number_UE_DB_300 << ","
			<< (double)cs->Offloaded_UE_Number / (double)non_outage_UE << "," << cs->uelist.at(0).coor_X << "," << cs->uelist.at(0).coor_Y << endl;
	}
	if (analysis_mode == 1)
	{
		fstream detail_analysis;
		char filename_detail[50];
		double t_max = T_threshold;
		int x = t_max * 100;
		if (UE_dis_type == uniform)
			sprintf_s(filename_detail, "%s_UE%d_BSinfo_%d.csv", algorithm_name, cs->uelist.size());
		if (UE_dis_type == hotspot)
			sprintf_s(filename_detail, "hs_%s_UE%d_BSinfo.csv", algorithm_name, cs->uelist.size());

		detail_analysis.open(filename_detail, ios::out | ios::app);
		for (int i = 0; i < cs->bslist.size(); i++)
			detail_analysis << cs->bslist.at(i).num << "," << UE_DB[i][0] << "," << UE_DB[i][1] << "," << UE_DB[i][2] << "," << cs->bslist.at(i).systemT << "," << cs->bslist.at(i).T_max << endl;
	}
}
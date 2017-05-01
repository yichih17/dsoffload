#include<iostream>
#include<vector>
#include<fstream>
#include"define.h"
#define analysis_mode 1		// 1:show the detail information of BS (delay budget distribution, system time)
#define output_mode 0		// 0:output csv; 1:print on the screen

using namespace std;

void result_output(vector <BS> *bslist, vector <UE> *uelist, char algorithm_name[])
{
	int non_outage_UE = 0;	//Τ硈钡BSUE计秖
	int outage_UE = 0;		//礚硈钡BSUE计秖
	int number_eNB = 0;		//LTE eNB计秖
	int number_AP = 0;		//WIFI AP计秖
	int number_UE_LTE = 0;	//LTE eNB UE计秖
	int number_UE_WIFI = 0;	//WIFI AP UE计秖

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
	double throughput = 0;

	for (int i = 0; i < bslist->size(); i++)
	{
		if (bslist->at(i).type == macro)
		{
			number_eNB++;
			avg_T_LTE += bslist->at(i).systemT;
			avg_UE_number_LTE += bslist->at(i).connectingUE.size();
			for (int j = 0; j < bslist->at(i).connectingUE.size(); j++)
			{
				number_UE_LTE++;
				double capacity = get_C(bslist->at(i).connectingUE.at(j));
				capacity_LTEUE.push_back(capacity);
				avg_capacity_LTEUE += capacity;
				if (capacity > bslist->at(i).connectingUE.at(j)->bit_rate)
					throughput += bslist->at(i).connectingUE.at(j)->bit_rate;
				else
					throughput += capacity;
				T_LTEUE.push_back(bslist->at(i).systemT);
				avg_T_UE_LTE += bslist->at(i).systemT;
				if (bslist->at(i).systemT < bslist->at(i).connectingUE.at(j)->delay_budget)
					DB_satisfied++;
			}
		}	
		else
		{
			number_AP++;
			avg_T_WIFI += bslist->at(i).systemT;
			avg_UE_number_WIFI += bslist->at(i).connectingUE.size();
			for (int j = 0; j < bslist->at(i).connectingUE.size(); j++)
			{
				number_UE_WIFI++;
				double capacity = get_C(bslist->at(i).connectingUE.at(j));
				capacity_WIFIUE.push_back(capacity);
				avg_capacity_WIFIUE += capacity;
				if (capacity > bslist->at(i).connectingUE.at(j)->bit_rate)
					throughput += bslist->at(i).connectingUE.at(j)->bit_rate;
				else
					throughput += capacity;
				T_WIFIUE.push_back(bslist->at(i).systemT);
				avg_T_UE_WIFI += bslist->at(i).systemT;
				if (bslist->at(i).systemT < bslist->at(i).connectingUE.at(j)->delay_budget)
					DB_satisfied++;
			}
		}

		avg_T += bslist->at(i).systemT;
		avg_UE_number += bslist->at(i).connectingUE.size();		
	}

	non_outage_UE = number_UE_LTE + number_UE_WIFI;
	outage_UE = uelist->size() - non_outage_UE;

	avg_T /= bslist->size();
	avg_T_LTE /= (double)number_eNB;
	avg_T_WIFI /= (double)number_AP;
	
	avg_UE_number /= bslist->size();
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

	for (int i = 0; i < bslist->size(); i++)
	{
		if (bslist->at(i).type == macro)
		{
			stdev_T_LTE += pow(bslist->at(i).systemT - avg_T_LTE, 2);
			stdev_UE_number_LTE += pow(bslist->at(i).connectingUE.size() - avg_UE_number_LTE, 2);
		}
		else
		{
			stdev_T_WIFI += pow(bslist->at(i).systemT - avg_T_WIFI, 2);
			stdev_UE_number_WIFI += pow(bslist->at(i).connectingUE.size() - avg_UE_number_WIFI, 2);
		}

		stdev_T += pow(bslist->at(i).systemT - avg_T, 2);
		stdev_UE_number += pow(bslist->at(i).connectingUE.size() - avg_UE_number, 2);
	}

	stdev_T /= bslist->size();
	stdev_T = sqrt(stdev_T);
	stdev_T_LTE /= (double)number_eNB;
	stdev_T_LTE = sqrt(stdev_T_LTE);
	stdev_T_WIFI /= (double)number_AP;
	stdev_T_WIFI = sqrt(stdev_T_WIFI);

	stdev_UE_number /= bslist->size();
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
		sprintf_s(filename_result, "%s_UE%d_result.csv", algorithm_name, uelist->size());
	if (UE_dis_type == hotspot)
		sprintf_s(filename_result, "hs_%s_UE%d_result.csv", algorithm_name, uelist->size());
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
			<< avg_T_UE_LTE << "," << avg_T_UE_WIFI << "," << stdev_T_UE_WIFI << "," << DB_satisfied << "," << throughput << endl;
	}
	else
	{
		cout << outage_UE << "," << avg_T << "," << stdev_UE_number << "," << avg_capacity_UE << "," << stdev_capacity_UE << ","
			<< avg_T_LTE << "," << avg_T_WIFI << "," << stdev_T_WIFI << ","
			<< avg_UE_number_LTE << "," << avg_UE_number_WIFI << ","
			<< avg_capacity_LTEUE << "," << stdev_capacity_UE_LTE << "," << avg_capacity_WIFIUE << "," << stdev_capacity_UE_WIFI << ","
			<< DB_satisfied << "," << throughput << endl;
	}
	if (analysis_mode == 1)
	{
		fstream detail_analysis;
		char filename_detail[50];
		if (UE_dis_type == uniform)
			sprintf_s(filename_detail, "%s_UE%d_BSinfo.csv", algorithm_name, uelist->size());
		if (UE_dis_type == hotspot)
			sprintf_s(filename_detail, "hs_%s_UE%d_BSinfo.csv", algorithm_name, uelist->size());

		detail_analysis.open(filename_detail, ios::out | ios::trunc);
		for (int i = 0; i < bslist->size(); i++)
		{
			int db50 = 0, db100 = 0, db300 = 0;
			for (int j = 0; j < bslist->at(i).connectingUE.size(); j++)
			{
				int db = bslist->at(i).connectingUE.at(j)->delay_budget;
				if (bslist->at(i).connectingUE.at(j)->delay_budget == 50)
					db50++;
				else if (bslist->at(i).connectingUE.at(j)->delay_budget == 100)
					db100++;
				else if (bslist->at(i).connectingUE.at(j)->delay_budget == 300)
					db300++;
			}
			double systemT = bslist->at(i).systemT;
			int db_limit = bslist->at(i).systemT_constraint;
			detail_analysis << bslist->at(i).num << "," << db50 << "," << db100 << "," << db300 << "," << bslist->at(i).systemT << "," << bslist->at(i).systemT_constraint << endl;
		}
	}
}
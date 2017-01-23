#include"define.h"
#include<iostream>
#include<iomanip>
#include<fstream>
#include<string>
#include<vector>

using namespace std;

vector<UE> vuelist;
vector<BS> vbslist;

int macro_cover[] = { 303, 357, 407, 449, 512, 565, 623, 688, 784, 894, 1019, 1162, 1325, 1511, 1732 };
int ap_cover[] = { 26, 28, 30, 39, 50, 60, 68, 82 };
int macro_SINR[] = { 19.5, 17, 15, 13.5, 11.5, 10, 8.5, 7, 5, 3, 1, -1, -3, -5, -7 };
int ap_SINR[] = { 4, 7, 9, 12, 16, 20, 21, 22 };


void initialconfig()
{
	/*Define Macro eNB*/
	BS macro;
	macro.coor_X = 0;
	macro.coor_Y = 0;
	macro.type = device_type::macro;
	vbslist.push_back(macro);
}

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
		while (!freadUE.eof())
		{
			char bufferx[10], buffery[10];
			freadUE >> bufferx;				//讀取x座標
			freadUE >> buffery;				//讀取y座標
			if (bufferx[0] != '\0')
			{
				UE temp;
				temp.coor_X = stof(bufferx);		//String to double
				temp.coor_Y = stof(buffery);		//把座標給UE
				temp.delaybg = 50;
				vuelist.push_back(temp);
			}
		}
	}
	freadUE.close();
}

void readAP()
{
	ifstream freadAP;
	freadAP.open("AP_dis.txt", ios::in);
	if (freadAP.fail())						//抓計事本內的UE座標
	{
		cout << "AP分布不存在\n產生新分布" << endl;
		distribution(ap);
		readAP();
	}
	else
	{
		while (!freadAP.eof())
		{
			char bufferx[10], buffery[10];
			freadAP >> bufferx;				//讀取x座標
			freadAP >> buffery;				//讀取y座標
			if (bufferx[0] != '\0')
			{
				BS temp;
				temp.coor_X = stof(bufferx);		//String to double
				temp.coor_Y = stof(buffery);		//把座標給AP
				temp.type = ap;
				vbslist.push_back(temp);
			}
		}
	}
	freadAP.close();
}

double getCQI(UE u, BS b)
{
	int CQI;
	double distance = sqrt(pow((b.coor_X - u.coor_X), 2) - pow((b.coor_Y - u.coor_Y), 2));
	if (b.type == macro)
	{
		for (int i = 0; i < 15; i++)
		{
			if (distance > macro_cover[i])
				CQI = i + 1;
			else
				return CQI;
		}
	}
	if (b.type == ap)
	{
		for (int i = 0; i < 8; i++)
		{
			if (distance > ap_cover[i])
				CQI = i + 1;
			else
				return CQI;
		}
	}
}


int main()
{
	initialconfig();

/* UE and AP location initial */
	readUE();		//讀入UE
	cout << "Number of UE :" << vuelist.size() << "\n";
	readAP();		//讀入BS
	cout << "Number of BS :" << vbslist.size() << "\n";
	/*
	for (int i = 0; i < vuelist.size(); i++)
	{
		cout << "UE " << i << ": X=" << vuelist[i].coor_X << ", Y=" << vuelist[i].coor_Y << "; DB=" << vuelist[i].delaybg << "\n";
	}
	for (int i = 0; i < vbslist.size(); i++)
	{
		cout << "BS " << i << ": X=" << vbslist[i].coor_X << ", Y=" << vbslist[i].coor_Y << "; type=" << vbslist[i].type << "\n";
	}
	*/
	
	
	for (int i = 0; i < vuelist.size(); i++)
	{
		vector <int> CQI;
		for (int j = 0; j < vbslist.size(); j++)
		{
			if (int C = getCQI(vuelist[i], vbslist[j]) != 0)
				CQI.push_back(C);
		}
	}



	return 0;
}
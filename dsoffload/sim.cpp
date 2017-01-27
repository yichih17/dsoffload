#include"define.h"
#include<iostream>
#include<iomanip>
#include<fstream>
#include<string>
#include<vector>

using namespace std;

vector<UE> vuelist;
vector<BS> vbslist;

int macro_cover[] = { 1732, 1511, 1325, 1162, 1019, 894, 784, 688, 623, 565, 512, 449, 407, 357, 303 };
int ap_cover[] = { 82, 68, 60, 50, 39, 30, 28, 26 };
double macro_SINR[] = { -7, -5, -3, -1, 1, 3, 5, 7, 8.5, 10, 11.5, 13.5, 15, 17, 19.5 };
int ap_SINR[] = { 22, 21, 20, 16, 12, 9, 7, 4 };


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

int getCQI(UE u, BS b)
{
	int CQI = 0 ;
	double distance = sqrt(pow((b.coor_X - u.coor_X), 2) + pow((b.coor_Y - u.coor_Y), 2));
	if (b.type == macro)
	{
		for (int i = 0; i < 15; i++)
		{
			if (distance < macro_cover[i])
				CQI = i+1;
			else
				return CQI;
		}
	}
	if (b.type == ap)
	{
		for (int i = 0; i < 8; i++)
		{
			if (distance < ap_cover[i])
				CQI = i+1;
			else
				return CQI;
		}
	}
	return CQI;
}


int main()
{
	initialconfig();

	//UE and AP location initial
	readUE();		//讀入UE
	cout << "Number of UE :" << vuelist.size() << "\n";
	readAP();		//讀入BS
	cout << "Number of BS :" << vbslist.size() << "\n";

/*	// Show UE coordinates
	for (int i = 0; i < vuelist.size(); i++)
		cout << "UE " << i << ": X=" << vuelist[i].coor_X << ", Y=" << vuelist[i].coor_Y << "; DB=" << vuelist[i].delaybg << "\n";
	for (int i = 0; i < vbslist.size(); i++)
		cout << "BS " << i << ": X=" << vbslist[i].coor_X << ", Y=" << vbslist[i].coor_Y << "; type=" << vbslist[i].type << "\n";
*/

	//
	for (int i = 0; i < vuelist.size(); i++)
		for (int j = 0; j < vbslist.size(); j++)
		{
			int CQI = getCQI(vuelist[i], vbslist[j]);
			if (CQI != 0)
			{
				vuelist[i].CQI_to_neiborBS.push_back(CQI);
				vuelist[i].neiborBS.push_back(&vbslist[j]);
			}
		}
	
	//Show # of neibor BS
/*	for (int i = 0; i < vuelist.size(); i++)
		cout << vuelist[i].neiborBS.size() << ", ";
*/
	

	return 0;
}
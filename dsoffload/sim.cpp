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
double macro_SINR[] = { 19.5, 17, 15, 13.5, 11.5, 10, 8.5, 7, 5, 3, 1, -1, -3, -5, -7 };
int ap_SINR[] = { 4, 7, 9, 12, 16, 20, 21, 22 };

void initialconfig()
{
	/*Define Macro eNB*/
	BS macro;
	macro.coor_X = 0;
	macro.coor_Y = 0;
	macro.type = BS_type::macro;
	vbslist.push_back(macro);
}

void readUE()
{
	ifstream freadUE;
	freadUE.open("UE_dis.txt", ios::in);
	if (freadUE.fail())						//��p�ƥ�����UE�y��
		cout << "UE��m�L�k���\n" << endl;
	else
	{
		while (!freadUE.eof())
		{
			double coor_x, coor_y;
			char bufferx[10], buffery[10];
			freadUE >> bufferx;				//Ū��x�y��
			freadUE >> buffery;				//Ū��y�y��
			if (bufferx[0] != '\0')
			{
				UE temp;
				temp.coor_X = stof(bufferx);		//String to double
				temp.coor_Y = stof(buffery);		//��y�е�UE
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
	if (freadAP.fail())						//��p�ƥ�����UE�y��
		cout << "AP��m�L�k���\n" << endl;
	else
	{
		while (!freadAP.eof())
		{
			double coor_x, coor_y;
			char bufferx[10], buffery[10];
			freadAP >> bufferx;				//Ū��x�y��
			freadAP >> buffery;				//Ū��y�y��
			if (bufferx[0] != '\0')
			{
				BS temp;
				temp.coor_X = stof(bufferx);		//String to double
				temp.coor_Y = stof(buffery);		//��y�е�AP
				temp.type = ap;
				vbslist.push_back(temp);
			}
		}
	}
	freadAP.close();
}

int main()
{
/* distribution generate */
	//distributioninit();

	initialconfig();

/* UE and AP location initial */
	readUE();		//Ū�JUE
	cout << "Number of UE :" << vuelist.size() << "\n";
	readAP();		//Ū�JBS
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



	return 0;
}
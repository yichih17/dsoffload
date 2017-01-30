#include"define.h"
#include<iostream>
#include<iomanip>
#include<fstream>
#include<string>
#include<vector>

using namespace std;

int macro_cover[] = { 1732, 1511, 1325, 1162, 1019, 894, 784, 688, 623, 565, 512, 449, 407, 357, 303 };
int ap_cover[] = { 82, 68, 60, 50, 39, 30, 28, 26 };
double macro_SINR[] = { -7, -5, -3, -1, 1, 3, 5, 7, 8.5, 10, 11.5, 13.5, 15, 17, 19.5 };
int ap_SINR[] = { 22, 21, 20, 16, 12, 9, 7, 4 };
double lte_eff[15] = { 0.1523, 0.2344, 0.377, 0.6016, 0.8770, 1.1758, 1.4766, 1.9141, 2.4063, 2.7305, 3.3223, 3.9023, 4.5234, 5.1152, 5.5546 };
double wifi_capacity[8] = { 6500, 13000, 19500, 26000,	39000, 52000, 58500, 65000 };

vector <UE> vuelist;
vector <BS> vbslist;

/*���Ҫ�l�]�w*/
void initialconfig()
{
	//Define Macro eNB
	BS macro;
	macro.coor_X = 0;
	macro.coor_Y = 0;
	macro.type = device_type::macro;
	macro.BSnum = 0;
	macro.lambda = 0;
	vbslist.push_back(macro);
}

/*Ū��UE����(�y��)*/
void readUE()
{
	ifstream freadUE;
	freadUE.open("UE_dis.txt", ios::in);
	if (freadUE.fail())						//��p�ƥ�����UE�y��
	{
		cout << "UE�������s�b\n���ͷs����" << endl;
		distribution(ue);
		readUE();
	}
	else
	{
		int num = 1;
		while (!freadUE.eof())
		{
			char bufferx[10], buffery[10];
			freadUE >> bufferx;				//Ū��x�y��
			freadUE >> buffery;				//Ū��y�y��
			if (bufferx[0] != '\0')
			{
				UE temp;
				temp.coor_X = stof(bufferx);//String to double
				temp.coor_Y = stof(buffery);//��y�е�UE
				temp.lambdai = 3;
				temp.psize = pktsize;
				temp.UEnum = num++;
				vuelist.push_back(temp);
			}
		}
	}
	freadUE.close();
}

/*Ū��AP����(�y��)*/
void readAP()
{
	ifstream freadAP;
	freadAP.open("AP_dis.txt", ios::in);
	if (freadAP.fail())						//��p�ƥ�����UE�y��
	{
		cout << "AP�������s�b\n���ͷs����" << endl;
		distribution(ap);
		readAP();
	}
	else
	{
		int i = 1;
		while (!freadAP.eof())
		{
			char bufferx[10], buffery[10];
			freadAP >> bufferx;				//Ū��x�y��
			freadAP >> buffery;				//Ū��y�y��
			if (bufferx[0] != '\0')
			{
				BS temp;
				temp.coor_X = stof(bufferx);//String to double
				temp.coor_Y = stof(buffery);//��y�е�AP
				temp.type = ap;				//�]�w��a�x����(AP)
				temp.BSnum = i++;
				temp.lambda = 0;
				vbslist.push_back(temp);
			}
		}
	}
	freadAP.close();
}
/*
��s�Ҧ�UE��neiborBS�M��B�PneiborBS��CQIB
void updata_CQI()
{
	//Count the CQI to all BS of a UE
	for (int i = 0; i < vuelist.size(); i++)
	{
		vuelist[i].CQI_to_neiborBS.clear();
		vuelist[i].neiborBS.clear();
		for (int j = 0; j < vbslist.size(); j++)
		{
			int CQI = getCQI2(&vuelist[i], &vbslist[j]);		//�p��UE�PBS��CQI
			if (CQI != 0)									//�p�GUE�bBS���d��
			{
				vuelist[i].CQI_to_neiborBS.push_back(CQI);	//����CQI
				vuelist[i].neiborBS.push_back(&vbslist[j]);	//�NBS�[�JneighborBS�M��
			}
		}
	}
		
	//Show # of neibor BS
	for (int i = 0; i < vuelist.size(); i++)
		cout << vuelist[i].neiborBS.size() << ", ";

}*/

int main()
{
	initialconfig();	

	//UE and AP location initial
	readAP();		//Ū�JBS
	cout << "Number of BS :" << vbslist.size() << "\n";
	readUE();		//Ū�JUE
	cout << "Number of UE :" << vuelist.size() << "\n";

/*	// Show UE coordinates
	for (int i = 0; i < vuelist.size(); i++)
		cout << "UE " << i << ": X=" << vuelist[i].coor_X << ", Y=" << vuelist[i].coor_Y << "; DB=" << vuelist[i].delaybg << "\n";
	for (int i = 0; i < vbslist.size(); i++)
		cout << "BS " << i << ": X=" << vbslist[i].coor_X << ", Y=" << vbslist[i].coor_Y << "; type=" << vbslist[i].type << "\n";
*/
	//updata_CQI();

	//UEs associate
	for (int i = 0; i < vuelist.size(); i++)
	{
		vuelist[i].connecting_BS = findbs(&vuelist[i]);
		vuelist[i].connecting_BS->connectingUE.push_back(&vuelist[i]);
	//	cout << vuelist[i].connecting_BS->BSnum << ", ";
	}

	//debug
	cout << "BS has m UE:\n";
	for (int i = 0; i < vbslist.size(); i++)
		cout << vbslist[i].connectingUE.size() << ", ";
	return 0;
}
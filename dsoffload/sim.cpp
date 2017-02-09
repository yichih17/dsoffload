#include"define.h"
#include<iostream>
#include<iomanip>
#include<fstream>
#include<string>
#include<vector>

using namespace std;

vector <UE> vuelist;
vector <BS> vbslist;

/*���Ҫ�l�]�w*/
void initialconfig()
{
	//Define Macro eNB
	BS macro;
	macro.coor_X = 0;
	macro.coor_Y = 0;
	macro.type = type_bs::macro;
	macro.num = 0;
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
		int num = 1;						//num = UE�s��
		while (!freadUE.eof())
		{
			char bufferx[10], buffery[10];
			freadUE >> bufferx;				//Ū��x�y��
			freadUE >> buffery;				//Ū��y�y��
			if (bufferx[0] != '\0')			//txt�ɳ̫�@��O����Ÿ��A�J��N����Ū��
			{
				UE temp;
				temp.num = num++;			//assign�s���A�M��index++
				temp.coor_X = stof(bufferx);//String to double
				temp.coor_Y = stof(buffery);//��y�е�UE
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
				temp.num = i++;
				temp.type = ap;				//�]�w��a�x����(AP)
				temp.coor_X = stof(bufferx);//String to double
				temp.coor_Y = stof(buffery);//��y�е�AP
				//initialize
				temp.connectingUE.clear();
				temp.lambda = 0;
				temp.systemT = 0;			
				vbslist.push_back(temp);
			}
		}
	}
	freadAP.close();
}

int main()
{

	initialconfig();
	countAPrange();
	//UE and AP location initial
	readAP();		//Ū�JBS
	cout << "Number of BS :" << vbslist.size() << "\n";
	readUE();		//Ū�JUE
	cout << "Number of UE :" << vuelist.size() << "\n";

	//packet_arrival();
	/*	// Show UE coordinates
		for (int i = 0; i < vuelist.size(); i++)
			cout << "UE " << i << ": X=" << vuelist[i].coor_X << ", Y=" << vuelist[i].coor_Y << "; DB=" << vuelist[i].delaybg << "\n";
		for (int i = 0; i < vbslist.size(); i++)
			cout << "BS " << i << ": X=" << vbslist[i].coor_X << ", Y=" << vbslist[i].coor_Y << "; type=" << vbslist[i].type << "\n";
	*/

	/*Outage: ��UE�i�s������a�x�ҶW�X�t���A�N�|�S����a�x�i�s���A�]���w�q��outage */
	int outage_minT = 0;			//min T algorithm��outage UE�ƶq
	int outage_sinr = 0;			//sinr algorithm��outage UE�ƶq	**������
	int outage_proposed = 0;		//proposed algorithm��outage UE�ƶq	**������

	//UEs associate
	for (int i = 0; i < vuelist.size(); i++)
	{
		BS* target_bs = findbs_minT(&vuelist[i]);
		if (target_bs == NULL)
			outage_minT++;
		else
			add_UE_to_BS(&vuelist[i], target_bs);
//		vuelist[i].connecting_BS = findbs(&vuelist[i]);
//		vuelist[i].connecting_BS->connectingUE.push_back(&vuelist[i]);
//		vuelist[i].connecting_BS->lambda += vuelist[i].lambdai;
		//	cout << vuelist[i].connecting_BS->BSnum << ", ";
	}

	//debug
	size_t count = 0;
	for (int i = 0; i < vbslist.size(); i++)
	{
		printf("BS%3d has %4zd UE, T is ", vbslist[i].num, vbslist[i].connectingUE.size());
		cout << vbslist[i].systemT << "\n";
		count += vbslist[i].connectingUE.size();
	}
	cout << outage_minT;
	return 0;
}
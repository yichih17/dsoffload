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
				vbslist.push_back(temp);
			}
		}
	}
	freadAP.close();
}

/*�p��UE�PBS��CQI*/
int getCQI(UE* u, BS* b)
{
	int CQI = 0 ;
	double distance = sqrt(pow((b->coor_X - u->coor_X), 2) + pow((b->coor_Y - u->coor_Y), 2));	//�p��Z��
	if (b->type == macro)	//�p��LTE��CQI
		for (int i = 0; i < 15; i++)
		{
			if (distance < macro_cover[i])
				CQI = i+1;
			else
				return CQI;
		}
	if (b->type == ap)		//�p��Wifi��CQI
		for (int i = 0; i < 8; i++)
		{
			if (distance < ap_cover[i])
				CQI = i+1;
			else
				return CQI;
		}
	return CQI;				//���F����warning���o���[
}

/*��s�Ҧ�UE��neiborBS�M��B�PneiborBS��CQI*/
void updata_CQI()
{
	//Count the CQI to all BS of a UE
	for (int i = 0; i < vuelist.size(); i++)
	{
		vuelist[i].CQI_to_neiborBS.clear();
		vuelist[i].neiborBS.clear();
		for (int j = 0; j < vbslist.size(); j++)
		{
			int CQI = getCQI(&vuelist[i], &vbslist[j]);		//�p��UE�PBS��CQI
			if (CQI != 0)									//�p�GUE�bBS���d��
			{
				vuelist[i].CQI_to_neiborBS.push_back(CQI);	//����CQI
				vuelist[i].neiborBS.push_back(&vbslist[j]);	//�NBS�[�JneighborBS�M��
			}
		}
	}
		
/*	//Show # of neibor BS
	for (int i = 0; i < vuelist.size(); i++)
		cout << vuelist[i].neiborBS.size() << ", ";
*/
}

double getCwifi(int CQI)
{
	return wifi_capacity[CQI];
}

double getClte(int CQI)
{
	return resource_element * lte_eff[CQI] * total_RBG;
}

double getC(UE* u, BS* b)
{
	int CQI = getCQI(u, b);
	if (b->type == macro)
		return getClte(CQI);
	if (b->type == ap)
		return getCwifi(CQI);
	return 9487;
}

double getT(UE* u, BS* b, int CQI)
{
	//�պ�s�[�JUE u�᪺lambda
	double lambda = 0;
	for (int i = 0; i < b->connectingUE.size(); i++)
		lambda += b->connectingUE[i]->lambdai;
	lambda += u->lambdai;
	cout << "BS" << b->BSnum << " lambda: " << lambda;
	//1. �պ�UE u�[�J��i�o��capacity
	double capa_u;		
	if (b->type == macro)
		capa_u = getClte(CQI) / (b->connectingUE.size() + 1);
	if (b->type == ap)
		capa_u = getCwifi(CQI) / (b->connectingUE.size() + 1);
	cout << " capacity: " << capa_u;
	//2. �պ�UE u�[�J�᪺Xj(avg. service time)
	double Xj = pktsize / capa_u * (u->lambdai / lambda);		//UE u��Xij(service time)
	for (int i = 0; i< b->connectingUE.size(); i++)				//�[�W�쥻�bBS��UE��Xij
	{
		//Xj += Xij * (lambdai/lambda)
		//    = (pktsize /capacity) * (lambdai/lambda)
		if (b->type == macro)
			Xj += pktsize / (getClte(getCQI(b->connectingUE[i], b)) / b->connectingUE.size()) * (b->connectingUE[i]->lambdai/ lambda);
		if (b->type == ap)
			Xj += pktsize / (getCwifi(getCQI(b->connectingUE[i], b)) / b->connectingUE.size()) * (b->connectingUE[i]->lambdai / lambda);
	}
	cout << " Xj:" << Xj;
	//3. �պ�UE u�[�J�᪺Xj2
	double Xj2 = pow(pktsize / capa_u, 2) * (u->lambdai / lambda);
	for (int i = 0; i < b->connectingUE.size(); i++)				//�[�W�쥻�bBS��UE��Xij2
	{
		//Xj2 = Xij2 * (lambdai/lambda)
		//    = (paksize / capacity)^2 * (lambdai/lambda)
		if (b->type == macro)
			Xj += (pktsize / (getClte(getCQI(b->connectingUE[i], b)) / b->connectingUE.size()), 2) * (b->connectingUE[i]->lambdai / lambda);
		if (b->type == ap)
			Xj += (pktsize / (getCwifi(getCQI(b->connectingUE[i], b)) / b->connectingUE.size()), 2) * (b->connectingUE[i]->lambdai / lambda);
	}

	//4. �պ�UE u�[�J�᪺T
	double T = Xj + (lambda * Xj2) / (1 - (lambda / Xj));
	cout << " T: " << T << endl;
	return T;
}

/*�M��X�A��BS�s��*/
BS* findBS(UE *u)
{
	vector <BS*> neiborBS;
	BS* candidateBS = &vbslist[0];
	double minT = 999;
	//�M��UE��neiborBS
	for (int i = 0; i < vbslist.size(); i++)
	{
		int CQI = getCQI(u, &vbslist[i]);
		if (CQI != 0)
			neiborBS.push_back(&vbslist[i]);
	}
	//�qneiborBS����A�X��BS
	for (int i = 0; i < u->neiborBS.size(); i++)
	{
		int CQI = getCQI(u, u->neiborBS[i]);
		double T = getT(u, u->neiborBS[i], CQI);
		if (T < minT)
		{
			candidateBS = neiborBS[i];
			minT = T;
		}
		else if (T == minT)
		{
			double CapaA, CapaB;
			CapaA = getC(u, candidateBS);
			CapaB = getC(u, u->neiborBS[i]);
			if (CapaB > CapaA)
			{
				candidateBS = u->neiborBS[i];
				minT = T;
			}
		}
	}
	return candidateBS;
}

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
	updata_CQI();
	//UEs associate
	for (int i = 0; i < vuelist.size(); i++)
	{
		vuelist[i].connecting_BS = findBS(&vuelist[i]);
		vuelist[i].connecting_BS->connectingUE.push_back(&vuelist[i]);
	}

	//debug
	cout << "BS has m UE:\n";
	for (int i = 0; i < vbslist.size(); i++)
		cout << vbslist[i].connectingUE.size() << ", ";
	return 0;
}
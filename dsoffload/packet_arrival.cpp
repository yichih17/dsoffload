#include<iostream>
#include<fstream>
#include<sstream>
#include <time.h>
#include"define.h"

using namespace std;

/* Exponential Distribution with parameter x(lamda) */
double exp_distribution(double x)
{
	double y, z;
	y = (double)(rand() + 1) / (double)(RAND_MAX + 1);
	z = (double)log(y) * (double)(-1 / x);
	return z;
}

/* int��string */
string IntToString(int &i)
{
	string s;
	stringstream ss(s);
	ss << i;
	return ss.str();
}

/*����UE��packet arrival rate*/
void packet_arrival()
{
	//�w�qUE�A������ (*���ӥi�W�[UE���h�˩ʡA�V�J�U���P�A��������UE)
	//VoIP: bit rate(10K bps) packet size(800 bits)
	for (int i = 0; i < number_ue; i++)
	{
		//Voip: bit rate(10Kbps) packet size(800bits) delay budget(100ms)
		vuelist[i].bit_rate = 10;
		vuelist[i].packet_size = 800;
		vuelist[i].delay_budget = 100;
	}

	//��l�Ƽ����Ѽ�
	int timer = 1;						// �Ψӭp��ثe�{�����i�צp��
	string filename;					// �ɮצW��
	fstream WriteFile;					// �ŧifstream����
	double buffer_timer;				// �C��UE�beNB�̹���buffer���ɶ��b
	double inter_arrival_time = 0.0;	// packet��inter-arrival time
	bool across_TTI;					// �ΨӧP�_UE���ɶ��b�Apacket��inter-arrival time���L��L��TTI
	srand((unsigned)time(NULL));		// �üƺؤl

	for (int i = 0; i < number_ue; i++)
	{
		buffer_timer = 0.0;
		across_TTI = false;
		string UEIndex = IntToString(i);
		filename = "D:\\UE pattern\\UE" + UEIndex + ".txt";
		WriteFile.open(filename, ios::out | ios::trunc);
		if (WriteFile.fail())
			cout << "�ɮ׶}�ҥ���" << endl;
		else
		{
			for (int t = 0; t < TTI; t++)
			{
				while (buffer_timer <= t + 1)
				{
					WriteFile.setf(ios::fixed, ios::floatfield);
					WriteFile.precision(3);
					if (across_TTI)							// across_TTI: �ΨӬ�arrival time���L�W�L�o��TTI
						WriteFile << buffer_timer << endl;	// �O��arrival time
					else
					{
						inter_arrival_time = exp_distribution(vuelist[i].bit_rate / vuelist[i].packet_size);
						buffer_timer += inter_arrival_time;
					}
					if (buffer_timer > t + 1)
					{
						across_TTI = 1;
						break;
					}
					else if (across_TTI)
					{
						across_TTI = 0;
					}
					else
						WriteFile << buffer_timer << endl;
				}
			}
		}
		WriteFile.close();
	}
	cout << "UE arrival time�w����" << endl;
}
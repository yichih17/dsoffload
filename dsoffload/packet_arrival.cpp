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

/* int轉string */
string IntToString(int &i)
{
	string s;
	stringstream ss(s);
	ss << i;
	return ss.str();
}

/*產生UE的packet arrival rate*/
void packet_arrival()
{
	//定義UE服務類型 (*未來可增加UE的多樣性，混入各不同服務類型的UE)
	//VoIP: bit rate(10K bps) packet size(800 bits)
	for (int i = 0; i < number_ue; i++)
	{
		//Voip: bit rate(10Kbps) packet size(800bits) delay budget(100ms)
		vuelist[i].bit_rate = 10;
		vuelist[i].packet_size = 800;
		vuelist[i].delay_budget = 100;
	}

	//初始化模擬參數
	int timer = 1;						// 用來計算目前程式的進度如何
	string filename;					// 檔案名稱
	fstream WriteFile;					// 宣告fstream物件
	double buffer_timer;				// 每個UE在eNB裡對應buffer的時間軸
	double inter_arrival_time = 0.0;	// packet的inter-arrival time
	bool across_TTI;					// 用來判斷UE的時間軸，packet的inter-arrival time有無跨過此TTI
	srand((unsigned)time(NULL));		// 亂數種子

	for (int i = 0; i < number_ue; i++)
	{
		buffer_timer = 0.0;
		across_TTI = false;
		string UEIndex = IntToString(i);
		filename = "D:\\UE pattern\\UE" + UEIndex + ".txt";
		WriteFile.open(filename, ios::out | ios::trunc);
		if (WriteFile.fail())
			cout << "檔案開啟失敗" << endl;
		else
		{
			for (int t = 0; t < TTI; t++)
			{
				while (buffer_timer <= t + 1)
				{
					WriteFile.setf(ios::fixed, ios::floatfield);
					WriteFile.precision(3);
					if (across_TTI)							// across_TTI: 用來看arrival time有無超過這個TTI
						WriteFile << buffer_timer << endl;	// 記錄arrival time
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
	cout << "UE arrival time已完成" << endl;
}
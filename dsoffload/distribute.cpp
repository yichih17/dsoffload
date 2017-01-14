#include<iostream>
#include<fstream>
#include<string.h>
#include<random>
#include"define.h"

using std::string;
using std::fstream;
using std::cout;
using std::cin;
using std::endl;
using std::ios;



template <class T>
void uniformdistribution(T &equip)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> theta(0, 360);
	std::uniform_real_distribution<> k(0, 1);
	double r = R * sqrt(k(gen));
	double angel = (theta(gen));
	equip.coor_X = r * std::sin(angel);
	equip.coor_Y = r * std::cos(angel);
}

void distributioninit()
{
	/*generate UE*/
	UE* ue;
	ue = new UE[UE_number];
	fstream UEWrite;
	UEWrite.open("UE_dis.txt", ios::out | ios::trunc);
	if (UEWrite.fail())
		cout << "檔案無法開啟" << endl;
	else
		for (int i = 0; i < UE_number; i++)
		{
			uniformdistribution(ue[i]);
			UEWrite << ue[i].coor_X << " " << ue[i].coor_Y << endl;
		}
	UEWrite.close();

	/*generate AP*/
	BS* AP;
	AP = new BS[AP_number];
	fstream APWrite;
	APWrite.open("AP_dis.txt", ios::out | ios::trunc);
	if (APWrite.fail())
		cout << "檔案無法開啟" << endl;
	else
		for (int i = 0; i < AP_number; i++)
		{
			uniformdistribution(AP[i]);
			APWrite << AP[i].coor_X << " " << AP[i].coor_Y << endl;
		}
	APWrite.close();
}

double getSINR(UE ue, BS bs)
{
	double distance = sqrt(pow((bs.coor_X - ue.coor_X), 2) - pow((bs.coor_Y - ue.coor_Y), 2));
	double SINR = 0;
	switch (bs.type)
	{
	case macro:
		if (distance <= 1723.0)
			SINR = 19.5;
		if (distance <= 1511)
			SINR = 17;
		if (distance <= 1325)
			SINR = 15;
		if (distance <= 1162)
			SINR = 13.5;
		if (distance <= 1019)
			SINR = 11.5;
		if (distance <= 894)
			SINR = 10;
		if (distance <= 784)
			SINR = 8.5;
		if (distance <= 688)
			SINR = 7;
		if (distance <= 623)
			SINR = 5;
		if (distance <= 565)
			SINR = 3;
		if (distance <= 512)
			SINR = 1;
		if (distance <= 449)
			SINR = -1;
		if (distance <= 407)
			SINR = -3;
		if (distance <= 357)
			SINR = -5;
		if (distance <= 303)
			SINR = -7;
		break;
	case ap:
		if (distance <= 82)
			SINR = 22;
		if (distance <= 68)
			SINR = 21;
		if (distance <= 60)
			SINR = 20;
		if (distance <= 50)
			SINR = 16;
		if (distance <= 39)
			SINR = 12;
		if (distance <= 30)
			SINR = 9;
		if (distance <= 28)
			SINR = 6;
		if (distance <= 26)
			SINR = 4;
		break;
	default:
		break;
	}
	return SINR;
}
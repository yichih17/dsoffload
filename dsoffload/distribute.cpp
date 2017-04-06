#include<iostream>
#include<fstream>
#include<string.h>
#include<random>
#include<time.h>
#include"define.h"

using std::string;
using std::fstream;
using std::cout;
using std::cin;
using std::endl;
using std::ios;
using std::vector;

double hs_ratio = 0.8;

template <class T>
void uniformdistribution(T* equip)
{
	std::random_device rd;							//integer random number generator that produces non-deterministic random numbers. 
	std::mt19937 gen(rd());							//Mersenne Twister 19937 generator, generate a random number seed
	std::uniform_real_distribution<> theta(0, 360);	//definition of a uniform distribution range, a random number between 0 and 360
	std::uniform_real_distribution<> k(0, 1);		//definition of a uniform distribution range, a random number between 0 and 1
	
	//random a angle and random a radius, to gennerate a coordinate for UE
	double r = R * sqrt(k(gen));
	double angel = (theta(gen));
	equip->coor_X = r * std::sin(angel);
	equip->coor_Y = r * std::cos(angel);
}

bool in_hotspot(UE *u)
{
	double distance = 0;
	for (int i = 1; i <= (vbslist.size() - 1) * (1 - hs_ratio); i++)
	{
		distance = get_distance(u, &vbslist[i]);
		if (distance <= range_ap[0])
			return true;
	}
	return false;
}

void distribution(int AP_number, int UE_number)
{
	//產生AP座標
	for (int i = 0; i < AP_number; i++)
	{
		BS ap; 
		uniformdistribution(&ap);
		vbslist.push_back(ap);
	}		
	
	//產生UE座標
	if (UE_dis_type == hotspot)
	{
		srand((unsigned)time(NULL));			//給定亂數種子
		int hotspot_UE_number = 0;
		int non_hotspot_UE_number = 0;
		for (int i = 0; i < UE_number; i++)
		{
			bool hs_u = true;
			int ran = rand() % 10;
			if (ran < 8)	//八成機率要hotspot
				hs_u = true;	//現在這個UE要在hotspot內			
			else
				hs_u = false;	//現在這個UE要在hotspot外

			if (hotspot_UE_number >= UE_number * hs_ratio)
				hs_u = false;
			if (non_hotspot_UE_number >= UE_number*(1 - hs_ratio))
				hs_u = true;

			if (hs_u == true)
				hotspot_UE_number++;
			else
				non_hotspot_UE_number++;

			UE ue;
			do
			{
				uniformdistribution(&ue);
			} while (in_hotspot(&ue) != hs_u);
			vuelist.push_back(ue);
		}
	}
	else
		for (int i = 0; i < UE_number; i++)
		{
			UE ue;
			uniformdistribution(&ue);
			vuelist.push_back(ue);
		}
	
	//輸出AP座標
	fstream APwrite;
	APwrite.open("AP_dis.txt", ios::out | ios::trunc);
	if (APwrite.fail())
		cout << "檔案無法開啟" << endl;
	else
	{
		for (int i = 0; i < AP_number; i++)
			APwrite << vbslist[i].coor_X << " " << vbslist[i].coor_Y << endl;
	}
	APwrite.close();

	//輸出UE座標
	fstream UEwrite;
	UEwrite.open("UE_dis.txt", ios::out | ios::trunc);
	if (UEwrite.fail())
		cout << "檔案無法開啟" << endl;
	else
	{
		for (int i = 0; i < UE_number; i++)
			UEwrite << vuelist[i].coor_X << " " << vuelist[i].coor_Y << endl;
	}
	UEwrite.close();
}
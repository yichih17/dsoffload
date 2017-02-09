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
void uniformdistribution(T* equip)
{
	std::random_device rd;		//integer random number generator that produces non-deterministic random numbers. 
	std::mt19937 gen(rd());		//Mersenne Twister 19937 generator, generate a random number seed
	std::uniform_real_distribution<> theta(0, 360);		//definition of a uniform distribution range, a random number between 0 and 360
	std::uniform_real_distribution<> k(0, 1);		//definition of a uniform distribution range, a random number between 0 and 1
	/*random a angle and random a radius, to gennerate a coordinate for UE*/
	double r = R * sqrt(k(gen));
	double angel = (theta(gen));
	equip->coor_X = r * std::sin(angel);
	equip->coor_Y = r * std::cos(angel);
}

/* Generate UE/BS distribution */
void distribution(type_bs dtype)
{
	/*generate UE distribution*/
	if (dtype == ue)
	{
		fstream UEWrite;
		UEWrite.open("UE_dis.txt", ios::out | ios::trunc);
		if (UEWrite.fail())
			cout << "檔案無法開啟" << endl;
		else
			for (int i = 0; i < number_ue; i++)
			{
				UE ue;
				uniformdistribution(&ue);
				UEWrite << ue.coor_X << " " << ue.coor_Y << endl;
			}
		UEWrite.close();
	}
	/*generate AP distribution*/
	if (dtype == ap)
	{
		fstream APWrite;
		APWrite.open("AP_dis.txt", ios::out | ios::trunc);
		if (APWrite.fail())
			cout << "檔案無法開啟" << endl;
		else
			for (int i = 0; i < number_ap; i++)
			{
				BS ap;
				uniformdistribution(&ap);
				APWrite << ap.coor_X << " " << ap.coor_Y << endl;
			}
		APWrite.close();
	}
}
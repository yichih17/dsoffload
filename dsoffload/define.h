#ifndef _DEFIEN_H
#define _DEFIEN_H

#define UE_number 500	//UE個數
#define AP_number 1000	//AP個數
#define R 1723			//Macro eNB半徑
#define Macro_power 46	//Macro eNB transmissino power
#define AP_power 23		//AP transmission power

#include<vector>

enum device_type{macro, ap, ue};


/*attribute of BS*/
struct BS
{
	double coor_X, coor_Y;
	double systemT;
	device_type type;
};

/*attribute of UE*/
struct UE
{
	double coor_X, coor_Y;
	std::vector <int> CQI_to_neiborBS;
	std::vector <BS*> neiborBS;
	double Capacity_to_BS;

	int connecting_BS;

	int delaybg;
};



void distribution(device_type dtype);
double getSINR(UE ue, BS bs);



#endif // !_DEFIEN_H

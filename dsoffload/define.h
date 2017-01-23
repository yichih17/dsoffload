#ifndef _DEFIEN_H
#define _DEFIEN_H

#define UE_number 500	//UE個數
#define AP_number 30	//AP個數
#define R 1723			//Macro eNB半徑
#define Macro_power 46	//Macro eNB transmissino power
#define AP_power 23		//AP transmission power

enum device_type{macro, ap, ue};

/*attribute of UE*/
typedef struct UE
{
	double coor_X, coor_Y;
	bool neibor_BS;
	double SINR_to_BS;
	double Capacity_to_BS;

	int connecting_BS;

	int delaybg;
};

/*attribute of BS*/
typedef struct BS
{
	double coor_X, coor_Y;
	double systemT;
	device_type type;
};

void distribution(device_type dtype);
double getSINR(UE ue, BS bs);



#endif // !_DEFIEN_H

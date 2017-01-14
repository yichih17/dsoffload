#ifndef _DEFIEN_H
#define _DEFIEN_H
#define UE_number 500
#define AP_number 30
#define R 1723
#define Macro_power 46
#define AP_power 23


enum BS_type{macro, ap};
extern int BS_number;

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
	BS_type type;
};

void distributioninit();
double getSINR(UE ue, BS bs);




#endif // !_DEFIEN_H

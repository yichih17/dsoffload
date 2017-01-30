#ifndef _DEFIEN_H
#define _DEFIEN_H

#define UE_number 500	//UE�Ӽ�
#define AP_number 1000	//AP�Ӽ�
#define R 1723			//Macro eNB�b�|
#define Macro_power 46	//Macro eNB transmissino power
#define AP_power 23		//AP transmission power

#define pktsize 500		//Packet size (bit)
#define subcarrier 12	//number of subcarrier (12)
#define	total_symbol 14	//number of symbol in a RB (14)
#define ctrl_symbol 2	//number of symbol for control signal in a RB (1~3)
#define resource_element (total_symbol-ctrl_symbol)*subcarrier	//number of resource element per RB for data
#define total_RBG 100

#include<vector>

enum device_type{macro, ap, ue};

struct BS;
struct UE;
extern std::vector <BS> vbslist;
extern std::vector <UE> vuelist;

/*attribute of BS*/
struct BS
{
	int BSnum;
	double coor_X, coor_Y;
	double lambda;
	double systemT;
	std::vector <UE*> connectingUE;
	device_type type;
};

/*attribute of UE*/
struct UE
{
	int UEnum;
	double coor_X, coor_Y;
	std::vector <int> CQI_to_neiborBS;
	std::vector <BS*> neiborBS;
	std::vector <double> C_to_BS;			//capacity C*
	std::vector <double> T_to_BS;			//system time T*
	double Xij;
	BS* connecting_BS;

	int delaybg;
	double lambdai;
	int psize;
};

//distribute.cpp
extern void distribution(device_type dtype);

//function.cpp
extern double getdis(UE* u, BS* b);
extern int getCQI2(UE* u, BS* b);
extern double predictCapa(UE* u, BS* b);
extern double getCapa(UE* u);
extern double predictT(UE* u, BS* b);
extern double getT(BS* b);




#endif // !_DEFIEN_H

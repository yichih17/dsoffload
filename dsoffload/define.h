#ifndef _DEFIEN_H
#define _DEFIEN_H

#define number_ue 5000	//UE�Ӽ�
#define number_ap 500	//AP�Ӽ�
#define power_macro 46	//transmissino power of macro eNB
#define power_ap 23		//transmission power of wifi ap
#define R 1723			//Macro eNB�b�|

#define pktsize 500.0	//Packet size (bit)
#define subcarrier 12	//number of subcarrier (12)
#define	total_symbol 14	//number of symbol in a RB (14)
#define ctrl_symbol 2	//number of symbol for control signal in a RB (1~3)
#define resource_element (total_symbol - ctrl_symbol) * subcarrier	//number of resource element per RB for data
#define total_RBG 100	//number of total RBG(Resource Block Group)

#define TTI 1000		//total simulation time (TTI)


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
	std::vector <BS*> availBS;
	double Xij;
	BS* connecting_BS;

	int delay_budget;
	double lambdai;
	double packet_size;
	double bit_rate;
};

//function declare
//in distribute.cpp
extern void distribution(device_type dtype);
//in packet_arrival.cpp
extern void packet_arrival();
//in function.cpp
extern void countAPrange();
extern double getDistance(UE* u, BS* b);
extern int getCQI(UE* u, BS* b);
extern double predict_Capacity(UE* u, BS* b);
extern double getCapacity(UE* u, BS* b);
extern double getCapacity(UE* u);
extern double predictT(UE* u, BS* b);
extern BS* findbs(UE* u);
extern void BSbaddUEu(UE* u, BS* b);

#endif // !_DEFIEN_H

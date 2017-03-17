#ifndef _DEFIEN_H
#define _DEFIEN_H

#define power_macro 46	//transmissino power of macro eNB
#define power_ap 23		//transmission power of wifi ap
#define R 1723			//Macro eNB¥b®|

#define pktsize 500.0	//Packet size (bit)
#define subcarrier 12	//number of subcarrier (12)
#define	total_symbol 14	//number of symbol in a RB (14)
#define ctrl_symbol 2	//number of symbol for control signal in a RB (1~3)
#define resource_element (total_symbol - ctrl_symbol) * subcarrier	//number of resource element per RB for data
#define total_RBG 100	//number of total RBG(Resource Block Group)

#define TTI 1000		//total simulation time (TTI)

#include<vector>

enum type_bs { macro, ap, ue };
enum type_ue { type1, type2, type3, type4};

struct BS;
struct UE;
extern std::vector <BS> vbslist;
extern std::vector <UE> vuelist;

extern int max_depth;
int range_macro[];
int range_ap[];

/*attribute of BS*/
struct BS
{
	int num;
	type_bs type;
	double coor_X, coor_Y;
	double lambda;
	double systemT;
	std::vector <UE*> connectingUE;
};

/*attribute of UE*/
struct UE
{
	int num;
	type_ue type;
	double coor_X, coor_Y;
	double lambdai;
	int delay_budget;
	double packet_size;
	double bit_rate;
	BS* connecting_BS;
	std::vector <BS*> availBS;
};

struct result
{
	int outage_number;
};

struct connection_status
{
	std::vector <BS> bslist;
	std::vector <UE> uelist;
	int influence;
	int outage_dso;
};

//function declare
//in distribute.cpp
extern void distribution(type_bs dtype, int number);
//in packet_arrival.cpp
extern void packet_arrival();
//in function.cpp
//extern void countAPrange();
extern double calc_distance(UE* u, BS* b);
extern int calc_CQI(UE* u, BS* b);
extern double predict_Capacity(UE* u, BS* b);
extern double getCapacity(UE* u, BS* b);
extern double getCapacity(UE* u);
extern double predictT(UE* u, BS* b);
extern double getT(BS* b);
extern double getrho(BS* b);
extern BS* findbs_minT(UE *u, std::vector <BS> *bslist);
extern bool findbs_dso(UE* u, connection_status* cs, int k);
extern void add_UE_to_BS(UE* u, BS* b);
extern void availbs(UE* u, std::vector<BS> *bslist);

//SINR-based.cpp
void findbs_sinr(UE *u, std::vector <BS> *bslist);

//result.cpp
void result_output(std::vector <BS> *bslist, std::vector <UE> *uelist, char algorithm_name[]);

#endif // !_DEFIEN_H

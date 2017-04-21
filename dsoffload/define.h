#ifndef _DEFIEN_H
#define _DEFIEN_H

#define power_macro 46.0	//transmissino power of macro eNB
#define power_ap 23.0		//transmission power of wifi ap
#define R 1723				//Macro eNB¥b®|

#define pktsize 500.0	//Packet size (bit)
#define subcarrier 12	//number of subcarrier (12)
#define	total_symbol 14	//number of symbol in a RB (14)
#define ctrl_symbol 2	//number of symbol for control signal in a RB (1~3)
#define resource_element (total_symbol - ctrl_symbol) * subcarrier	//number of resource element per RB for data
#define total_RBG 100	//number of total RBG(Resource Block Group)

#define TTI 1000		//total simulation time (TTI)

#define UE_dis_type 1	//distribution mode of UE (0:uniform, 1:hotspot)
#define UE_type_number 3
#define rho_max 0.99

#include<vector>

enum type_bs { macro, ap, ue };
enum type_ue { type1, type2, type3, type4 };
enum type_distribution {uniform, hotspot};

struct BS;
struct UE;
extern std::vector <BS> vbslist;
extern std::vector <UE> vuelist;

int range_macro[];
int range_ap[];
extern double macro_eff[15];
extern double ap_capacity[8];

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
	int CQI;
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

struct hotspot
{
	double coor_x;
	double coor_y;
};

//function declare
//in distribute.cpp
extern void distribution(int AP_number, int UE_number);
//in packet_arrival.cpp
extern void packet_arrival();

//SINR-based.cpp
void findbs_sinr(UE *u, std::vector <BS> *bslist);

//delay-sensitive.cpp
double get_distance(UE* u, BS* b);
int get_CQI(UE* u, BS* b);
double get_C(UE* u);
double predict_C(UE* u, BS* b);
double predict_C(UE* u, BS* b, int CQI);
double predict_T(UE* u, BS* b);
double predict_T(UE* u, BS* b, int CQI);
double update_T(BS* b);
bool findbs_dso(UE* u, connection_status* cs, int depth, int depth_max);
bool findbs_ex(UE* u, connection_status* cs, int depth, int depth_max);
void findbs_minT(UE *u, std::vector <BS> *bslist);
void findbs_capa(UE *u, std::vector <BS> *bslist);

//result.cpp
void result_output(std::vector <BS> *bslist, std::vector <UE> *uelist, char algorithm_name[]);

#endif // !_DEFIEN_H

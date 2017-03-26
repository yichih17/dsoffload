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

#define UE_dis_type 0	//distribution mode of UE (0:uniform, 1:hotspot)

#include<vector>

enum type_bs { macro, ap, ue };
enum type_ue { type1, type2, type3, type4};
enum type_distribution {uniform, hotspot};

struct BS;
struct UE;
extern std::vector <BS> vbslist;
extern std::vector <UE> vuelist;

extern int max_depth;
int range_macro[];
int range_ap[];
extern double macro_eff[15];
extern double ap_capacity[8];

extern int calc_dis_count;
extern int calc_cqi_count;
extern int availbs_count;
extern int predict_capacity_count;
extern int getcapacity1_count;
extern int getcapacity2_count;
extern int predictT_count;
extern int getT_count;
extern int is_influence_ue_count;
extern int is_all_ue_be_satisify_count;
extern int ue_join_bs_count;
extern int check_satisfy_count;

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
extern bool findbs_dso(UE* u, connection_status* cs, int k);
extern void availbs(UE* u, std::vector<BS> *bslist);

//SINR-based.cpp
void findbs_sinr(UE *u, std::vector <BS> *bslist);

//minT.cpp
void findbs_minT(UE *u, std::vector <BS> *bslist);

//delay-sensitive.cpp
bool findbs_dso_test(UE* u, connection_status* cs, int depth);

//result.cpp
void result_output(std::vector <BS> *bslist, std::vector <UE> *uelist, char algorithm_name[]);

#endif // !_DEFIEN_H

#ifndef _DEFIEN_H
#define _DEFIEN_H

//System Parameter
#define power_macro 46.0	//Transmissino power of macro eNB (dBm)
#define power_ap 23.0		//Transmission power of wifi ap (dBm)
#define R 1723				//Radius of macro eNB (m)
#define subcarrier 12		//Number of subcarrier (12)
#define	total_symbol 14		//Number of symbol in a RBG (14)
#define ctrl_symbol 2		//Number of symbol for control signal in a RB (1~3)
#define resource_element (total_symbol - ctrl_symbol) * subcarrier	//Number of resource element per RB for data
#define total_RBG 100		//Number of total RBG (20Mhz=100)

//Simulation Parameter
#define UE_dis_type 1		//Distribution of UE (0:uniform, 1:hotspot)
#define UE_type_number 3	//Number of UE type (In different QoS)
#define rho_max 0.9999		//Max utilization of BSs
#define BS_T_max 99999		//Max delay of BSs
#define ThreadExeMode 1		//0:Program executing in the sequence of algorithm name; 1:Program executing in the sequence of execution time

#define UE_type1_pkt_size 8000
#define UE_type1_bit_rate 300
#define UE_type1_delay_budget 50

#define UE_type2_pkt_size 8000
#define UE_type2_bit_rate 300
#define UE_type2_delay_budget 100

#define UE_type3_pkt_size 8000
#define UE_type3_bit_rate 300
#define UE_type3_delay_budget 300

//Debug function
#define read_mode 0			//UE distribution input mode. 0:store UE/BS coordination in program; 1:output UE/BS coordinaiton to txt file, and read from txt file later.
#define analysis_mode 0		//For analysis the detail information of BSs (include the distribution of UE delay budget, the system time, the system time constrain). 1:output detail
#define output_mode 0		//Output form. 0:output csv file; 1:print on the screen

#include<vector>

enum type_bs { macro, ap };
enum type_ue { type1, type2, type3 };
enum type_distribution { uniform, hotspot };

struct BS;
struct UE;
extern std::vector <BS> vbslist;
extern std::vector <UE> vuelist;

int range_macro[];
int range_ap[];
extern double macro_eff[15];
extern double ap_capacity[8];
extern double eNB_capacity[15];

/* Attribute of BS */
struct BS								//BS = eNB or AP
{
	int num;							//編號
	type_bs type;						//類型
	double coor_X, coor_Y;				//座標
	double lambda;						//Arrival rate
	double systemT;						//Delay
	double T_max;						//Delay upper bound
	std::vector <UE*> connectingUE;		//連接的UE
	int db50;							//Delay budget= 50的UE個數
	int db100;							//Delay budget=100的UE個數
	int db300;							//Delay budget=300的UE個數
};

/* Attribute of UE */
struct UE
{
	int num;							//編號
	type_ue type;						//類型 (type 1~3)
	double coor_X, coor_Y;				//座標
	double lambdai;						//Arrival rate
	int delay_budget;
	double packet_size;
	double bit_rate;
	BS* connecting_BS;					//已連接的BS
	int CQI;							//與已連接BS的CQI
	std::vector <BS*> availBS;			//可連接的BS清單
};

/* 系統所有的連接狀況 */
struct connection_status
{
	std::vector <BS> bslist;			//基地台清單
	std::vector <UE> uelist;			//UE清單
	int influence;						//該層影響的UE個數
	int Offloaded_UE_Number;			//總共offload的UE數量
};

/* Function declare */
//distribute.cpp
extern void distribution(int AP_number, int UE_number);

//SINR-based.cpp
void findbs_sinr(UE *u, std::vector <BS> *bslist);

//delay-sensitive.cpp
double get_distance(UE* u, BS* b);
int get_CQI(UE* u, BS* b);
double get_C(UE* u);
double predict_T(UE* u, BS* b);
double predict_T(UE* u, BS* b, int CQI);
double update_T(BS* b);
bool influence(BS* b, double T);
int influence(UE *u);
bool all_ue_satisfy(BS* b, double T);
bool ue_cp(UE* a, UE* b);
bool findbs_dso(UE* u, connection_status* cs, int depth, int depth_max, int DB_th);
void findbs_minT(UE *u, std::vector <BS> *bslist);
void findbs_capa(UE *u, std::vector <BS> *bslist);

//result.cpp
void result_output(connection_status *cs, char algorithm_name[]);

#endif // !_DEFIEN_H

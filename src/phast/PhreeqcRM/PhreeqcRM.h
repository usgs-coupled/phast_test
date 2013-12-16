#if !defined(PHREEQCRM_H_INCLUDED)
#define PHREEQCRM_H_INCLUDED
#include "PHRQ_base.h"
#include "IPhreeqcPhast.h"
#include "StorageBin.h"
#include <vector>
#include <list>
#include <set>

class PHRQ_io;
class IPhreeqc;

/*! @brief Enumeration used to return error codes.
*/
typedef enum {
	IRM_OK            =  0,  /*!< Success */
	IRM_OUTOFMEMORY   = -1,  /*!< Failure, Out of memory */
	IRM_BADVARTYPE    = -2,  /*!< Failure, Invalid VAR type */
	IRM_INVALIDARG    = -3,  /*!< Failure, Invalid argument */
	IRM_INVALIDROW    = -4,  /*!< Failure, Invalid row */
	IRM_INVALIDCOL    = -5,  /*!< Failure, Invalid column */
	IRM_BADINSTANCE   = -6,  /*!< Failure, Invalid rm instance id */
	IRM_FAIL          = -7,  /*!< Failure, Unspecified */
} IRM_RESULT;

class PhreeqcRM: public PHRQ_base
{
public:
	static void             CleanupReactionModuleInstances(void);
	static int              CreateReactionModule(int *nxyz, int *nthreads = NULL);
	static IRM_RESULT       DestroyReactionModule(int *n);
    static void             ErrorStop(const char * str = NULL, long l = -1);
	static PhreeqcRM      * GetInstance(int *n);
	static PHRQ_io &        GetRmIo(void) {return PhreeqcRM::phast_io;};

	PhreeqcRM(int *nxyz = NULL, int *thread_count = NULL, PHRQ_io * io=NULL);
	~PhreeqcRM(void);
	
	// Key methods	
	IRM_RESULT                                CreateMapping(int *grid2chem);
	IRM_RESULT                                DumpModule(int *dump_on, int *use_gz = NULL);
	int                                       FindComponents();
	IRM_RESULT                                GetConcentrations(double * c);
	IRM_RESULT                                InitialPhreeqc2Concentrations( 
                                                   double *c,
                                                   int n_boundary, 
                                                   int dim, 
                                                   int *boundary_solution1,
                                                   int *boundary_solution2 = NULL,
                                                   double *boundary_fraction = NULL);
	IRM_RESULT                                InitialPhreeqc2Module(
                                                   int *initial_conditions1 = NULL,
                                                   int *initial_conditions2 = NULL,	
                                                   double *fraction1 = NULL);
	int                                       LoadDatabase(const char * database);
	IRM_RESULT                                RunFile(int *initial_phreeqc, int * workers, int *utility, const char *chemistry_name);
	IRM_RESULT                                RunString(int *initial_phreeqc, int * workers, int *utility, const char *chemistry_name);
	IRM_RESULT                                RunCells(void);

	// Utilities
	static std::string                        Char2TrimString(const char * str, long l = -1);
	static bool                               FileExists(const std::string &name);
	static void                               FileRename(const std::string &temp_name, const std::string &name, 
		                                           const std::string &backup_name);

	// TODO ///////////////////////////
	void                           Calculate_well_ph(double *c, double * ph, double * alkalinity);
	void                           Convert_to_molal(double *c, int n, int dim);
	void                           Write_bc_raw(int *solution_list, int * bc_solution_count, 
                                        int * solution_number, 
                                        const std::string &prefix);

	// Getters 
	const std::vector < std::vector <int> > & GetBack(void) {return this->back;}
	std::vector<double> &                     GetCellVolume(void) {return this->cell_volume;}
	const int                                 GetChemistryCellCount(void) const {return this->count_chemistry;}
	const std::vector<std::string> &          GetComponents(void) const {return this->components;}
	const std::string                         GetDatabaseFileName(void) const {return this->database_file_name;}
	std::vector<double> &                     GetDensity(void); 
	const std::vector < int> &                GetEndCell(void) const {return this->end_cell;} 
	const std::string                         GetFilePrefix(void) const {return this->file_prefix;}
	const int                                 GetGridCellCount(void) const {return this->nxyz;}
	int                                       GetInputUnitsSolution(void) {return this->input_units_Solution;}
	int                                       GetInputUnitsPPassemblage(void) {return this->input_units_PPassemblage;}
	int                                       GetInputUnitsExchange(void) {return this->input_units_Exchange;}
	int                                       GetInputUnitsSurface(void) {return this->input_units_Surface;}
	int                                       GetInputUnitsGasPhase(void) {return this->input_units_GasPhase;}
	int                                       GetInputUnitsSSassemblage(void) {return this->input_units_SSassemblage;}
	int                                       GetInputUnitsKinetics(void) {return this->input_units_Kinetics;}
	const int                                 GetMpiMyself(void) const {return this->mpi_myself;}
	const int                                 GetMpiTasks(void) const {return this->mpi_tasks;}
	int                                       GetNthreads() {return this->nthreads;}
	int                                       GetNthSelectedOutputUserNumber(int *i);
	const bool                                GetPartitionUZSolids(void) const {return this->partition_uz_solids;}
	std::vector<double> &                     GetPoreVolume(void) {return this->pore_volume;}
	std::vector<double> &                     GetPoreVolumeZero(void) {return this->pore_volume_zero;} 
	std::vector<double> &                     GetPressure(void) {return this->pressure;}
	std::vector<int> &                        GetPrintChemistryMask (void) {return this->print_chem_mask;}
	const bool                                GetPrintChemistryOn(void) const {return this->print_chemistry_on;}  
	int                                       GetRebalanceMethod(void) const {return this->rebalance_method;}
	double                                    GetRebalanceFraction(void) const {return this->rebalance_fraction;}
	std::vector<double> &                     GetSaturation(void) {return this->saturation;}
	IRM_RESULT                                GetSelectedOutput(double *so);
	int                                       GetSelectedOutputColumnCount(void);
	int                                       GetSelectedOutputCount(void);
	IRM_RESULT                                GetSelectedOutputHeading(int *icol, std::string &heading);
	const bool                                GetSelectedOutputOn(void) const {return this->selected_output_on;}
	int                                       GetSelectedOutputRowCount(void);	
	const std::vector < int> &                GetStartCell(void) const {return this->start_cell;} 
	const bool                                GetStopMessage(void) const {return this->stop_message;}
	std::vector<double> &                     GetTemperature(void) {return this->tempc;}
	double                                    GetTime(void) const {return this->time;} 
	double                                    GetTimeStep(void) const {return this->time_step;}
	const double                              GetTimeConversion(void) const {return this->time_conversion;} 
	std::vector<IPhreeqcPhast *> &            GetWorkers() {return this->workers;}

	// Setters 
	IRM_RESULT                                SetChemistryFileName(const char * prefix = NULL);
	IRM_RESULT                                SetConcentrations(double * t = NULL); 
	int								          SetCurrentSelectedOutputUserNumber(int i = -1);
	IRM_RESULT                                SetDatabaseFileName(const char * db = NULL);
	IRM_RESULT                                SetCellVolume(double * t = NULL);
	IRM_RESULT                                SetDensity(double * t = NULL); 
	IRM_RESULT                                SetFilePrefix(std::string &fn); 
	IRM_RESULT                                SetFilePrefix(const char * prefix = NULL);
	IRM_RESULT                                SetPartitionUZSolids(int t = -1);
	IRM_RESULT                                SetPoreVolume(double * t = NULL); 
	IRM_RESULT                                SetPoreVolumeZero(double * t = NULL);
	IRM_RESULT                                SetPrintChemistryMask(int * t = NULL);
	IRM_RESULT                                SetPrintChemistryOn(int t = -1);
	IRM_RESULT                                SetPressure(double * t = NULL);  
	IRM_RESULT                                SetRebalanceFraction(double t = 0.0); 
	IRM_RESULT                                SetRebalanceMethod(int t); 
	IRM_RESULT                                SetSaturation(double * t); 
	void                                      SetSelectedOutputOn(int *t);
	void                                      SetStopMessage(bool t = false); 
	IRM_RESULT                                SetTemperature(double * t = NULL);
	IRM_RESULT                                SetTime(double *t = NULL);
	IRM_RESULT                                SetTimeConversion(double *t);
	IRM_RESULT                                SetTimeStep(double *t = NULL);
	//void                                      SetUnits(int *sol, int *pp, int *ex, int *surf, int *gas, int *ss, int *k);
	void                                      SetUnitsExchange(int *i);
	void                                      SetUnitsGasPhase(int *i);
	void                                      SetUnitsKinetics(int *i);
	void                                      SetUnitsPPassemblage(int *i);
	void                                      SetUnitsSolution(int *i);
	void                                      SetUnitsSSassemblage(int *i);
	void                                      SetUnitsSurface(int *i);
protected:
	void                                      BeginTimeStep(void);
	IRM_RESULT                                CellInitialize(
		                                          int i, 
		                                          int n_user_new, 
		                                          int *initial_conditions1,
		                                          int *initial_conditions2, 
		                                          double *fraction1,
		                                          std::set<std::string> error_set);
	int                                       CheckSelectedOutput();
	void                                      Concentrations2Solutions(int n, std::vector<double> &c);
	void                                      cxxSolution2concentration(cxxSolution * cxxsoln_ptr, std::vector<double> & d);
	cxxStorageBin &                           Get_phreeqc_bin(void) {return this->phreeqc_bin;}
	void                                      PartitionUZ(int n, int iphrq, int ihst, double new_frac);
	//void                                      Pressures2Solutions(int n, std::vector<double> &t);
	void                                      RebalanceLoad(void);
	void                                      RebalanceLoadPerCell(void);
	void                                      RunCellsThread(int i);
	IRM_RESULT                                RunFileThread(int n);
	IRM_RESULT                                RunStringThread(int n, std::string & input);
	void                                      Scale_solids(int n, int iphrq, LDBLE frac);
	void                                      SetEndCells(void);
	//void                                      Temperatures2Solutions(int n, std::vector<double> &t);
	void                                      TransferCells(cxxStorageBin &t_bin, int old, int nnew);
	void                                      WriteError(const char * item);
	void                                      WriteOutput(const char * item);

protected:
	std::string database_file_name;
	std::string chemistry_file_name;
	std::string file_prefix;
	cxxStorageBin uz_bin;
	cxxStorageBin phreeqc_bin;
	int mpi_myself;
	int mpi_tasks;
	std::vector <std::string> components;	// list of components to be transported
	std::vector <double> gfw;				// gram formula weights converting mass to moles (1 for each component)
	double gfw_water;						// gfw of water
	bool partition_uz_solids;
	int nxyz;								// number of nodes 
	int count_chemistry;					// number of cells for chemistry
	double time;						    // time from transport, sec 
	double time_step;					    // time step from transport, sec
	double time_conversion;					// time conversion factor, multiply to convert to preferred time unit for output
	//std::vector<double> concentration;		// nxyz by ncomps concentrations nxyz:components
	std::vector <double> old_saturation;	// saturation fraction from previous step
	std::vector<double> saturation;	        // nxyz saturation fraction
	std::vector<double> pressure;			// nxyz current pressure
	std::vector<double> pore_volume;		// nxyz current pore volumes 
	std::vector<double> pore_volume_zero;	// nxyz initial pore volumes
	std::vector<double> cell_volume;		// nxyz geometric cell volumes
	std::vector<double> tempc;				// nxyz temperature Celsius
	std::vector<double> density;			// nxyz density
	std::vector<int> print_chem_mask;		// nxyz print flags for output file
	bool rebalance_method;                  // rebalance method 0 std, 1 by_cell
	double rebalance_fraction;			    // parameter for rebalancing process load for parallel	
	int input_units_Solution;
	int input_units_PPassemblage;
	int input_units_Exchange;
	int input_units_Surface;
	int input_units_GasPhase;
	int input_units_SSassemblage;
	int input_units_Kinetics;
	std::vector <int> forward;				// mapping from nxyz cells to count_chem chemistry cells
	std::vector <std::vector <int> > back;	// mapping from count_chem chemistry cells to nxyz cells 

	// print flags
	bool print_chemistry_on;				// print flag for chemistry output file 
	bool selected_output_on;				// create selected output

	bool stop_message;

	// threading
	int nthreads;
	std::vector<IPhreeqcPhast *> workers;
	std::vector<int> start_cell;
	std::vector<int> end_cell;

private:
	friend class RM_interface;
	static PHRQ_io phast_io;
	static std::map<size_t, PhreeqcRM*> Instances;
	static size_t InstancesIndex;

	
};
#endif // !defined(PHREEQCRM_H_INCLUDED)

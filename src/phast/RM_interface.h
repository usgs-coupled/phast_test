/*! @file IPhreeqc.h
	@brief C/Fortran Documentation
*/
#ifndef RM_INTERFACE_H
#define RM_INTERFACE_H
#include "IPhreeqc.h"
#include "Var.h"
#if defined(_MSC_VER)
#define FC_FUNC_(name,NAME) NAME
#endif

#if defined(FC_FUNC_)
// Called from Fortran or C++
#define RM_calculate_well_ph               FC_FUNC_ (rm_calculate_well_ph,             RM_CALCULATE_WELL_PH)
#define RM_close_files                     FC_FUNC_ (rm_close_files,                   RM_CLOSE_FILES)    
#define RM_convert_to_molal                FC_FUNC_ (rm_convert_to_molal,              RM_CONVERT_TO_MOLAL)   
#define RM_create                          FC_FUNC_ (rm_create,                        RM_CREATE)
#define RM_destroy                         FC_FUNC_ (rm_destroy,                       RM_DESTROY)
#define RM_distribute_initial_conditions   FC_FUNC_ (rm_distribute_initial_conditions, RM_DISTRIBUTE_INITIAL_CONDITIONS)
#define RM_distribute_initial_conditions_mix  FC_FUNC_ (rm_distribute_initial_conditions_mix, RM_DISTRIBUTE_INITIAL_CONDITIONS_MIX)
#define RM_error                           FC_FUNC_ (rm_error,                         RM_ERROR)
#define RM_find_components                 FC_FUNC_ (rm_find_components,               RM_FIND_COMPONENTS)
//#define RM_forward_and_back                FC_FUNC_ (rm_forward_and_back,              RM_FORWARD_AND_BACK)
#define RM_fractions2solutions             FC_FUNC_ (rm_fractions2solutions,           RM_FRACTIONS2SOLUTIONS)
#define RM_get_component                   FC_FUNC_ (rm_get_component,                 RM_GET_COMPONENT)
#define RM_initial_phreeqc_run             FC_FUNC_ (rm_initial_phreeqc_run,           RM_INITIAL_PHREEQC_RUN)
#define RM_load_database                   FC_FUNC_ (rm_load_database,                 RM_LOAD_DATABASE)
#define RM_log_screen_prt                  FC_FUNC_ (rm_log_screen_prt,                RM_LOG_SCREEN_PRT)
#define RM_open_files                      FC_FUNC_ (rm_open_files,                    RM_OPEN_FILES)
#define RM_pass_data                       FC_FUNC_ (rm_pass_data,                     RM_PASS_DATA)
#define RM_pass_transient_data             FC_FUNC_ (rm_pass_transient_data,           RM_PASS_TRANSIENT_DATA)
#define RM_run_cells                       FC_FUNC_ (rm_run_cells,                     RM_RUN_CELLS)
#define RM_send_restart_name               FC_FUNC_ (rm_send_restart_name,             RM_SEND_RESTART_NAME)
#define RM_setup_boundary_conditions       FC_FUNC_ (rm_setup_boundary_conditions,     RM_SETUP_BOUNDARY_CONDITIONS)
#define RM_set_mapping                     FC_FUNC_ (rm_set_mapping,                   RM_SET_MAPPING)
#define RM_set_input_units                 FC_FUNC_ (rm_set_input_units,               RM_SET_INPUT_UNITS)
#define RM_solutions2fractions             FC_FUNC_ (rm_solutions2fractions,           RM_SOLUTIONS2FRACTIONS)
#define RM_transport                       FC_FUNC_ (rm_transport,                     RM_TRANSPORT)
#define RM_write_bc_raw                    FC_FUNC_ (rm_write_bc_raw,                  RM_WRITE_BC_RAW)
#define RM_write_output                    FC_FUNC_ (rm_write_output,                  RM_WRITE_OUTPUT)
#define RM_write_restart		   FC_FUNC_ (rm_write_restart,                 RM_WRITE_RESTART)
#define RM_zone_flow_write_chem            FC_FUNC_ (rm_zone_flow_write_chem,          RM_ZONE_FLOW_WRITE_CHEM)
// Calls to Fortran
#define transport_component_thread         FC_FUNC_ (transport_component_thread,       TRANSPORT_COMPONENT_THREAD)
#define zone_flow_write_chem               FC_FUNC_ (zone_flow_write_chem,             ZONE_FLOW_WRITE_CHEM)
#define logprt_c                           FC_FUNC_ (logprt_c,                         LOGPRT_C)
#define screenprt_c                        FC_FUNC_ (screenprt_c,                      SCREENPRT_C)
#define errprt_c                           FC_FUNC_ (errprt_c,                         ERRPRT_C)
#define warnprt_c                          FC_FUNC_ (warnprt_c,                        WARNPRT_C)
#endif
#ifdef SKIP
#define RM_calculate_well_ph                  rm_calculate_well_ph
#define RM_close_files                        rm_close_files
#define RM_convert_to_molal                   rm_convert_to_molal
#define RM_create                             rm_create
#define RM_destroy                            rm_destroy
#define RM_distribute_initial_conditions      rm_distribute_initial_conditions
#define RM_error                              rm_error
#define RM_find_components                    rm_find_components
#define RM_forward_and_back                   rm_forward_and_back
#define RM_fractions2solutions                rm_fractions2solutions
#define RM_get_component                      rm_get_component
#define RM_initial_phreeqc_run                rm_initial_phreeqc_run
#define RM_load_database                      rm_load_database
#define RM_log_screen_prt                     rm_log_screen_prt
#define RM_open_files                         rm_open_files
#define RM_pass_data                          rm_pass_data
#define RM_pass_transient_data                rm_pass_transient_data
#define RM_run_cells                          rm_run_cells
#define RM_send_restart_name                  rm_send_restart_name
#define RM_setup_boundary_conditions          rm_setup_boundary_conditions
#define RM_solutions2fractions                rm_solutions2fractions
#define RM_transport                          rm_transport
#define RM_write_bc_raw                       rm_write_bc_raw
#define RM_write_output                       rm_write_output
#define RM_write_restart					  rm_write_restart
#define RM_zone_flow_write_chem               rm_zone_flow_write_chem
#define transport_component_thread            transport_component_thread
#endif

class RM_interface
{
public:
	static int Create_reaction_module(int nthreads);
	static IPQ_RESULT Destroy_reaction_module(int n);
	static Reaction_module* Get_instance(int n);
	static void CleanupReactionModuleInstances(void);
	static PHRQ_io phast_io;

private:
	friend class Reaction_module;
	static std::map<size_t, Reaction_module*> Instances;
	static size_t InstancesIndex;
};

#if defined(__cplusplus)
extern "C" {
#endif
void RM_calculate_well_ph(int *id, double *c, double * ph, double * alkalinity);
void RM_close_files(int * solute);
void RM_convert_to_molal(int *id, double *c, int *n, int *dim);
int  RM_create(int *nthreads);
int  RM_destroy(int *id);
void RM_distribute_initial_conditions(int *id,
		int *initial_conditions1);		// 7 x nxyz end-member 1
void RM_distribute_initial_conditions_mix(int *id,
		int *initial_conditions1,		// 7 x nxyz end-member 1
		int *initial_conditions2,		// 7 x nxyz end-member 2
		double *fraction1			    // 7 x nxyz fraction of end-member 1
		);
void RM_error(int *id);
int RM_find_components(int *id);
void RM_forward_and_back(int *id,
		int *initial_conditions, 
		int *axes);
void RM_get_component(int * id, int * num, char *chem_name, int l1);
void RM_initial_phreeqc_run(int * id, char *db_name, char *chem_name, char *prefix_name, int l1, int l2, int l3);
void RM_log_screen_prt(char *err_str, long l);

void RM_mpi_barrier(int *comm, int *ierr);
void RM_mpi_bcast(void *buffer, int *count, int *datatype, int *root, int *comm, int *ierr);
void RM_mpi_comm_create(int *comm, int *group, int *newcom, int *ierr);
void RM_mpi_comm_group(int *comm, int *group, int *ierr);
void RM_mpi_get_address(int *location, int *address, int *ierr);
void RM_mpi_group_incl(int *group, int *n, int *ranks, int *newgroup, int *ierr);
void RM_mpi_recv(void *buffer, int *count, int *datatype, int *source, int *tag, int *comm, int* status, int *ierr);
void RM_mpi_send(void *buffer, int *count, int *datatype, int *dest, int *tag, int *comm, int *ierr);
void RM_mpi_type_commit(int *datatype, int *ierr);
void RM_mpi_type_create_struct(int *count,
  int *array_of_blocklengths,
  int *array_of_displacements,
  int *array_of_types,
  int *newtype, int *ierr);
void RM_mpi_type_free(int *datatype, int *ierr);
double RM_mpi_wtime(void);

void RM_open_files(int * solute, char * prefix, int l_prefix);
void RM_open_error_file(void);
void RM_open_output_file(char * prefix, int l_prefix);
void RM_open_punch_file(char * prefix, int l_prefix);
void RM_open_log_file(char * prefix, int l_prefix);
void RM_pass_data(int *id,
             bool *fresur,
			 bool *steady_flow, 
			 int *nx, int *ny, int *nz,			// number of nodes each coordinate direction
			 double *cnvtmi,					// conversion factor for time
			 double *x_node,					// nxyz array of X coordinates for nodes 
			 double *y_node,					// nxyz array of Y coordinates for nodes  
			 double *z_node,					// nxyz array of Z coordinates for nodes 
			 double *pv0,						// nxyz initial pore volumes
			 double *volume, 					// nxyz geometric cell volumes 
			 int * printzone_chem,				// nxyz print flags for output file
			 int * printzone_xyz,				// nxyz print flags for chemistry XYZ file
			 int * rebalance_method,            // 0 std; 1 by_cell
			 double *rebalance_fraction_hst,	// parameter for rebalancing process load for parallel	
			 double *fraction,                  // mass fraction array
			 int *mpi_myself,                   // let reaction_module know mpi_myself
			 int *mpi_tasks                     // let reaction_module know mpi_myself
			 );
void RM_run_cells(int *id,
			 int * prslm,							// solution method print flag 
			 int * print_out,						// print flag for output file 
			 int * print_sel,						// print flag for selected output
			 int * print_hdf,						// print flag for hdf file
			 int * print_restart,					// print flag for writing restart file 
			 double *time_hst,					    // time from transport 
			 double *time_step_hst,				    // time step from transport
 			 double *fraction,					    // mass fractions nxyz:components
			 double *frac,							// saturation fraction
			 double *pv,                            // nxyz current pore volumes 
			 int *nxyz,
			 int *count_comps,
			 int * stop_msg);
void RM_send_restart_name(int *id, char * s, long l);
void RM_setup_boundary_conditions(
			int *id,
			int *n_boundary, 
			int *boundary_solution1,  
			int *boundary_solution2, 
			double *fraction1,
			double *boundary_fraction, 
			int *dim);
void RM_set_mapping (int *id, int *grid2chem); 
void RM_set_input_units (int *id, int *sol, int *pp, int *ex, int *surf, int *gas, int *ss, int *kin);
void RM_solutions2fractions(int *id);
void RM_transport(int *id, int *ncomps);
void RM_write_bc_raw(int *id, 
			int *solution_list, 
			int * bc_solution_count, 
			int * solution_number, 
			char *prefix, 
			int prefix_l);
void RM_write_output(int *id);
void RM_write_restart(int *id);
void RM_zone_flow_write_chem(int *);

void errprt_c(const char *err_str, long l);
void logprt_c(const char *err_str, long l);
void screenprt_c(const char *err_str, long l);
void warnprt_c(const char *err_str, long l);

void RM_errprt(const std::string & e_string);
void RM_warnprt(const std::string & e_string);
void RM_logprt(const std::string & e_string);
void RM_screenprt(const std::string & e_string);
extern void transport_component(int *i);
extern void transport_component_thread(int *i);
extern void zone_flow_write_chem(void);

#if defined(__cplusplus)
}
#endif
// Global functions
//inline std::string trim_right(const std::string &source , const std::string& t = " \t")
//{
//	std::string str = source;
//	return str.erase( str.find_last_not_of(t) + 1);
//}
//
//inline std::string trim_left( const std::string& source, const std::string& t = " \t")
//{
//	std::string str = source;
//	return str.erase(0 , source.find_first_not_of(t) );
//}
//
//inline std::string trim(const std::string& source, const std::string& t = " \t")
//{
//	std::string str = source;
//	return trim_left( trim_right( str , t) , t );
//} 
#endif // RM_INTERFACE_H

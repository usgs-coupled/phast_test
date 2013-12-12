/*! @file RM_interface.h
	@brief C/Fortran Documentation
*/
#ifndef RM_INTERFACE_H
#define RM_INTERFACE_H
//#include "IPhreeqc.h"
//#include "Var.h"


#if defined(_MSC_VER)
#define FC_FUNC_(name,NAME) NAME
#endif

#if defined(FC_FUNC_)
// Called from Fortran or C++
#define RM_calculate_well_ph               FC_FUNC_ (rm_calculate_well_ph,             RM_CALCULATE_WELL_PH)
#define RM_CloseFiles                      FC_FUNC_ (rm_closefiles,                    RM_CLOSEFILES)    
#define RM_convert_to_molal                FC_FUNC_ (rm_convert_to_molal,              RM_CONVERT_TO_MOLAL)   
#define RM_Create                          FC_FUNC_ (rm_create,                        RM_CREATE)
#define RM_CreateMapping                   FC_FUNC_ (rm_createmapping,                 RM_CREATEMAPPING)
#define RM_Destroy                         FC_FUNC_ (rm_destroy,                       RM_DESTROY)
#define RM_DumpModule                      FC_FUNC_ (rm_dumpmodule,                    RM_DUMPMODULE)
#define RM_Error                           FC_FUNC_ (rm_error,                         RM_ERROR)
#define RM_ErrorMessage                    FC_FUNC_ (rm_errormessage,                  RM_ERRORMESSAGE)
#define RM_FindComponents                  FC_FUNC_ (rm_findcomponents,                RM_FINDCOMPONENTS)
#define RM_GetChemistryCellCount           FC_FUNC_ (rm_getchemistrycellcount,         RM_GETNCHEMISTRYCELLCOUNT)
#define RM_GetComponent                    FC_FUNC_ (rm_getcomponent,                  RM_GETCOMPONENT)
#define RM_GetConcentrations               FC_FUNC_ (rm_getconcentrations,             RM_GETCONCENTRATIONS)
#define RM_GetFilePrefix                   FC_FUNC_ (rm_getfileprefix,                 RM_GETFILEPREFIX)
#define RM_GetMpiMyself                    FC_FUNC_ (rm_getmpimyself,                  RM_GETMPIMYSELF)
#define RM_GetMpiTasks                     FC_FUNC_ (rm_getmpitasks,                   RM_GETMPITASKS)
#define RM_GetGridCellCount                FC_FUNC_ (rm_getgridcellcount,              RM_GETGRIDCELLCOUNT)
#define RM_GetNthSelectedOutputUserNumber  FC_FUNC_ (rm_getnthselectedoutputusernumber, RM_GETNTHSELECTEDOUTPUTUSERNUMBER)
#define RM_GetSelectedOutput               FC_FUNC_ (rm_getselectedoutput,             RM_GETSELECTEDOUTPUT)
#define RM_GetSelectedOutputColumnCount    FC_FUNC_ (rm_getselectedoutputcolumncount,  RM_GETSELECTEDOUTPUTCOLUMNCOUNT)
#define RM_GetSelectedOutputCount          FC_FUNC_ (rm_getselectedoutputcount,        RM_GETSELECTEDOUTPUTCOUNT)
#define RM_GetSelectedOutputHeading        FC_FUNC_ (rm_getselectedoutputheading,      RM_GETSELECTEDOUTPUTHEADING)
#define RM_GetSelectedOutputRowCount       FC_FUNC_ (rm_getselectedoutputrowcount,     RM_GETSELECTEDOUTPUTROWCOUNT)
#define RM_GetTime                         FC_FUNC_ (rm_gettime,                       RM_GETTIME)
#define RM_GetTimeConversion               FC_FUNC_ (rm_gettimeconversion,             RM_GETTIMECONVERSION)
#define RM_GetTimeStep                     FC_FUNC_ (rm_gettimestep,                   RM_GETTIMESTEP)
#define RM_InitialPhreeqc2Concentrations   FC_FUNC_ (rm_initialphreeqc2concentrations, RM_INITIALPHREEQC2CONCENTRATIONS)
#define RM_InitialPhreeqc2Module           FC_FUNC_ (rm_initialphreeqc2module,         RM_INITIALPHREEQC2MODULE)
#define RM_LoadDatabase                    FC_FUNC_ (rm_loaddatabase,                  RM_LOADDATABASE)
#define RM_LogMessage                      FC_FUNC_ (rm_logmessage,                    RM_LOGMESSAGE)
#define RM_LogScreenMessage                FC_FUNC_ (rm_logscreenmessage,              RM_LOGSCREENMESSAGE)
#define RM_OpenFiles                       FC_FUNC_ (rm_openfiles,                     RM_OPENFILES)
#define RM_RunCells                        FC_FUNC_ (rm_runcells,                      RM_RUNCELLS)
#define RM_RunFile                         FC_FUNC_ (rm_runfile,                          RM_RUNFILE)
#define RM_RunString                       FC_FUNC_ (rm_runstring,                     RM_RUNSTRING)
#define RM_ScreenMessage                   FC_FUNC_ (rm_screenmessage,                 RM_SCREENMESSAGE)
#define RM_SetCellVolume				   FC_FUNC_ (rm_setcellvolume,                 RM_SETCELLVOLUME)
#define RM_SetCurrentSelectedOutputUserNumber  FC_FUNC_ (rm_setcurrentselectedoutputusernumber, RM_SETCURRENTSELECTEDOUTPUTUSERNUMBER)
#define RM_SetDensity                      FC_FUNC_ (rm_setdensity,                    RM_SETDENSITY)
#define RM_SetFilePrefix                   FC_FUNC_ (rm_setfileprefix,                 RM_SETFILEPREFIX)
#define RM_SetPartitionUZSolids            FC_FUNC_ (rm_setpartitionuzsolids,          RM_SETPARTITIONUZSOLIDS)
#define RM_SetPrintChemistryOn             FC_FUNC_ (rm_setprintchemistryon,           RM_SETPRINTCHEMISTRYON)
#define RM_SetPrintChemistryMask           FC_FUNC_ (rm_setprintchemistrymask,         RM_SETPRINTCHEMISTRYMASK)
#define RM_SetPressure                     FC_FUNC_ (rm_setpressure,                   RM_SETPRESSURE)
#define RM_SetPoreVolume                   FC_FUNC_ (rm_setporevolume,                 RM_SETPOREVOLUME)
#define RM_SetPoreVolumeZero               FC_FUNC_ (rm_setporevolumezero,             RM_SETPOREVOLUMEZERO)
#define RM_SetRebalance                    FC_FUNC_ (rm_setrebalance,                  RM_SETREBALANCE)
#define RM_SetSaturation                   FC_FUNC_ (rm_setsaturation,                 RM_SETSATURATION)
#define RM_SetSelectedOutputOn             FC_FUNC_ (rm_setselectedoutputon,           RM_SETSELECTEDOUTPUTON)
#define RM_SetTemperature                  FC_FUNC_ (rm_settemperature,                RM_SETTEMPERATURE)
#define RM_SetTimeConversion               FC_FUNC_ (rm_settimeconversion,             RM_SETTIMECONVERSION)
#define RM_SetUnits                        FC_FUNC_ (rm_setunits,                      RM_SETUNITS)
#define RM_write_bc_raw                    FC_FUNC_ (rm_write_bc_raw,                  RM_WRITE_BC_RAW)
#define RM_write_output                    FC_FUNC_ (rm_write_output,                  RM_WRITE_OUTPUT)
#define RM_WarningMessage                  FC_FUNC_ (rm_warningmessage,                RM_WARNINGMESSAGE)
#endif

#if defined(__cplusplus)
extern "C" {
#endif

void       RM_calculate_well_ph(int *id, double *c, double * ph, double * alkalinity);
/**
 *  Closes the output file and log file. 
 *  @see                 @ref RM_OpenFiles
 *  MPI:
 *       Has effect only for root process.
 *  @par Fortran90 Interface:
 *  @htmlonly
 *  <CODE>
 *  <PRE>  
 *      SUBROUTINE RM_CloseFiles()
 *          IMPLICIT NONE
 *      END SUBROUTINE RM_CloseFiles
 *  </PRE>
 *  </CODE>
 *  @endhtmlonly
 */
void       RM_CloseFiles(void);
void       RM_convert_to_molal(int *id, double *c, int *n, int *dim);
/**
 *  Creates a reaction module. 
 *  @param nxyz                   The number of cells in the in the user's model.
 *  @param nthreads               When using OPENMP, the number of worker threads to be used (optional).
 *  @see                 @ref RM_Destroy
 *  MPI:
 *       Called by all processes.
 *       Nthreads has no effect on the MPI version, all processes have one worker thread.
 *  @par Fortran90 Interface:
 *  @htmlonly
 *  <CODE>
 *  <PRE>  
 *       INTEGER FUNCTION RM_Create(nxyz, nthreads) 
 *          IMPLICIT NONE
 *          INTEGER, INTENT(in) :: nxyz
 *          INTEGER, OPTIONAL, INTENT(in) :: nthreads
 *      END FUNCTION RM_Create
 *  </PRE>
 *  </CODE>
 *  @endhtmlonly
 */
int        RM_Create(int *nxyz, int *nthreads = NULL);
/**
 *  Provides a many-to-one mapping of cells in the user's model to cells for which chemistry need to be run. 
 *  @param id                   The instance id returned from @ref RM_Create.
 *  @param grid2chem            An nxyz list of values: Nonnegative is a chemistry cell number, negative is an inactive cell.
 *  @see                        ???
 *  The mapping is used to eliminate inactive cells and to use symmetry to decrease the number of cells for which chemistry must be run.
 *  Default is a one-to-one mapping; all user cells are chemistry cells (equivalent to grid2chem values of 0,1,2,3...).
 *  MPI:
 *     Called by all processes.
 *     All arguments required for root process.
 *     Except for id, arguments are optional for non-root processes. 
 *       
 *  @par Fortran90 Interface:
 *  @htmlonly
 *  <CODE>
 *  <PRE>  
 *      INTEGER FUNCTION RM_CreateMapping(id, grid2chem)
 *          IMPLICIT NONE
 *          INTEGER, INTENT(in) :: id
 *          INTEGER, OPTIONAL :: grid2chem
 *      END FUNCTION RM_CreateMapping
 *  </PRE>
 *  </CODE>
 *  @endhtmlonly
 */
IRM_RESULT RM_CreateMapping (int *id, int *grid2chem = NULL); 
/**
 *  Destroys a reaction module. 
 *  @param id               The instance id returned from @ref RM_Create.
 *  @see                    @ref RM_Destroy
 *  MPI:
 *     Called by all processes.
 *  @par Fortran90 Interface:
 *  @htmlonly
 *  <CODE>
 *  <PRE>          
 *      INTEGER FUNCTION RM_Destroy(id)
 *          IMPLICIT NONE
 *          INTEGER, INTENT(in) :: id
 *      END FUNCTION RM_Destroy
 *  </PRE>
 *  </CODE>
 *  @endhtmlonly
 */
IRM_RESULT RM_Destroy(int *id);
/**
 *  Writes the contents of all workers to file in _RAW formats, including SOLUTIONs and all reactants.
 *  @param id               The instance id returned from @ref RM_Create.
 *  @param dump_on          Signal for writing the dump file: 1 true, 0 false.
 *  @param use_gz           Signal to use gz compression for the dump file: 1 true, 0 false.
 *  File name is prefix.dump or prefix.dump.gz.
 *  @see                    @ref RM_SetFilePrefix
 *  MPI:
 *     Called by all processes.
 *     Id and dump_on required for root process.
 *     Except for id, arguments are optional for non-root processes. 
 *  @par Fortran90 Interface:
 *  @htmlonly
 *  <CODE>
 *  <PRE>         
 *      INTEGER FUNCTION RM_DumpModule(id, dump_on, use_gz) 
 *          IMPLICIT NONE
 *          INTEGER, INTENT(in) :: id
 *          INTEGER, OPTIONAL, INTENT(in) :: dump_on, use_gz
 *      END FUNCTION RM_DumpModule
 *  </PRE>
 *  </CODE>
 *  @endhtmlonly
 */
IRM_RESULT RM_DumpModule(int *id, int *dump_on = NULL, int *use_gz = NULL);
/**
 *  Aborts reaction all modules and stops program execution. 
 *  @param id                   The instance id returned from @ref RM_Create (optional).
 *  @see                 @ref RM_ErrorMessage
 *  @par Fortran90 Interface:
 *  MPI:
 *       Aborts program when called from any process.
 *  @htmlonly
 *  <CODE>
 *  <PRE>    
 *      SUBROUTINE RM_Error(id)
 *          IMPLICIT NONE
 *          INTEGER, OPTIONAL, INTENT(in) :: id
 *      END SUBROUTINE RM_Error
 *  </PRE>
 *  </CODE>
 *  @endhtmlonly
 */
void       RM_Error(const char * err_str = NULL, long l = -1);
/**
 *  Send an error message to the screen, output file, and log file. 
 *  @param str           String to be sent.
 *  @param l             Length of the string buffer (automatic in Fortran, optional in C).
 *  @see                 RM_Error, RM_LogMessage, RM_LogScreenMessage, RM_ScreenMessage, RM_WarningMessage. 
 *  MPI:
 *       Can be called from any process.
 *  @par Fortran90 Interface:
 *  @htmlonly
 *  <CODE>
 *  <PRE>        
 *      SUBROUTINE RM_ErrorMessage(errstr)
 *          IMPLICIT NONE
 *          CHARACTER(*), INTENT(in) :: errstr
 *      END SUBROUTINE RM_ErrorMessage
 *  </PRE>
 *  </CODE>
 *  @endhtmlonly
 */
void       RM_ErrorMessage(const char *err_str, long l = -1);
/**
 *  Returns the number of items in the list of elements included in solutions and reactants in the IPhreeqcPhast workers.
 *  @param id            The instance id returned from @ref RM_Create.
 *  @retval Number of components
 *  @retval If negative, IRM_RESULT error code.
 *  The list is the set of components that need to be transported.
 *  @see                 @ref RM_GetComponent 
 *  MPI:
 *     Called by all processes.
 *  @par Fortran90 Interface:
 *  @htmlonly
 *  <CODE>
 *  <PRE>  
 *      INTEGER FUNCTION RM_FindComponents(id) 
 *          IMPLICIT NONE
 *          INTEGER, INTENT(in) :: id
 *      END FUNCTION RM_FindComponents  
 *  </PRE>
 *  </CODE>
 *  @endhtmlonly
 */
int        RM_FindComponents(int *id);
/**
 *  Returns the number of chemistry cells in the reaction module.
 *  @param id            The instance id returned from @ref RM_Create.
 *  @retval Number of chemistry cells.
 *  @retval If negative, IRM_RESULT error code.
 *  The number of chemistry cells is determined by the mapping defined in RM_CreateMapping.
 *  The number of chemistry cells is less than or equal to the number of cells in the user's model.
 *  @see                 @ref RM_CreateMapping, @ref RM_GetGridCellCount. 
 *  MPI:
 *     Called by all processes.
 *  @par Fortran90 Interface:
 *  @htmlonly
 *  <CODE>
 *  <PRE>  
 *      INTEGER FUNCTION RM_FindComponents(id) 
 *          IMPLICIT NONE
 *          INTEGER, INTENT(in) :: id
 *      END FUNCTION RM_FindComponents  
 *  </PRE>
 *  </CODE>
 *  @endhtmlonly
 */
int        RM_GetChemistryCellCount(int *id);
/**
 *  Retrieves an item from the list of elements included in solutions and reactants in the IPhreeqcPhast workers.
 *  @param id            The instance id returned from @ref RM_Create.
 *  @param num           The number of the component to be retrieved (zero based, less than the result of RM_FindComponents).
 *  @param chem_name     The string value associated with component num.
 *  @retval IRM_OK
 *  @retval If negative, IRM_RESULT error code.
 *  @see                 @ref RM_FindComponents 
 *  MPI:
 *     Called by root.
 *  @par Fortran90 Interface:
 *  @htmlonly
 *  <CODE>
 *  <PRE>  
 *      INTEGER FUNCTION RM_GetComponent(id, num, comp_name)
 *          IMPLICIT NONE
 *          INTEGER, INTENT(in) :: id, num
 *          CHARACTER, INTENT(out) :: comp_name
 *      END FUNCTION RM_GetComponent 
 *  </PRE>
 *  </CODE>
 *  @endhtmlonly
 */
IRM_RESULT RM_GetComponent(int * id, int * num, char *chem_name, int l1 = -1);
IRM_RESULT RM_GetFilePrefix(int *id, char *prefix, long l = -1);
/**
 *  Returns the number of grid cells in the user's model.
 *  @param id            The instance id returned from @ref RM_Create.
 *  @retval Number of grid cells in the user's model.
 *  @retval If negative, IRM_RESULT error code.
 *  The mapping from grid cells to chemistry cells is defined by RM_CreateMapping.
 *  The number of chemistry cells may be less than the number of grid cells if there are inactive regions or symmetry in the model definition.
 *  @see                 @ref RM_CreateMapping, @ref RM_GetChemistryCellCount. 
 *  MPI:
 *     Called by all processes.
 *  @par Fortran90 Interface:
 *  @htmlonly
 *  <CODE>
 *  <PRE>  
 *      INTEGER FUNCTION RM_GetGridCellCount(id)
 *          IMPLICIT NONE
 *          INTEGER, INTENT(in) :: id
 *      END FUNCTION RM_GetGridCellCount
 *  </PRE>
 *  </CODE>
 *  @endhtmlonly
 */
int        RM_GetGridCellCount(int *id);
int        RM_GetMpiMyself(int *id);
int        RM_GetMpiTasks(int *id);
int        RM_GetNthSelectedOutputUserNumber(int *id, int *i);
IRM_RESULT RM_GetSelectedOutput(int *id, double *so = NULL);
int        RM_GetSelectedOutputColumnCount(int *id);
int        RM_GetSelectedOutputCount(int *id);
IRM_RESULT RM_GetSelectedOutputHeading(int *id, int * icol, char * heading, int length);
int        RM_GetSelectedOutputRowCount(int *id);
double     RM_GetTime(int *id);
double     RM_GetTimeConversion(int *id);
double     RM_GetTimeStep(int *id);
/**
 *  Fills an array (c) with concentrations from solutions in the InitialPhreeqc instance, useful for obtaining concentrations for boundary conditions.
 *  @param id                  The instance id returned from @ref RM_Create.
 *  @param c                   Array of concentrations extracted from InitialPhreeqc. The dimension of c is equivalent to Fortran allocation (dim, ncomp), where ncomp is the number of components returned from RM_FindComponents.
 *  @param n_boundary          The number of boundary condition cells that need to be filled.
 *  @param dim                 The maximum number of boundary conditions cells that will fit in the c array.
 *  @param boundary_solution1  Array of solution index numbers (at least n_boundary long).
 *  @param boundary_solution2  Array of solution index numbers that are defined to mix with boundary_solution1 (dimensioned at least n_boundary) (optional).
 *  @param fraction1           Fraction of boundary_solution1 that mixes with (1-fraction1) of boundary_solution2 (dimensioned at least n_boundary) (optional).
 *  @retval IRM_OK
 *  @retval If negative, IRM_RESULT error code.
 *  If boundary_solution2 and fraction1 are omitted, no mixing is used; concentrations are derived from boundary_solution1 only.
 *  A negative value for boundary_solution2 implies no mixing, and the associated value for fraction1 is ignored. 
 *  @see                 @ref RM_FindComponents 
 *  MPI:
 *     Called by the root process.
 *  @par Fortran90 Interface:
 *  @htmlonly
 *  <CODE>
 *  <PRE> 
 *      INTEGER FUNCTION RM_InitialPhreeqc2Concentrations(id, c, n_boundary, dim, bc_sol1, bc_sol2, f1)   
 *              IMPLICIT NONE
 *              INTEGER :: id
 *              DOUBLE PRECISION, INTENT(OUT) :: c
 *              INTEGER, INTENT(IN) :: n_boundary, dim, bc_sol1
 *              INTEGER, INTENT(IN), OPTIONAL :: bc_sol2
 *              DOUBLE PRECISION, INTENT(IN), OPTIONAL :: f1
 *      END FUNCTION RM_InitialPhreeqc2Concentrations    
 *  </PRE>
 *  </CODE>
 *  @endhtmlonly
 */
IRM_RESULT RM_InitialPhreeqc2Concentrations(
                int *id,
                double *c,
                int *n_boundary,
                int *dim, 
                int *boundary_solution1,  
                int *boundary_solution2 = NULL, 
                double *fraction1 = NULL);
/**
 *  Transfer results from the InitialPhreeqc IPhreeqc instance to the reaction module workers. 
 *  @param id                   The instance id returned from @ref RM_Create.
 *  @param initial_conditions1  Array containing index numbers of solutions and reactants.
 *  @param initial_conditions2  Array containing index numbers of solutions and reactants (optional).
 *  @param fraction1            Array containing fraction of initial_condition1 (when initial_conditions2 is defined) (optional).
 *
 *  The structure of the initial condition arrays are 1D equivalent to a Fortran allocation
 *  of (nxyz,7), that is the first nxyz elements are solution numbers. The order of indexes
 *  is as follows (1) SOLUTIONS, (2) EQUILIBRIUM_PHASES, (3) EXCHANGE, (4) SURFACE, (5) GAS_PHASE,
 *  (6) SOLID_SOLUTIONS, and (7) KINETICS.
 *  @see                 @ref RM_InitialPhreeqcRunFile
 *  MPI:
 *     Called by all processes.
 *	   Id and initial_conditions1 are required for the root process.
 *     Except for id, arguments are optional for non-root processes. 
 *  @par Fortran90 Interface:
 *  @htmlonly
 *  <CODE>
 *  <PRE>  
 *      INTEGER FUNCTION RM_InitialPhreeqc2Concentrations(id, c, n_boundary, dim, bc_sol1, bc_sol2, f1)   
 *              IMPLICIT NONE
 *              INTEGER :: id
 *              DOUBLE PRECISION, INTENT(OUT) :: c
 *              INTEGER, INTENT(IN) :: n_boundary, dim, bc_sol1
 *              INTEGER, INTENT(IN), OPTIONAL :: bc_sol2
 *              DOUBLE PRECISION, INTENT(IN), OPTIONAL :: f1
 *      END FUNCTION RM_InitialPhreeqc2Concentrations    
 *  </PRE>
 *  </CODE>
 *  @endhtmlonly
 */
IRM_RESULT RM_InitialPhreeqc2Module(int *id,
                int *initial_conditions1 = NULL,		// 7 x nxyz end-member 1
                int *initial_conditions2 = NULL,		// 7 x nxyz end-member 2
                double *fraction1 = NULL);			    // 7 x nxyz fraction of end-member 1
/**
 *  Load a database for the InitialPhreeqc and all worker IPhreeqc instances. 
 *  @param id            The instance id returned from @ref RM_Create.
 *  @param db_name       String containing the database name.
 *  @param l             Length of the db_name string buffer (automatic in Fortran, optional in C).
 *  @see                 ???
 *  MPI:
 *     Called by all processes.
 *     Except for id, arguments are optional for non-root processes.
 *  @par Fortran90 Interface:
 *  @htmlonly
 *  <CODE>
 *  <PRE>  
 *      INTEGER FUNCTION RM_LoadDatabase(id, db) 
 *          IMPLICIT NONE
 *          INTEGER, INTENT(in) :: id
 *          CHARACTER, OPTIONAL, INTENT(in) :: db
 *      END FUNCTION RM_LoadDatabase 
 *  </PRE>
 *  </CODE>
 *  @endhtmlonly
 */
int        RM_LoadDatabase(int *id, const char *db_name = NULL, long l = -1);
/**
 *  Send a message to the log file. 
 *  @param str           String to be sent.
 *  @param l             Length of the string buffer (automatic in Fortran, optional in C).
 *  @see                 RM_ErrorMessage, RM_LogScreenMessage, RM_ScreenMessage, RM_WarningMessage. 
 *  MPI:
 *     Can be called by any process.
 *  @par Fortran90 Interface:
 *  @htmlonly
 *  <CODE>
 *  <PRE>
 *      SUBROUTINE RM_LogMessage(str) 
 *          IMPLICIT NONE
 *          CHARACTER :: str
 *      END SUBROUTINE RM_LogMessage  
 *  </PRE>
 *  </CODE>
 *  @endhtmlonly
 */
void       RM_LogMessage(const char *str, long l = -1);
/**
 *  Send a message to the screen and the log file. 
 *  @param str           String to be sent.
 *  @param l             Length of the string buffer (automatic in Fortran, optional in C).
 *  @see                 RM_ErrorMessage, RM_LogMessage, RM_ScreenMessage, RM_WarningMessage. 
 *  @par Fortran90 Interface:
 *  @htmlonly
 *  <CODE>
 *  <PRE>
 *      SUBROUTINE RM_LogMessage(str) 
 *          IMPLICIT NONE
 *          CHARACTER :: str
 *      END SUBROUTINE RM_LogMessage  
 *  </PRE>
 *  </CODE>
 *  @endhtmlonly
 */
void       RM_LogScreenMessage(const char *str, long l = -1);
/**
 *  Transfer concentrations from the module workers to the concentration an array of concentrations (c). 
 *  @param id                   The instance id returned from @ref RM_Create.
 *  @param c                    Array containing concentrations with dimensions equivalent to Fortran (nxyz, ncomps), where nxyz is the number of user grid cells and ncomps is the result of RM_FindComponents.
 *  @see                        @ref RM_FindComponents, @ref RM_Concentrations2Module, @ref RM_SetUnits
 *  Units of concentration for c are defined by the solution definition for RM_SetUnits.
 *  MPI:
 *     Called by all processes.
 *	   Id and c are required for the root process.
 *     Except for id, arguments are optional for non-root processes. 
 *  @par Fortran90 Interface:
 *  @htmlonly
 *  <CODE>
 *  <PRE>  
 *      SUBROUTINE RM_GetConcentrations(id, c)   
 *          IMPLICIT NONE
 *          INTEGER :: id
 *          DOUBLE PRECISION, OPTIONAL :: c
 *      END SUBROUTINE RM_GetConcentrations 
 *  </PRE>
 *  </CODE>
 *  @endhtmlonly
 */
void       RM_GetConcentrations(int *id, double *c = NULL);
/**
 *  Opens the output file and log file. 
 *  @see                  @ref RM_SetFilePrefix @ref RM_CloseFiles
 *  Files are named prefix.chem.txt and prefix.log.txt
 *  MPI:
 *       Has effect only for root process.
 *  @par Fortran90 Interface:
 *  @htmlonly
 *  <CODE>
 *  <PRE>        
 *      INTEGER FUNCTION RM_OpenFiles(id) 
 *          IMPLICIT NONE
 *          INTEGER, INTENT(in) :: id
 *      END FUNCTION RM_OpenFiles
 *  </PRE>
 *  </CODE>
 *  @endhtmlonly
 */
IRM_RESULT RM_OpenFiles(int * id);
/**
 *  Transfer array of concentrations to the reaction module workers. 
 *  @param id            The instance id returned from @ref RM_Create.
 *  @param time          The current time of the simulation, in seconds.
 *  @param time_step     The time over which kinetic reactions will be integrated, in seconds.
 *  @param concentration The array of concentrations to be transferred (nxyz of component 1, nxyz component 2, ...)
 *  @param stop_msg      Signal the end of the simulation (1 stop, 0 continue).
 *  @see                 ???
 *  @par Fortran90 Interface:
 *  @htmlonly
 *  <CODE>
 *  <PRE>
 *      SUBROUTINE RM_RunCells(id, time, time_step, c, stop_msg)   
 *          IMPLICIT NONE
 *          INTEGER :: id
 *          DOUBLE PRECISION :: time, time_step, c
 *          INTEGER :: stop_msg
 *      END SUBROUTINE RM_RunCells     
 *  </PRE>
 *  </CODE>
 *  @endhtmlonly
 */
void       RM_RunCells(int *id,
                double *time,					        // time from transport 
                double *time_step,				        // time step from transport
                double *concentration,					// mass fractions nxyz:components
                int * stop_msg);
/**
 *  Send a message to the screen. 
 *  @param str           String to be sent to the screen.
 *  @param l             Length of the string buffer (automatic in Fortran, optional in C).
 *  @see                 RM_ErrorMessage, RM_LogMessage, RM_LogScreenMessage, RM_WarningMessage. 
 *  @par Fortran90 Interface:
 *  @htmlonly
 *  <CODE>
 *  <PRE>
 *      SUBROUTINE RM_ScreenMessage(str) 
 *          IMPLICIT NONE
 *          CHARACTER :: str
 *      END SUBROUTINE RM_ScreenMessage     
 *  </PRE>
 *  </CODE>
 *  @endhtmlonly
 */
/**
 *  Run a PHREEQC file by the InitialPhreeqc (and all worker IPhreeqc instances, currently). 
 *  @param id            The instance id returned from @ref RM_Create.
 *  @param chem_name     String containing the name of the PHREEQC file to run.
 *  @param l             Length of the chem_name string buffer (automatic in Fortran, optional in C).
 *  @see                 ???
 *  MPI:
 *     Called by all processes.
 *     Except for id, arguments are optional for non-root processes.
 *  @par Fortran90 Interface:
 *  @htmlonly
 *  <CODE>
 *  <PRE>  
 *      INTEGER FUNCTION RM_RunFile(id, initial_phreeqc, workers, utility, chem_name)
 *          IMPLICIT NONE
 *          INTEGER, INTENT(in) :: id
 *          INTEGER, OPTIONAL, INTENT(in) :: initial_phreeqc, workers, utility
 *          CHARACTER, OPTIONAL, INTENT(in) :: chem_name
 *      END FUNCTION RM_RunFile 
 *  </PRE>
 *  </CODE>
 *  @endhtmlonly
 */
int        RM_RunFile(int *id, int *initial_phreeqc, int * workers, int *utility, const char *chem_name = NULL, long l = -1);
/**
 *  Run a PHREEQC file by the InitialPhreeqc (and all worker IPhreeqc instances, currently). 
 *  @param id            The instance id returned from @ref RM_Create.
 *  @param chem_name     String containing the name of the PHREEQC file to run.
 *  @param l             Length of the chem_name string buffer (automatic in Fortran, optional in C).
 *  @see                 ???
 *  <H>
 *  MPI:
 *     Called by all processes.
 *     Except for id, arguments are optional for non-root processes.
 *  @par Fortran90 Interface:
 *  @htmlonly
 *  <CODE>
 *  <PRE>  
 *      INTEGER FUNCTION RM_RunFile(id, initial_phreeqc, workers, utility, input_string)
 *          IMPLICIT NONE
 *          INTEGER, INTENT(in) :: id
 *          INTEGER, OPTIONAL, INTENT(in) :: initial_phreeqc, workers, utility
 *          CHARACTER, OPTIONAL, INTENT(in) :: input_string
 *      END FUNCTION RM_RunFile 
 *  </PRE>
 *  </CODE>
 *  @endhtmlonly
 */
int        RM_RunString(int *id, int *initial_phreeqc, int * workers, int *utility, const char * input_string = NULL, long l = -1);
void       RM_ScreenMessage(const char *str, long l = -1);
void       RM_SetCellVolume(int *id, double *t);
int        RM_SetCurrentSelectedOutputUserNumber(int *id, int *i);
void       RM_SetDensity(int *id, double *t);
void       RM_SetDumpModuleOn(int *id, int *dump_on = NULL);
IRM_RESULT RM_SetFilePrefix(int *id, const char *prefix = NULL, long l = -1);
void       RM_SetPartitionUZSolids(int *id, int *t);
void       RM_SetPoreVolume(int *id, double *t);
void       RM_SetPoreVolumeZero(int *id, double *t);
int        RM_SetPrintChemistryOn(int *id, int *print_chem);
void       RM_SetPrintChemistryMask(int *id, int *t);
void       RM_SetPressure(int *id, double *t);
void       RM_SetRebalance(int *id, int *method, double *f);
void       RM_SetSaturation(int *id, double *t);
IRM_RESULT RM_SetSelectedOutputOn(int *id, int *selected_output = NULL);
void       RM_SetTemperature(int *id, double *t);
void       RM_SetTimeConversion(int *id, double *t);
void       RM_SetUnits (int *id, int *sol=NULL, int *pp=NULL, int *ex=NULL, 
                int *surf=NULL, int *gas=NULL, int *ss=NULL, int *kin=NULL);
/**
 *  Send an warning message to the screen and log file. 
 *  @param str           String to be sent.
 *  @param l             Length of the string buffer (automatic in Fortran, optional in C).
 *  @see                 RM_ErrorMessage, RM_LogMessage, RM_LogScreenMessage, RM_ScreenMessage. 
 *  MPI:
 *       Can be called from any process.
 *  @par Fortran90 Interface:
 *  @htmlonly
 *  <CODE>
 *  <PRE>        
 *      SUBROUTINE RM_ErrorMessage(errstr)
 *          IMPLICIT NONE
 *          CHARACTER(*), INTENT(in) :: errstr
 *      END SUBROUTINE RM_ErrorMessage
 *  </PRE>
 *  </CODE>
 *  @endhtmlonly
 */
void       RM_WarningMessage(const char *warn_str, long l = -1);
void       RM_write_bc_raw(int *id, 
                int *solution_list, 
                int * bc_solution_count, 
                int * solution_number, 
                char *prefix, 
                int prefix_l);
void RM_write_output(int *id);

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

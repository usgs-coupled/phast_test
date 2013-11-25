#include "Reaction_module.h"
#include "RM_interface.h"
#include "PHRQ_base.h"
#include "PHRQ_io.h"
#include "IPhreeqc.h"
#include "IPhreeqc.hpp"
#include "IPhreeqcPhast.h"
#include <assert.h>
#include "System.h"
#include "gzstream.h"
#include "KDtree/KDtree.h"
#include "cxxMix.h"
#include "Solution.h"
#include "Exchange.h"
#include "Surface.h"
#include "PPassemblage.h"
#include "SSassemblage.h"
#include "cxxKinetics.h"
#include "GasPhase.h"
#include "CSelectedOutput.hxx"
#include <time.h>
#include "hdf.h"
#ifdef THREADED_PHAST
#include <omp.h>
#endif
#ifdef USE_MPI
#include <mpi.h>
#endif
#include "Phreeqc.h"
Reaction_module::Reaction_module(int *nxyz_arg, int *thread_count, PHRQ_io *io)
	//
	// constructor
	//
: PHRQ_base(io)
{
	int n = 1;
#ifdef THREADED_PHAST
	
#if defined(_WIN32)
	SYSTEM_INFO sysinfo;
	GetSystemInfo( &sysinfo );

	n = sysinfo.dwNumberOfProcessors;
#else
	// Linux, Solaris, Aix, Mac 10.4+
	n = sysconf( _SC_NPROCESSORS_ONLN );
#endif
#ifdef OTHERS
int mib[4];
size_t len = sizeof(numCPU); 

/* set the mib for hw.ncpu */
mib[0] = CTL_HW;
mib[1] = HW_AVAILCPU;  // alternatively, try HW_NCPU;

/* get the number of CPUs from the system */
sysctl(mib, 2, &numCPU, &len, NULL, 0);

if( numCPU < 1 ) 
{
     mib[1] = HW_NCPU;
     sysctl( mib, 2, &numCPU, &len, NULL, 0 );

     if( numCPU < 1 )
     {
          numCPU = 1;
     }
}
#endif
#ifdef SKIP
#ifdef _WIN32
	if (thread_count == 0)
	{
		char *str;
		str = getenv("NUMBER_OF_PROCESSORS");
		n = atoi(str);
	}
#endif
#endif
#endif
	// Determine mpi_myself
	this->mpi_myself = 0;
	this->mpi_tasks = 1;
#ifdef USE_MPI
	if (MPI_Comm_size(MPI_COMM_WORLD, &this->mpi_tasks) != MPI_SUCCESS)
	{
		error_msg("MPI communicator not defined", 1);
	}

	if (MPI_Comm_rank(MPI_COMM_WORLD, &this->mpi_myself) != MPI_SUCCESS)
	{
		error_msg("MPI communicator not defined", 1);
	}
#endif
	if (mpi_myself == 0)
	{
		if (*nxyz_arg == NULL)
		{
			std::ostringstream errstr;
			errstr << "Number of grid cells (nxyz) not defined in creating Reaction_module"; 
			error_msg(errstr.str().c_str(), 1);
		}
		this->nxyz = *nxyz_arg;
	}
#ifdef USE_MPI
	MPI_Bcast(&this->nxyz, 1, MPI_INT, 0, MPI_COMM_WORLD);
	this->nthreads = 1;
#else
	this->nthreads = n;
	if (*thread_count != NULL)
	{
		this->nthreads = (*thread_count > 0) ? *thread_count : n;
	}
#endif

	// last one is to calculate well pH
	for (int i = 0; i <= this->nthreads; i++)
	{
		this->workers.push_back(new IPhreeqcPhast);
	}
	if (this->Get_workers()[0])
	{
		std::map<size_t, Reaction_module*>::value_type instance(this->Get_workers()[0]->Get_Index(), this);
		RM_interface::Instances.insert(instance);
	}
	else
	{
		std::cerr << "Reaction module not created." << std::endl;
		exit(4);
	}


	this->gfw_water = 18.;						// gfw of water
	this->count_chem = 0;
	this->free_surface = false;					// free surface calculation
	this->steady_flow = false;					// steady-state flow calculation
	this->time = 0;							    // scalar time from transport 
	this->time_step = 0;					    // scalar time step from transport
	this->time_conversion = NULL;				// scalar conversion factor for time
	this->rebalance_fraction = 0.5;				// parameter for rebalancing process load for parallel	

	// print flags
	this->print_chem = false;					// print flag for chemistry output file 
	this->selected_output_on = false;			// Create selected output
	this->print_restart = false;				// print flag for writing restart file 
	this->input_units_Solution = 3;				// 1 mg/L, 2 mmol/L, 3 kg/kgs
	this->input_units_PPassemblage = 1;			// water 1, rock 2
	this->input_units_Exchange = 1;			    // water 1, rock 2
	this->input_units_Surface = 1;			    // water 1, rock 2
	this->input_units_GasPhase = 1;			    // water 1, rock 2
	this->input_units_SSassemblage = 1;			// water 1, rock 2
	this->input_units_Kinetics = 1;			    // water 1, rock 2

	this->stop_message = false;

	// initialize arrays
	for (int i = 0; i < this->nxyz; i++)
	{
		forward.push_back(i);
		std::vector<int> temp;
		temp.push_back(i);
		back.push_back(temp);
		x_node.push_back((double) i);
		y_node.push_back(0.0);
		z_node.push_back(0.0);
		saturation.push_back(1.0);
		pv.push_back(0.1);
		pv0.push_back(0.1);
		volume.push_back(1.0);
		print_chem_mask.push_back(0);
		density.push_back(1.0);
		pressure.push_back(1.0);
		tempc.push_back(25.0);
	}
}
Reaction_module::~Reaction_module(void)
{
	std::map<size_t, Reaction_module*>::iterator it = RM_interface::Instances.find(this->Get_workers()[0]->Get_Index());

	for (int i = 0; i <= it->second->Get_nthreads(); i++)
	{
		delete it->second->Get_workers()[i];
	}
	if (it != RM_interface::Instances.end())
	{
		RM_interface::Instances.erase(it);
	}

}

// Reaction_module methods

/* ---------------------------------------------------------------------- */
void
Reaction_module::Calculate_well_ph(double *c, double * pH, double * alkalinity)
/* ---------------------------------------------------------------------- */
{

	// convert mass fraction to moles and store in d
	std::vector<double> d;  
	size_t k;
	for (k = 0; k < this->components.size(); k++)
	{	
		d.push_back(c[k] * 1000.0/gfw[k]);
	}

	// Store in NameDouble
	cxxNameDouble nd;
	for (k = 3; k < components.size(); k++)
	{
		if (d[k] <= 1e-14) d[k] = 0.0;
		nd.add(components[k].c_str(), d[k]);
	}	

	cxxSolution cxxsoln(this->Get_io());	
	cxxsoln.Update(d[0] + 2.0/gfw_water, d[1] + 1.0/gfw_water, d[2], nd);
	cxxStorageBin temp_bin;
	temp_bin.Set_Solution(0, &cxxsoln);

	// Copy all entities numbered 1 into IPhreeqc
	this->Get_workers()[this->nthreads]->Get_PhreeqcPtr()->cxxStorageBin2phreeqc(temp_bin, 0);
	std::string input;
	input.append("RUN_CELLS; -cell 0; SELECTED_OUTPUT; -reset false; -pH; -alkalinity; END");
	this->Get_workers()[0]->RunString(input.c_str());

	VAR pvar;
	this->Get_workers()[this->nthreads]->GetSelectedOutputValue(1,0,&pvar);
	*pH = pvar.dVal;
	this->Get_workers()[this->nthreads]->GetSelectedOutputValue(1,1,&pvar);
	*alkalinity = pvar.dVal;

	// Alternatively
	//*pH = -(this->phast_iphreeqc_worker->Get_PhreeqcPtr()->s_hplus->la);
	//*alkalinity = this->phast_iphreeqc_worker->Get_PhreeqcPtr()->total_alkalinity / 
	//	this->phast_iphreeqc_worker->Get_PhreeqcPtr()->mass_water_aq_x;
	return;
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::Cell_initialize(
					int i, 
					int n_user_new, 
					int *initial_conditions1,
					int *initial_conditions2, 
					double *fraction1,
					int exchange_units, 
					int surface_units, 
					int ssassemblage_units,
					int ppassemblage_units, 
					int gasphase_units, 
					int kinetics_units,
					double porosity_factor,
					std::set<std::string> error_set)
/* ---------------------------------------------------------------------- */
{
	int n_old1, n_old2;
	double f1;

	cxxStorageBin initial_bin;
	/*
	 *   Copy solution
	 */
	n_old1 = initial_conditions1[7 * i];
	n_old2 = initial_conditions2[7 * i];
	if (phreeqc_bin.Get_Solutions().find(n_old1) == phreeqc_bin.Get_Solutions().end())
	{
		std::ostringstream e_stream;
		e_stream << "Initial condition SOLUTION " << n_old1 << " not found.";
		error_set.insert(e_stream.str());
	}
	if (n_old2 > 0 && phreeqc_bin.Get_Solutions().find(n_old2) == phreeqc_bin.Get_Solutions().end())
	{
		std::ostringstream e_stream;
		e_stream << "Initial condition SOLUTION " << n_old2 << " not found.";
		error_set.insert(e_stream.str());
	}
	f1 = fraction1[7 * i];
	if (n_old1 >= 0)
	{
		cxxMix mx;
		mx.Add(n_old1, f1);
		if (n_old2 >= 0)
			mx.Add(n_old2, 1 - f1);
		cxxSolution cxxsoln(phreeqc_bin.Get_Solutions(), mx, n_user_new);
		initial_bin.Set_Solution(n_user_new, &cxxsoln);
	}

	/*
	 *   Copy pp_assemblage
	 */
	n_old1 = initial_conditions1[7 * i + 1];
	n_old2 = initial_conditions2[7 * i + 1];
	if (n_old1 > 0 && phreeqc_bin.Get_PPassemblages().find(n_old1) == phreeqc_bin.Get_PPassemblages().end())
	{
		std::ostringstream e_stream;
		e_stream << "Initial condition EQUILIBRIUM_PHASES " << n_old1 << " not found.";
		error_set.insert(e_stream.str());
	}
	if (n_old2 > 0 && phreeqc_bin.Get_PPassemblages().find(n_old2) == phreeqc_bin.Get_PPassemblages().end())
	{
		std::ostringstream e_stream;
		e_stream << "Initial condition EQUILIBRIUM_PHASES " << n_old2 << " not found.";
		error_set.insert(e_stream.str());
	}
	f1 = fraction1[7 * i + 1];
	if (n_old1 >= 0)
	{
		cxxMix mx;
		mx.Add(n_old1, f1);
		if (n_old2 >= 0)
			mx.Add(n_old2, 1 - f1);
		if (ppassemblage_units == 2)
		{
			mx.Multiply(porosity_factor);
		}
		cxxPPassemblage cxxentity(phreeqc_bin.Get_PPassemblages(), mx,
								  n_user_new);
		initial_bin.Set_PPassemblage(n_user_new, &cxxentity);
	}
	/*
	 *   Copy exchange assemblage
	 */

	n_old1 = initial_conditions1[7 * i + 2];
	n_old2 = initial_conditions2[7 * i + 2];
	if (n_old1 > 0 && phreeqc_bin.Get_Exchangers().find(n_old1) == phreeqc_bin.Get_Exchangers().end())
	{
		std::ostringstream e_stream;
		e_stream << "Initial condition EXCHANGE " << n_old1 << " not found.";
		error_set.insert(e_stream.str());
	}
	if (n_old2 > 0 && phreeqc_bin.Get_Exchangers().find(n_old2) == phreeqc_bin.Get_Exchangers().end())
	{
		std::ostringstream e_stream;
		e_stream << "Initial condition EXCHANGE " << n_old2 << " not found.";
		error_set.insert(e_stream.str());
	}
	f1 = fraction1[7 * i + 2];
	if (n_old1 >= 0)
	{
		cxxMix mx;
		mx.Add(n_old1, f1);
		if (n_old2 >= 0)
			mx.Add(n_old2, 1 - f1);
		if (exchange_units == 2)
		{
			mx.Multiply(porosity_factor);
		}
		cxxExchange cxxexch(phreeqc_bin.Get_Exchangers(), mx, n_user_new);
		initial_bin.Set_Exchange(n_user_new, &cxxexch);
	}
	/*
	 *   Copy surface assemblage
	 */
	n_old1 = initial_conditions1[7 * i + 3];
	n_old2 = initial_conditions2[7 * i + 3];
	if (n_old1 > 0 && phreeqc_bin.Get_Surfaces().find(n_old1) == phreeqc_bin.Get_Surfaces().end())
	{
		std::ostringstream e_stream;
		e_stream << "Initial condition SURFACE " << n_old1 << " not found.";
		error_set.insert(e_stream.str());
	}
	if (n_old2 > 0 && phreeqc_bin.Get_Surfaces().find(n_old2) == phreeqc_bin.Get_Surfaces().end())
	{
		std::ostringstream e_stream;
		e_stream << "Initial condition SURFACE " << n_old2 << " not found.";
		error_set.insert(e_stream.str());
	}
	f1 = fraction1[7 * i + 3];
	if (n_old1 >= 0)
	{
		cxxMix mx;
		mx.Add(n_old1, f1);
		if (n_old2 >= 0)
			mx.Add(n_old2, 1 - f1);
		if (surface_units == 2)
		{
			mx.Multiply(porosity_factor);
		}
		cxxSurface cxxentity(phreeqc_bin.Get_Surfaces(), mx, n_user_new);
		initial_bin.Set_Surface(n_user_new, &cxxentity);
	}
	/*
	 *   Copy gas phase
	 */
	n_old1 = initial_conditions1[7 * i + 4];
	n_old2 = initial_conditions2[7 * i + 4];
	if (n_old1 > 0 && phreeqc_bin.Get_GasPhases().find(n_old1) == phreeqc_bin.Get_GasPhases().end())
	{
		std::ostringstream e_stream;
		e_stream << "Initial condition GAS_PHASE " << n_old1 << " not found.";
		error_set.insert(e_stream.str());
	}
	if (n_old2 > 0 && phreeqc_bin.Get_GasPhases().find(n_old2) == phreeqc_bin.Get_GasPhases().end())
	{
		std::ostringstream e_stream;
		e_stream << "Initial condition GAS_PHASE " << n_old2 << " not found.";
		error_set.insert(e_stream.str());
	}
	f1 = fraction1[7 * i + 4];
	if (n_old1 >= 0)
	{
		cxxMix mx;
		mx.Add(n_old1, f1);
		if (n_old2 >= 0)
			mx.Add(n_old2, 1 - f1);
		if (gasphase_units == 2)
		{
			mx.Multiply(porosity_factor);
		}
		cxxGasPhase cxxentity(phreeqc_bin.Get_GasPhases(), mx, n_user_new);
		initial_bin.Set_GasPhase(n_user_new, &cxxentity);
	}
	/*
	 *   Copy solid solution
	 */
	n_old1 = initial_conditions1[7 * i + 5];
	n_old2 = initial_conditions2[7 * i + 5];
	if (n_old1 > 0 && phreeqc_bin.Get_SSassemblages().find(n_old1) == phreeqc_bin.Get_SSassemblages().end())
	{
		std::ostringstream e_stream;
		e_stream << "Initial condition SOLID_SOLUTIONS " << n_old1 << " not found.";
		error_set.insert(e_stream.str());
	}
	if (n_old2 > 0 && phreeqc_bin.Get_SSassemblages().find(n_old2) == phreeqc_bin.Get_SSassemblages().end())
	{
		std::ostringstream e_stream;
		e_stream << "Initial condition SOLID_SOLUTIONS " << n_old2 << " not found.";
		error_set.insert(e_stream.str());
	}
	f1 = fraction1[7 * i + 5];
	if (n_old1 >= 0)
	{
		cxxMix mx;
		mx.Add(n_old1, f1);
		if (n_old2 >= 0)
			mx.Add(n_old2, 1 - f1);
		if (ssassemblage_units == 2)
		{
			mx.Multiply(porosity_factor);
		}
		cxxSSassemblage cxxentity(phreeqc_bin.Get_SSassemblages(), mx,
								  n_user_new);
		initial_bin.Set_SSassemblage(n_user_new, &cxxentity);
	}
	/*
	 *   Copy kinetics
	 */
	n_old1 = initial_conditions1[7 * i + 6];
	n_old2 = initial_conditions2[7 * i + 6];
	if (n_old1 > 0 && phreeqc_bin.Get_Kinetics().find(n_old1) == phreeqc_bin.Get_Kinetics().end())
	{
		std::ostringstream e_stream;
		e_stream << "Initial condition KINETICS " << n_old1 << " not found.";
		error_set.insert(e_stream.str());
	}
	if (n_old2 > 0 && phreeqc_bin.Get_SSassemblages().find(n_old2) == phreeqc_bin.Get_SSassemblages().end())
	{
		std::ostringstream e_stream;
		e_stream << "Initial condition KINETICS " << n_old2 << " not found.";
		error_set.insert(e_stream.str());
	}
	f1 = fraction1[7 * i + 6];
	if (n_old1 >= 0)
	{
		cxxMix mx;
		mx.Add(n_old1, f1);
		if (n_old2 >= 0)
			mx.Add(n_old2, 1 - f1);
		if (kinetics_units == 2)
		{
			mx.Multiply(porosity_factor);
		}
		cxxKinetics cxxentity(phreeqc_bin.Get_Kinetics(), mx, n_user_new);
		initial_bin.Set_Kinetics(n_user_new, &cxxentity);
	}
	this->Get_workers()[0]->Get_PhreeqcPtr()->cxxStorageBin2phreeqc(initial_bin);
	return;
}
/* ---------------------------------------------------------------------- */
int 
Reaction_module::CheckSelectedOutput()
/* ---------------------------------------------------------------------- */
{
#ifdef USE_MPI
	if (this->mpi_tasks <= 1) return VR_OK;
	
	// check number of selected output
	{
		int nso = (int) this->workers[0]->CSelectedOutputMap.size();
		// Gather number of selected output at root
		std::vector<int> recv_buffer;
		recv_buffer.resize(this->mpi_tasks);
		MPI_Gather(&nso, 1, MPI_INT, recv_buffer.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);
		for (int i = 1; i < this->mpi_tasks; i++)
		{
			if (recv_buffer[i] != recv_buffer[0])
			{
				error_msg("MPI processes have different number of selected output definitions.", STOP);
			}
		}
	}

	// check number of columns
	{
		std::map < int, CSelectedOutput >::iterator it = this->workers[0]->CSelectedOutputMap.begin();
		for ( ; it != this->workers[0]->CSelectedOutputMap.end(); it++)
		{
			int col = (int) it->second.GetColCount();
			// Gather number of columns at root
			std::vector<int> recv_buffer;
			recv_buffer.resize(this->mpi_tasks);
			MPI_Gather(&col, 1, MPI_INT, recv_buffer.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);
			for (int i = 1; i < this->mpi_tasks; i++)
			{
				if (recv_buffer[i] != recv_buffer[0])
				{
					error_msg("MPI processes have different number of selected output columns.", STOP);
				}
			}
		}
	}

	// check headings
	{
		std::map < int, CSelectedOutput >::iterator it = this->workers[0]->CSelectedOutputMap.begin();
		for ( ; it != this->workers[0]->CSelectedOutputMap.end(); it++)
		{
			std::string headings;
			int length;
			// Make string with headings
			int col = (int) it->second.GetColCount();
			for (int i = 0; i < col; i++)
			{
				CVar cvar;
				cvar = it->second.Get(0, i);
				if (cvar.type == TT_STRING)
				{
					headings.append(cvar.sVal);
				}
				else
				{
					error_msg("MPI processes has selected output column that is not a string.", STOP);
				}
			}
			

			if (this->mpi_myself == 0)
			{
				length = (int) headings.size();
			}
			MPI_Bcast(&length,  1, MPI_INT, 0, MPI_COMM_WORLD);

			// Broadcast string
			char *headings_bcast = new char[length + 1];
			if (this->mpi_myself == 0)
			{
				strcpy(headings_bcast, headings.c_str());
			}

			MPI_Bcast(headings_bcast, length + 1, MPI_CHARACTER, 0, MPI_COMM_WORLD);
			
			int equal = strcmp(headings_bcast, headings.c_str()) == 0 ? 1 : 0;

			std::vector<int> recv_buffer;
			recv_buffer.resize(this->mpi_tasks);
			MPI_Gather(&equal, 1, MPI_INT, recv_buffer.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);
			if (mpi_myself == 0)
			{
				for (int i = 1; i < this->mpi_tasks; i++)
				{
					if (recv_buffer[i] == 0)
					{
						error_msg("MPI processes have different column headings.", STOP);
					}
				}
			}
		}
	}
	// Count rows
	{	
		std::map < int, CSelectedOutput >::iterator it = this->workers[0]->CSelectedOutputMap.begin();
		for ( ; it != this->workers[0]->CSelectedOutputMap.end(); it++)
		{
			int rows = (int) it->second.GetRowCount() - 1;
			std::vector<int> recv_buffer;
			recv_buffer.resize(this->mpi_tasks);
			MPI_Gather(&rows, 1, MPI_INT, recv_buffer.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);
			if (this->mpi_myself == 0) 
			{
				int count = 0;
				for (int n = 0; n < this->mpi_tasks; n++)
				{ 
					count += recv_buffer[n];
				}
				if (count != this->count_chem)
				{
					error_msg("Sum of rows is not equal to count_chem.", STOP);
				}
			}
		}
	}
#else
	if (this->nthreads <= 1) return VR_OK;
	
	// check number of selected output
	{
		for (int i = 1; i < this->mpi_tasks; i++)
		{
			if (this->workers[i]->CSelectedOutputMap.size() != this->workers[0]->CSelectedOutputMap.size())
			{
				error_msg("Threads have different number of selected output definitions.", STOP);
			}
		}
	}

	// check number of columns
	{		
		for (int n = 1; n < this->mpi_tasks; n++)
		{
			std::map < int, CSelectedOutput >::iterator root_it = this->workers[0]->CSelectedOutputMap.begin();
			std::map < int, CSelectedOutput >::iterator n_it = this->workers[n]->CSelectedOutputMap.begin();
			for( ; root_it != this->workers[0]->CSelectedOutputMap.end(); root_it++, n_it++)
			{
				if (root_it->second.GetColCount() != n_it->second.GetColCount())
				{
					error_msg("Threads have different number of selected output columns.", STOP);
				}
			}
		}
	}

	// check headings
	{		
		for (int n = 1; n < this->mpi_tasks; n++)
		{
			std::map < int, CSelectedOutput >::iterator root_it = this->workers[0]->CSelectedOutputMap.begin();
			std::map < int, CSelectedOutput >::iterator n_it = this->workers[n]->CSelectedOutputMap.begin();
			for( ; root_it != this->workers[0]->CSelectedOutputMap.end(); root_it++, n_it++)
			{
				for (int i = 0; i < root_it->second.GetColCount(); i++)
				{
					CVar root_cvar;
					root_it->second.Get(0, i, &root_cvar);
					CVar n_cvar;
					n_it->second.Get(0, i, &n_cvar);
					if (root_cvar.type != TT_STRING || n_cvar.type != TT_STRING)
					{
						error_msg("Threads has selected output column that is not a string.", STOP);
					}
					if (strcmp(root_cvar.sVal, n_cvar.sVal) != 0)
					{
						error_msg("Threads have different column headings.", STOP);
					}
				}
			}
		}
	}

	// Count rows
	{			
		for (int nso = 0; nso < (int) this->workers[0]->CSelectedOutputMap.size(); nso++)
		{
			int n_user = this->GetNthSelectedOutputUserNumber(&nso);
			int count = 0;
			for (int n = 0; n < this->nthreads; n++)
			{ 
				std::map < int, CSelectedOutput >::iterator n_it = this->workers[n]->CSelectedOutputMap.find(n_user);
				count += (int) n_it->second.GetRowCount() - 1;
			}
			if (count != this->count_chem)
			{
				error_msg("Sum of rows is not equal to count_chem.", STOP);
			}
		}
	}
#endif
	return VR_OK;
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::Convert_to_molal(double *c, int n, int dim)
/* ---------------------------------------------------------------------- */
{
/*
 *  convert c from mass fraction to moles
 *  The c array is dimensioned c(dim,ns).
 *  n is the number of rows that are used.
 *  In f90 dim = n and is often the number of
 *    cells in the domain.
 */
	int i;
	for (i = 0; i < n; i++)
	{
		double *ptr = &c[i];
		size_t k;
		for (k = 0; k < this->components.size(); k++)
		{	
			ptr[k * dim] *= 1000.0/this->gfw[k];
		}
	}
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::cxxSolution2concentration(cxxSolution * cxxsoln_ptr, std::vector<double> & d)
/* ---------------------------------------------------------------------- */
{
	d.clear();

	// Simplify totals
	{
	  cxxNameDouble nd = cxxsoln_ptr->Get_totals().Simplify_redox();
	  cxxsoln_ptr->Set_totals(nd);
	}

	// convert units
	switch (this->input_units_Solution)
	{
	case 1:  // convert to mg/L 
			d.push_back(cxxsoln_ptr->Get_total_h() * this->gfw[0] * 1000. / cxxsoln_ptr->Get_soln_vol()); 
			d.push_back(cxxsoln_ptr->Get_total_o() * this->gfw[1] * 1000. / cxxsoln_ptr->Get_soln_vol()); 
			d.push_back(cxxsoln_ptr->Get_cb() * this->gfw[2] * 1000. / cxxsoln_ptr->Get_soln_vol()); 
			for (size_t i = 3; i < this->components.size(); i++)
			{
				d.push_back(cxxsoln_ptr->Get_total(components[i].c_str()) * this->gfw[i] / 1000. / cxxsoln_ptr->Get_soln_vol()); 
			}
		break;
	case 2:  // convert to mol/L
		{
			d.push_back(cxxsoln_ptr->Get_total_h() / cxxsoln_ptr->Get_soln_vol()); 
			d.push_back(cxxsoln_ptr->Get_total_o() / cxxsoln_ptr->Get_soln_vol()); 
			d.push_back(cxxsoln_ptr->Get_cb()  / cxxsoln_ptr->Get_soln_vol()); 
			for (size_t i = 3; i < this->components.size(); i++)
			{
				d.push_back(cxxsoln_ptr->Get_total(components[i].c_str())  / cxxsoln_ptr->Get_soln_vol()); 
			}	
		}
		break;
	case 3:  // convert to mass fraction kg/kgs
		{
			double kgs = cxxsoln_ptr->Get_soln_vol() * cxxsoln_ptr->Get_density();
			d.push_back(cxxsoln_ptr->Get_total_h() * this->gfw[0] / 1000. / kgs); 
			d.push_back(cxxsoln_ptr->Get_total_o() * this->gfw[1] / 1000. / kgs); 
			d.push_back(cxxsoln_ptr->Get_cb() * this->gfw[2] / 1000. / kgs); 
			for (size_t i = 3; i < this->components.size(); i++)
			{
				d.push_back(cxxsoln_ptr->Get_total(components[i].c_str()) * this->gfw[i] / 1000. / kgs); 
			}	
		}
		break;
	}
}
/* ---------------------------------------------------------------------- */
IRM_RESULT
Reaction_module::Distribute_initial_conditions_mix(
					int id, 
					int *initial_conditions1,
					int *initial_conditions2, 
					double *fraction1)
/* ---------------------------------------------------------------------- */
{
	/*
	 *      nxyz - number of cells
	 *      initial_conditions1 - Fortran, 7 x nxyz integer array, containing
	 *      entity numbers for
	 *           solution number
	 *           pure_phases number
	 *           exchange number
	 *           surface number
	 *           gas number
	 *           solid solution number
	 *           kinetics number
	 *      initial_conditions2 - Fortran, 7 x nxyz integer array, containing
	 *			 entity numbers
	 *      fraction1 - Fortran 7 x n_cell  double array, fraction for entity 1  
	 *
	 *      Routine mixes solutions, pure_phase assemblages,
	 *      exchangers, surface complexers, gases, solid solution assemblages,
	 *      and kinetics for each cell.
	 *   
	 *      saves results in restart_bin and then the reaction module
	 */
	int i, j;
	IRM_RESULT rtn = IRM_OK;
	/*
	* Make copy of initial conditions for use in restart file
	*/
	for (i = 0; i < nxyz; i++)
	{
		j = 7 * i;
		have_Solution.push_back(initial_conditions1[j]);
		j++;
		have_PPassemblage.push_back(initial_conditions1[j]);
		j++;
		have_Exchange.push_back(initial_conditions1[j]);
		j++;
		have_Surface.push_back(initial_conditions1[j]);
		j++;
		have_GasPhase.push_back(initial_conditions1[j]);
		j++;
		have_SSassemblage.push_back(initial_conditions1[j]);
		j++;
		have_Kinetics.push_back(initial_conditions1[j]);
	}
	/*
	 *  Copy solution, exchange, surface, gas phase, kinetics, solid solution for each active cell.
	 *  Does nothing for indexes less than 0 (i.e. restart files)
	 */

	// calculate cells for each thread or process
	Set_end_cells();

#ifdef USE_MPI
	int begin = this->start_cell[this->mpi_myself];
	int end = this->end_cell[this->mpi_myself] + 1;
	size_t count_negative_porosity = 0;
	std::set<std::string> error_set;
	
	for (int k = begin; k < end; k++)
	{	
		j = k;                          /* j is count_chem number */
		i = this->back[j][0];           /* i is ixyz number */

		assert(forward[i] >= 0);
		assert (volume[i] > 0.0);
		double porosity = pv0[i] / volume[i];
		if (pv0[i] < 0 || volume[i] < 0)
		{
			std::ostringstream errstr;
			errstr << "Negative volume in cell " << i << ": volume, " << volume[i]; 
			errstr << "\t initial volume, " << this->pv0[i] << ".",
			count_negative_porosity++;
			error_msg(errstr.str().c_str());
			rtn = IRM_FAIL;
			continue;
		}
		assert (porosity > 0.0);
		double porosity_factor = (1.0 - porosity) / porosity;
		Cell_initialize(i, j, initial_conditions1, initial_conditions2,
			fraction1,
			this->input_units_Exchange, this->input_units_Surface, this->input_units_SSassemblage,
			this->input_units_PPassemblage, this->input_units_GasPhase, this->input_units_Kinetics,
			porosity_factor,
			error_set);
	}
#else
	size_t count_negative_porosity = 0;
	std::set<std::string> error_set;
	for (i = 0; i < nxyz; i++)
	{							        /* i is ixyz number */
		j = this->forward[i];			/* j is count_chem number */
		if (j < 0)
			continue;
		assert(forward[i] >= 0);
		assert (volume[i] > 0.0);
		double porosity = pv0[i] / volume[i];
		if (pv0[i] < 0 || volume[i] < 0)
		{
			std::ostringstream errstr;
			errstr << "Negative volume in cell " << i << ": volume, " << volume[i]; 
			errstr << "\t initial volume, " << this->pv0[i] << ".",
			count_negative_porosity++;
			error_msg(errstr.str().c_str());
			rtn = IRM_FAIL;
			continue;
		}
		assert (porosity > 0.0);
		double porosity_factor = (1.0 - porosity) / porosity;
		Cell_initialize(i, j, initial_conditions1, initial_conditions2,
			fraction1,
			this->input_units_Exchange, this->input_units_Surface, this->input_units_SSassemblage,
			this->input_units_PPassemblage, this->input_units_GasPhase, this->input_units_Kinetics,
			porosity_factor,
			error_set);
	}
#endif
	if (error_set.size() > 0)
	{
		rtn = IRM_FAIL;
		std::set<std::string>::iterator it = error_set.begin();
		for (; it != error_set.end(); it++)
		{
			error_msg(it->c_str(), 0);
		}
	}
	if (count_negative_porosity > 0)
	{
		rtn = IRM_FAIL;
		std::ostringstream errstr;
		errstr << "Negative initial volumes may be due to initial head distribution.\n"
			"Make initial heads greater than or equal to the elevation of the node for each cell.\n"
			"Increase porosity, decrease specific storage, or use free surface boundary.";
		error_msg(errstr.str().c_str(), 1);
	}
	/*
	 * Read any restart files
	 */
	cxxStorageBin restart_bin;
	for (std::map < std::string, int >::iterator it = FileMap.begin();
		 it != FileMap.end(); it++)
	{
		int
			ifile = -100 - it->second;

		// use gsztream
		igzstream
			myfile;
		myfile.open(it->first.c_str());
		if (!myfile.good())

		{
			rtn = IRM_FAIL;
			std::ostringstream errstr;
			errstr << "File could not be opened: " << it->first.c_str();
			error_msg(errstr.str().c_str());
			break;
		}

		CParser	cparser(myfile, this->Get_io());
		cparser.set_echo_file(CParser::EO_NONE);
		cparser.set_echo_stream(CParser::EO_NONE);

		// skip headers
		while (cparser.check_line("restart", false, true, true, false) ==
			   PHRQ_io::LT_EMPTY);

		// read number of lines of index
		int	n = -1;
		if (!(cparser.get_iss() >> n) || n < 4)
		{
			rtn = IRM_FAIL;
			std::ostringstream errstr;
			errstr << "File does not have node locations: " << it->first.c_str() << "\nPerhaps it is an old format restart file.";
			error_msg(errstr.str().c_str(), 1);
			myfile.close();
			break;
		}

		// points are x, y, z, cell_no
		std::vector < Point > pts;
		// index:
		// 0 solution
		// 1 ppassemblage
		// 2 exchange
		// 3 surface
		// 4 gas phase
		// 5 ss_assemblage
		// 6 kinetics

		std::vector<int> c_index;
		for (i = 0; i < n; i++)
		{
			cparser.check_line("restart", false, false, false, false);
			double
				x,
				y,
				z,
				v;
			cparser.get_iss() >> x;
			cparser.get_iss() >> y;
			cparser.get_iss() >> z;
			cparser.get_iss() >> v;
			pts.push_back(Point(x, y, z, v));

			int dummy;
			// c_index defines entities present for each cell in restart file
			for (j = 0; j < 7; j++)
			{
				cparser.get_iss() >> dummy;
				c_index.push_back(dummy);
			}
		}
		KDtree
		index_tree(pts);

		cxxStorageBin tempBin;
		tempBin.read_raw(cparser);

		for (j = 0; j < count_chem; j++)	/* j is count_chem number */
		{
			i = this->back[j][0];   /* i is nxyz number */
			Point p(this->x_node[i], this->y_node[i], this->z_node[i]);
			int	k = (int) index_tree.Interpolate3d(p);	// k is index number in tempBin

			// solution
			if (initial_conditions1[i * 7] == ifile)
			{
				if (c_index[k * 7] != -1)	// entity k should be defined in tempBin
				{
					if (tempBin.Get_Solution(k) != NULL)
					{
						restart_bin.Set_Solution(j, tempBin.Get_Solution(k));
					}
					else
					{
						assert(false);
						initial_conditions1[7 * i] = -1;
					}
				}
			}

			// PPassemblage
			if (initial_conditions1[i * 7 + 1] == ifile)
			{
				if (c_index[k * 7 + 1] != -1)	// entity k should be defined in tempBin
				{
					if (tempBin.Get_PPassemblage(k) != NULL)
					{
						restart_bin.Set_PPassemblage(j, tempBin.Get_PPassemblage(k));
					}
					else
					{
						assert(false);
						initial_conditions1[7 * i + 1] = -1;
					}
				}
			}

			// Exchange
			if (initial_conditions1[i * 7 + 2] == ifile)
			{
				if (c_index[k * 7 + 2] != -1)	// entity k should be defined in tempBin
				{
					if (tempBin.Get_Exchange(k) != NULL)
					{
						restart_bin.Set_Exchange(j, tempBin.Get_Exchange(k));
					}
					else
					{
						assert(false);
						initial_conditions1[7 * i + 2] = -1;
					}
				}
			}

			// Surface
			if (initial_conditions1[i * 7 + 3] == ifile)
			{
				if (c_index[k * 7 + 3] != -1)	// entity k should be defined in tempBin
				{
					if (tempBin.Get_Surface(k) != NULL)
					{
						restart_bin.Set_Surface(j, tempBin.Get_Surface(k));
					}
					else
					{
						assert(false);
						initial_conditions1[7 * i + 3] = -1;
					}
				}
			}

			// Gas phase
			if (initial_conditions1[i * 7 + 4] == ifile)
			{
				if (c_index[k * 7 + 4] != -1)	// entity k should be defined in tempBin
				{
					if (tempBin.Get_GasPhase(k) != NULL)
					{
						restart_bin.Set_GasPhase(j, tempBin.Get_GasPhase(k));
					}
					else
					{
						assert(false);
						initial_conditions1[7 * i + 4] = -1;
					}
				}
			}

			// Solid solution
			if (initial_conditions1[i * 7 + 5] == ifile)
			{
				if (c_index[k * 7 + 5] != -1)	// entity k should be defined in tempBin
				{
					if (tempBin.Get_SSassemblage(k) != NULL)
					{
						restart_bin.Set_SSassemblage(j, tempBin.Get_SSassemblage(k));
					}
					else
					{
						assert(false);
						initial_conditions1[7 * i + 5] = -1;
					}
				}
			}

			// Kinetics
			if (initial_conditions1[i * 7 + 6] == ifile)
			{
				if (c_index[k * 7 + 6] != -1)	// entity k should be defined in tempBin
				{
					if (tempBin.Get_Kinetics(k) != NULL)
					{
						restart_bin.Set_Kinetics(j, tempBin.Get_Kinetics(k));
					}
					else
					{
						assert(false);
						initial_conditions1[7 * i + 6] = -1;
					}
				}
			}
		}
		myfile.close();
	}

#ifdef USE_MPI	
	for (i = this->start_cell[this->mpi_myself]; i <= this->end_cell[this->mpi_myself]; i++)
	{
		this->Get_workers()[0]->Get_PhreeqcPtr()->cxxStorageBin2phreeqc(restart_bin,i);
	}
#else
	// put restart definitions in reaction module
	this->Get_workers()[0]->Get_PhreeqcPtr()->cxxStorageBin2phreeqc(restart_bin);

	for (int n = 1; n < this->nthreads; n++)
	{
		std::ostringstream delete_command;
		delete_command << "DELETE; -cells\n";
		for (i = this->start_cell[n]; i <= this->end_cell[n]; i++)
		{
			cxxStorageBin sz_bin;
			this->Get_workers()[0]->Get_PhreeqcPtr()->phreeqc2cxxStorageBin(sz_bin, i);
			this->Get_workers()[n]->Get_PhreeqcPtr()->cxxStorageBin2phreeqc(sz_bin, i);
			delete_command << i << "\n";
		}
		if (this->Get_workers()[0]->RunString(delete_command.str().c_str()) > 0) RM_Error(0);
	}
#endif
	// initialize uz
	old_saturation.insert(old_saturation.begin(), nxyz, 1.0);
	return rtn;
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::Error_stop(void)
/* ---------------------------------------------------------------------- */
{
	int n = (int) this->Get_workers()[0]->Get_Index();
	RM_Error(&n);
}
/* ---------------------------------------------------------------------- */
bool
Reaction_module::File_exists(const std::string &name)
/* ---------------------------------------------------------------------- */
{
	FILE *stream;
	if ((stream = fopen(name.c_str(), "r")) == NULL)
	{
		return false;				/* doesn't exist */
	}
	fclose(stream);
	return true;					/* exists */
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::File_rename(const std::string &temp_name, const std::string &name, 
	const std::string &backup_name)
/* ---------------------------------------------------------------------- */
{
	if (this->File_exists(name))
	{
		if (this->File_exists(backup_name.c_str()))
			remove(backup_name.c_str());
		rename(name.c_str(), backup_name.c_str());
	}
	rename(temp_name.c_str(), name.c_str());
}
/* ---------------------------------------------------------------------- */
int
Reaction_module::Find_components(void)	
/* ---------------------------------------------------------------------- */
{
/*
 *   Counts components in any defined solution, gas_phase, exchanger,
 *   surface, or pure_phase_assemblage
 *
 *   Returns 
 *		n_comp, which is total, including H, O, elements, and Charge
 *      names, which contains character strings with names of components
 */
	// Always include H, O, Charge
	this->components.clear();
	this->components.push_back("H");
	this->components.push_back("O");
	this->components.push_back("Charge");

	// Get other components
	IPhreeqcPhast * phast_iphreeqc_worker = this->Get_workers()[0];
	size_t count_components = phast_iphreeqc_worker->GetComponentCount();
	size_t i;
	for (i = 0; i < count_components; i++)
	{
		std::string comp(phast_iphreeqc_worker->GetComponent((int) i));
		assert (comp != "H");
		assert (comp != "O");
		assert (comp != "Charge");
		assert (comp != "charge");

		this->components.push_back(comp);
	}
	// Calculate gfw for components
	for (i = 0; i < components.size(); i++)
	{
		if (components[i] == "Charge")
		{
			this->gfw.push_back(1.0);
		}
		else
		{
			this->gfw.push_back(phast_iphreeqc_worker->Get_gfw(components[i].c_str()));
		}
	}
	if (this->mpi_myself == 0)
	{
		std::ostringstream outstr;
		outstr << "List of Components:\n" << std::endl;
		for (i = 0; i < this->components.size(); i++)
		{
			outstr << "\t" << i + 1 << "\t" << this->components[i].c_str() << std::endl;
		}
		Write_output(outstr.str().c_str());
	}
	return (int) this->components.size();
}
/* ---------------------------------------------------------------------- */
int
Reaction_module::GetNthSelectedOutputUserNumber(int *i)
/* ---------------------------------------------------------------------- */
{
	if (i != NULL && *i >= 0) 
	{
		return this->workers[0]->GetNthSelectedOutputUserNumber(*i);
	}
	return VR_INVALIDARG;
}
/* ---------------------------------------------------------------------- */
int
Reaction_module::GetSelectedOutput(double *so)
/* ---------------------------------------------------------------------- */
{
	int n_user = this->workers[0]->GetCurrentSelectedOutputUserNumber();
#ifdef USE_MPI
	MPI_Bcast(&n_user,  1, MPI_INT, 0, MPI_COMM_WORLD);
	if (n_user >= 0)
	{
		std::map< int, CSelectedOutput >::iterator it = this->workers[0]->CSelectedOutputMap.find(n_user);
		if (it != this->workers[0]->CSelectedOutputMap.end())
		{
			this->SetCurrentSelectedOutputUserNumber(&n_user);
			int ncol = this->GetSelectedOutputColumnCount();
			int local_start_cell = 0;
			std::vector<double> dbuffer;
			dbuffer.resize(this->nxyz*ncol);
			for (int n = 0; n < this->mpi_tasks; n++)
			{
				int nrow;	
				if (this->mpi_myself == n)
				{	
					if (this->mpi_myself == 0) 
					{	
						it->second.Doublize(nrow, ncol, dbuffer.data());
					}
					else
					{
						it->second.Doublize(nrow, ncol, dbuffer.data());
						int length[2];
						length[0] = nrow;
						length[1] = ncol;
						MPI_Send(length, 2, MPI_INT, 0, 0, MPI_COMM_WORLD);
						MPI_Send(dbuffer.data(), nrow*ncol, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
					}

				}
				else if (this->mpi_myself == 0)
				{	
					MPI_Status mpi_status;
					int length[2];
					MPI_Recv(length, 2, MPI_INT, n, 0, MPI_COMM_WORLD, &mpi_status);
					nrow = length[0];
					ncol = length[1];
					MPI_Recv(dbuffer.data(), nrow*ncol, MPI_DOUBLE, n, 0, MPI_COMM_WORLD, &mpi_status);
				}
				if (mpi_myself == 0)
				{
					// Now write data from the process to so
					for (size_t icol = 0; icol < ncol; icol++)
					{
						for (size_t irow = 0; irow < nrow; irow++)
						{
							int ichem = local_start_cell + (int) irow;
							for (size_t k = 0; k < back[ichem].size(); k++)
							{
								int ixyz = back[ichem][k];
								so[icol*this->nxyz + ixyz] = dbuffer[icol*nrow + irow];
							}
						}
					}
					local_start_cell += nrow;
				}
			}
		}
		else
		{
			error_msg("Did not find current selected output in CSelectedOutputMap in  GetSelectedOutput", STOP);
		}
	}
#else

	if (n_user >= 0)
	{
		this->SetCurrentSelectedOutputUserNumber(&n_user);
		int ncol = this->GetSelectedOutputColumnCount();
		std::vector<double> dbuffer;
		dbuffer.resize(this->nxyz*ncol);
		int local_start_cell = 0;
		for (int n = 0; n < this->nthreads; n++)
		{
			int nrow_x, ncol_x;
			std::map< int, CSelectedOutput>::iterator cso_it = this->workers[n]->CSelectedOutputMap.find(n_user);
			if (cso_it != this->workers[n]->CSelectedOutputMap.end())
			{
				cso_it->second.Doublize(nrow_x, ncol_x, dbuffer.data());
				//assert(nrow_x == nrow);
				assert(ncol_x = ncol);

				// Now write data from thread to so
				for (size_t icol = 0; icol < ncol; icol++)
				{
					for (size_t irow = 0; irow < nrow_x; irow++)
					{
						int ichem = local_start_cell + (int) irow;
						for (size_t k = 0; k < back[ichem].size(); k++)
						{
							int ixyz = back[ichem][k];
							so[icol*this->nxyz + ixyz] = dbuffer[icol*nrow_x + irow];
						}
					}
				}
			}
			else
			{
				return VR_INVALIDARG;
			}
			local_start_cell += nrow_x;
		}
		return VR_OK;
	}
#endif
	return VR_INVALIDARG;
}
/* ---------------------------------------------------------------------- */
int
Reaction_module::GetSelectedOutputColumnCount()
/* ---------------------------------------------------------------------- */
{	
	if (this->workers[0]->CurrentSelectedOutputUserNumber >= 0)
	{
		std::map< int, CSelectedOutput >::iterator it = this->workers[0]->CSelectedOutputMap.find(
			this->workers[0]->CurrentSelectedOutputUserNumber);
		if (it != this->workers[0]->CSelectedOutputMap.end())
		{
			return (int) it->second.GetColCount();
		}
	}
	return VR_INVALIDARG;
}
/* ---------------------------------------------------------------------- */
int 
Reaction_module::GetSelectedOutputCount(void)
/* ---------------------------------------------------------------------- */
{	
	return (int) this->workers[0]->CSelectedOutputMap.size();
}
/* ---------------------------------------------------------------------- */
int
Reaction_module::GetSelectedOutputHeading(int *icol, std::string &heading)
/* ---------------------------------------------------------------------- */
{
	if (this->workers[0]->CurrentSelectedOutputUserNumber >= 0)
	{
		std::map< int, CSelectedOutput >::iterator it = this->workers[0]->CSelectedOutputMap.find(
			this->workers[0]->CurrentSelectedOutputUserNumber);
		if (it != this->workers[0]->CSelectedOutputMap.end())
		{
			VAR pVar;
			VarInit(&pVar);
			if (icol != NULL && it->second.Get(0, *icol, &pVar) == VR_OK)
			{
				if (pVar.type == TT_STRING)
				{
					heading = pVar.sVal;
					return VR_OK;
				}
			}
		}
	}
	return VR_INVALIDARG;
}
/* ---------------------------------------------------------------------- */
int
Reaction_module::GetSelectedOutputRowCount()
/* ---------------------------------------------------------------------- */
{
	return this->nxyz;
}
/* ---------------------------------------------------------------------- */
std::string 
Reaction_module::Cptr2TrimString(const char * str, long l)
/* ---------------------------------------------------------------------- */
{
	std::string stdstr;
	if (str)
	{
		if (l >= 0)
		{
			std::string tstr(str, l);
			stdstr = tstr;
		}
		else
		{
			stdstr = str;
		}
	}
	stdstr = trim(stdstr);
	return stdstr;
};
/* ---------------------------------------------------------------------- */
void
Reaction_module::Concentrations2Threads(int n)
/* ---------------------------------------------------------------------- */
{
	// assumes total H, total O, and charge are transported
	int i, j, k;

#ifdef USE_MPI
	int start = this->start_cell[this->mpi_myself];
	int end = this->end_cell[this->mpi_myself];
#else
	int start = this->start_cell[n];
	int end = this->end_cell[n];
#endif

	for (j = start; j <= end; j++)
	{
		std::vector<double> d;  // scratch space to convert from mass fraction to moles
		// j is count_chem number
		i = this->back[j][0];
		if (j < 0) continue;

		switch (this->input_units_Solution)
		{
		case 1:  // mg/L to mol/L
			{
				double *ptr = &this->concentration[i];
				// convert to mol/L
				for (k = 0; k < (int) this->components.size(); k++)
				{	
					d.push_back(ptr[this->nxyz * k] * 1e-3 / this->gfw[k]);
				}	
			}
			break;
		case 2:  // mol/L
			{
				double *ptr = &this->concentration[i];
				// convert to mol/L
				for (k = 0; k < (int) this->components.size(); k++)
				{	
					d.push_back(ptr[this->nxyz * k]);
				}	
			}
			break;
		case 3:  // mass fraction, kg/kg solution to mol/L
			{
				double *ptr = &this->concentration[i];
				// convert to mol/L
				for (k = 0; k < (int) this->components.size(); k++)
				{	
					d.push_back(ptr[this->nxyz * k] * 1000.0 / this->gfw[k] * density[i]);
				}	
			}
			break;
		}

		// convert mol/L to moles per cell
		for (k = 0; k < (int) this->components.size(); k++)
		{	
			d[k] *= this->pv[i] / this->pv0[i]; // * saturation[i];
		}
				
		// update solution 
		cxxNameDouble nd;
		for (k = 3; k < (int) components.size(); k++)
		{
			if (d[k] <= 1e-14) d[k] = 0.0;
			nd.add(components[k].c_str(), d[k]);
		}	

		cxxSolution *soln_ptr = this->Get_workers()[n]->Get_solution(j);
		if (soln_ptr)
		{
			soln_ptr->Update(d[0], d[1], d[2], nd);
			soln_ptr->Set_patm(this->pressure[i]);
			soln_ptr->Set_tc(this->tempc[i]);
		}
	}
	return;
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::Concentrations2Module(void)
/* ---------------------------------------------------------------------- */
{
#ifdef THREADED_PHAST
	omp_set_num_threads(this->nthreads);
	#pragma omp parallel 
	#pragma omp for
#endif
	// For MPI nthreads = 1
	for (int n = 0; n < this->nthreads; n++)
	{
		this->Concentrations2Threads(n);
	}	
}
#ifdef SKIP
/* ---------------------------------------------------------------------- */
void
Reaction_module::Init_uz(void)
/* ---------------------------------------------------------------------- */
{
	int	i;

	if (this->free_surface && !this->steady_flow)
	{
		for (i = 0; i < nxyz; i++)
		{
			old_saturation.push_back(1.0);
		}
	}
}
#endif
/* ---------------------------------------------------------------------- */
void
Reaction_module::Initial_phreeqc_run_thread(int n)
/* ---------------------------------------------------------------------- */
{
		IPhreeqcPhast * iphreeqc_phast_worker = this->Get_workers()[n];
		int ipp_id = (int) iphreeqc_phast_worker->Get_Index();

		iphreeqc_phast_worker->SetOutputFileOn(false);
		iphreeqc_phast_worker->SetErrorFileOn(false);
		iphreeqc_phast_worker->SetLogFileOn(false);
		iphreeqc_phast_worker->SetSelectedOutputStringOn(false);
		if (n == 0)
		{
			iphreeqc_phast_worker->SetSelectedOutputFileOn(true);
			iphreeqc_phast_worker->SetOutputStringOn(true);
		}
		else
		{
			iphreeqc_phast_worker->SetSelectedOutputFileOn(false);
			iphreeqc_phast_worker->SetOutputStringOn(false);
		}

		// Load database
		if (iphreeqc_phast_worker->LoadDatabase(this->database_file_name.c_str()) > 0) RM_Error(&ipp_id);
		if (n == 0)
		{
			Write_output(iphreeqc_phast_worker->GetOutputString());
		}

		// Run chemistry file
		if (iphreeqc_phast_worker->RunFile(this->chemistry_file_name.c_str()) > 0) RM_Error(&ipp_id);

		// Create a StorageBin with initial PHREEQC for boundary conditions
		if (n == 0)
		{
			Write_output(iphreeqc_phast_worker->GetOutputString());
			this->Get_phreeqc_bin().Clear();
			this->Get_workers()[0]->Get_PhreeqcPtr()->phreeqc2cxxStorageBin(this->Get_phreeqc_bin());
		}
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::InitialPhreeqcRun(std::string &database_name, std::string &chemistry_name, std::string &prefix)
/* ---------------------------------------------------------------------- */
{
	/*
	*  Run PHREEQC to obtain PHAST reactants
	*/
	this->database_file_name = database_name;
	this->chemistry_file_name = chemistry_name;
	this->file_prefix = prefix;

	// load database and run chemistry file
	// Eventually need an copy operator for IPhreeqcPhast
#ifdef THREADED_PHAST
	omp_set_num_threads(this->nthreads+1);
	#pragma omp parallel 
	#pragma omp for
#endif
	for (int n = 0; n <= this->nthreads; n++)
	{
		Initial_phreeqc_run_thread(n);
	} 	

}
/* ---------------------------------------------------------------------- */
void
Reaction_module::Partition_uz_thread(int n, int iphrq, int ihst, double new_frac)
/* ---------------------------------------------------------------------- */
{
	int n_user;
	double s1, s2, uz1, uz2;

	/* 
	 * repartition solids for partially saturated cells
	 */

	if ((fabs(this->old_saturation[ihst] - new_frac) > 1e-8) ? true : false)
		return;

	n_user = iphrq;

	if (new_frac >= 1.0)
	{
		/* put everything in saturated zone */
		uz1 = 0;
		uz2 = 0;
		s1 = 1.0;
		s2 = 1.0;
	}
	else if (new_frac <= 1e-10)
	{
		/* put everything in unsaturated zone */
		uz1 = 1.0;
		uz2 = 1.0;
		s1 = 0.0;
		s2 = 0.0;
	}
	else if (new_frac > this->old_saturation[ihst])
	{
		/* wetting cell */
		uz1 = 0.;
		uz2 = (1.0 - new_frac) / (1.0 - this->old_saturation[ihst]);
		s1 = 1.;
		s2 = 1.0 - uz2;
	}
	else
	{
		/* draining cell */
		s1 = new_frac / this->old_saturation[ihst];
		s2 = 0.0;
		uz1 = 1.0 - s1;
		uz2 = 1.0;
	}
	cxxMix szmix, uzmix;
	szmix.Add(0, s1);
	szmix.Add(1, s2);
	uzmix.Add(0, uz1);
	uzmix.Add(1, uz2);
	/*
	 *   Calculate new compositions
	 */

	cxxStorageBin sz_bin;
	IPhreeqcPhast *phast_iphreeqc_worker = this->workers[n];
	phast_iphreeqc_worker->Put_cell_in_storage_bin(sz_bin, n_user);

//Exchange
	if (sz_bin.Get_Exchange(n_user) != NULL)
	{
		cxxStorageBin tempBin;
		tempBin.Set_Exchange(0, sz_bin.Get_Exchange(n_user));
		tempBin.Set_Exchange(1, this->uz_bin.Get_Exchange(n_user));
		cxxExchange newsz(tempBin.Get_Exchangers(), szmix, n_user);
		cxxExchange newuz(tempBin.Get_Exchangers(), uzmix, n_user);
		sz_bin.Set_Exchange(n_user, &newsz);
		this->uz_bin.Set_Exchange(n_user, &newuz);
	}
//PPassemblage
	if (sz_bin.Get_PPassemblage(n_user) != NULL)
	{
		cxxStorageBin tempBin;
		tempBin.Set_PPassemblage(0, sz_bin.Get_PPassemblage(n_user));
		tempBin.Set_PPassemblage(1, this->uz_bin.Get_PPassemblage(n_user));
		cxxPPassemblage newsz(tempBin.Get_PPassemblages(), szmix, n_user);
		cxxPPassemblage newuz(tempBin.Get_PPassemblages(), uzmix, n_user);
		sz_bin.Set_PPassemblage(n_user, &newsz);
		this->uz_bin.Set_PPassemblage(n_user, &newuz);
	}
//Gas_phase
	if (sz_bin.Get_GasPhase(n_user) != NULL)
	{
		cxxStorageBin tempBin;
		tempBin.Set_GasPhase(0, sz_bin.Get_GasPhase(n_user));
		tempBin.Set_GasPhase(1, this->uz_bin.Get_GasPhase(n_user));
		cxxGasPhase newsz(tempBin.Get_GasPhases(), szmix, n_user);
		cxxGasPhase newuz(tempBin.Get_GasPhases(), uzmix, n_user);
		sz_bin.Set_GasPhase(n_user, &newsz);
		this->uz_bin.Set_GasPhase(n_user, &newuz);
	}
//SSassemblage
	if (sz_bin.Get_SSassemblage(n_user) != NULL)
	{
		cxxStorageBin tempBin;
		tempBin.Set_SSassemblage(0, sz_bin.Get_SSassemblage(n_user));
		tempBin.Set_SSassemblage(1, this->uz_bin.Get_SSassemblage(n_user));
		cxxSSassemblage newsz(tempBin.Get_SSassemblages(), szmix, n_user);
		cxxSSassemblage newuz(tempBin.Get_SSassemblages(), uzmix, n_user);
		sz_bin.Set_SSassemblage(n_user, &newsz);
		this->uz_bin.Set_SSassemblage(n_user, &newuz);
	}
//Kinetics
	if (sz_bin.Get_Kinetics(n_user) != NULL)
	{
		cxxStorageBin tempBin;
		tempBin.Set_Kinetics(0, sz_bin.Get_Kinetics(n_user));
		tempBin.Set_Kinetics(1, this->uz_bin.Get_Kinetics(n_user));
		cxxKinetics newsz(tempBin.Get_Kinetics(), szmix, n_user);
		cxxKinetics newuz(tempBin.Get_Kinetics(), uzmix, n_user);
		sz_bin.Set_Kinetics(n_user, &newsz);
		this->uz_bin.Set_Kinetics(n_user, &newuz);
	}
//Surface
	if (sz_bin.Get_Surface(n_user) != NULL)
	{
		cxxStorageBin tempBin;
		tempBin.Set_Surface(0, sz_bin.Get_Surface(n_user));
		tempBin.Set_Surface(1, this->uz_bin.Get_Surface(n_user));
		cxxSurface newsz(tempBin.Get_Surfaces(), szmix, n_user);
		cxxSurface newuz(tempBin.Get_Surfaces(), uzmix, n_user);
		sz_bin.Set_Surface(n_user, &newsz);
		this->uz_bin.Set_Surface(n_user, &newuz);
	}

	// Put back in reaction module
	phast_iphreeqc_worker->Get_cell_from_storage_bin(sz_bin, n_user);

	/*
	 *   Eliminate uz if new fraction 1.0
	 */
	if (new_frac >= 1.0)
	{
		this->uz_bin.Remove(iphrq);
	}

	this->old_saturation[ihst] = new_frac;
}
#ifdef USE_MPI
/* ---------------------------------------------------------------------- */
void
Reaction_module::Rebalance_load_per_cell(void)
/* ---------------------------------------------------------------------- */
{
	if (this->mpi_tasks <= 1) return;
	if (this->mpi_tasks > count_chem) return;
#include <time.h>

	// vectors for each cell (count_chem)
	std::vector<double> recv_cell_times, normalized_cell_times;
	recv_cell_times.resize(this->count_chem);
	
	// vectors for each process (mpi_tasks)
	std::vector<double> standard_time, task_fraction, task_time;
	
	// Assume homogeneous cluster for now
	double tasks_total = 0;
	for (size_t i = 0; i < (size_t) mpi_tasks; i++)
	{
		standard_time.push_back(1.0);   // For heterogeneous cluster, need times for a standard task here
		tasks_total += 1.0 / standard_time[i];
	}

	for (size_t i = 0; i < (size_t) mpi_tasks; i++)
	{
		task_fraction.push_back((1.0 / standard_time[i]) / tasks_total);
	}
	// Collect times
	IPhreeqcPhast * phast_iphreeqc_worker = this->workers[0];
	// manager
	if (mpi_myself == 0)
	{
		recv_cell_times.insert(recv_cell_times.begin(), 
			phast_iphreeqc_worker->Get_cell_clock_times().begin(),
			phast_iphreeqc_worker->Get_cell_clock_times().end());
	}

	// workers
	for (int i = 1; i < mpi_tasks; i++)
	{
		int n = end_cell[i] - start_cell[i] + 1;
		if (mpi_myself == i)
		{
			MPI_Send(phast_iphreeqc_worker->Get_cell_clock_times().data(), n, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
		}
		if (mpi_myself == 0)
		{
			MPI_Status mpi_status;
			MPI_Recv((void *) &recv_cell_times[start_cell[i]], n, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &mpi_status);
		}
	}

	// Root normalizes times, calculates efficiency, rebalances work
	double normalized_total_time = 0;
	double max_task_time = 0;
	// working space
	std::vector<int> start_cell_new;
	std::vector<int> end_cell_new;
	start_cell_new.resize(mpi_tasks, 0);
	end_cell_new.resize(mpi_tasks, 0);

	if (mpi_myself == 0)
	{
		// Normalize times
		max_task_time = 0;
		for (size_t i = 0; i < (size_t) mpi_tasks; i++)
		{		
			double task_sum = 0;
			// normalize cell_times with standard_time
			for (size_t j = (size_t) start_cell[i]; j <= (size_t) end_cell[i]; j++)
			{
				task_sum += recv_cell_times[j];
				normalized_cell_times.push_back(recv_cell_times[j]/standard_time[i]);
				normalized_total_time += normalized_cell_times.back();
			}
			task_time.push_back(task_sum);
			max_task_time = (task_sum > max_task_time) ? task_sum : max_task_time;
		}

		// calculate efficiency
		double efficiency = 0;
		for (size_t i = 0; i < (size_t) mpi_tasks; i++)
		{
			efficiency += task_time[i] / max_task_time * task_fraction[i];
		}
		std::cerr << "          Estimated efficiency of chemistry without communication: " << 
					   (float) (100. * efficiency) << "\n";

		// Split up work
		double f_low, f_high;
		f_high = 1 + 0.5 / ((double) mpi_tasks);
		f_low = 1;
		int j = 0;
		for (size_t i = 0; i < (size_t) mpi_tasks - 1; i++)
		{
			if (i > 0)
			{
				start_cell_new[i] = end_cell_new[i - 1] + 1;
			}
			double sum_work = 0;
			double temp_sum_work = 0;
			bool next = true;
			while (next)
			{
				temp_sum_work += normalized_cell_times[j] / normalized_total_time;
				if ((temp_sum_work < task_fraction[i]) && (((size_t) count_chem - j) > (size_t) (mpi_tasks - i)))
					//(temp_sum_work < f_high * task_fraction[i]) || (sum_work < 0.5 * task_fraction[i])
					//) 
					//&&
					//(count_chem - j) > (mpi_tasks - i)
					//)
				{
					sum_work = temp_sum_work;
					j++;
					next = true;
				}
				else
				{
					if (j == start_cell_new[i])
					{
						end_cell_new[i] = j;
						j++;
					}
					else
					{
						end_cell_new[i] = j - 1;
					}
					next = false;
				}
			}
		}
		assert(j < count_chem);
		assert(mpi_tasks > 1);
		start_cell_new[mpi_tasks - 1] = end_cell_new[mpi_tasks - 2] + 1;
		end_cell_new[mpi_tasks - 1] = count_chem - 1;

		// Apply rebalance fraction
		for (size_t i = 0; i < (size_t) mpi_tasks - 1; i++)
		{
			int	icells;
			icells = (int) (((double) (end_cell_new[i] - end_cell[i])) * (this->rebalance_fraction) );
			if (icells == 0)
			{
				icells = end_cell_new[i] - end_cell[i];
			}
			end_cell_new[i] = end_cell[i] + icells;
			start_cell_new[i + 1] = end_cell_new[i] + 1;
		}
	}
	
	/*
	 *   Broadcast new subcolumns
	 */
	
	MPI_Bcast((void *) start_cell_new.data(), mpi_tasks, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast((void *) end_cell_new.data(), mpi_tasks, MPI_INT, 0, MPI_COMM_WORLD);
	
	/*
	 *   Redefine columns
	 */
	int nnew = 0;
	int old = 0;
	int change = 0;
	std::vector<std::vector<int> > change_list;

	for (int k = 0; k < this->count_chem; k++)
	{
		int i = k;
		//int iphrq = i;			/* iphrq is 1 to count_chem */
		int ihst = this->back[i][0];	/* ihst is 1 to nxyz */
		old_saturation[ihst] = saturation[ihst];    /* update all old_frac */
		while (k > end_cell[old])
		{
			old++;
		}
		while (k > end_cell_new[nnew])
		{
			nnew++;
		}

		if (old == nnew)
			continue;
		change++;

		// Need to send cell from old task to nnew task
		std::vector<int> ints;
		ints.push_back(k);
		ints.push_back(old);
		ints.push_back(nnew);
		change_list.push_back(ints);
	}

	// Transfer cells
	int transfers = 0;
	int count=0;
	if (change_list.size() > 0)
	{
		std::vector<std::vector<int> >::const_iterator it = change_list.begin();
		int pold = (*it)[1];
		int pnew = (*it)[2];
		cxxStorageBin t_bin;
		for (; it != change_list.end(); it++)
		{
			int n = (*it)[0];
			int old = (*it)[1];
			int nnew = (*it)[2];
			if (old != pold || nnew != pnew)
			{
				transfers++;
				this->Transfer_cells(t_bin, pold, pnew);
				t_bin.Clear();
				pold = old;
				pnew = nnew;
				
			}	
			count++;
			// Put cell in t_bin
			if (this->mpi_myself == pold)
			{
				phast_iphreeqc_worker->Get_PhreeqcPtr()->phreeqc2cxxStorageBin(t_bin, n);
				std::ostringstream del;
				del << "DELETE; -cell " << n << "\n";
				phast_iphreeqc_worker->RunString(del.str().c_str());
			}
		}
		// Last transfer
		transfers++;
		this->Transfer_cells(t_bin, pold, pnew);
	}
	
	for (int i = 0; i < this->mpi_tasks; i++)
	{
		start_cell[i] = start_cell_new[i];
		end_cell[i] = end_cell_new[i];
	}

	if (this->mpi_myself == 0)
	{
		std::cerr << "          Cells shifted between processes   " << change << "\n";
		//std::cerr << "          Number of mpi cell transfers      " << transfers << "\n";
	}
	return;
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::Rebalance_load(void)
/* ---------------------------------------------------------------------- */
{
	if (this->mpi_tasks <= 1) return;
	if (this->mpi_tasks > count_chem) return;
	if (this->rebalance_method != 0)
	{
		return Rebalance_load_per_cell();
	}
#include <time.h>

	// working space
	std::vector<int> start_cell_new;
	std::vector<int> end_cell_new;
	for (int i = 0; i < this->mpi_tasks; i++)
	{
		start_cell_new.push_back(0);
		end_cell_new.push_back(0);
	}
	std::vector<int> cells_v;
	std::ostringstream error_stream;
	bool good_enough = false;

	// Calculate time per cell for this process
	IPhreeqcPhast * phast_iphreeqc_worker = this->workers[0];
	int cells = this->end_cell[this->mpi_myself] - this->start_cell[this->mpi_myself] + 1;
	double time_per_cell = phast_iphreeqc_worker->Get_thread_clock_time()/((double) cells);

	// Gather times at root
	std::vector<double> recv_buffer;
	recv_buffer.resize(this->mpi_tasks);
	MPI_Gather(&time_per_cell, 1, MPI_DOUBLE, recv_buffer.data(), 1, MPI_DOUBLE, 0,
			   MPI_COMM_WORLD);
	
	if (this->mpi_myself == 0)
	{
		// Normalize
		double total = 0;
		for (int i = 0; i < this->mpi_tasks; i++)
		{
			assert(recv_buffer[i] > 0);
			total += recv_buffer[0] / recv_buffer[i];
		}

		// Set first and last cells
		double new_n = this->count_chem / total; /* new_n is number of cells for root */


		// Calculate number of cells per process, rounded to lower number
		int	total_cells = 0;
		int n = 0;
		for (int i = 0; i < this->mpi_tasks; i++)
		{
			n = (int) floor(new_n * recv_buffer[0] / recv_buffer[i]);
			if (n < 1)
				n = 1;
			cells_v.push_back(n);
			total_cells += n;
		}

		// Distribute cells from rounding down
		int diff_cells = this->count_chem - total_cells;
		if (diff_cells > 0)
		{
			for (int j = 0; j < diff_cells; j++)
			{
				int min_cell = 0;
				double min_time = (cells_v[0] + 1) * recv_buffer[0];
				for (int i = 1; i < this->mpi_tasks; i++)
				{
					if ((cells_v[i] + 1) * recv_buffer[i] < min_time)
					{
						min_cell = i;
						min_time = (cells_v[i] + 1) * recv_buffer[i];
					}
				}
				cells_v[min_cell] += 1;
			}
		}
		else if (diff_cells < 0)
		{
			for (int j = 0; j < -diff_cells; j++)
			{
				int max_cell = -1;
				double max_time = 0;
				for (int i = 0; i < this->mpi_tasks; i++)
				{
					if (cells_v[i] > 1)
					{
						if ((cells_v[i] - 1) * recv_buffer[i] > max_time)
						{
							max_cell = i;
							max_time = (cells_v[i] - 1) * recv_buffer[i];
						}
					}
				}
				cells_v[max_cell] -= 1;
			}
		}

		// Fill in subcolumn ends
		int last = -1;
		for (int i = 0; i < this->mpi_tasks; i++)
		{
			start_cell_new[i] = last + 1;
			end_cell_new[i] = start_cell_new[i] + cells_v[i] - 1;
			last = end_cell_new[i];
		}

		// Check that all cells are distributed
		if (end_cell_new[this->mpi_tasks - 1] != this->count_chem - 1)
		{
			error_stream << "Failed: " << diff_cells << ", count_cells " << this->count_chem << ", last cell "
				<< end_cell_new[this->mpi_tasks - 1] << "\n";
			for (int i = 0; i < this->mpi_tasks; i++)
			{
				error_stream << i << ": first " << start_cell_new[i] << "\tlast " << end_cell_new[i] << "\n";
			}
			error_stream << "Failed to redistribute cells." << "\n";
			error_msg(error_stream.str().c_str(), STOP);
		}

		// Compare old and new times
		double max_old = 0.0;
		double max_new = 0.0;
		for (int i = 0; i < this->mpi_tasks; i++)
		{
			double t = cells_v[i] * recv_buffer[i];
			if (t > max_new)
				max_new = t;
			t = (end_cell[i] - start_cell[i] + 1) * recv_buffer[i];
			if (t > max_old)
				max_old = t;
		}
		std::cerr << "          Estimated efficiency of chemistry " << (float) ((LDBLE) 100. * max_new / max_old) << "\n";


		if ((max_old - max_new) / max_old < 0.05)
		{
			for (int i = 0; i < this->mpi_tasks; i++)
			{
				start_cell_new[i] = start_cell[i];
				end_cell_new[i] = end_cell[i];
			}
			good_enough = true;
		}
		else
		{
			for (int i = 0; i < this->mpi_tasks - 1; i++)
			{
				int icells = (int) ((end_cell_new[i] - end_cell[i]) * this->rebalance_fraction);
				end_cell_new[i] = end_cell[i] + icells;
				start_cell_new[i + 1] = end_cell_new[i] + 1;
			}
		}
	}

	/*
	 *   Broadcast new subcolumns
	 */
	
	MPI_Bcast((void *) start_cell_new.data(), mpi_tasks, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast((void *) end_cell_new.data(), mpi_tasks, MPI_INT, 0, MPI_COMM_WORLD);
	
	/*
	 *   Redefine columns
	 */
	int nnew = 0;
	int old = 0;
	int change = 0;
	std::vector<std::vector<int> > change_list;

	for (int k = 0; k < this->count_chem; k++)
	{
		int i = k;
		//int iphrq = i;			/* iphrq is 1 to count_chem */
		int ihst = this->back[i][0];	/* ihst is 1 to nxyz */
		old_saturation[ihst] = saturation[ihst];    /* update all old_frac */
		while (k > end_cell[old])
		{
			old++;
		}
		while (k > end_cell_new[nnew])
		{
			nnew++;
		}

		if (old == nnew)
			continue;
		change++;

		// Need to send cell from old task to nnew task
		std::vector<int> ints;
		ints.push_back(k);
		ints.push_back(old);
		ints.push_back(nnew);
		change_list.push_back(ints);
	}

	// Transfer cells
	int transfers = 0;
	int count=0;
	if (change_list.size() > 0)
	{
		std::vector<std::vector<int> >::const_iterator it = change_list.begin();
		int pold = (*it)[1];
		int pnew = (*it)[2];
		cxxStorageBin t_bin;
		for (; it != change_list.end(); it++)
		{
			int n = (*it)[0];
			int old = (*it)[1];
			int nnew = (*it)[2];
			if (old != pold || nnew != pnew)
			{
				transfers++;
				this->Transfer_cells(t_bin, pold, pnew);
				t_bin.Clear();
				pold = old;
				pnew = nnew;
				
			}	
			count++;
			// Put cell in t_bin
			if (this->mpi_myself == pold)
			{
				phast_iphreeqc_worker->Get_PhreeqcPtr()->phreeqc2cxxStorageBin(t_bin, n);
				std::ostringstream del;
				del << "DELETE; -cell " << n << "\n";
				phast_iphreeqc_worker->RunString(del.str().c_str());
			}
		}
		// Last transfer
		transfers++;
		this->Transfer_cells(t_bin, pold, pnew);
	}
	
	for (int i = 0; i < this->mpi_tasks; i++)
	{
		start_cell[i] = start_cell_new[i];
		end_cell[i] = end_cell_new[i];
	}

	if (this->mpi_myself == 0)
	{
		std::cerr << "          Cells shifted between threads     " << change << "\n";
		//std::cerr << "          Number of mpi cell transfers      " << transfers << "\n";
	}
	return;
}
void
Reaction_module::Transfer_cells(cxxStorageBin &t_bin, int old, int nnew)
{
	if (this->mpi_myself == old)
	{
		std::ostringstream raw_stream;
		t_bin.dump_raw(raw_stream, 0);

		int size = (int) raw_stream.str().size();
		MPI_Send(&size, 1, MPI_INT, nnew, 0, MPI_COMM_WORLD);
		MPI_Send((void *) raw_stream.str().c_str(), size, MPI_CHARACTER, nnew, 0, MPI_COMM_WORLD);	
	}
	else if (this->mpi_myself == nnew)
	{	
		MPI_Status mpi_status;
		// Transfer cells		
		int size;
		MPI_Recv(&size, 1, MPI_INT, old, 0, MPI_COMM_WORLD, &mpi_status);
		std::vector<char> raw_buffer;
		raw_buffer.resize(size + 1);
		MPI_Recv((void *) raw_buffer.data(), size, MPI_CHARACTER, old, 0, MPI_COMM_WORLD, &mpi_status);
		raw_buffer[size] = '\0';

		// RunString to enter in module
		IPhreeqcPhast * phast_iphreeqc_worker = this->workers[0];
		phast_iphreeqc_worker->RunString(raw_buffer.data());
	}
}
#else
/* ---------------------------------------------------------------------- */
void
Reaction_module::Rebalance_load(void)
/* ---------------------------------------------------------------------- */
{
	// Threaded version
	if (this->nthreads <= 1) return;
	if (this->nthreads > count_chem) return;
#include <time.h>
	if (this->rebalance_method != 0)
	{
		Rebalance_load_per_cell();
		return; 
	}
	std::vector<int> start_cell_new;
	std::vector<int> end_cell_new;
	for (int i = 0; i < this->nthreads; i++)
	{
		start_cell_new.push_back(0);
		end_cell_new.push_back(0);
	}

	std::vector<int> cells_v;
	bool error = false;
	std::ostringstream error_stream;
	/*
	 *  Gather times of all tasks
	 */
	std::vector<double> recv_buffer;

	double total = 0;
	for (int i = 0; i < this->nthreads; i++)
	{
		IPhreeqcPhast * phast_iphreeqc_worker = this->workers[i];
		int cells = this->end_cell[i] - this->start_cell[i] + 1;
		//std::cerr << "Time: " << i << "  " << phast_iphreeqc_worker->Get_thread_clock_time() << 
		//	"Time per cell: " << phast_iphreeqc_worker->Get_thread_clock_time()/ ((double) cells)  << "\n";
		recv_buffer.push_back(phast_iphreeqc_worker->Get_thread_clock_time()/((double) cells));
		if (recv_buffer.back() <= 0)
		{
			error_stream << "Time for cell " << i << ": " << recv_buffer.back() << "\n";
			recv_buffer.back() = 1;
			//error = true;
			break;
		}
		total += recv_buffer[0] / recv_buffer.back();
		//std::cerr << "Total: " << total << "  " << recv_buffer[0] / recv_buffer.back() << "\n";
	}
	for (int i = 0; i < this->nthreads; i++)
	{

	}
	
	if (error)
	{
		error_msg(error_stream.str().c_str(), STOP);
	}

	/*
	 *  Set first and last cells
	 */
	double new_n = this->count_chem / total; /* new_n is number of cells for root */
	int	total_cells = 0;
	int n = 0;
	/*
	*  Calculate number of cells per process, rounded to lower number
	*/
	for (int i = 0; i < this->nthreads; i++)
	{
		n = (int) floor(new_n * recv_buffer[0] / recv_buffer[i]);
		if (n < 1)
			n = 1;
		cells_v.push_back(n);
		total_cells += n;
	}
	/*
	*  Distribute cells from rounding down
	*/
	int diff_cells = this->count_chem - total_cells;
	if (diff_cells > 0)
	{
		for (int j = 0; j < diff_cells; j++)
		{
			int min_cell = 0;
			double min_time = (cells_v[0] + 1) * recv_buffer[0];
			for (int i = 1; i < this->nthreads; i++)
			{
				if ((cells_v[i] + 1) * recv_buffer[i] < min_time)
				{
					min_cell = i;
					min_time = (cells_v[i] + 1) * recv_buffer[i];
				}
			}
			cells_v[min_cell] += 1;
		}
	}
	else if (diff_cells < 0)
	{
		for (int j = 0; j < -diff_cells; j++)
		{
			int max_cell = -1;
			double max_time = 0;
			for (int i = 0; i < this->nthreads; i++)
			{
				if (cells_v[i] > 1)
				{
					if ((cells_v[i] - 1) * recv_buffer[i] > max_time)
					{
						max_cell = i;
						max_time = (cells_v[i] - 1) * recv_buffer[i];
					}
				}
			}
			cells_v[max_cell] -= 1;
		}
	}
	/*
	*  Fill in subcolumn ends
	*/
	int last = -1;
	for (int i = 0; i < this->nthreads; i++)
	{
		start_cell_new[i] = last + 1;
		end_cell_new[i] = start_cell_new[i] + cells_v[i] - 1;
		last = end_cell_new[i];
	}
	/*
	*  Check that all cells are distributed
	*/
	if (end_cell_new[this->nthreads - 1] != this->count_chem - 1)
	{
		error_stream << "Failed: " << diff_cells << ", count_cells " << this->count_chem << ", last cell "
			<< end_cell_new[this->nthreads - 1] << "\n";
		for (int i = 0; i < this->nthreads; i++)
		{
			error_stream << i << ": first " << start_cell_new[i] << "\tlast " << end_cell_new[i] << "\n";
		}
		error_stream << "Failed to redistribute cells." << "\n";
		error_msg(error_stream.str().c_str(), STOP);
	}
	/*
	*   Compare old and new times
	*/
	double max_old = 0.0;
	double max_new = 0.0;
	for (int i = 0; i < this->nthreads; i++)
	{
		double t = cells_v[i] * recv_buffer[i];
		if (t > max_new)
			max_new = t;
		t = (end_cell[i] - start_cell[i] + 1) * recv_buffer[i];
		if (t > max_old)
			max_old = t;
	}
	std::cerr << "          Estimated efficiency of chemistry " << (float) ((LDBLE) 100. * max_new / max_old) << "\n";


	if ((max_old - max_new) / max_old < 0.05)
	{
		for (int i = 0; i < this->nthreads; i++)
		{
			start_cell_new[i] = start_cell[i];
			end_cell_new[i] = end_cell[i];
		}
	}
	else
	{
		for (int i = 0; i < this->nthreads - 1; i++)
		{
			int icells = (int) ((end_cell_new[i] - end_cell[i]) * this->rebalance_fraction);
			end_cell_new[i] = end_cell[i] + icells;
			start_cell_new[i + 1] = end_cell_new[i] + 1;
		}
	}
	/*
	 *   Redefine columns
	 */
	int nnew = 0;
	int old = 0;
	int change = 0;

	for (int k = 0; k < this->count_chem; k++)
	{
		int i = k;
		int iphrq = i;			/* iphrq is 1 to count_chem */
		//int ihst = this->back[i][0];	/* ihst is 1 to nxyz */
		while (k > end_cell[old])
		{
			old++;
		}
		while (k > end_cell_new[nnew])
		{
			nnew++;
		}

		if (old == nnew)
			continue;
		change++;
		IPhreeqcPhast * old_worker = this->workers[old];
		IPhreeqcPhast * new_worker = this->workers[nnew];
		cxxStorageBin temp_bin; 
		old_worker->Get_PhreeqcPtr()->phreeqc2cxxStorageBin(temp_bin, iphrq);
		new_worker->Get_PhreeqcPtr()->cxxStorageBin2phreeqc(temp_bin, iphrq);
		std::ostringstream del;
		del << "DELETE; -cell " << iphrq << "\n";
		old_worker->RunString(del.str().c_str());
	}

	for (int i = 0; i < this->nthreads; i++)
	{
		start_cell[i] = start_cell_new[i];
		end_cell[i] = end_cell_new[i];
		IPhreeqcPhast * worker = this->workers[i];
		worker->Set_start_cell(start_cell_new[i]);
		worker->Set_end_cell(end_cell_new[i]);
	}
	std::cerr << "          Cells shifted between threads     " << change << "\n";

	return;
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::Rebalance_load_per_cell(void)
/* ---------------------------------------------------------------------- */
{
	// Threaded version
	if (this->nthreads <= 1) return;
	if (this->nthreads > count_chem) return;
#include <time.h>

	// vectors for each cell (count_chem)
	std::vector<double> recv_cell_times, normalized_cell_times;
	
	// vectors for each process (mpi_tasks)
	std::vector<double> standard_time, task_fraction, task_time;
	
	// Assume homogeneous cluster for now
	double tasks_total = 0;
	for (size_t i = 0; i < (size_t) this->nthreads; i++)
	{
		standard_time.push_back(1.0);   // For heterogeneous cluster, need times for a standard task here
		tasks_total += 1.0 / standard_time[i];
	}

	for (size_t i = 0; i < (size_t) this->nthreads; i++)
	{
		task_fraction.push_back((1.0 / standard_time[i]) / tasks_total);
	}


	for (size_t i = 0; i < (size_t) this->nthreads; i++)
	{
		IPhreeqcPhast * phast_iphreeqc_worker = this->workers[i];
		//std::cerr << "Thread: " << i << " " << phast_iphreeqc_worker->Get_cell_clock_times().size() << std::endl;
		std::vector<double>::const_iterator cit;
		for (cit = phast_iphreeqc_worker->Get_cell_clock_times().begin();
			cit != phast_iphreeqc_worker->Get_cell_clock_times().end();
			cit++)
		{
			recv_cell_times.push_back(*cit);
			//std::cerr << "Threadxx: " << i << " " << recv_cell_times.back() << std::endl;
		}
	}

	// Root normalizes times, calculates efficiency, rebalances work
	double normalized_total_time = 0;
	double max_task_time = 0;
	// working space
	std::vector<int> start_cell_new;
	std::vector<int> end_cell_new;
	start_cell_new.resize(this->nthreads, 0);
	end_cell_new.resize(this->nthreads, 0);

	//if (mpi_myself == 0)
	{
		// Normalize times
		max_task_time = 0;
		for (size_t i = 0; i < (size_t) this->nthreads; i++)
		{		
			double task_sum = 0;
			// normalize cell_times with standard_time
			for (size_t j = (size_t) start_cell[i]; j <= (size_t) end_cell[i]; j++)
			{
				task_sum += recv_cell_times[j];
				normalized_cell_times.push_back(recv_cell_times[j]/standard_time[i]);
				normalized_total_time += normalized_cell_times.back();
			}
			task_time.push_back(task_sum);
			max_task_time = (task_sum > max_task_time) ? task_sum : max_task_time;
		}

		// calculate efficiency
		double efficiency = 0;
		for (size_t i = 0; i < (size_t) this->nthreads; i++)
		{
			efficiency += task_time[i] / max_task_time * task_fraction[i];
		}
		std::cerr << "          Estimated efficiency of chemistry without communication: " << 
					   (float) (100. * efficiency) << "\n";;

		// Split up work
		double f_low, f_high;
		f_high = 1 + 0.5 / ((double) this->nthreads);
		f_low = 1;
		int j = 0;
		for (size_t i = 0; i < (size_t) this->nthreads - 1; i++)
		{
			if (i > 0)
			{
				start_cell_new[i] = end_cell_new[i - 1] + 1;
			}
			double sum_work = 0;
			double temp_sum_work = 0;
			bool next = true;
			while (next)
			{
				temp_sum_work += normalized_cell_times[j] / normalized_total_time;
				if ((temp_sum_work < task_fraction[i]) && ((count_chem - (int) j) > (this->nthreads - (int) i)))
				{
					sum_work = temp_sum_work;
					j++;
					next = true;
				}
				else
				{
					if (j == start_cell_new[i])
					{
						end_cell_new[i] = j;
						j++;
					}
					else
					{
						end_cell_new[i] = j - 1;
					}
					next = false;
				}
			}
		}
		assert(j < count_chem);
		assert(this->nthreads > 1);
		start_cell_new[this->nthreads - 1] = end_cell_new[this->nthreads - 2] + 1;
		end_cell_new[this->nthreads - 1] = count_chem - 1;

		// Apply rebalance fraction
		for (size_t i = 0; i < (size_t) this->nthreads - 1; i++)
		{
			int	icells;
			icells = (int) (((double) (end_cell_new[i] - end_cell[i])) * (this->rebalance_fraction) );
			if (icells == 0)
			{
				icells = end_cell_new[i] - end_cell[i];
			}
			end_cell_new[i] = end_cell[i] + icells;
			start_cell_new[i + 1] = end_cell_new[i] + 1;
		}
	}
	
	
	/*
	 *   Redefine columns
	 */
	int nnew = 0;
	int old = 0;
	int change = 0;

	for (int k = 0; k < this->count_chem; k++)
	{
		int i = k;
		int iphrq = i;			/* iphrq is 1 to count_chem */
		//int ihst = this->back[i][0];	/* ihst is 1 to nxyz */
		while (k > end_cell[old])
		{
			old++;
		}
		while (k > end_cell_new[nnew])
		{
			nnew++;
		}

		if (old == nnew)
			continue;
		change++;
		IPhreeqcPhast * old_worker = this->workers[old];
		IPhreeqcPhast * new_worker = this->workers[nnew];
		cxxStorageBin temp_bin; 
		old_worker->Get_PhreeqcPtr()->phreeqc2cxxStorageBin(temp_bin, iphrq);
		new_worker->Get_PhreeqcPtr()->cxxStorageBin2phreeqc(temp_bin, iphrq);
		std::ostringstream del;
		del << "DELETE; -cell " << iphrq << "\n";
		old_worker->RunString(del.str().c_str());
	}

	for (int i = 0; i < this->nthreads; i++)
	{
		start_cell[i] = start_cell_new[i];
		end_cell[i] = end_cell_new[i];
		IPhreeqcPhast * worker = this->workers[i];
		worker->Set_start_cell(start_cell_new[i]);
		worker->Set_end_cell(end_cell_new[i]);
	}
	std::cerr << "          Cells shifted between threads     " << change << "\n";

	return;
}
#endif
#ifdef USE_MPI
/* ---------------------------------------------------------------------- */
void
Reaction_module::RunCells()
/* ---------------------------------------------------------------------- */
{
/*
 *   Routine takes mass fractions from HST, equilibrates each cell,
 *   and returns new mass fractions to HST
 */

/*
 *   Update solution compositions in sz_bin
 */
	
	//clock_t t0 = clock();
	for (int n = 0; n < this->nthreads; n++)
	{
		IPhreeqcPhast * phast_iphreeqc_worker = this->workers[n];
		phast_iphreeqc_worker->Set_out_stream(new ostringstream); 
		phast_iphreeqc_worker->Set_punch_stream(new ostringstream);
	}
#ifdef THREADED_PHAST
	omp_set_num_threads(this->nthreads);
	#pragma omp parallel 
	#pragma omp for
#endif
	for (int n = 0; n < this->nthreads; n++)
	{
		Run_cells_thread(n);
	} 

	std::vector<char> char_buffer;
	std::vector<double> double_buffer;
	for (int n = 0; n < this->mpi_tasks; n++)
	{

		// write output results
		if (this->print_chem)
		{		
			// Need to transfer output stream to root and print
			if (this->mpi_myself == n)
			{
				if (n == 0)
				{
					Write_output(this->workers[0]->Get_out_stream().str().c_str());
					delete &this->workers[0]->Get_out_stream();
				}
				else
				{
					int size = (int) this->workers[0]->Get_out_stream().str().size();
					MPI_Send(&size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
					MPI_Send((void *) this->workers[0]->Get_out_stream().str().c_str(), size, MPI_CHARACTER, 0, 0, MPI_COMM_WORLD);
					delete &this->workers[0]->Get_out_stream();
				}	
			}
			else if (this->mpi_myself == 0)
			{
				MPI_Status mpi_status;
				int size;
				MPI_Recv(&size, 1, MPI_INT, n, 0, MPI_COMM_WORLD, &mpi_status);
				char_buffer.resize(size + 1);
				MPI_Recv((void *) char_buffer.data(), size, MPI_CHARACTER, n, 0, MPI_COMM_WORLD, &mpi_status);
				char_buffer[size] = '\0';
				Write_output(char_buffer.data());
			}
		}
		// write restart
		if (this->print_restart)
		{
			this->Write_restart();
		}
	} 	
	this->CheckSelectedOutput();

	// Rebalance load
	this->Rebalance_load();
	//std::cerr << "Running: " << (double) (clock() - t0) << std::endl;
}
#else
/* ---------------------------------------------------------------------- */
void
Reaction_module::RunCells()
/* ---------------------------------------------------------------------- */
{
/*
 *   Routine takes mass fractions from HST, equilibrates each cell,
 *   and returns new mass fractions to HST
 */

/*
 *   Update solution compositions in sz_bin
 */
	//clock_t t0 = clock();


	for (int n = 0; n < this->nthreads; n++)
	{
		IPhreeqcPhast * phast_iphreeqc_worker = this->workers[n];
		phast_iphreeqc_worker->Set_out_stream(new ostringstream); 
		phast_iphreeqc_worker->Set_punch_stream(new ostringstream);
	}
#ifdef THREADED_PHAST
	omp_set_num_threads(this->nthreads);
	#pragma omp parallel 
	#pragma omp for
#endif
	for (int n = 0; n < this->nthreads; n++)
	{
		Run_cells_thread(n);
	} 
	for (int n = 0; n < this->nthreads; n++)
	{
		// write output results
		if (this->print_chem)
		{
			Write_output(this->workers[n]->Get_out_stream().str().c_str());
		}
		delete &this->workers[n]->Get_out_stream();
		// write restart
		if (this->print_restart)
		{
			this->Write_restart();
		}
	} 	
	this->CheckSelectedOutput();

	// Rebalance load
	this->Rebalance_load();
	//std::cerr << "Running: " << (double) (clock() - t0) << std::endl;
}
#endif
/* ---------------------------------------------------------------------- */
void 
Reaction_module::Run_cells_thread(int n)
/* ---------------------------------------------------------------------- */
{
	/*
	*   Routine takes mass fractions from HST, equilibrates each cell,
	*   and returns new mass fractions to HST
	*/

	/*
	*   Update solution compositions 
	*/
	clock_t t0 = clock();
	//this->Fractions2Solutions_thread(n);

	int i, j;
	IPhreeqcPhast *phast_iphreeqc_worker = this->Get_workers()[n];

	// selected output IPhreeqcPhast
	phast_iphreeqc_worker->CSelectedOutputMap.clear();
	std::vector<int> types;
	std::vector<long> longs;
	std::vector<double> doubles;
	std::string strings;

	// Do not write to files from phreeqc, run_cells writes files
	phast_iphreeqc_worker->SetLogFileOn(false);
	phast_iphreeqc_worker->SetSelectedOutputFileOn(false);
	phast_iphreeqc_worker->SetDumpFileOn(false);
	phast_iphreeqc_worker->SetDumpStringOn(false);
	phast_iphreeqc_worker->SetOutputFileOn(false);
	phast_iphreeqc_worker->SetErrorFileOn(false);
#ifdef USE_MPI
	int start = this->start_cell[this->mpi_myself];
	int end = this->end_cell[this->mpi_myself];
#else
	int start = this->start_cell[n];
	int end = this->end_cell[n];
#endif
	phast_iphreeqc_worker->Get_cell_clock_times().clear();
	for (i = start; i <= end; i++)
	{							/* i is count_chem number */
		j = back[i][0];			/* j is nxyz number */
#ifdef USE_MPI
		phast_iphreeqc_worker->Get_cell_clock_times().push_back(- (double) MPI_Wtime());
#else
		phast_iphreeqc_worker->Get_cell_clock_times().push_back(- (double) clock());
#endif
		// Set local print flags
		bool pr_chem = this->print_chem && (this->print_chem_mask[j] != 0);

		// partition solids between UZ and SZ
		if (this->free_surface && !this->steady_flow)	
		{
			this->Partition_uz_thread(n, i, j, this->saturation[j]);
		}

		// ignore small saturations
		bool active = true;
		if (this->saturation[j] <= 1e-10) 
		{
			this->saturation[j] = 0.0;
			active = false;
		}

		if (active)
		{
			// set cell number, pore volume got Basic functions
			phast_iphreeqc_worker->Set_cell_volumes(i, pv0[j], this->saturation[j], volume[j]);

			// Adjust for fractional saturation and pore volume
			if (this->free_surface && !this->steady_flow)
			{
				this->Scale_solids(n, i, 1.0 / this->saturation[j]);
			}
			
			if (!(this->free_surface && !this->steady_flow) && !steady_flow)
			{
				if (pv0[j] != 0 && pv[j] != 0 && pv0[j] != pv[j])
				{
					cxxSolution * cxxsol = phast_iphreeqc_worker->Get_solution(i);
					cxxsol->multiply(pv[j] / pv0[j]);
				}
			}

			// Set print flags
			phast_iphreeqc_worker->SetOutputStringOn(pr_chem);

			// do the calculation
			std::ostringstream input;
			input << "RUN_CELLS\n";
			input << "  -start_time " << (this->time - this->time_step) << "\n";
			input << "  -time_step  " << this->time_step << "\n";
			input << "  -cells      " << i << "\n";
			input << "END" << "\n";
			if (phast_iphreeqc_worker->RunString(input.str().c_str()) < 0) Error_stop();

			// Adjust for fractional saturation and pore volume
			if (this->free_surface && !this->steady_flow)
				this->Scale_solids(n, i, this->saturation[j]);
			assert(pv0[j] != 0);
			assert(pv[j] != 0);
			if (!(this->free_surface && !this->steady_flow) && !steady_flow)
			{
				if (pv0[j] != 0 && pv[j] != 0 && pv0[j] != pv[j])
				{
					cxxSolution * cxxsol = phast_iphreeqc_worker->Get_solution(i);
					cxxsol->multiply(pv0[j] / pv[j]);
				}
			}
			// Write output file
			if (pr_chem)
			{
				char line_buff[132];
				sprintf(line_buff, "Time %g. Cell %d: x=%15g\ty=%15g\tz=%15g\n",
					(this->time) * (this->time_conversion), j + 1, x_node[j],  y_node[j],
					z_node[j]);
				phast_iphreeqc_worker->Get_out_stream() << line_buff;
				phast_iphreeqc_worker->Get_out_stream() << phast_iphreeqc_worker->GetOutputString();
			}

			// Write hdf file
			if (this->selected_output_on)
			{

				// Add selected output values to IPhreeqcPhast CSelectedOutputMap's
				std::map< int, CSelectedOutput* >::iterator it = phast_iphreeqc_worker->SelectedOutputMap.begin();
				for ( ; it != phast_iphreeqc_worker->SelectedOutputMap.end(); it++)
				{
					int n_user = it->first;
					std::map< int, CSelectedOutput >::iterator ipp_it = phast_iphreeqc_worker->CSelectedOutputMap.find(n_user);
					if (ipp_it == phast_iphreeqc_worker->CSelectedOutputMap.end())
					{
						CSelectedOutput cso;
						phast_iphreeqc_worker->CSelectedOutputMap[n_user] = cso;
						ipp_it = phast_iphreeqc_worker->CSelectedOutputMap.find(n_user);
					}
					types.clear();
					longs.clear();
					doubles.clear();
					strings.clear();
					it->second->Serialize(types, longs, doubles, strings);
					ipp_it->second.DeSerialize(types, longs, doubles, strings);
				}
			}
		} // end active
		else
		{
			if (pr_chem)
			{
				std::ostringstream line;
				line << "Time " << (this->time) * (this->time_conversion);
				line << ". Cell " << j + 1 << ": ";
				line << "x= " << x_node[j] << "\t";
				line << "y= " << y_node[j] << "\t";
				line << "z= " << z_node[j] << "\n";
				line << "Cell is dry.\n";
				phast_iphreeqc_worker->Get_out_stream() << line.str().c_str();
			}
			// Write hdf file
			if (this->selected_output_on)
			{	
				// Add selected output values to IPhreeqcPhast CSelectedOutputMap's
				int columns = phast_iphreeqc_worker->GetSelectedOutputColumnCount();
				std::map< int, CSelectedOutput* >::iterator it = phast_iphreeqc_worker->SelectedOutputMap.begin();
				for ( ; it != phast_iphreeqc_worker->SelectedOutputMap.end(); it++)
				{
					int iso = it->first;
					std::map< int, CSelectedOutput >::iterator ipp_it = phast_iphreeqc_worker->CSelectedOutputMap.find(iso);
					if (ipp_it == phast_iphreeqc_worker->CSelectedOutputMap.end())
					{
						CSelectedOutput cso;
						phast_iphreeqc_worker->CSelectedOutputMap[iso] = cso;
						ipp_it = phast_iphreeqc_worker->CSelectedOutputMap.find(iso);
						for (int i = 0; i < columns; i++)
						{
							VAR pvar, pvar1;
							VarInit(&pvar);
							VarInit(&pvar1);
							phast_iphreeqc_worker->GetSelectedOutputValue(0, i, &pvar);
							ipp_it->second.PushBack(pvar.sVal, pvar1);
						}
					}
					ipp_it->second.EndRow();
				}
			}
		}
#ifdef USE_MPI
		phast_iphreeqc_worker->Get_cell_clock_times().back() += (double) MPI_Wtime();
#else
		phast_iphreeqc_worker->Get_cell_clock_times().back() += (double) clock();
#endif

	} // end one cell
#ifndef USE_MPI
//	this->Solutions2Fractions_thread(n);
#endif
	clock_t t_elapsed = clock() - t0;

#ifdef USE_MPI
	//std::cerr << "          Process: " << this->mpi_myself << " Time: " << (double) t_elapsed << " Cells: " << this->end_cell[this->mpi_myself] - this->start_cell[this->mpi_myself] + 1 << std::endl;
#else
	//std::cerr << "          Thread: " << n << " Time: " << (double) t_elapsed << " Cells: " << this->end_cell[n] - this->start_cell[n] + 1 << "\n";
#endif
	phast_iphreeqc_worker->Set_thread_clock_time((double) t_elapsed);
	
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::Scale_solids(int n, int iphrq, LDBLE frac)
/* ---------------------------------------------------------------------- */
{
	int n_user;

	/* 
	 * repartition solids for partially saturated cells
	 */

	//if (equal(old_frac[ihst], new_frac, 1e-8) == TRUE)  return(OK);

	n_user = iphrq;
	cxxMix cxxmix;
	cxxmix.Add(n_user, frac);
	/*
	 *   Scale compositions
	 */
	cxxStorageBin sz_bin;
	IPhreeqcPhast *phast_iphreeqc_worker = this->workers[n];
	phast_iphreeqc_worker->Put_cell_in_storage_bin(sz_bin, n_user);
	if (sz_bin.Get_Exchange(n_user) != NULL)
	{
		cxxExchange cxxentity(sz_bin.Get_Exchangers(), cxxmix, n_user);
		sz_bin.Set_Exchange(n_user, &cxxentity);
	}
	if (sz_bin.Get_PPassemblage(n_user) != NULL)
	{
		cxxPPassemblage cxxentity(sz_bin.Get_PPassemblages(), cxxmix, n_user);
		sz_bin.Set_PPassemblage(n_user, &cxxentity);
	}
	if (sz_bin.Get_GasPhase(n_user) != NULL)
	{
		cxxGasPhase cxxentity(sz_bin.Get_GasPhases(), cxxmix, n_user);
		sz_bin.Set_GasPhase(n_user, &cxxentity);
	}
	if (sz_bin.Get_SSassemblage(n_user) != NULL)
	{
		cxxSSassemblage cxxentity(sz_bin.Get_SSassemblages(), cxxmix, n_user);
		sz_bin.Set_SSassemblage(n_user, &cxxentity);
	}
	if (sz_bin.Get_Kinetics(n_user) != NULL)
	{
		cxxKinetics cxxentity(sz_bin.Get_Kinetics(), cxxmix, n_user);
		sz_bin.Set_Kinetics(n_user, &cxxentity);
	}
	if (sz_bin.Get_Surface(n_user) != NULL)
	{
		cxxSurface cxxentity(sz_bin.Get_Surfaces(), cxxmix, n_user);
		sz_bin.Set_Surface(n_user, &cxxentity);
	}
	phast_iphreeqc_worker->Get_cell_from_storage_bin(sz_bin, n_user);
	return;
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::Send_restart_name(std::string &name)
/* ---------------------------------------------------------------------- */
{
	if (mpi_myself == 0)
	{
		int	i = (int) this->FileMap.size();
		this->FileMap[name] = i;
	}
#ifdef USE_MPI
	if (mpi_myself == 0)
	{
		int n = (int) name.size();
		MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast((void *) name.c_str(), (int) name.size(), MPI_CHARACTER, 0, MPI_COMM_WORLD);
	}
	else
	{
		int n;
		MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
		std::string str;
		str.reserve(n);
		MPI_Bcast((void *) str.c_str(), n, MPI_CHARACTER, 0, MPI_COMM_WORLD);
		int	i = (int) this->FileMap.size();
		this->FileMap[str] = i;	
	}
#endif
}
/* ---------------------------------------------------------------------- */
int 
Reaction_module::SetCurrentSelectedOutputUserNumber(int *i)
{
	if (i != NULL && *i >= 0)
	{
		return this->workers[0]->SetCurrentSelectedOutputUserNumber(*i);
	}
	return VR_INVALIDARG;
}

/* ---------------------------------------------------------------------- */
IRM_RESULT
Reaction_module::SetFilePrefix(const char * prefix, long l)
/* ---------------------------------------------------------------------- */
{
	if (this->mpi_myself == 0)
	{	
		this->file_prefix = Cptr2TrimString(prefix, l);
	}
#ifdef USE_MPI
	int l1 = 0;
	if (mpi_myself == 0)
	{
		l1 = (int) this->file_prefix.size();
	}
	MPI_Bcast(&l1, 1, MPI_INT, 0, MPI_COMM_WORLD);
	this->file_prefix.resize(l1);
	MPI_Bcast((void *) this->file_prefix.c_str(), l1, MPI_CHARACTER, 0, MPI_COMM_WORLD);
#endif
	if (this->file_prefix.size() > 0)
	{
		return IRM_OK;
	}
	return IRM_INVALIDARG;
}
/* ---------------------------------------------------------------------- */
IRM_RESULT
Reaction_module::SetFilePrefix(std::string &prefix)
/* ---------------------------------------------------------------------- */
{
	this->file_prefix.clear();
	if (mpi_myself == 0)
	{
		this->file_prefix = trim(prefix);
	}
#ifdef USE_MPI
	int l = 0;
	if (mpi_myself == 0)
	{
		l = (int) prefix.size();
	}
	MPI_Bcast(&l, 1, MPI_INT, 0, MPI_COMM_WORLD);
	this->file_prefix.resize(l);
	MPI_Bcast((void *) this->file_prefix.c_str(), l, MPI_CHARACTER, 0, MPI_COMM_WORLD);
#endif
	return IRM_OK;
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::SetConcentration(double *t)
/* ---------------------------------------------------------------------- */
{
	size_t ncomps = this->components.size();
	if (this->concentration.size() < this->nxyz)
	{
		this->concentration.resize((size_t) (this->nxyz * ncomps), 0.0);	
	}
	if (mpi_myself == 0)
	{
		if (t == NULL) error_msg("NULL pointer in Set_concentration", 1);
		memcpy(this->concentration.data(), t, (size_t) (this->nxyz * ncomps * sizeof(double)));
	}
#ifdef USE_MPI
	MPI_Bcast(this->concentration.data(), this->nxyz * (int) ncomps, MPI_DOUBLE, 0, MPI_COMM_WORLD);
#endif
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::SetDensity(double *t)
/* ---------------------------------------------------------------------- */
{
	if (this->density.size() < this->nxyz)
	{
		this->density.resize((size_t) (this->nxyz), 0.0);
	}
	if (mpi_myself == 0)
	{
		if (t == NULL) error_msg("NULL pointer in Set_density", 1);
		memcpy(this->density.data(), t, (size_t) (this->nxyz * sizeof(double)));
	}
#ifdef USE_MPI
	MPI_Bcast(this->density.data(), this->nxyz, MPI_DOUBLE, 0, MPI_COMM_WORLD);
#endif
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::Set_end_cells(void)
/* ---------------------------------------------------------------------- */
{
#ifdef USE_MPI
	int ntasks = this->mpi_tasks;
#else
	int ntasks = this->nthreads;
#endif
	int n = this->count_chem / ntasks;
	int extra = this->count_chem - n*ntasks;
	std::vector<int> cells;
	for (int i = 0; i < extra; i++)
	{
		cells.push_back(n+1);
	}
	for (int i = extra; i < ntasks; i++)
	{
		cells.push_back(n);
	}
	int cell0 = 0;
	for (int i = 0; i < ntasks; i++)
	{
		this->start_cell.push_back(cell0);
		this->end_cell.push_back(cell0 + cells[i] - 1);
		cell0 = cell0 + cells[i];
	}
}
/* ---------------------------------------------------------------------- */
void 
Reaction_module::Set_free_surface(int * t)
/* ---------------------------------------------------------------------- */
{
	if (mpi_myself == 0)
	{
		if (t == NULL) error_msg("NULL pointer in Set_free_surface", 1);
		this->free_surface = (*t != 0);
	}
#ifdef USE_MPI
	MPI_Bcast(&this->free_surface, 1, MPI_LOGICAL, 0, MPI_COMM_WORLD);
#endif
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::Set_input_units(int *sol, int *pp, int *ex, int *surf, int *gas, int *ss, int *kin)
/* ---------------------------------------------------------------------- */
{
	int local_sol, local_pp, local_ex, local_surf, local_gas, local_ss, local_kin;
	if (mpi_myself == 0)
	{
		local_sol  = (sol  != NULL) ? *sol  : -1;
		local_pp   = (pp   != NULL) ? *pp   : -1;
		local_ex   = (ex   != NULL) ? *ex   : -1;
		local_surf = (surf != NULL) ? *surf : -1;
		local_gas  = (gas  != NULL) ? *gas  : -1;
		local_ss   = (ss   != NULL) ? *ss   : -1;
		local_kin  = (kin  != NULL) ? *kin  : -1;
	}
#ifdef USE_MPI
	MPI_Bcast(&local_sol,  1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&local_pp,   1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&local_ex,   1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&local_surf, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&local_gas,  1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&local_ss,   1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&local_kin,  1, MPI_INT, 0, MPI_COMM_WORLD);
#endif

	if (local_sol >= 0)
	{
		Set_input_units_Solution(local_sol);
	}
	if (local_pp >= 0)
	{
		Set_input_units_PPassemblage(local_pp);
	}
	if (local_ex >= 0)
	{
		Set_input_units_Exchange(local_ex);
	}
	if (local_surf >= 0)
	{
		Set_input_units_Surface(local_surf);
	}	
	if (local_gas >= 0)
	{
		Set_input_units_GasPhase(local_gas);
	}
	if (local_ss >= 0)
	{
		Set_input_units_SSassemblage(local_ss);
	}
	if (local_kin >= 0)
	{
		Set_input_units_Kinetics(local_kin);
	}
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::Set_mapping(int *t)
/* ---------------------------------------------------------------------- */
{
	std::vector<int> grid2chem;
	grid2chem.resize(this->nxyz);
	if (mpi_myself == 0)
	{
		if (t == NULL) error_msg("NULL pointer in Set_mapping", 1);
		memcpy(grid2chem.data(), t, (size_t) (this->nxyz * sizeof(int)));
	}
#ifdef USE_MPI
	MPI_Bcast(grid2chem.data(), this->nxyz, MPI_INT, 0, MPI_COMM_WORLD);
#endif

	back.clear();
	forward.clear();

	// find count_chem
	this->count_chem = 0;
	for (int i = 0; i < this->nxyz; i++)
	{
		if (grid2chem[i] > count_chem)
		{
			count_chem = grid2chem[i];
		}
	}
	count_chem ++; 

	for (int i = 0; i < count_chem; i++)
	{
		std::vector<int> temp;
		back.push_back(temp);
	}
	for (int i = 0; i < this->nxyz; i++)
	{
		int n = grid2chem[i];
		if (n >= count_chem)
		{
			error_msg("Error in cell out of range in mapping (grid to chem).", STOP);
		}

		// copy to forward
		forward.push_back(n);

		// add to back
		if (n >= 0) 
		{
			back[n].push_back(i);
		}
	}
	
	// set -1 for back items > 0
	for (int i = 0; i < this->count_chem; i++)
	{
		// add to back
		for (size_t j = 1; j < back[i].size(); j++)
		{
			int n = back[i][j];
			forward[n] = -1;
		}
	}
	// check that all count_chem have at least 1 cell
	for (int i = 0; i < this->count_chem; i++)
	{
		if (back[i].size() == 0)
		{
			error_msg("Error in building inverse mapping (chem to grid).", STOP);
		}
	}
}
/* ---------------------------------------------------------------------- */
void 
Reaction_module::Set_print_chem(int *t)
/* ---------------------------------------------------------------------- */
{
	if (mpi_myself == 0 && t != NULL)
	{
		this->print_chem = (*t != 0);
	}
#ifdef USE_MPI
	MPI_Bcast(&this->print_chem, 1, MPI_LOGICAL, 0, MPI_COMM_WORLD);
#endif
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::Set_print_chem_mask(int * t)
/* ---------------------------------------------------------------------- */
{
	if (this->print_chem_mask.size() < this->nxyz)
	{
		this->print_chem_mask.resize(this->nxyz);
	}
	if (this->mpi_myself == 0)
	{
		if (t == NULL) error_msg("NULL pointer in Set_print_chem_mask", 1);
		memcpy(this->print_chem_mask.data(), t, (size_t) (this->nxyz * sizeof(int)));
	}
#ifdef USE_MPI	
	MPI_Bcast(this->print_chem_mask.data(), this->nxyz, MPI_INT, 0, MPI_COMM_WORLD);
#endif
}
void
Reaction_module::SetSelectedOutputOn(int *t)
{
	if (mpi_myself == 0 && t != NULL)
	{
		this->selected_output_on = *t != 0;
	}
#ifdef USE_MPI
	MPI_Bcast(&this->selected_output_on, 1, MPI_LOGICAL, 0, MPI_COMM_WORLD);
#endif
}
void 
Reaction_module::Set_print_restart(int *t)
{
	if (mpi_myself == 0 && t != NULL)
	{
		this->print_restart = *t != 0;
	}
#ifdef USE_MPI
	MPI_Bcast(&this->print_restart, 1, MPI_LOGICAL, 0, MPI_COMM_WORLD);
#endif
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::SetPressure(double *t)
/* ---------------------------------------------------------------------- */
{
	if (this->pressure.size() < this->nxyz)
	{
		this->pressure.resize(this->nxyz);
	}
	if (mpi_myself == 0)
	{
		if (t == NULL) error_msg("NULL pointer in Set_pressure", 1);
		memcpy(this->pressure.data(), t, (size_t) (this->nxyz * sizeof(double)));
	}
#ifdef USE_MPI
	MPI_Bcast(this->pressure.data(), this->nxyz, MPI_DOUBLE, 0, MPI_COMM_WORLD);
#endif
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::SetPv(double *t)
/* ---------------------------------------------------------------------- */
{
	if (this->pv.size() < this->nxyz)
	{
		this->pv.resize(this->nxyz);
	}
	if (mpi_myself == 0)
	{
		if (t == NULL) error_msg("NULL pointer in Set_pv", 1);
		memcpy(this->pv.data(), t, (size_t) (this->nxyz * sizeof(double)));
	}
#ifdef USE_MPI
	MPI_Bcast(this->pv.data(), this->nxyz, MPI_DOUBLE, 0, MPI_COMM_WORLD);
#endif
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::SetPv0(double *t)
/* ---------------------------------------------------------------------- */
{
	if (this->pv0.size() < this->nxyz)
	{
		this->pv0.resize(this->nxyz);
	}
	if (mpi_myself == 0)
	{
		if (t == NULL) error_msg("NULL pointer in Set_pv0", 1);
		memcpy(this->pv0.data(), t, (size_t) (this->nxyz * sizeof(double)));
	}
#ifdef USE_MPI
	MPI_Bcast(pv0.data(), this->nxyz, MPI_DOUBLE, 0, MPI_COMM_WORLD);
#endif
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::Set_rebalance_fraction(double *t)
/* ---------------------------------------------------------------------- */
{
	if (mpi_myself == 0)
	{
		if (t == NULL) error_msg("NULL pointer in Set_rebalance_fraction", 1);
		this->rebalance_fraction = *t;
	}
#ifdef USE_MPI
	MPI_Bcast(&(this->rebalance_fraction), 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
#endif
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::SetRebalanceMethod(int *t)
/* ---------------------------------------------------------------------- */
{
	if (mpi_myself == 0)
	{
		if (t == NULL) error_msg("NULL pointer in SetRebalanceMethod", 1);
		this->rebalance_method = (*t != 0);
	}
#ifdef USE_MPI
	MPI_Bcast(&(this->rebalance_method), 1, MPI_LOGICAL, 0, MPI_COMM_WORLD);
#endif
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::SetSaturation(double *t)
/* ---------------------------------------------------------------------- */
{
	if (this->saturation.size() < this->nxyz)
	{
		this->saturation.resize(this->nxyz);
	}
	if (mpi_myself == 0)
	{
		if (t == NULL) error_msg("NULL pointer in Set_saturation", 1);
		memcpy(this->saturation.data(), t, (size_t) (this->nxyz * sizeof(double)));
	}
#ifdef USE_MPI
	MPI_Bcast(this->saturation.data(), this->nxyz, MPI_DOUBLE, 0, MPI_COMM_WORLD);
#endif
}
void 
Reaction_module::Set_steady_flow(int *t)
{
	if (mpi_myself == 0)
	{
		if (t == NULL) error_msg("NULL pointer in Set_steady_flow", 1);
		this->steady_flow = (*t != 0);
	}
#ifdef USE_MPI
	MPI_Bcast(&this->steady_flow, 1, MPI_LOGICAL, 0, MPI_COMM_WORLD);
#endif
}
void 
Reaction_module::Set_stop_message(bool t)
{
	if (mpi_myself == 0)
	{
		this->stop_message = t;
	}
#ifdef USE_MPI
	MPI_Bcast(&this->stop_message, 1, MPI_LOGICAL, 0, MPI_COMM_WORLD);
#endif
}

/* ---------------------------------------------------------------------- */
void
Reaction_module::SetTemperature(double *t)
/* ---------------------------------------------------------------------- */
{
	if (this->tempc.size() < this->nxyz)
	{
		this->tempc.resize(this->nxyz);
	}
	if (mpi_myself == 0)
	{
		if (t == NULL) error_msg("NULL pointer in Set_tempc", 1);
		memcpy(this->tempc.data(), t, (size_t) (this->nxyz * sizeof(double)));
	}
#ifdef USE_MPI
	MPI_Bcast(this->tempc.data(), this->nxyz, MPI_DOUBLE, 0, MPI_COMM_WORLD);
#endif
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::SetTime(double *t)
/* ---------------------------------------------------------------------- */
{
	if (mpi_myself == 0)
	{
		if (t == NULL) error_msg("NULL pointer in Set_time", 1);
		this->time = *t;
	}
#ifdef USE_MPI
	MPI_Bcast(&this->time, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
#endif
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::SetTimeConversion(double *t)
/* ---------------------------------------------------------------------- */
{
	if (mpi_myself == 0)
	{
		if (t == NULL) error_msg("NULL pointer in SetTimeConversion", 1);
		this->time_conversion = *t;
	}
#ifdef USE_MPI
	MPI_Bcast(&this->time_conversion, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
#endif
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::SetTimeStep(double *t)
/* ---------------------------------------------------------------------- */
{
	if (this->mpi_myself == 0)
	{
		if (t == NULL) error_msg("NULL pointer in Set_time_step", 1);
		this->time_step = *t;
	}
#ifdef USE_MPI
	MPI_Bcast(&this->time_step, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
#endif
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::SetVolume(double *t)
/* ---------------------------------------------------------------------- */
{
	if (this->volume.size() < this->nxyz)
	{
		this->volume.resize(this->nxyz);
	}
	if (mpi_myself == 0)
	{
		if (t == NULL) error_msg("NULL pointer in Set_volume", 1);
		memcpy(this->volume.data(), t, (size_t) (this->nxyz * sizeof(double)));
	}
#ifdef USE_MPI
	MPI_Bcast(this->volume.data(), this->nxyz, MPI_DOUBLE, 0, MPI_COMM_WORLD);
#endif
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::Set_x_node(double * t)
/* ---------------------------------------------------------------------- */
{
	if (this->x_node.size() < this->nxyz)
	{
		this->x_node.resize(this->nxyz);
	}
	if (this->mpi_myself == 0)
	{
		if (t == NULL) error_msg("NULL pointer in Set_x_node", 1);
		memcpy(this->x_node.data(), t, (size_t) (this->nxyz * sizeof(double)));
	}
#ifdef USE_MPI	
	MPI_Bcast(this->x_node.data(), this->nxyz, MPI_DOUBLE, 0, MPI_COMM_WORLD);
#endif
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::Set_y_node(double * t)
/* ---------------------------------------------------------------------- */
{
	if (this->y_node.size() < this->nxyz)
	{
		this->y_node.resize(this->nxyz);
	}
	if (this->mpi_myself == 0)
	{
		if (t == NULL) error_msg("NULL pointer in Set_y_node", 1);
		memcpy(this->y_node.data(), t, (size_t) (this->nxyz * sizeof(double)));
	}
#ifdef USE_MPI	
	MPI_Bcast(this->y_node.data(), this->nxyz, MPI_DOUBLE, 0, MPI_COMM_WORLD);
#endif
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::Set_z_node(double * t)
/* ---------------------------------------------------------------------- */
{
	if (this->z_node.size() < this->nxyz)
	{
		this->z_node.resize(this->nxyz);
	}
	if (this->mpi_myself == 0)
	{
		if (t == NULL) error_msg("NULL pointer in Set_z_node", 1);
		memcpy(this->z_node.data(), t, (size_t) (this->nxyz * sizeof(double)));
	}
#ifdef USE_MPI	
	MPI_Bcast(this->z_node.data(), this->nxyz, MPI_DOUBLE, 0, MPI_COMM_WORLD);
#endif
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::Setup_boundary_conditions(
					const int n_boundary, 
					int *boundary_solution1,
					int *boundary_solution2, 
					double *fraction1,
					double *boundary_fraction, 
					int dim)
/* ---------------------------------------------------------------------- */
{
/*
 *   Routine takes a list of solution numbers and returns a set of
 *   mass fractions
 *   Input: n_boundary - number of boundary conditions in list
 *          boundary_solution1 - list of first solution numbers to be mixed
 *          boundary_solution2 - list of second solution numbers to be mixed
 *          fraction1 - fraction of first solution 0 <= f <= 1
 *          dim - leading dimension of array boundary mass fractions
 *                must be >= to n_boundary
 *
 *   Output: boundary_fraction - mass fractions for boundary conditions
 *                             - dimensions must be >= n_boundary x n_comp
 *
 */
	int	i, n_old1, n_old2;
	double f1, f2;

	for (i = 0; i < n_boundary; i++)
	{
		cxxMix mixmap;
		n_old1 = boundary_solution1[i];
		n_old2 = boundary_solution2[i];
		f1 = fraction1[i];
		f2 = 1 - f1;
		mixmap.Add(n_old1, f1);
		if (f2 > 0.0)
		{
			mixmap.Add(n_old2, f2);
		}
		
		// Make mass fractions in d
		cxxSolution	cxxsoln(phreeqc_bin.Get_Solutions(), mixmap, 0);
		std::vector<double> d;
		cxxSolution2concentration(&cxxsoln, d);
		//cxxSolution2fraction(&cxxsoln, d);

		// Put mass fractions in boundary_fraction
		double *d_ptr = &boundary_fraction[i];
		size_t j;
		for (j = 0; j < components.size(); j++)
		{
			d_ptr[dim * j] = d[j];
		}
	}
}
#ifdef USE_MPI
/* ---------------------------------------------------------------------- */
void
Reaction_module::Module2Concentrations(double * c)
/* ---------------------------------------------------------------------- */
{
	// convert Reaction module solution data to concentrations for transport
	MPI_Status mpi_status;
	std::vector<double> d;  // scratch space to convert from moles to mass fraction
	std::vector<double> solns;
	cxxNameDouble::iterator it;

	if (mpi_myself == 0)
	{
		if (c == NULL) error_msg("NULL pointer in Module2Concentrations", 1);
	}
	int n = this->mpi_myself;
	for (int j = this->start_cell[n]; j <= this->end_cell[n]; j++)
	{
		// load fractions into d
		cxxSolution * cxxsoln_ptr = this->Get_workers()[0]->Get_solution(j);
		assert (cxxsoln_ptr);
		this->cxxSolution2concentration(cxxsoln_ptr, d);
		for (int i = 0; i < (int) this->components.size(); i++)
		{
			solns.push_back(d[i]);
		}
	}

	// make buffer to recv solutions
	double * recv_solns = new double[(size_t) this->count_chem * this->components.size()];

	// each process has its own vector of solution components
	// gather vectors to root
	for (int n = 1; n < this->mpi_tasks; n++)
	{
		int count = this->end_cell[n] - this->start_cell[n] + 1;
		int num_doubles = count * (int) this->components.size();
		if (this->mpi_myself == n)
		{
			MPI_Send((void *) solns.data(), num_doubles, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
		}
		else if (this->mpi_myself == 0)
		{
			MPI_Recv(recv_solns, num_doubles, MPI_DOUBLE, n, 0, MPI_COMM_WORLD, &mpi_status);
			for (int i = 0; i < num_doubles; i++)
			{
				solns.push_back(recv_solns[i]);
			}
		}
	}
	
	// delete recv buffer
	delete recv_solns;

	// Write vector into c
	if (this->mpi_myself == 0)
	{
		assert (solns.size() == this->count_chem*this->components.size());
		int n = 0;
		for (int j = 0; j < count_chem; j++)
		{
			std::vector<double> d;
			for (size_t i = 0; i < this->components.size(); i++)
			{
				d.push_back(solns[n++]);
			}
			std::vector<int>::iterator it;
			for (it = this->back[j].begin(); it != this->back[j].end(); it++)
			{
				double *d_ptr = &c[*it];
				size_t i;
				for (i = 0; i < this->components.size(); i++)
				{
					d_ptr[this->nxyz * i] = d[i];
				}
			}
		}
	}

}
#else
/* ---------------------------------------------------------------------- */
void
Reaction_module::Module2Concentrations(double * c)
/* ---------------------------------------------------------------------- */
{
	// convert Reaction module solution data to hst mass fractions

	std::vector<double> d;  // scratch space to convert from moles to mass fraction
	cxxNameDouble::iterator it;

	int j; 
	if (c == NULL) error_msg("NULL pointer in Module2Concentrations", 1);
	for (int n = 0; n < this->nthreads; n++)
	{
		for (j = this->start_cell[n]; j <= this->end_cell[n]; j++)
		{
			// load fractions into d
			cxxSolution * cxxsoln_ptr = this->Get_workers()[n]->Get_solution(j);
			assert (cxxsoln_ptr);
			this->cxxSolution2concentration(cxxsoln_ptr, d);

			// store in fraction at 1, 2, or 4 places depending on chemistry dimensions
			std::vector<int>::iterator it;
			for (it = this->back[j].begin(); it != this->back[j].end(); it++)
			{
				double *d_ptr = &c[*it];
				size_t i;
				for (i = 0; i < this->components.size(); i++)
				{
					d_ptr[this->nxyz * i] = d[i];
				}
			}
		}
	}
}
#endif
#ifdef USE_MPI
/* ---------------------------------------------------------------------- */
void
Reaction_module::Write_bc_raw(int *solution_list, int * bc_solution_count, 
			int * solution_number, const std::string &fn)
/* ---------------------------------------------------------------------- */
{
	// solution_list is Fortran nxyz number
	MPI_Bcast(solution_number, 1, MPI_INT, 0, MPI_COMM_WORLD);
	if (*solution_number == 0) return;
	MPI_Bcast(bc_solution_count, 1, MPI_INT, 0, MPI_COMM_WORLD);


	// Broadcast solution list
	std::vector<int> my_solution_list;
	if (mpi_myself == 0)
	{
		MPI_Bcast(solution_list, *bc_solution_count, MPI_INT, 0, MPI_COMM_WORLD);
	}
	else
	{
		my_solution_list.resize((size_t) bc_solution_count);
		MPI_Bcast(my_solution_list.data(), *bc_solution_count, MPI_INT, 0, MPI_COMM_WORLD);
	}

	std::ofstream ofs;
	// Open file on root
	if (this->mpi_myself == 0)
	{
		ofs.open(fn.c_str(), std::ios_base::app);
		if (!ofs.is_open())
		{
			std::ostringstream e_msg;
			e_msg << "Could not open file. " << fn;
			Write_error(e_msg.str().c_str());
			Error_stop();
		}
	}

	// dump solutions to oss
	std::ostringstream oss;
	for (int i = 0; i < *bc_solution_count; i++)
	{
		int raw_number = *solution_number + i;
		int n_fort = solution_list[i];
		int n_chem = this->forward[n_fort - 1];
		if (n_chem >= this->start_cell[this->mpi_myself] || 
			n_chem <= this->end_cell[this->mpi_myself])
		{
			oss << "# Fortran cell " << n_fort << ". Time " << (this->time) * (this->time_conversion) << "\n";
			this->workers[0]->Get_solution(n_chem)->dump_raw(oss, 0, &raw_number);
		}
	}

	// Write root solutions
	if (this->mpi_myself == 0 && oss.str().size() > 0)
	{
		ofs << oss.str();
	}

	// Retrieve from nonroot processes
	std::vector<char> char_buffer;
	for (int i = 1; i < this->mpi_tasks; i++)
	{
		int buffer_length = 0;
		MPI_Status mpi_status;

		// Send dump string to root
		if (mpi_myself == i)
		{
			buffer_length = (int) oss.str().size();
			MPI_Send(&buffer_length, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
			if (buffer_length > 0)
			{
				MPI_Send((void *) oss.str().c_str(), buffer_length, MPI_CHAR, 0, 0,
					MPI_COMM_WORLD);
			}
		}
		// Write dump string to file
		else if (mpi_myself == 0)
		{
			MPI_Recv(&buffer_length, 1, MPI_INT, i, 0, MPI_COMM_WORLD,
				&mpi_status);			
			if (buffer_length > 0)
			{
				char_buffer.resize(buffer_length + 1);
				MPI_Recv(char_buffer.data(), buffer_length, MPI_CHAR, i, 0,
					MPI_COMM_WORLD, &mpi_status);
				char_buffer[buffer_length] = '\0';
				ofs << char_buffer.data(); 
			}
		}
	}

	// Final write
	if (this->mpi_myself == 0)
	{
		ofs << "# Done with zone for time step." << std::endl;
		ofs.close();
	}

	return;
}
#else
/* ---------------------------------------------------------------------- */
void
Reaction_module::Write_bc_raw(int *solution_list, int * bc_solution_count, 
			int * solution_number, const std::string &fn)
/* ---------------------------------------------------------------------- */
{
	if (*solution_number == 0) return;
	std::ofstream ofs;
	ofs.open(fn.c_str(), std::ios_base::app);
	if (!ofs.is_open())
	{
		std::ostringstream e_msg;
		e_msg << "Could not open file. " << fn;
		Write_error(e_msg.str().c_str());
		Error_stop();
	}

	int raw_number = *solution_number;
	for (int i = 0; i < *bc_solution_count; i++)
	{
		int n_fort = solution_list[i];
		int n_chem = this->forward[n_fort - 1];
		if (n_chem >= 0)
		{
			IPhreeqcPhast * phast_iphreeqc_worker = NULL;
			for (int j = 0; j < this->nthreads; j++)
			{
				if (j >= start_cell[j] && j <= end_cell[j])
				{
					phast_iphreeqc_worker = this->workers[j];
				}
			}
			if (phast_iphreeqc_worker)
			{
				ofs << "# Fortran cell " << n_fort << ". Time " << (this->time) * (this->time_conversion) << "\n";
				cxxSolution *soln_ptr=  phast_iphreeqc_worker->Get_solution(n_chem);
				soln_ptr->dump_raw(ofs, 0, &raw_number);
				raw_number++;
			}
			else
			{
				assert(false);
			}
		}
		else
		{
			assert(false);
		}
	}
	ofs << "# Done with zone for time step." << std::endl;
	ofs.close();
	return;
}
#endif
/* ---------------------------------------------------------------------- */
void
Reaction_module:: Write_error(const char * item)
/* ---------------------------------------------------------------------- */
{
	RM_interface::phast_io.error_msg(item);
}
/* ---------------------------------------------------------------------- */
void
Reaction_module:: Write_log(const char * item)
/* ---------------------------------------------------------------------- */
{
	RM_interface::phast_io.log_msg(item);
}
/* ---------------------------------------------------------------------- */
void
Reaction_module:: Write_output(const char * item)
/* ---------------------------------------------------------------------- */
{
	RM_interface::phast_io.output_msg(item);
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::Write_restart(void)
/* ---------------------------------------------------------------------- */
{
	std::string char_buffer;
	ogzstream ofs_restart;
	std::string temp_name("temp_restart_file.gz");
	std::string name(this->file_prefix);
	std::string backup_name(this->file_prefix);
	if (mpi_myself == 0)
	{
		name.append(".restart.gz");
		backup_name.append(".restart.backup.gz");

		// open file 
		ofs_restart.open(temp_name.c_str());
		if (!ofs_restart.good())
		{
			std::ostringstream errstr;
			errstr << "Temporary restart file could not be opened: " << temp_name;
			error_msg(errstr.str().c_str(), 1);	
		}

		// write header
		ofs_restart << "#PHAST restart file" << std::endl;
		time_t now = ::time(NULL);
		ofs_restart << "#Prefix: " << this->file_prefix << std::endl;
		ofs_restart << "#Date: " << ctime(&now);
		ofs_restart << "#Current model time: " << this->time << std::endl;
		ofs_restart << "#nyz: " << this->nxyz << std::endl;

		// write index
		int i, j;
		ofs_restart << count_chem << std::endl;
		for (j = 0; j < count_chem; j++)	/* j is count_chem number */
		{
			for (size_t k = 0; k < back[j].size(); k++)
			{
				i = back[j][k];			/* i is nxyz number */
				ofs_restart << x_node[i] << "  " << y_node[i] << "  " <<
					z_node[i] << "  " << j << "  ";
				// solution 
				ofs_restart << have_Solution[i] << "  ";
				// pp_assemblage
				ofs_restart << have_PPassemblage[i] << "  ";
				// exchange
				ofs_restart << have_Exchange[i] << "  ";
				// surface
				ofs_restart << have_Surface[i] << "  ";
				// gas_phase
				ofs_restart << have_GasPhase[i] << "  ";
				// solid solution
				ofs_restart << have_SSassemblage[i] << "  ";
				// kinetics
				ofs_restart << have_Kinetics[i] << std::endl;
			}
		}
	}

	// write data
#ifdef USE_MPI
	this->workers[0]->SetDumpStringOn(true); 
	std::ostringstream in;

	// write raw to ostringstream
	for (int j = this->start_cell[this->mpi_myself]; j <= this->end_cell[this->mpi_myself]; j++)
	{
		cxxStorageBin temp_bin;
		this->Get_workers()[0]->Get_PhreeqcPtr()->phreeqc2cxxStorageBin(temp_bin, j);
		for (size_t k = 0; k < back[j].size(); k++)
		{
			int i = back[j][k];			/* i is nxyz number */ 
			temp_bin.dump_raw(in, j, 0, &i);
		}
	}
	//in << "DUMP; -cells " << this->start_cell[this->mpi_myself] << "-" << this->end_cell[this->mpi_myself] << "\n";
	//this->workers[0]->RunString(in.str().c_str());
	for (int n = 0; n < this->mpi_tasks; n++)
	{
		// Need to transfer output stream to root and print
		if (this->mpi_myself == n)
		{
			if (n == 0)
			{
				//ofs_restart << this->Get_workers()[0]->GetDumpString();
				ofs_restart << in.str().c_str();
			}
			else
			{
				//int size = (int) strlen(this->workers[0]->GetDumpString());
				//MPI_Send(&size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
				//MPI_Send((void *) this->workers[0]->GetDumpString(), size, MPI_CHARACTER, 0, 0, MPI_COMM_WORLD);
				int size = (int) strlen(in.str().c_str());
				MPI_Send(&size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
				MPI_Send((void *) in.str().c_str(), size, MPI_CHARACTER, 0, 0, MPI_COMM_WORLD);
			}	
		}
		else if (this->mpi_myself == 0)
		{
			MPI_Status mpi_status;
			int size;
			MPI_Recv(&size, 1, MPI_INT, n, 0, MPI_COMM_WORLD, &mpi_status);
			char_buffer.resize(size+1);
			MPI_Recv((void *) char_buffer.c_str(), size, MPI_CHARACTER, n, 0, MPI_COMM_WORLD, &mpi_status);
			char_buffer[size] = '\0';
			ofs_restart << char_buffer;
		}
	}
#else
	for (int n = 0; n < (int) this->workers.size() - 1; n++)
	{
		//this->workers[n]->SetDumpStringOn(true); 
		for (int j = this->start_cell[n]; j <= this->end_cell[n]; j++)
		{
			cxxStorageBin temp_bin;
			this->workers[n]->Get_PhreeqcPtr()->phreeqc2cxxStorageBin(temp_bin, j);
			for (size_t k = 0; k < back[j].size(); k++)
			{
				int i = back[j][k];			/* i is nxyz number */ 
				temp_bin.dump_raw(ofs_restart, j, 0, &i);
			}
		}
		//this->workers[n]->SetDumpStringOn(true); 
		//std::ostringstream in;
		//in << "DUMP; -cells " << this->start_cell[n] << "-" << this->end_cell[n] << "\n";
		//this->workers[n]->RunString(in.str().c_str());
		//ofs_restart << this->Get_workers()[n]->GetDumpString();
	}
#endif

	ofs_restart.close();
	// rename files
	this->File_rename(temp_name.c_str(), name.c_str(), backup_name.c_str());
}
/* ---------------------------------------------------------------------- */
void
Reaction_module:: Write_screen(const char * item)
/* ---------------------------------------------------------------------- */
{
	RM_interface::phast_io.screen_msg(item);
}
/* ---------------------------------------------------------------------- */
void
Reaction_module:: Write_xyz(const char * item)
/* ---------------------------------------------------------------------- */
{
	RM_interface::phast_io.punch_msg(item);
}
/* ---------------------------------------------------------------------- */


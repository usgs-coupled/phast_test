#include "Reaction_module.h"
#include "RM_interface.h"
#include "PHRQ_base.h"
#include "PHRQ_io.h"
#include "IPhreeqc.h"
#include "IPhreeqc.hpp"
#include "IPhreeqcPhast.h"
#include "Phreeqc.h"
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
#include <time.h>
Reaction_module::Reaction_module(PHRQ_io *io)
	//
	// default constructor for cxxExchComp 
	//
: PHRQ_base(io)
{
	this->phast_iphreeqc_worker = new IPhreeqcPhast;
	std::map<size_t, Reaction_module*>::value_type instance(this->phast_iphreeqc_worker->Get_Index(), this);
	RM_interface::Instances.insert(instance);

	//RM_interface::Instances[phast_iphreeqc_worker->Index] = Reaction_module_ptr;
	//std::pair<std::map<size_t, Reaction_module*>::iterator, bool> pr = RM_interface::Instances.insert(instance);

	this->mpi_myself = 0;
	this->mpi_tasks = 1;

	this->gfw_water = 18.;						// gfw of water
	this->count_chem = 0;
	this->free_surface = false;					// free surface calculation
	this->steady_flow = false;					// steady-state flow calculation
	this->transient_free_surface = false;		// free surface and not steady flow
	this->nxyz = 0;								// number of nodes 
	this->nx = this->ny = this->nz = 0;			// number of nodes in each coordinate direction
	this->time_hst = 0;							// scalar time from transport 
	this->time_step_hst = 0;					// scalar time step from transport
	this->cnvtmi = NULL;						// scalar conversion factor for time
	this->x_node = NULL;						// nxyz array of X coordinates for nodes
	this->y_node = NULL;						// nxyz array of Y coordinates for nodes 
	this->z_node = NULL;						// nxyz array of Z coordinates for nodes
	this->fraction = NULL;						// nxyz by ncomps mass fractions nxyz:components
	this->frac = NULL;							// nxyz saturation fraction
	this->pv = NULL;							// nxyz current pore volumes 
	this->pv0 = NULL;							// nxyz initial pore volumes
	this->volume = NULL;						// nxyz geometric cell volumes 
	this->printzone_chem = NULL;				// nxyz print flags for output file
	this->printzone_xyz = NULL;					// nxyz print flags for chemistry XYZ file 
	this->ic1 = NULL;							// reactant number for end member 1
	this->rebalance_fraction_hst = NULL;		// parameter for rebalancing process load for parallel	

	// print flags
	this->prslm = false;						// solution method print flag 
	this->print_chem = false;					// print flag for chemistry output file 
	this->print_xyz = false;					// print flag for selected output
	this->print_hdf = false;					// print flag for hdf file
	this->print_restart = false;				// print flag for writing restart file 
}
Reaction_module::~Reaction_module(void)
{
	std::map<size_t, Reaction_module*>::iterator it = RM_interface::Instances.find(this->phast_iphreeqc_worker->Get_Index());
	delete phast_iphreeqc_worker;
	if (it != RM_interface::Instances.end())
	{
		RM_interface::Instances.erase(it);
	}

}
#ifdef SKIP
/* ---------------------------------------------------------------------- */
int
Reaction_module::Load_database(std::string database_name)
/* ---------------------------------------------------------------------- */
{
	this->database_file_name = database_name;
	if (this->phast_iphreeqc_worker->LoadDatabase(this->database_file_name.c_str()) != 0) 
	{
		std::ostringstream errstr;
		errstr << this->phast_iphreeqc_worker->GetErrorString() << std::endl;
		error_msg(errstr.str().c_str(), 1);
		return 0;
	}
	this->gfw_water = this->phast_iphreeqc_worker->Get_gfw("H2O");
	return 1;
}
#endif
#ifdef SKIP
/* ---------------------------------------------------------------------- */
int
Reaction_module::Initial_phreeqc_run(std::string chemistry_name)
/* ---------------------------------------------------------------------- */
{
	/*
	*  Run PHREEQC to obtain PHAST reactants
	*/

	/*
	 *   initialize HDF
	 */
#ifdef HDF5_CREATE
// TODO, implement HDF	HDF_Init(prefix.c_str(), prefix.size());
#endif
	/*
	 *   initialize merge
	 */
	//TODO MPI and merge
#if defined(USE_MPI) && defined(HDF5_CREATE) && defined(MERGE_FILES)
	output_close(OUTPUT_ECHO);
	MergeInit(prefix, prefix_l, *solute);	/* opens .chem.txt,  .chem.xyz.tsv, .log.txt */
#endif


	/*
	 *   Run  input file
	 */
	if (this->mpi_myself == 0)
	{
		this->Get_io()->log_msg("Initial PHREEQC run.\n");
	}
	this->phast_iphreeqc_worker->AccumulateLine("PRINT; -status false; -other false");
	if (this->phast_iphreeqc_worker->RunAccumulated() != 0)
	{
		std::ostringstream errstr;
		errstr << phast_iphreeqc_worker->GetErrorString() << std::endl;
		error_msg(errstr.str().c_str(), 1);
	}

	if (this->phast_iphreeqc_worker->RunFile(chemistry_name.c_str()) != 0) 
	{
		std::ostringstream errstr;
		errstr << this->phast_iphreeqc_worker->GetErrorString() << std::endl;
		error_msg(errstr.str().c_str(), 1);
	}
	// make phreeqc_bin
	this->phreeqc_bin.Clear();
	this->phast_iphreeqc_worker->Get_PhreeqcPtr()->phreeqc2cxxStorageBin(phreeqc_bin);
	if (this->mpi_myself == 0)
	{
		this->Get_io()->log_msg("\nSuccessfully processed chemistry data file.\n");
	}

	return 1;
}
#endif
#ifdef SKIP
/* ---------------------------------------------------------------------- */
int
Reaction_module::Initial_phreeqc_run(std::string database_name, std::string chemistry_name)
/* ---------------------------------------------------------------------- */
{
	/*
	*  Run PHREEQC to obtain PHAST reactants
	*/

	/*
	 *   initialize HDF
	 */
#ifdef HDF5_CREATE
// TODO, implement HDF	HDF_Init(prefix.c_str(), prefix.size());
#endif
	/*
	 *   initialize merge
	 */
	//TODO MPI and merge
#if defined(USE_MPI) && defined(HDF5_CREATE) && defined(MERGE_FILES)
	output_close(OUTPUT_ECHO);
	MergeInit(prefix, prefix_l, *solute);	/* opens .chem.txt,  .chem.xyz.tsv, .log.txt */
#endif
	int rm_id = (int) this->phast_iphreeqc_worker->Get_Index();

	// Load database
    if (SetOutputStringOn(rm_id, true) < 0) RM_error(&rm_id);
    if (SetSelectedOutputFileOn(rm_id, true) < 0) RM_error(&rm_id);
    if (LoadDatabase(rm_id, database_name.c_str()) < 0) RM_error(&rm_id);
    RM_write_output(&rm_id);

	// Run chemistry file
    if (RunFile(rm_id, chemistry_name.c_str()) < 0) RM_error(&rm_id);
    RM_write_output(&rm_id);


    // Create a StorageBin with initial PHREEQC for boundary conditions
	this->phreeqc_bin.Clear();
	this->phast_iphreeqc_worker->Get_PhreeqcPtr()->phreeqc2cxxStorageBin(phreeqc_bin);

	return 1;
}
#endif
/* ---------------------------------------------------------------------- */
void
Reaction_module::Distribute_initial_conditions(
	    int ip_id,
		int *initial_conditions1,
		int *initial_conditions2,	
		double *fraction1,
		int exchange_units,
		int surface_units,
		int ssassemblage_units,
		int ppassemblage_units,
		int gasphase_units,
		int kinetics_units)
/* ---------------------------------------------------------------------- */
{
	/*
	*  Use indices defined in initial_conditions and fractions
	*  to mix reactants and store as initial conditions in a StorageBin
	*/
	/*
	 *      nxyz - number of cells
	 *      initial_conditions1 - Fortran, 7 x nxyz integer array, containing
	 *           solution number
	 *           pure_phases number
	 *           exchange number
	 *           surface number
	 *           gas number
	 *           solid solution number
	 *           kinetics number
	 *      initial_conditions2 - Fortran, 7 x nxyz integer array, containing
	 *      fraction for 1 - Fortran, 7 x nxyz double array, containing
	 *
	 *      Routine mixes solutions, pure_phase assemblages,
	 *      exchangers, surface complexers, gases, solid solution assemblages,
	 *      and kinetics for each cell.
	 */
	int
		i,
		j;

	// Initial IPhreeqc output
	SetOutputFileOn(ip_id, 0);
	SetDumpStringOn(ip_id, 1);
	SetSelectedOutputFileOn(ip_id, 0);
	SetDumpFileOn(ip_id, 0);
	// Reaction module output
	int rm_id = (int) this->phast_iphreeqc_worker->Get_Index();
	SetOutputFileOn(rm_id, 0);
	SetDumpStringOn(rm_id, 0);
	SetSelectedOutputFileOn(rm_id, 0);
	SetDumpFileOn(ip_id, 0);

	std::ostringstream mix_string;
	std::ostringstream copy_string;

	// Save pointer
	ic1 = initial_conditions1;
	/*
	 *  Generate strings to produce
	 *  solution, exchange, surface, gas phase, kinetics, solid solution for each active cell.
	 *  writes to negative cell numbers, then copies to positive cell numbers
	 *  Does nothing for initial condition indexes less than 0 (i.e. restart files)
	 */
	size_t count_negative_porosity = 0;
	for (i = 0; i < this->nxyz; i++)
	{							        /* i is nxyz number */
		j = this->forward[i];			/* j is count_chem number */
		if (j < 0)
			continue;
		assert(this->forward[i] >= 0);
		assert (this->volume[i] > 0.0);
		double porosity = this->pv0[i] / this->volume[i];
		if (this->pv0[i] < 0 || this->volume[i] < 0)
		{
			std::ostringstream errstr;
			errstr << "Negative volume in cell " << i << ": volume, " 
				<< volume[i] << "\t initial volume, " << this->pv0[i] << ".",
			count_negative_porosity++;
			error_msg(errstr.str().c_str());
			continue;
		}
		assert (porosity > 0.0);
		double porosity_factor = (1.0 - porosity) / porosity;
		this->Cell_initialize(mix_string, copy_string, i, j, initial_conditions1, initial_conditions2,
			fraction1,
			exchange_units, surface_units, ssassemblage_units,
			ppassemblage_units, gasphase_units, kinetics_units,
			porosity_factor);
	}
	if (count_negative_porosity > 0)
	{
		std::ostringstream errstr;
		errstr << "Negative initial volumes may be due to initial head distribution.\n"
			"Make initial heads greater than or equal to the elevation of the node for each cell.\n"
			"Increase porosity, decrease specific storage, or use free surface boundary.";
		error_msg(errstr.str().c_str(), 1);
	}

	// Make initial conditions
	if (RunString(rm_id, mix_string.str().c_str()) < 0) RM_error(&rm_id);
	if (RunString(rm_id, copy_string.str().c_str()) < 0) RM_error(&rm_id);


	/*
	 * Read any restart files
	 */
	for (std::map < std::string, int >::iterator it = this->FileMap.begin();
		 it != this->FileMap.end(); it++)
	{
		int
			ifile = -100 - it->second;

		// use gsztream
		igzstream myfile;
		myfile.open(it->first.c_str());
		if (!myfile.good())

		{
			std::ostringstream errstr;
			errstr << "File could not be opened: " << it->first.c_str();
			error_msg(errstr.str().c_str());
			break;
		}

		std::ostringstream oss;
		CParser	cparser(myfile, this->Get_io());
		cparser.set_echo_file(CParser::EO_NONE);
		cparser.set_echo_stream(CParser::EO_NONE);

		// skip headers
		while (cparser.check_line("restart", false, true, true, false) ==
			   PHRQ_io::LT_EMPTY);

		// read number of lines of index
		int
			n = -1;
		if (!(cparser.get_iss() >> n) || n < 4)
		{
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
		KDtree index_tree(pts);

		// restart definitions
		cxxStorageBin restartBin;
		restartBin.read_raw(cparser);

		// cell definitions to be transferred to Reaction Module
		cxxStorageBin rmBin;

		for (j = 0; j < this->count_chem; j++)	/* j is count_chem number */
		{
			i = this->back[j][0];   /* i is nxyz number */
			Point p(this->x_node[i], this->y_node[i], this->z_node[i]);

			// find data closest to current grid node
			int	k = (int) index_tree.Interpolate3d(p);	// k is index number in restartBin

			// solution
			if (initial_conditions1[i * 7] == ifile)
			{
				if (c_index[k * 7] != -1)	// entity k should be defined in restartBin
				{
					if (restartBin.Get_Solution(k) != NULL)
					{
						rmBin.Set_Solution(j, restartBin.Get_Solution(k));
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
				if (c_index[k * 7 + 1] != -1)	// entity k should be defined in restartBin
				{
					if (restartBin.Get_PPassemblage(k) != NULL)
					{
						rmBin.Set_PPassemblage(j, restartBin.Get_PPassemblage(k));
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
				if (c_index[k * 7 + 2] != -1)	// entity k should be defined in restartBin
				{
					if (restartBin.Get_Exchange(k) != NULL)
					{
						rmBin.Set_Exchange(j, restartBin.Get_Exchange(k));
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
				if (c_index[k * 7 + 3] != -1)	// entity k should be defined in restartBin
				{
					if (restartBin.Get_Surface(k) != NULL)
					{
						rmBin.Set_Surface(j, restartBin.Get_Surface(k));
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
				if (c_index[k * 7 + 4] != -1)	// entity k should be defined in restartBin
				{
					if (restartBin.Get_GasPhase(k) != NULL)
					{
						rmBin.Set_GasPhase(j, restartBin.Get_GasPhase(k));
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
				if (c_index[k * 7 + 5] != -1)	// entity k should be defined in restartBin
				{
					if (restartBin.Get_SSassemblage(k) != NULL)
					{
						rmBin.Set_SSassemblage(j, restartBin.Get_SSassemblage(k));
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
				if (c_index[k * 7 + 6] != -1)	// entity k should be defined in restartBin
				{
					if (restartBin.Get_Kinetics(k) != NULL)
					{
						rmBin.Set_Kinetics(j, restartBin.Get_Kinetics(k));
					}
					else
					{
						assert(false);
						initial_conditions1[7 * i + 6] = -1;
					}
				}
			}
		}

		// transfer to Reaction Module
		this->phast_iphreeqc_worker->Get_PhreeqcPtr()->cxxStorageBin2phreeqc(rmBin);
		myfile.close();
	}

	// initialize uz
	if (this->transient_free_surface)
	{
		for (i = 0; i < this->nxyz; i++)
		{
			this->old_frac.push_back(1.0);
		}
	}
}
#ifdef SKIP
/* ---------------------------------------------------------------------- */
void
Reaction_module::Distribute_initial_conditions(
	    int ip_id,
		int *initial_conditions1,
		int *initial_conditions2,	
		double *fraction1,
		int exchange_units,
		int surface_units,
		int ssassemblage_units,
		int ppassemblage_units,
		int gasphase_units,
		int kinetics_units)
/* ---------------------------------------------------------------------- */
{
	/*
	*  Use indices defined in initial_conditions and fractions
	*  to mix reactants and store as initial conditions in a StorageBin
	*/
	/*
	 *      nxyz - number of cells
	 *      initial_conditions1 - Fortran, 7 x nxyz integer array, containing
	 *           solution number
	 *           pure_phases number
	 *           exchange number
	 *           surface number
	 *           gas number
	 *           solid solution number
	 *           kinetics number
	 *      initial_conditions2 - Fortran, 7 x nxyz integer array, containing
	 *      fraction for 1 - Fortran, 7 x nxyz double array, containing
	 *
	 *      Routine mixes solutions, pure_phase assemblages,
	 *      exchangers, surface complexers, gases, solid solution assemblages,
	 *      and kinetics for each cell.
	 */
	int
		i,
		j;

	// Initial IPhreeqc output
	SetOutputFileOn(ip_id, 0);
	SetDumpStringOn(ip_id, 1);
	SetSelectedOutputFileOn(ip_id, 0);
	SetDumpFileOn(ip_id, 0);
	// Reaction module output
	SetOutputFileOn(this->index, 0);
	SetDumpStringOn(this->index, 0);
	SetSelectedOutputFileOn(this->index, 0);
	SetDumpFileOn(ip_id, 0);

	std::ostringstream mix_string;
	std::ostringstream copy_string;

	// Save pointer
	ic1 = initial_conditions1;
	/*
	 *  Copy solution, exchange, surface, gas phase, kinetics, solid solution for each active cell.
	 *  Does nothing for indexes less than 0 (i.e. restart files)
	 */
	size_t count_negative_porosity = 0;
	for (i = 0; i < this->nxyz; i++)
	{							        /* i is nxyz number */
		j = this->forward[i];			/* j is count_chem number */
		if (j < 0)
			continue;
		assert(this->forward[i] >= 0);
		assert (this->volume[i] > 0.0);
		double porosity = this->pv0[i] / this->volume[i];
		if (this->pv0[i] < 0 || this->volume[i] < 0)
		{
			std::ostringstream errstr;
			errstr << "Negative volume in cell " << i << ": volume, " 
				<< volume[i] << "\t initial volume, " << this->pv0[i] << ".",
			count_negative_porosity++;
			error_msg(errstr.str().c_str());
			continue;
		}
		assert (porosity > 0.0);
		double porosity_factor = (1.0 - porosity) / porosity;
		this->Cell_initialize(mix_string, copy_string, i, j, initial_conditions1, initial_conditions2,
			fraction1,
			exchange_units, surface_units, ssassemblage_units,
			ppassemblage_units, gasphase_units, kinetics_units,
			porosity_factor);
	}
	if (count_negative_porosity > 0)
	{
		std::ostringstream errstr;
		errstr << "Negative initial volumes may be due to initial head distribution.\n"
			"Make initial heads greater than or equal to the elevation of the node for each cell.\n"
			"Increase porosity, decrease specific storage, or use free surface boundary.";
		error_msg(errstr.str().c_str());
	}

	// Make initial conditions
	if (RunString(this->index, mix_string.str().c_str()) < 0) RM_error(&this->index);
	if (RunString(this->index, copy_string.str().c_str()) < 0) RM_error(&this->index);

	/*
	 * Read any restart files
	 */
	for (std::map < std::string, int >::iterator it = this->FileMap.begin();
		 it != this->FileMap.end(); it++)
	{
		int
			ifile = -100 - it->second;

		// use gsztream
		igzstream myfile;
		myfile.open(it->first.c_str());
		if (!myfile.good())

		{
			std::ostringstream errstr;
			errstr << "File could not be opened: " << it->first.c_str();
			error_msg(errstr.str().c_str());
			break;
		}

		std::ostringstream oss;
		CParser	cparser(myfile, this->Get_io());
		cparser.set_echo_file(CParser::EO_NONE);
		cparser.set_echo_stream(CParser::EO_NONE);

		// skip headers
		while (cparser.check_line("restart", false, true, true, false) ==
			   PHRQ_io::LT_EMPTY);

		// read number of lines of index
		int
			n = -1;
		if (!(cparser.get_iss() >> n) || n < 4)
		{
			std::ostringstream errstr;
			errstr << "File does not have node locations: " << it->first.c_str() << "\nPerhaps it is an old format restart file.";
			error_msg(errstr.str().c_str());
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
		//int * index = (int *) PHRQ_malloc((size_t) (n * 7 * sizeof(int)));
		int * cell_index = new int( (n * 7 * 20 * sizeof(int)) );

		for (i = 0; i < n; i++)
		{
			cparser.check_line("restart", false, false, false, false);
			double
				x,
				y,
				z,
				v;
			int dummy;
			cparser.get_iss() >> x;
			cparser.get_iss() >> y;
			cparser.get_iss() >> z;
			cparser.get_iss() >> v;
			pts.push_back(Point(x, y, z, v));

			//cparser.get_iss() >> cell_index[i * 7];
			//cparser.get_iss() >> cell_index[i * 7 + 1];
			//cparser.get_iss() >> cell_index[i * 7 + 2];
			//cparser.get_iss() >> cell_index[i * 7 + 3];
			//cparser.get_iss() >> cell_index[i * 7 + 4];
			//cparser.get_iss() >> cell_index[i * 7 + 5];
			//cparser.get_iss() >> cell_index[i * 7 + 6];
			cparser.get_iss() >> dummy;
			cparser.get_iss() >> dummy;
			cparser.get_iss() >> dummy;
			cparser.get_iss() >> dummy;
			cparser.get_iss() >> dummy;
			cparser.get_iss() >> dummy;
			cparser.get_iss() >> dummy;
			//cell_index[i * 7] = dummy;
			//cparser.get_iss() >> dummy;
			//cell_index[i * 7 + 1] = dummy;
			//cparser.get_iss() >> dummy;
			//cell_index[i * 7 + 2] = dummy;
			//cparser.get_iss() >> dummy;
			//cell_index[i * 7 + 3] = dummy;
			//cparser.get_iss() >> dummy;
			//cell_index[i * 7 + 4] = dummy;
			//cparser.get_iss() >> dummy;
			////cell_index[i * 7 + 5] = dummy;
			//cparser.get_iss() >> dummy;
			//cell_index[i * 7 + 6] = dummy;
		}
		KDtree index_tree(pts);

		// make an IPhreeqc
		int restart_id = CreateIPhreeqc();
		if (restart_id < 0) RM_error(&restart_id);
		if (LoadDatabase(restart_id, database_file_name.c_str()) < 0) RM_error(&restart_id);

		while (cparser.check_line("restart", false, false, false, false) !=  PHRQ_io::LT_EOF)
		{
			AccumulateLine(restart_id, cparser.line().c_str());
		}
		RunAccumulated(restart_id);
		//cxxStorageBin tempBin;
		//tempBin.read_raw(cparser);

		std::ostringstream restart_mix_string;
		std::ostringstream restart_copy_string;
		std::ostringstream restart_dump_string;
		restart_dump_string << "DUMP; -cells\n";
		std::set<int> cell_nos;

		for (j = 0; j < this->count_chem; j++)	/* j is count_chem number */
		{
			i = this->back[j][0];   /* i is nxyz number */
			Point p(this->x_node[i], this->y_node[i], this->z_node[i]);
			int	k = (int) index_tree.Interpolate3d(p);	// k is index number in tempBin
			mix_string << "DELETE; -cell " << -j << "\n";

			// solution
			if (initial_conditions1[i * 7] == ifile)
			{
				if (cell_index[k * 7] != -1)	// entity k should be defined in tempBin
				{
					restart_mix_string << "MIX_SOLUTION " << -j << "; " << k << "  1.0\n";
					restart_copy_string << "COPY solution " << -j << "  " << j << "\n";
					restart_copy_string << "DELETE; -solution " << -j << "\n";
					cell_nos.insert(-j);
				}
			}

			// PPassemblage
			if (initial_conditions1[i * 7 + 1] == ifile)
			{
				if (cell_index[k * 7 + 1] != -1)	// entity k should be defined in tempBin
				{
					restart_mix_string << "MIX_EQUILIBRIUM_PHASES " << -j << "; " << k << "  1.0\n";
					restart_copy_string << "COPY equilibrium_phases " << -j << "  " << j << "\n";
					restart_copy_string << "DELETE; -equilibrium_phases " << -j << "\n";
					cell_nos.insert(-j);
				}
			}

			// Exchange
			if (initial_conditions1[i * 7 + 2] == ifile)
			{
				if (cell_index[k * 7 + 2] != -1)	// entity k should be defined in tempBin
				{
					restart_mix_string << "MIX_EXCHANGE " << -j << "; " << k << "  1.0\n";
					restart_copy_string << "COPY exchange " << -j << "  " << j << "\n";
					restart_copy_string << "DELETE; -exchange " << -j << "\n";
					cell_nos.insert(-j);
				}
			}

			// Surface
			if (initial_conditions1[i * 7 + 3] == ifile)
			{
				if (cell_index[k * 7 + 3] != -1)	// entity k should be defined in tempBin
				{
					restart_mix_string << "MIX_SURFACE " << -j << "; " << k << "  1.0\n";
					restart_copy_string << "COPY surface " << -j << "  " << j << "\n";
					restart_copy_string << "DELETE; -surface " << -j << "\n";
					cell_nos.insert(-j);
				}
			}

			// Gas phase
			if (initial_conditions1[i * 7 + 4] == ifile)
			{
				if (cell_index[k * 7 + 4] != -1)	// entity k should be defined in tempBin
				{
					restart_mix_string << "MIX_GAS_PHASE " << -j << "; " << k << "  1.0\n";
					restart_copy_string << "COPY gas_phase " << -j << "  " << j << "\n";
					restart_copy_string << "DELETE; -gas_phase " << -j << "\n";
					cell_nos.insert(-j);
				}
			}

			// Solid solution
			if (initial_conditions1[i * 7 + 5] == ifile)
			{
				if (cell_index[k * 7 + 5] != -1)	// entity k should be defined in tempBin
				{
					restart_mix_string << "MIX_SOLID_SOLUTION " << -j << "; " << k << "  1.0\n";
					restart_copy_string << "COPY solid_solution " << -j << "  " << j << "\n";
					restart_copy_string << "DELETE; -solid_solution " << -j << "\n";
					cell_nos.insert(-j);
				}
			}

			// Kinetics
			if (initial_conditions1[i * 7 + 6] == ifile)
			{
				if (cell_index[k * 7 + 6] != -1)	// entity k should be defined in tempBin
				{
					restart_mix_string << "MIX_KINETICS " << -j << "; " << k << "  1.0\n";
					restart_copy_string << "COPY kinetics " << -j << "  " << j << "\n";
					restart_copy_string << "DELETE; -kinetics " << -j << "\n";
					cell_nos.insert(-j);
				}
			}
			if (cell_nos.find(-j) != cell_nos.end())
			{
				restart_dump_string << -j << "\n";
				restart_copy_string << "COPY cell " << -j << "  " << j << "\n";
				restart_copy_string << "DELETE cell " << -j << "\n";
			}
		}
        // generate cells
		if (RunString(restart_id, restart_mix_string.str().c_str()) < 0) RM_error(&restart_id);
		// dump cells
		SetDumpStringOn(restart_id, 1);
		if (RunString(restart_id, restart_dump_string.str().c_str()) < 0) RM_error(&restart_id);
		// load cells into Reaction Module
		if (RunString(this->index, GetDumpString(restart_id)) < 0) RM_error(&restart_id);
		if (RunString(this->index, restart_copy_string.str().c_str()) < 0) RM_error(&restart_id);

		// clean up
		DestroyIPhreeqc(restart_id);
		myfile.close();
		delete cell_index;
	}
}
#endif
#ifdef SKIP
/* ---------------------------------------------------------------------- */
void
Reaction_module::Distribute_initial_conditions(
	    int ip_id,
		int *initial_conditions1,
		int *initial_conditions2,	
		double *fraction1,
		int exchange_units,
		int surface_units,
		int ssassemblage_units,
		int ppassemblage_units,
		int gasphase_units,
		int kinetics_units)
/* ---------------------------------------------------------------------- */
{
	/*
	*  Use indices defined in initial_conditions and fractions
	*  to mix reactants and store as initial conditions in a StorageBin
	*/
	/*
	 *      nxyz - number of cells
	 *      initial_conditions1 - Fortran, 7 x nxyz integer array, containing
	 *           solution number
	 *           pure_phases number
	 *           exchange number
	 *           surface number
	 *           gas number
	 *           solid solution number
	 *           kinetics number
	 *      initial_conditions2 - Fortran, 7 x nxyz integer array, containing
	 *      fraction for 1 - Fortran, 7 x nxyz double array, containing
	 *
	 *      Routine mixes solutions, pure_phase assemblages,
	 *      exchangers, surface complexers, gases, solid solution assemblages,
	 *      and kinetics for each cell.
	 */
	int
		i,
		j;

	// Initial IPhreeqc output
	SetOutputFileOn(ip_id, 0);
	SetDumpStringOn(ip_id, 1);
	SetSelectedOutputFileOn(ip_id, 0);
	SetDumpFileOn(ip_id, 0);
	// Reaction module output
	SetOutputFileOn(this->index, 0);
	SetDumpStringOn(this->index, 0);
	SetSelectedOutputFileOn(this->index, 0);
	SetDumpFileOn(ip_id, 0);

	std::ostringstream mix_string;
	std::ostringstream copy_string;

	// Save pointer
	ic1 = initial_conditions1;
	/*
	 *  Copy solution, exchange, surface, gas phase, kinetics, solid solution for each active cell.
	 *  Does nothing for indexes less than 0 (i.e. restart files)
	 */
	size_t count_negative_porosity = 0;
	for (i = 0; i < this->nxyz; i++)
	{							        /* i is nxyz number */
		j = this->forward[i];			/* j is count_chem number */
		if (j < 0)
			continue;
		assert(this->forward[i] >= 0);
		assert (this->volume[i] > 0.0);
		double porosity = this->pv0[i] / this->volume[i];
		if (this->pv0[i] < 0 || this->volume[i] < 0)
		{
			std::ostringstream errstr;
			errstr << "Negative volume in cell " << i << ": volume, " 
				<< volume[i] << "\t initial volume, " << this->pv0[i] << ".",
			count_negative_porosity++;
			error_msg(errstr.str().c_str());
			continue;
		}
		assert (porosity > 0.0);
		double porosity_factor = (1.0 - porosity) / porosity;
		this->Cell_initialize(mix_string, copy_string, i, j, initial_conditions1, initial_conditions2,
			fraction1,
			exchange_units, surface_units, ssassemblage_units,
			ppassemblage_units, gasphase_units, kinetics_units,
			porosity_factor);
	}
	if (count_negative_porosity > 0)
	{
		std::ostringstream errstr;
		errstr << "Negative initial volumes may be due to initial head distribution.\n"
			"Make initial heads greater than or equal to the elevation of the node for each cell.\n"
			"Increase porosity, decrease specific storage, or use free surface boundary.";
		error_msg(errstr.str().c_str());
	}

	// Make initial conditions
	if (RunString(this->index, mix_string.str().c_str()) < 0) RM_error(&this->index);
	if (RunString(this->index, copy_string.str().c_str()) < 0) RM_error(&this->index);

	/*
	 * Read any restart files
	 */
	for (std::map < std::string, int >::iterator it = this->FileMap.begin();
		 it != this->FileMap.end(); it++)
	{
		int
			ifile = -100 - it->second;

		// use gsztream
		igzstream myfile;
		myfile.open(it->first.c_str());
		if (!myfile.good())

		{
			std::ostringstream errstr;
			errstr << "File could not be opened: " << it->first.c_str();
			error_msg(errstr.str().c_str());
			break;
		}

		std::ostringstream oss;
		CParser	cparser(myfile, this->Get_io());
		cparser.set_echo_file(CParser::EO_NONE);
		cparser.set_echo_stream(CParser::EO_NONE);

		// skip headers
		while (cparser.check_line("restart", false, true, true, false) ==
			   PHRQ_io::LT_EMPTY);

		// read number of lines of index
		int
			n = -1;
		if (!(cparser.get_iss() >> n) || n < 4)
		{
			std::ostringstream errstr;
			errstr << "File does not have node locations: " << it->first.c_str() << "\nPerhaps it is an old format restart file.";
			error_msg(errstr.str().c_str());
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
		//int * index = (int *) PHRQ_malloc((size_t) (n * 7 * sizeof(int)));
		int * index = new int( (n * 7 * sizeof(int)) );

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

			cparser.get_iss() >> index[i * 7];
			cparser.get_iss() >> index[i * 7 + 1];
			cparser.get_iss() >> index[i * 7 + 2];
			cparser.get_iss() >> index[i * 7 + 3];
			cparser.get_iss() >> index[i * 7 + 4];
			cparser.get_iss() >> index[i * 7 + 5];
			cparser.get_iss() >> index[i * 7 + 6];
		}
		KDtree index_tree(pts);

		cxxStorageBin tempBin;
		tempBin.read_raw(cparser);

		for (j = 0; j < this->count_chem; j++)	/* j is count_chem number */
		{
			//i = back[j].list[0];	/* i is nxyz number */
			i = this->back[j][0];
			Point p(this->x_node[i], this->y_node[i], this->z_node[i]);
			int	k = (int) index_tree.Interpolate3d(p);	// k is index number in tempBin

			// solution
			if (initial_conditions1[i * 7] == ifile)
			{
				if (index[k * 7] != -1)	// entity k should be defined in tempBin
				{
					if (tempBin.Get_Solution(k) != NULL)
					{
						this->sz_bin.Set_Solution(j, tempBin.Get_Solution(k));
					}
					else
					{
						initial_conditions1[7 * i] = -1;
					}
				}
			}

			// PPassemblage
			if (initial_conditions1[i * 7 + 1] == ifile)
			{
				if (index[k * 7 + 1] != -1)	// entity k should be defined in tempBin
				{
					if (tempBin.Get_PPassemblage(k) != NULL)
					{
						this->sz_bin.Set_PPassemblage(j, tempBin.Get_PPassemblage(k));
					}
					else
					{
						initial_conditions1[7 * i + 1] = -1;
					}
				}
			}

			// Exchange
			if (initial_conditions1[i * 7 + 2] == ifile)
			{
				if (index[k * 7 + 2] != -1)	// entity k should be defined in tempBin
				{
					if (tempBin.Get_Exchange(k) != NULL)
					{
						this->sz_bin.Set_Exchange(j, tempBin.Get_Exchange(k));
					}
					else
					{
						initial_conditions1[7 * i + 2] = -1;
					}
				}
			}

			// Surface
			if (initial_conditions1[i * 7 + 3] == ifile)
			{
				if (index[k * 7 + 3] != -1)	// entity k should be defined in tempBin
				{
					if (tempBin.Get_Surface(k) != NULL)
					{
						this->sz_bin.Set_Surface(j, tempBin.Get_Surface(k));
					}
					else
					{
						initial_conditions1[7 * i + 3] = -1;
					}
				}
			}

			// Gas phase
			if (initial_conditions1[i * 7 + 4] == ifile)
			{
				if (index[k * 7 + 4] != -1)	// entity k should be defined in tempBin
				{
					if (tempBin.Get_GasPhase(k) != NULL)
					{
						this->sz_bin.Set_GasPhase(j, tempBin.Get_GasPhase(k));
					}
					else
					{
						initial_conditions1[7 * i + 4] = -1;
					}
				}
			}

			// Solid solution
			if (initial_conditions1[i * 7 + 5] == ifile)
			{
				if (index[k * 7 + 5] != -1)	// entity k should be defined in tempBin
				{
					if (tempBin.Get_SSassemblage(k) != NULL)
					{
						this->sz_bin.Set_SSassemblage(j, tempBin.Get_SSassemblage(k));
					}
					else
					{
						initial_conditions1[7 * i + 5] = -1;
					}
				}
			}

			// Kinetics
			if (initial_conditions1[i * 7 + 6] == ifile)
			{
				if (index[k * 7 + 6] != -1)	// entity k should be defined in tempBin
				{
					if (tempBin.Get_Kinetics(k) != NULL)
					{
						this->sz_bin.Set_Kinetics(j, tempBin.Get_Kinetics(k));
					}
					else
					{
						initial_conditions1[7 * i + 6] = -1;
					}
				}
			}
		}
		myfile.close();
		delete index;
	}
}
#endif
/* ---------------------------------------------------------------------- */
void
Reaction_module::Forward_and_back(int *initial_conditions, int *naxes)
/* ---------------------------------------------------------------------- */
{
/*
 *   calculate mapping from full set of cells to subset needed for chemistry
 */
	int
		i,
		n,
		ii,
		jj,
		kk;

	bool axes[3];
	axes[0] = naxes[0] != 0;
	axes[1] = naxes[1] != 0;
	axes[2] = naxes[2] != 0;

	this->count_chem = 1;

	int ixy = this->nx * this->ny;
	int ixz = this->nx * this->nz;
	int iyz = this->ny * this->nz;
	int ixyz = this->nxyz;

	if (!axes[0] && !axes[1] && !axes[2])
	{
		error_msg("No active coordinate direction in DIMENSIONS keyword.", 1);
	}
	if (axes[0])
		this->count_chem *= this->nx;
	if (axes[1])
		this->count_chem *= this->ny;
	if (axes[2])
		this->count_chem *= this->nz;
/*
 *   malloc space
 */
	this->forward.reserve(ixyz);
/*
 *   xyz domain
 */
	if (axes[0] && axes[1] && (axes[2]))
	{
		n = 0;
		for (i = 0; i < ixyz; i++)
		{
			if (initial_conditions[7 * i] >= 0
				|| initial_conditions[7 * i] <= -100)
			{
				//this->forward[i] = n;
				this->forward.push_back(n);

				std::vector <int> temp;
				this->back.push_back(temp);
				this->back[n].push_back(i);
				n++;
			}
			else
			{
				//this->forward[i] = -1;
				this->forward.push_back(-1);
			}
		}
		this->count_chem = n;
	}
/*
 *   Copy xy plane
 */
	else if (axes[0] && axes[1] && !axes[2])
	{
		if (this->nz != 2)
		{
			std::ostringstream errstr;
			errstr << "Z direction should contain only two nodes for this 2D problem." ;
			error_msg(errstr.str().c_str(), 1);
		}
		n = 0;
		for (i = 0; i < ixyz; i++)
		{
			this->n_to_ijk(i, ii, jj, kk);
			if (kk == 0 && (initial_conditions[7 * i] >= 0 || initial_conditions[7 * i] <= -100) )
			{
				this->forward.push_back(n);

				std::vector <int> temp;
				this->back.push_back(temp);
				this->back[n].push_back(i);
				this->back[n].push_back(i + ixy);
				n++;
			}
			else
			{
				this->forward.push_back(-1);
			}
		}
		this->count_chem = n;
	}
/*
 *   Copy xz plane
 */
	else if (axes[0] && !axes[1] && axes[2])
	{
		if (this->ny != 2)
		{
			std::ostringstream errstr;
			errstr << "Y direction should contain only two nodes for this 2D problem." ;
			error_msg(errstr.str().c_str(), 1);
		}
		n = 0;
		for (i = 0; i < ixyz; i++)
		{
			this->n_to_ijk(i, ii, jj, kk);
			if (jj == 0	&& (initial_conditions[7 * i] >= 0 || initial_conditions[7 * i] <= -100))
			{
				this->forward.push_back(n);

				std::vector <int> temp;
				this->back.push_back(temp);
				this->back[n].push_back(i);
				this->back[n].push_back(i + this->nx);
				n++;
			}
			else
			{
				this->forward.push_back(-1);
			}
		}
		this->count_chem = n;
	}
/*
 *   Copy yz plane
 */
	else if (!axes[0] && axes[1] && axes[2])
	{
		if (this->nx != 2)
		{
			std::ostringstream errstr;
			errstr << "X direction should contain only two nodes for this 2D problem." ;
			error_msg(errstr.str().c_str(), 1);
		}

		n = 0;
		for (i = 0; i < ixyz; i++)
		{
			this->n_to_ijk(i, ii, jj, kk);
			if (ii == 0	&& (initial_conditions[7 * i] >= 0 || initial_conditions[7 * i] <= -100))
			{
				this->forward.push_back(n);

				std::vector <int> temp;
				this->back.push_back(temp);
				this->back[n].push_back(i);
				this->back[n].push_back(i + 1);
				n++;
			}
			else
			{
				this->forward.push_back(-1);
			}
		}
		this->count_chem = n;
	}
/*
 *   Copy x line
 */
	else if (axes[0] && !axes[1] && !axes[2])
	{
		if (this->ny != 2)
		{
			std::ostringstream errstr;
			errstr << "Y direction should contain only two nodes for this 1D problem." ;
			error_msg(errstr.str().c_str(), 1);
		}
		if (this->nz != 2)
		{
			std::ostringstream errstr;
			errstr << "Z direction should contain only two nodes for this 1D problem." ;
			error_msg(errstr.str().c_str(), 1);
		}

		n = 0;
		for (i = 0; i < ixyz; i++)
		{
			if (initial_conditions[i * 7] < 0 && initial_conditions[7 * i] > -100)
			{
				std::ostringstream errstr;
				errstr << "Cannot have inactive cells in a 1D simulation.";
				error_msg(errstr.str().c_str(), 1);
			}
			this->n_to_ijk(i, ii, jj, kk);
			if (jj == 0 && kk == 0)
			{
				this->forward.push_back(n);

				std::vector <int> temp;
				this->back.push_back(temp);
				this->back[n].push_back(i);
				this->back[n].push_back(i + this->nx);
				this->back[n].push_back(i + ixy);
				this->back[n].push_back(i + ixy + this->nx);
				n++;
			}
			else
			{
				this->forward.push_back(-1);
			}
		}
		this->count_chem = n;
	}
/*
 *   Copy y line
 */
	else if (!axes[0] && axes[1] && !axes[2])
	{
		if (this->nx != 2)
		{
			std::ostringstream errstr;
			errstr << "X direction should contain only two nodes for this 1D problem." ;
			error_msg(errstr.str().c_str(), 1);
		}
		if (this->nz != 2)
		{
			std::ostringstream errstr;
			errstr << "Z direction should contain only two nodes for this 1D problem." ;
			error_msg(errstr.str().c_str(), 1);
		}

		n = 0;
		for (i = 0; i < ixyz; i++)
		{
			if (initial_conditions[i * 7] < 0 && initial_conditions[7 * i] > -100)
			{
				std::ostringstream errstr;
				errstr << "Cannot have inactive cells in a 1D simulation.";
				error_msg(errstr.str().c_str(), 1);
			}
			this->n_to_ijk(i, ii, jj, kk);
			if (ii == 0 && kk == 0)
			{
				this->forward.push_back(n);

				std::vector <int> temp;
				this->back.push_back(temp);
				this->back[n].push_back(i);
				this->back[n].push_back(i + 1);
				this->back[n].push_back(i + ixy);
				this->back[n].push_back(i + ixy + 1);
				n++;
			}
			else
			{
				this->forward.push_back(-1);
			}
		}
		this->count_chem = n;
	}
/*
 *   Copy z line
 */
	else if (!axes[0] && !axes[1] && axes[2])
	{
		if (this->nx != 2)
		{
			std::ostringstream errstr;
			errstr << "X direction should contain only two nodes for this 1D problem." ;
			error_msg(errstr.str().c_str(), 1);
		}
		if (this->ny != 2)
		{
			std::ostringstream errstr;
			errstr << "Y direction should contain only two nodes for this 1D problem." ;
			error_msg(errstr.str().c_str(), 1);
		}
		n = 0;
		for (i = 0; i < ixyz; i++)
		{
			if (initial_conditions[i * 7] < 0 && initial_conditions[7 * i] > -100)
			{
				std::ostringstream errstr;
				errstr << "Cannot have inactive cells in a 1D simulation.";
				error_msg(errstr.str().c_str(), 1);
			}
			this->n_to_ijk(i, ii, jj, kk);
			if (ii == 0 && jj == 0)
			{
				this->forward.push_back(n);
				
				std::vector <int> temp;
				this->back.push_back(temp);
				this->back[n].push_back(i);
				this->back[n].push_back(i + 1);
				this->back[n].push_back(i + this->nx);
				this->back[n].push_back(i + this->nx + 1);
				n++;
			}
			else
			{
				this->forward.push_back(-1);
			}
		}
		this->count_chem = n;
	}
	return;
}
/* ---------------------------------------------------------------------- */
bool
Reaction_module::n_to_ijk(int n, int &i, int &j, int &k) 
/* ---------------------------------------------------------------------- */
{

	k = n / (this->nx * this->ny) ;
	j = (n % (this->nx * this->ny)) / this->nx;
	i = (n % (this->nx * this->ny)) % this->nx;

	if (k < 0 || k >= this->nz)
	{
		error_msg("Z index out of range");
		return false;
	}
	if (j < 0 || j >= this->ny)
	{
		error_msg("Y index out of range");
		return false;
	}
	if (i < 0 || i >= this->nx)
	{
		error_msg("X index out of range");
		return false;
	}
	return true;
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::Cell_initialize(
                    std::ostringstream & mix_string,
					std::ostringstream & copy_string,
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
					double porosity_factor)
/* ---------------------------------------------------------------------- */
{
	int n_old1, n_old2;
	double f1;



	
	mix_string << "DELETE; -cell " << -n_user_new << "; END\n";
	/*
	 *   Copy solution
	 */
	n_old1 = initial_conditions1[7 * i];
	n_old2 = initial_conditions2[7 * i];
	f1 = fraction1[7 * i];
	if (n_old1 >= 0)
	{
		mix_string << "MIX_SOLUTION " << -n_user_new << "\n";
		mix_string << n_old1 << "  " << f1 << "\n";
		if (n_old2 >= 0)
		{
			mix_string << n_old2 << "  " << 1 - f1 << "\n";
		}
	}
	/*
	 *   Copy pp_assemblage
	 */
	n_old1 = initial_conditions1[7 * i + 1];
	n_old2 = initial_conditions2[7 * i + 1];
	f1 = fraction1[7 * i + 1];

	if (n_old1 >= 0)
	{
		LDBLE pf = 1.0;
		if (ppassemblage_units == 2)
		{
			pf = porosity_factor;
		}
		mix_string << "MIX_EQUILIBRIUM_PHASES " << -n_user_new << "\n";
		mix_string << n_old1 << "  " << f1*pf << "\n";
		if (n_old2 >= 0)
		{
			mix_string << n_old2 << "  " << (1 - f1)*pf << "\n";
		}
	}
	/*
	 *   Copy exchange assemblage
	 */

	n_old1 = initial_conditions1[7 * i + 2];
	n_old2 = initial_conditions2[7 * i + 2];
	f1 = fraction1[7 * i + 2];
	if (n_old1 >= 0)
	{
		LDBLE pf = 1.0;
		if (exchange_units == 2)
		{
			pf = porosity_factor;
		}
		mix_string << "MIX_EXCHANGE " << -n_user_new << "\n";
		mix_string << n_old1 << "  " << f1*pf << "\n";
		if (n_old2 >= 0)
		{
			mix_string << n_old2 << "  " << (1 - f1)*pf << "\n";
		}
	}
	/*
	 *   Copy surface assemblage
	 */
	n_old1 = initial_conditions1[7 * i + 3];
	n_old2 = initial_conditions2[7 * i + 3];
	f1 = fraction1[7 * i + 3];
	if (n_old1 >= 0)
	{
		LDBLE pf = 1.0;
		if (surface_units == 2)
		{
			pf = porosity_factor;
		}
		mix_string << "MIX_SURFACE " << -n_user_new << "\n";
		mix_string << n_old1 << "  " << f1*pf << "\n";
		if (n_old2 >= 0)
		{
			mix_string << n_old2 << "  " << (1 - f1)*pf << "\n";
		}
	}
	/*
	 *   Copy gas phase
	 */
	n_old1 = initial_conditions1[7 * i + 4];
	n_old2 = initial_conditions2[7 * i + 4];
	f1 = fraction1[7 * i + 4];
	if (n_old1 >= 0)
	{
		LDBLE pf = 1.0;
		if (gasphase_units == 2)
		{
			pf = porosity_factor;
		}
		mix_string << "MIX_GAS_PHASE " << -n_user_new << "\n";
		mix_string << n_old1 << "  " << f1*pf << "\n";
		if (n_old2 >= 0)
		{
			mix_string << n_old2 << "  " << (1 - f1)*pf << "\n";
		}
	}
	/*
	 *   Copy solid solution
	 */
	n_old1 = initial_conditions1[7 * i + 5];
	n_old2 = initial_conditions2[7 * i + 5];
	f1 = fraction1[7 * i + 5];
	if (n_old1 >= 0)
	{
		LDBLE pf = 1.0;
		if (ssassemblage_units == 2)
		{
			pf = porosity_factor;
		}
		mix_string << "MIX_SOLID_SOLUTION " << -n_user_new << "\n";
		mix_string << n_old1 << "  " << f1*pf << "\n";
		if (n_old2 >= 0)
		{
			mix_string << n_old2 << "  " << (1 - f1)*pf << "\n";
		}
	}
	/*
	 *   Copy kinetics
	 */
	n_old1 = initial_conditions1[7 * i + 6];
	n_old2 = initial_conditions2[7 * i + 6];
	f1 = fraction1[7 * i + 6];
	if (n_old1 >= 0)
	{
		LDBLE pf = 1.0;
		if (kinetics_units == 2)
		{
			pf = porosity_factor;
		}
		mix_string << "MIX_KINETICS " << -n_user_new << "\n";
		mix_string << n_old1 << "  " << f1*pf << "\n";
		if (n_old2 >= 0)
		{
			mix_string << n_old2 << "  " << (1 - f1)*pf << "\n";
		}
	}

	copy_string << "COPY cell " << -n_user_new << "  " << n_user_new << "\n";
	copy_string << "DELETE; -cell " << -n_user_new << ";END\n";

	return;
}

#ifdef SKIP
/* ---------------------------------------------------------------------- */
void
Reaction_module::System_initialize(
                    int ip_id,
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
					double porosity_factor)
/* ---------------------------------------------------------------------- */
{
	int n_old1, n_old2;
	double f1;

	/*
	 *   Copy solution
	 */
	n_old1 = initial_conditions1[7 * i];
	n_old2 = initial_conditions2[7 * i];
	f1 = fraction1[7 * i];
	if (n_old1 >= 0)
	{
		cxxMix mx;
		mx.Add(n_old1, f1);
		if (n_old2 >= 0)
			mx.Add(n_old2, 1 - f1);
		cxxSolution cxxsoln(this->phreeqc_bin.Get_Solutions(), mx, n_user_new);
		this->sz_bin.Set_Solution(n_user_new, &cxxsoln);
	}

	/*
	 *   Copy pp_assemblage
	 */
	n_old1 = initial_conditions1[7 * i + 1];
	n_old2 = initial_conditions2[7 * i + 1];
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
		cxxPPassemblage cxxentity(this->phreeqc_bin.Get_PPassemblages(), mx,
								  n_user_new);
		this->sz_bin.Set_PPassemblage(n_user_new, &cxxentity);
	}
	/*
	 *   Copy exchange assemblage
	 */

	n_old1 = initial_conditions1[7 * i + 2];
	n_old2 = initial_conditions2[7 * i + 2];
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
		cxxExchange cxxexch(this->phreeqc_bin.Get_Exchangers(), mx, n_user_new);
		this->sz_bin.Set_Exchange(n_user_new, &cxxexch);
	}
	/*
	 *   Copy surface assemblage
	 */
	n_old1 = initial_conditions1[7 * i + 3];
	n_old2 = initial_conditions2[7 * i + 3];
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
		cxxSurface cxxentity(this->phreeqc_bin.Get_Surfaces(), mx, n_user_new);
		this->sz_bin.Set_Surface(n_user_new, &cxxentity);
	}
	/*
	 *   Copy gas phase
	 */
	n_old1 = initial_conditions1[7 * i + 4];
	n_old2 = initial_conditions2[7 * i + 4];
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
		cxxGasPhase cxxentity(this->phreeqc_bin.Get_GasPhases(), mx, n_user_new);
		this->sz_bin.Set_GasPhase(n_user_new, &cxxentity);
	}
	/*
	 *   Copy solid solution
	 */
	n_old1 = initial_conditions1[7 * i + 5];
	n_old2 = initial_conditions2[7 * i + 5];
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
		cxxSSassemblage cxxentity(this->phreeqc_bin.Get_SSassemblages(), mx,
								  n_user_new);
		this->sz_bin.Set_SSassemblage(n_user_new, &cxxentity);
	}
	/*
	 *   Copy kinetics
	 */
	n_old1 = initial_conditions1[7 * i + 6];
	n_old2 = initial_conditions2[7 * i + 6];
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
		cxxKinetics cxxentity(this->phreeqc_bin.Get_Kinetics(), mx, n_user_new);
		this->sz_bin.Set_Kinetics(n_user_new, &cxxentity);
	}

	return;
}
#endif
/* ---------------------------------------------------------------------- */
void
Reaction_module::Get_components(
					int *n_comp,		// returns number of components including H, O, charge
					char *names,		// array of component names
					int length)			// length of each component name from Fortran
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
	size_t count_components = this->phast_iphreeqc_worker->GetComponentCount();
	size_t i;
	for (i = 0; i < count_components; i++)
	{
		std::string comp(this->phast_iphreeqc_worker->GetComponent((int) i));
		assert (comp != "H");
		assert (comp != "O");
		assert (comp != "Charge");
		assert (comp != "charge");

		this->components.push_back(comp);
		char * ptr = &names[(i+3) * length];
		strncpy(ptr, comp.c_str(), length);
	}
	*n_comp = (int) this->components.size();

	// Calculate gfw for components
	for (i = 0; i < components.size(); i++)
	{
		if (components[i] == "Charge")
		{
			this->gfw.push_back(1.0);
		}
		else
		{
			this->gfw.push_back(this->phast_iphreeqc_worker->Get_gfw(components[i].c_str()));
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
	}
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::Fractions2Solutions(void)
/* ---------------------------------------------------------------------- */
{
	size_t i, j, k;

	std::vector<double> d;  // scratch space to convert from mass fraction to moles

	for (i = 0; i < (size_t) this->nxyz; i++)
	{
		// j is count_chem number
		j = this->forward[i];
		if (j < 0) continue;

		// get mass fractions and store as moles in d
		double *ptr = &this->fraction[i];
		for (k = 0; k < this->components.size(); k++)
		{	
			d.push_back(ptr[this->nxyz * k] * 1000.0/this->gfw[k]);
		}

		// update solution sz_bin solution
		cxxNameDouble nd;
		nd.add("H", d[0] + 2.0/gfw_water);
		nd.add("O", d[1] + 1.0/gfw_water);
		nd.add("Charge", d[2]);

		for (k = 3; k < components.size(); k++)
		{
			if (d[k] <= 1e-14) d[k] = 0.0;
			nd.add(components[k].c_str(), d[k]);
		}	
		this->sz_bin.Get_Solution((int) j)->Update(nd);
	}
	return;
}
#ifdef SKIP
/* ---------------------------------------------------------------------- */
void
Reaction_module::Unpack_fraction_array(void)
/* ---------------------------------------------------------------------- */
{
	size_t i, j, k;

	std::vector<double> d;  // scratch space to convert from mass fraction to moles

	for (i = 0; i < (size_t) this->nxyz; i++)
	{
		// j is count_chem number
		j = this->forward[i];
		if (j < 0) continue;

		// get mass fractions and store as moles in d
		double *ptr = &this->fraction[i];
		for (k = 0; k < this->components.size(); k++)
		{	
			d.push_back(ptr[this->nxyz * k] * 1000.0/this->gfw[k]);
		}

		// update solution sz_bin solution
		cxxNameDouble nd;
		nd.add("H", d[0] + 2.0/gfw_water);
		nd.add("O", d[1] + 1.0/gfw_water);
		nd.add("Charge", d[2]);

		for (k = 3; k < components.size(); k++)
		{
			if (d[k] <= 1e-14) d[k] = 0.0;
			nd.add(components[k].c_str(), d[k]);
		}	
		this->sz_bin.Get_Solution((int) j)->Update(nd);
	}
	return;
}
#endif
#ifdef SKIP
/* ---------------------------------------------------------------------- */
void
Reaction_module::Unpack_fraction_array(void)
/* ---------------------------------------------------------------------- */
{
	size_t i, j, k;

	std::vector<double> d;  // scratch space to convert from mass fraction to moles

	for (i = 0; i < (size_t) this->nxyz; i++)
	{
		// j is count_chem number
		j = this->forward[i];
		if (j < 0) continue;

		// get mass fractions and store as moles in d
		double *ptr = &this->fraction[i];
		for (k = 0; k < this->components.size(); k++)
		{	
			d.push_back(ptr[this->nxyz * k] * 1000.0/this->gfw[k]);
		}

		// update solution sz_bin solution
		cxxNameDouble nd;
		nd.add("H", d[0] + 2.0/gfw_water);
		nd.add("O", d[1] + 1.0/gfw_water);
		nd.add("Charge", d[2]);

		for (k = 3; k < components.size(); k++)
		{
			if (d[k] <= 1e-14) d[k] = 0.0;
			nd.add(components[k].c_str(), d[k]);
		}	
		this->sz_bin.Get_Solution((int) j)->Update(nd);
	}
	return;
}
#endif
/* ---------------------------------------------------------------------- */
void
Reaction_module::Solutions2Fractions(void)
/* ---------------------------------------------------------------------- */
{
	// convert Reaction module solution data to hst mass fractions

	std::vector<double> d;  // scratch space to convert from moles to mass fraction
	cxxNameDouble::iterator it;

	int j; 

	for (j = 0; j < this->count_chem; j++)
	{
		// load fractions into d
		cxxSolution * cxxsoln_ptr = this->sz_bin.Get_Solution(j);
		this->cxxSolution2fraction(cxxsoln_ptr, d);

		// store in fraction at 1, 2, or 4 places depending on chemistry dimensions
		std::vector<int>::iterator it;
		for (it = this->back[j].begin(); it != this->back[j].end(); it++)
		{
			double *d_ptr = &this->fraction[*it];
			size_t i;
			for (i = 0; i < this->components.size(); i++)
			{
				d_ptr[this->nxyz * i] = d[i];
			}
		}
	}
}
#ifdef SKIP
/* ---------------------------------------------------------------------- */
void
Reaction_module::Pack_fraction_array(void)
/* ---------------------------------------------------------------------- */
{

	std::vector<double> d;  // scratch space to convert from moles to mass fraction
	cxxNameDouble::iterator it;

	int j; 

	for (j = 0; j < this->count_chem; j++)
	{
		// load fractions into d
		cxxSolution * cxxsoln_ptr = this->sz_bin.Get_Solution(j);
		this->cxxSolution2fraction(cxxsoln_ptr, d);

		// store in fraction at 1, 2, or 4 places depending on chemistry dimensions
		std::vector<int>::iterator it;
		for (it = this->back[j].begin(); it != this->back[j].end(); it++)
		{
			double *d_ptr = &this->fraction[*it];
			size_t i;
			for (i = 0; i < this->components.size(); i++)
			{
				d_ptr[this->nxyz * i] = d[i];
			}
		}
	}
}
#endif
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
	nd.add("H", d[0] + 2.0/gfw_water);
	nd.add("O", d[1] + 1.0/gfw_water);
	nd.add("Charge", d[2]);

	for (k = 3; k < components.size(); k++)
	{
		if (d[k] <= 1e-14) d[k] = 0.0;
		nd.add(components[k].c_str(), d[k]);
	}	

	cxxSolution cxxsoln(this->Get_io());	
	cxxsoln.Update(nd);
	cxxStorageBin temp_bin;
	temp_bin.Set_Solution(1, &cxxsoln);

	// Copy all entities numbered 1 into IPhreeqc
	this->phast_iphreeqc_worker->Get_PhreeqcPtr()->cxxStorageBin2phreeqc(temp_bin, 1);
	std::string input;
	input.append("RUN_CELLS; -cell 1; SELECTED_OUTPUT; -reset false; -pH; -alkalinity; END");
	this->phast_iphreeqc_worker->RunString(input.c_str());

	VAR pvar;
	this->phast_iphreeqc_worker->GetSelectedOutputValue(1,0,&pvar);
	*pH = pvar.dVal;
	this->phast_iphreeqc_worker->GetSelectedOutputValue(1,1,&pvar);
	*alkalinity = pvar.dVal;

	// Alternatively
	//*pH = -(this->phast_iphreeqc_worker->Get_PhreeqcPtr()->s_hplus->la);
	//*alkalinity = this->phast_iphreeqc_worker->Get_PhreeqcPtr()->total_alkalinity / 
	//	this->phast_iphreeqc_worker->Get_PhreeqcPtr()->mass_water_aq_x;
	return;
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::setup_boundary_conditions(const int n_boundary, int *boundary_solution1,
						  int *boundary_solution2, double *fraction1,
						  double *boundary_fraction, int dim)
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
		cxxSolution2fraction(&cxxsoln, d);

		// Put mass fractions in boundary_fraction
		double *d_ptr = &boundary_fraction[i];
		size_t j;
		for (j = 0; j < components.size(); j++)
		{
			d_ptr[dim * j] = d[j];
		}
	}
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::cxxSolution2fraction(cxxSolution * cxxsoln_ptr, std::vector<double> & d)
/* ---------------------------------------------------------------------- */
{
	d.clear();

	d.push_back((cxxsoln_ptr->Get_total_h() - 2.0 / this->gfw_water) * this->gfw[0]/1000. ); 
	d.push_back((cxxsoln_ptr->Get_total_o() - 1.0 / this->gfw_water) * this->gfw[1]/1000.);
	d.push_back(cxxsoln_ptr->Get_cb() * this->gfw[2]/1000.);
	size_t i;
	for (i = 3; i < this->components.size(); i++)
	{
		d.push_back(cxxsoln_ptr->Get_total(components[i].c_str()) * this->gfw[i]/1000.);
	}
}

/* ---------------------------------------------------------------------- */
void
Reaction_module::Write_restart(void)
/* ---------------------------------------------------------------------- */
{

	std::string temp_name("temp_restart_file.gz");
	std::string name(this->file_prefix);
	name.append(".restart.gz");
	std::string backup_name(this->file_prefix);
	backup_name.append(".restart.backup.gz");

	// open file 
	ogzstream ofs_restart;
	ofs_restart.open(temp_name.c_str());
	if (!ofs_restart.good())
	{
		std::ostringstream errstr;
		errstr << "Temporary restart file could not be opened: " << temp_name;
		error_msg(errstr.str().c_str(), 1);	
	}

	// write header
	ofs_restart << "#PHAST restart file" << std::endl;
	time_t now = time(NULL);
	ofs_restart << "#Prefix: " << this->file_prefix << std::endl;
	ofs_restart << "#Date: " << ctime(&now);
	ofs_restart << "#Current model time: " << this->time_hst << std::endl;
	ofs_restart << "#nx, ny, nz: " << this->nx << ", " << this->ny << ", " << this->nz << std::
		endl;

	// write index
	int i, j;
	ofs_restart << count_chem << std::endl;
	for (j = 0; j < count_chem; j++)	/* j is count_chem number */
	{
		//i = back[j].list[0];	
		i = back[j][0];			/* i is nxyz number */
		ofs_restart << x_node[i] << "  " << y_node[i] << "  " <<
			z_node[i] << "  " << j << "  ";
		// solution 
		ofs_restart << ic1[7 * i] << "  ";
		// pp_assemblage
		ofs_restart << ic1[7 * i + 1] << "  ";
		// exchange
		ofs_restart << ic1[7 * i + 2] << "  ";
		// surface
		ofs_restart << ic1[7 * i + 3] << "  ";
		// gas_phase
		ofs_restart << ic1[7 * i + 4] << "  ";
		// solid solution
		ofs_restart << ic1[7 * i + 5] << "  ";
		// kinetics
		ofs_restart << ic1[7 * i + 6] << std::endl;
	}

	// write data
	sz_bin.dump_raw(ofs_restart, 0);
	ofs_restart.close();
	// rename files
	this->File_rename(temp_name.c_str(), name.c_str(), backup_name.c_str());
}
/* ---------------------------------------------------------------------- */
bool
Reaction_module::File_exists(const std::string name)
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
Reaction_module::File_rename(const std::string temp_name, const std::string name, 
	const std::string backup_name)
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
void
Reaction_module::Scale_cxxsystem(int iphrq, double frac)
/* ---------------------------------------------------------------------- */
{
	int n_user;

	/* 
	 * repartition solids for partially saturated cells
	 */

	n_user = iphrq;
	cxxMix cxxmix;
	cxxmix.Add(n_user, frac);
	/*
	 *   Scale compositions
	 */
	if (sz_bin.Get_Exchange(n_user) != NULL)
	{
		cxxExchange cxxexch(sz_bin.Get_Exchangers(), cxxmix, n_user);
		sz_bin.Set_Exchange(n_user, &cxxexch);
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
}

/* ---------------------------------------------------------------------- */
void
Reaction_module::Partition_uz(int iphrq, int ihst, double new_frac)
/* ---------------------------------------------------------------------- */
{
	int n_user;
	double s1, s2, uz1, uz2;

	/* 
	 * repartition solids for partially saturated cells
	 */

	if ((fabs(this->old_frac[ihst] - new_frac) <= 1e-8) ? true : false)
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
	else if (new_frac > this->old_frac[ihst])
	{
		/* wetting cell */
		uz1 = 0.;
		uz2 = (1.0 - new_frac) / (1.0 - this->old_frac[ihst]);
		s1 = 1.;
		s2 = 1.0 - uz2;
	}
	else
	{
		/* draining cell */
		s1 = new_frac / this->old_frac[ihst];
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

//Exchange
	if (this->sz_bin.Get_Exchange(n_user) != NULL)
	{
		cxxStorageBin tempBin;
		tempBin.Set_Exchange(0, this->sz_bin.Get_Exchange(n_user));
		tempBin.Set_Exchange(1, this->uz_bin.Get_Exchange(n_user));
		cxxExchange newsz(tempBin.Get_Exchangers(), szmix, n_user);
		cxxExchange newuz(tempBin.Get_Exchangers(), uzmix, n_user);
		this->sz_bin.Set_Exchange(n_user, &newsz);
		this->uz_bin.Set_Exchange(n_user, &newuz);
	}
//PPassemblage
	if (this->sz_bin.Get_PPassemblage(n_user) != NULL)
	{
		cxxStorageBin tempBin;
		tempBin.Set_PPassemblage(0, this->sz_bin.Get_PPassemblage(n_user));
		tempBin.Set_PPassemblage(1, this->uz_bin.Get_PPassemblage(n_user));
		cxxPPassemblage newsz(tempBin.Get_PPassemblages(), szmix, n_user);
		cxxPPassemblage newuz(tempBin.Get_PPassemblages(), uzmix, n_user);
		this->sz_bin.Set_PPassemblage(n_user, &newsz);
		this->uz_bin.Set_PPassemblage(n_user, &newuz);
	}
//Gas_phase
	if (this->sz_bin.Get_GasPhase(n_user) != NULL)
	{
		cxxStorageBin tempBin;
		tempBin.Set_GasPhase(0, this->sz_bin.Get_GasPhase(n_user));
		tempBin.Set_GasPhase(1, this->uz_bin.Get_GasPhase(n_user));
		cxxGasPhase newsz(tempBin.Get_GasPhases(), szmix, n_user);
		cxxGasPhase newuz(tempBin.Get_GasPhases(), uzmix, n_user);
		this->sz_bin.Set_GasPhase(n_user, &newsz);
		this->uz_bin.Set_GasPhase(n_user, &newuz);
	}
//SSassemblage
	if (this->sz_bin.Get_SSassemblage(n_user) != NULL)
	{
		cxxStorageBin tempBin;
		tempBin.Set_SSassemblage(0, this->sz_bin.Get_SSassemblage(n_user));
		tempBin.Set_SSassemblage(1, this->uz_bin.Get_SSassemblage(n_user));
		cxxSSassemblage newsz(tempBin.Get_SSassemblages(), szmix, n_user);
		cxxSSassemblage newuz(tempBin.Get_SSassemblages(), uzmix, n_user);
		this->sz_bin.Set_SSassemblage(n_user, &newsz);
		this->uz_bin.Set_SSassemblage(n_user, &newuz);
	}
//Kinetics
	if (this->sz_bin.Get_Kinetics(n_user) != NULL)
	{
		cxxStorageBin tempBin;
		tempBin.Set_Kinetics(0, this->sz_bin.Get_Kinetics(n_user));
		tempBin.Set_Kinetics(1, this->uz_bin.Get_Kinetics(n_user));
		cxxKinetics newsz(tempBin.Get_Kinetics(), szmix, n_user);
		cxxKinetics newuz(tempBin.Get_Kinetics(), uzmix, n_user);
		this->sz_bin.Set_Kinetics(n_user, &newsz);
		this->uz_bin.Set_Kinetics(n_user, &newuz);
	}
//Surface
	if (this->sz_bin.Get_Surface(n_user) != NULL)
	{
		cxxStorageBin tempBin;
		tempBin.Set_Surface(0, this->sz_bin.Get_Surface(n_user));
		tempBin.Set_Surface(1, this->uz_bin.Get_Surface(n_user));
		cxxSurface newsz(tempBin.Get_Surfaces(), szmix, n_user);
		cxxSurface newuz(tempBin.Get_Surfaces(), uzmix, n_user);
		this->sz_bin.Set_Surface(n_user, &newsz);
		this->uz_bin.Set_Surface(n_user, &newuz);
	}
	/*
	 *   Eliminate uz if new fraction 1.0
	 */
	if (new_frac >= 1.0)
	{
		this->uz_bin.Remove(iphrq);
	}

	this->old_frac[ihst] = new_frac;
}
/* ---------------------------------------------------------------------- */
void
Reaction_module::Init_uz(void)
/* ---------------------------------------------------------------------- */
{
	int	i;

	if (transient_free_surface)
	{
		for (i = 0; i < nxyz; i++)
		{
			old_frac.push_back(1.0);
		}
	}
}
#ifdef NEED_TO_IMPLEMENT
/* ---------------------------------------------------------------------- */
static void
EQUILIBRATE_SERIAL(double *fraction, int *dim, int *print_sel,
				   double *x_hst, double *y_hst, double *z_hst,
				   double *time_hst, double *time_step_hst, int *prslm,
				   double *cnvtmi, double *frac, int *printzone_chem,
				   int *printzone_xyz, int *print_out, int *print_hdf,
				   double *rebalance_fraction_hst, int *print_restart,
				   double *pv, double *pv0, int *steady_flow, double *volume)
/* ---------------------------------------------------------------------- */
{
/*
 *   Routine takes mass fractions from HST, equilibrates each cell,
 *   and returns new mass fractions to HST
 *
 *   Input: ixyz - number of cells in model
 *          dim - leading dimension of 2-d array fraction
 *
 *   Output: fraction - mass fractions of all components in all solutions
 *                      dimensions must be >= ixyz x n_comp
 */
	int
		i,
		j,
		tot_same,
		tot_iter,
		tot_zero,
		max_iter;
	int
		active;
	int
		n_user;
	LDBLE
		kin_time;
	static int
		write_headings = 1;

	pr.all = *print_out;
/*
 *   Update solution compositions
 */
	unpackcxx_from_hst(fraction, dim);
/*
 *   Calculate equilibrium
 */
	kin_time = *time_step_hst;
	timest = kin_time;

	state = PHAST;
	tot_same = 0;
	tot_iter = 0;
	tot_zero = 0;
	max_iter = 0;
	if (*print_sel == TRUE)
	{
		simulation++;
		pr.punch = TRUE;
	}
	else
	{
		pr.punch = FALSE;
	}
	pr.hdf = *print_hdf;

	BeginTimeStep(*print_sel, *print_out, *print_hdf);
	if (punch.in == TRUE && write_headings)
	{
		int
			pr_punch = pr.punch;
		pr.punch = TRUE;
		/*
		 * write headings
		 */
		punch.new_def = TRUE;
		output_msg(OUTPUT_PUNCH, "%15s\t%15s\t%15s\t%15s\t%2s\t", "x", "y",
				   "z", "time", "in");
		tidy_punch();
		write_headings = 0;
		pr.punch = pr_punch;
	}

	rate_sim_time_start = *time_hst - *time_step_hst;
	rate_sim_time_end = *time_hst;
	initial_total_time = 0;
	// free all c structures
	reinitialize();
	for (i = 0; i < count_chem; i++)
	{							/* i is count_chem number */
		j = back[i].list[0];	/* j is nxyz number */
		cell_pore_volume = pv0[j] * 1000.0 * frac[j];
		cell_volume = volume[j] * 1000.;
		cell_porosity = cell_pore_volume / cell_volume;
		cell_saturation = frac[j];
		/*
		   if (*time_hst > 0) {
		   std::ostringstream oss;
		   cxxSolution *cxxsoln_ptr = szBin.getSolution(i);
		   cxxsoln_ptr->dump_raw(oss,0);
		   std::cerr << oss.str();
		   }
		 */
		if (transient_free_surface == TRUE)
			partition_uz(i, j, frac[j]);
		if (frac[j] <= 1e-10)
			frac[j] = 0.0;
		// set flags
		active = FALSE;
		if (frac[j] > 0.0)
			active = TRUE;
		pr.all = FALSE;
		if (*print_out == TRUE && printzone_chem[j] == TRUE)
			pr.all = TRUE;
		pr.punch = FALSE;
		if (*print_sel == TRUE && printzone_xyz[j] == TRUE
			&& punch.in == TRUE)
			pr.punch = TRUE;

		BeginCell(*print_sel, *print_out, *print_hdf, j);

		if (pr.punch == TRUE)
		{
			output_msg(OUTPUT_PUNCH, "%15g\t%15g\t%15g\t%15g\t%2d\t",
					   x_hst[j], y_hst[j], z_hst[j], (*time_hst) * (*cnvtmi),
					   active);
			if (active == FALSE)
			{
				output_msg(OUTPUT_PUNCH, "\n");
			}
		}
		if (active)
		{
			cell_no = i;
			if (transient_free_surface == TRUE)
				scale_cxxsystem(i, 1.0 / frac[j]);
			if (transient_free_surface == FALSE && *steady_flow == FALSE)
			{
				if (pv0[j] != 0 && pv[j] != 0 && pv0[j] != pv[j])
				{
					cxxSolution *
						cxxsol = szBin.getSolution(i);
					cxxsol->multiply(pv[j] / pv0[j]);
				}
			}
			/*
			   std::ostringstream oss;
			   cxxSolution *cxxsoln_ptr = szBin.getSolution(i);
			   cxxsoln_ptr->dump_raw(oss,0);
			   std::cerr << oss.str();
			 */
			// copy cxx data to c structures
			szBin.cxxStorageBin2phreeqc(i);
			set_use_hst(i);
			n_user = i;
			set_initial_moles(n_user);
			run_reactions(n_user, kin_time, FALSE, 1.0);
			if (iterations == 0)
				tot_zero++;
			if (iterations > max_iter)
				max_iter = iterations;
			tot_same += same_model;
			tot_iter += iterations;
			sum_species();
			if (pr.all == TRUE)
			{
				output_msg(OUTPUT_MESSAGE,
						   "Time %g. Cell %d: x=%15g\ty=%15g\tz=%15g\n",
						   (*time_hst) * (*cnvtmi), j + 1, x_hst[j], y_hst[j],
						   z_hst[j]);
				print_using_hst(j + 1);
			}
			print_all();
			punch_all();
			/*
			 *   Save data
			 */
			solution_bsearch(i, &n_solution, TRUE);
			if (n_solution == -999)
			{
				error_msg("How did this happen?", STOP);
			}
			xsolution_save_hst(n_solution);
			if (save.exchange == TRUE)
			{
				exchange_bsearch(i, &n_exchange);
				xexchange_save_hst(n_exchange);
			}
			if (save.gas_phase == TRUE)
			{
				gas_phase_bsearch(i, &n_gas_phase);
				xgas_save_hst(n_gas_phase);
			}
			if (save.pp_assemblage == TRUE)
			{
				pp_assemblage_bsearch(i, &n_pp_assemblage);
				xpp_assemblage_save_hst(n_pp_assemblage);
			}
			if (save.surface == TRUE)
			{
				surface_bsearch(i, &n_surface);
				//xsurface_save_hst(n_surface);
				xsurface_save(i);
			}
			if (save.s_s_assemblage == TRUE)
			{
				s_s_assemblage_bsearch(i, &n_s_s_assemblage);
				xs_s_assemblage_save_hst(n_s_s_assemblage);
			}
			szBin.phreeqc2cxxStorageBin(i);
			if (transient_free_surface == TRUE)
				scale_cxxsystem(i, frac[j]);

			if (transient_free_surface == FALSE && *steady_flow == FALSE)
			{
				assert(pv0[j] != 0);
				assert(pv[j] != 0);
				if (pv0[j] != 0 && pv[j] != 0 && pv0[j] != pv[j])
				{
					cxxSolution *
						cxxsol = szBin.getSolution(i);
					cxxsol->multiply(pv0[j] / pv[j]);
				}
			}
		}
		else
		{
			if (pr.all == TRUE)
			{
				output_msg(OUTPUT_MESSAGE,
						   "Time %g. Cell %d: x=%15g\ty=%15g\tz=%15g\n",
						   (*time_hst) * (*cnvtmi), j + 1, x_hst[j], y_hst[j],
						   z_hst[j]);
				output_msg(OUTPUT_MESSAGE, "Cell is dry.\n");
			}
		}
		// free phreeqc structures
		reinitialize();
		EndCell(*print_sel, *print_out, *print_hdf, j);
	}
	/*
	 *  Write restart data
	 */
	if (*print_restart == 1)
		write_restart((*time_hst) * (*cnvtmi));

	EndTimeStep(*print_sel, *print_out, *print_hdf);
/*
 *   Put values back for HST
 */
	PACK_FOR_HST(fraction, dim);
/*
 *   Write screen and log messages
 */
	if (*prslm == TRUE)
	{
		sprintf(error_string, "          Total cells: %d", count_chem);
		output_msg(OUTPUT_SCREEN, "%s\n", error_string);
		output_msg(OUTPUT_ECHO, "%s\n", error_string);
		sprintf(error_string,
				"          Number of cells with same aqueous model: %d",
				tot_same);
		output_msg(OUTPUT_SCREEN, "%s\n", error_string);
		output_msg(OUTPUT_ECHO, "%s\n", error_string);
		sprintf(error_string, "          Total iterations all cells: %d",
				tot_iter);
		output_msg(OUTPUT_SCREEN, "%s\n", error_string);
		output_msg(OUTPUT_ECHO, "%s\n", error_string);
		sprintf(error_string,
				"          Number of cells with zero iterations: %d",
				tot_zero);
		output_msg(OUTPUT_SCREEN, "%s\n", error_string);
		output_msg(OUTPUT_ECHO, "%s\n", error_string);
		sprintf(error_string, "          Maximum iterations for one cell: %d",
				max_iter);
		output_msg(OUTPUT_SCREEN, "%s\n\n", error_string);
		output_msg(OUTPUT_ECHO, "%s\n\n", error_string);
	}
	return;
}
#endif
/* ---------------------------------------------------------------------- */
void
Reaction_module::Run_reactions()
/* ---------------------------------------------------------------------- */
{
/*
 *   Routine takes mass fractions from HST, equilibrates each cell,
 *   and returns new mass fractions to HST
 */
	// Do not write to files from phreeqc
	this->phast_iphreeqc_worker->SetLogFileOn(false);
	this->phast_iphreeqc_worker->SetSelectedOutputFileOn(false);
	this->phast_iphreeqc_worker->SetDumpFileOn(false);
	this->phast_iphreeqc_worker->SetOutputFileOn(false);

/*
 *   Update solution compositions in sz_bin
 */
	this->Fractions2Solutions();
	//this->Unpack_fraction_array();

	int i, j;
	for (i = 0; i < this->count_chem; i++)
	{							/* i is count_chem number */
		j = back[i][0];			/* j is nxyz number */

		// Set local print flags
		bool pr_chem = this->print_chem && (this->printzone_chem[j] != 0);
		bool pr_xyz = this->print_xyz && (this->printzone_xyz[i] != 0);
		bool pr_hdf = this->print_hdf;

		// partition solids between UZ and SZ
		if (transient_free_surface)	
			this->Partition_uz(i, j, frac[j]);

		// ignore small saturations
		bool active = false;
		if (frac[j] <= 1e-10) 
			frac[j] = 0.0;
		if (frac[j] > 0) 
			active = true;
		if (active)
		{
			// set cell number, pore volume functions for Basic
			this->phast_iphreeqc_worker->Set_cell_volumes(i, pv0[j], frac[j], volume[j]);

			// Adjust for fractional saturation and pore volume
			if (this->transient_free_surface)
				this->Scale_cxxsystem(i, 1.0 / frac[j]);
			if (!transient_free_surface && !steady_flow)
			{
				if (pv0[j] != 0 && pv[j] != 0 && pv0[j] != pv[j])
				{
					cxxSolution * cxxsol = sz_bin.Get_Solution(i);
					cxxsol->multiply(pv[j] / pv0[j]);
				}
			}

			// Move reactants from sz_bin to phreeqc
			this->phast_iphreeqc_worker->Set_cell(sz_bin, i);

			// TODO this->phast_iphreeqc_worker->SetOutputStringOn(pr_chem);

			// do the calculation
			std::ostringstream input;
			input << "RUN_CELLS" << std::endl;
			input << "  -start_time " << *this->time_hst << std::endl;
			input << "  -time_step  " << *this->time_step_hst << std::endl;
			input << "  -cells      " << i << std::endl;
			input << "END" << std::endl;

			this->phast_iphreeqc_worker->RunString(input.str().c_str());

			// Save reactants back in sz_bin
			this->phast_iphreeqc_worker->Get_cell(sz_bin, i);

			// Write chem.txt
			std::string header;
			if (pr_chem || pr_xyz)
			{
			}
			if (pr_chem)
			{
				std::ostringstream line;
				char chunk[150];
				sprintf(chunk, "Time %g. Cell %d: x=%15g\ty=%15g\tz=%15g\n",
						   (*time_hst) * (*cnvtmi), j + 1, x_node[j], y_node[j],
						   z_node[j]);
				line << chunk;
				//output_msg(OUTPUT_MESSAGE,
				//		   "Time %g. Cell %d: x=%15g\ty=%15g\tz=%15g\n",
				//		   (*time_hst) * (*cnvtmi), j + 1, x_hst[j], y_hst[j],
				//		   z_hst[j]);
				this->output_msg(line.str().c_str());  // TODO fix for PHAST
				// TODO? print_using_hst(j + 1);
				// TODO this->output_msg(this->phast_iphreeqc_worker->GetOutputString());
			}
			if (pr_xyz || pr_hdf)
			{
				std::vector<double> d;
				this->phast_iphreeqc_worker->Selected_out_to_double(1, d);


				std::vector<double>::iterator it;
				for (it = d.begin(); it != d.end(); it++)
				// Write chem.xyz			
				if (pr_xyz)
				{
					char chunk[150];
					sprintf(chunk, "%15g\t%15g\t%15g\t%15g\t%2d\t",
					   x_node[j], y_node[j], z_node[j], (*time_hst) * (*cnvtmi),
					  ((active) ? 1 : 0));
					std::ostringstream line;
					line << chunk;

					//output_msg(PHRQ_io::OUTPUT_PUNCH, "%15g\t%15g\t%15g\t%15g\t%2d\t",
					//   x_node[j], y_node[j], z_node[j], (*time_hst) * (*cnvtmi),
					//  ((active) ? 1 : 0));					
				}
				if (pr_hdf)
				{
				}
			}

		}


		if (this->print_xyz && this->printzone_xyz[i] != 0)
		{
			// need to capture selected output to xyz file
			sprintf(this->line_buffer, "%15g\t%15g\t%15g\t%15g\t%2d\t",
					   x_node[j], y_node[j], z_node[j], (*time_hst) * (*cnvtmi),
					   ((active) ? 1 : 0));
			this->io->punch_msg(line_buffer);
			if (active == FALSE)
			{
				this->io->punch_msg("\n");
			}
			else
			{
				// Go through VARs and write to XYZ file.
				std::vector<double> d;
				this->phast_iphreeqc_worker->Selected_out_to_double(1, d);
			}
		}

	}

}
/* ---------------------------------------------------------------------- */
void
Reaction_module::Send_restart_name(std::string name)
/* ---------------------------------------------------------------------- */
{
	int	i = (int) FileMap.size();
	FileMap[name] = i;
}

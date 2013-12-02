#include <windows.h>
#include <string>
#include <map>
#include <iostream>
#include "FileWriter.h"
#include "Reaction_module.h"
#include "RM_interface.h"
#include "H5Cpp.h"
#include "hdf.h"
#ifdef THREADED_PHAST
#include <omp.h>
#endif
#ifdef USE_MPI
#include "mpi.h"
#endif

#if defined(__cplusplus)
extern "C" {
#endif
extern void HDF_WRITE_INVARIANT(int *iso, int * mpi_myself);
extern void HDF_BEGIN_TIME_STEP(int *iso);
extern void HDF_END_TIME_STEP(int *iso);
#if defined(__cplusplus)
}
#endif
class FileWriterInfo: public PHRQ_base
{
public:
	FileWriterInfo();
	~FileWriterInfo(void);
	bool GetHDFInitialized(void) {return this->HDFInitialized;}
	void SetHDFInitialized(bool tf) {this->HDFInitialized = tf;}
	bool GetHDFInvariant(void) {return this->HDFInvariant;}
	void SetHDFInvariant(bool tf) {this->HDFInvariant = tf;}
	bool GetXYZInitialized(void) {return this->XYZInitialized;}
	void SetXYZInitialized(bool tf) {this->XYZInitialized = tf;}
	std::vector< std::ostream * > &GetXYZOstreams(void) {return this->XYZOstreams;}
	std::vector< std::string > &GetHeadings(void) {return this->Headings;}
	void SetHeadings(std::vector< std::string > &h) {this->Headings = h;}

protected:
	bool HDFInitialized;
	bool HDFInvariant;
	bool XYZInitialized;
	std::vector< std::string > Headings;
	std::vector < std::ostream * > XYZOstreams;
};
FileWriterInfo FileWriter;
// Constructor
FileWriterInfo::FileWriterInfo()
{
	this->io = new PHRQ_io;
	HDFInitialized = false;
	HDFInvariant = false;
	XYZInitialized = false;
}
// Destructor
FileWriterInfo::~FileWriterInfo()
{
	delete this->io;
}
/* ---------------------------------------------------------------------- */
void
WriteFiles(int *id, int *print_hdf, int *print_xyz, int *print_media,
	double *x_node, double *y_node, double *z_node, int *xyz_mask,
	double *saturation, int *mapping)
	/* ---------------------------------------------------------------------- */
{
	Reaction_module * Reaction_module_ptr = RM_interface::GetInstance(id);
	if (Reaction_module_ptr)
	{	
		int local_mpi_myself = RM_GetMpiMyself(id);
#ifdef USE_MPI
		int flags[2];
		if (local_mpi_myself == 0)
		{
			if (print_hdf == 0 || print_xyz == 0)
			{
				RM_ErrorMessage("Null pointer in WriteFiles");
				RM_Error(id);
			}
			flags[0] = *print_hdf;
			flags[1] = *print_xyz;
			MPI_Bcast(flags, 2, MPI_INT, 0, MPI_COMM_WORLD);
		}
		else
		{
			MPI_Bcast(flags, 2, MPI_INT, 0, MPI_COMM_WORLD);
		}
		*print_hdf = flags[0];
		*print_xyz = flags[1];
#endif
		if (*print_hdf != 0)
		{
			WriteHDF(id, print_hdf, print_media);
		}
		if (*print_xyz != 0)
		{
			WriteXYZ(id, print_xyz, x_node, y_node, z_node, 
				xyz_mask, saturation, mapping);
		}
	}
}

/* ---------------------------------------------------------------------- */
void
WriteHDF(int *id, int *print_hdf, int *print_media)
/* ---------------------------------------------------------------------- */
{
#ifdef HDF5_CREATE
	Reaction_module * Reaction_module_ptr = RM_interface::GetInstance(id);
	if (Reaction_module_ptr)
	{	
		int local_mpi_myself = RM_GetMpiMyself(id);
			
		int nso = RM_GetSelectedOutputCount(id);
		int nxyz = RM_GetSelectedOutputRowCount(id); 
		double current_time = RM_GetTimeConversion(id) * RM_GetTime(id);
		//
		// Initialize HDF
		//
		if (!FileWriter.GetHDFInitialized() && nso > 0 && *print_hdf != 0)
		{
			if (local_mpi_myself == 0)
			{
				for (int iso = 0; iso < nso; iso++)
				{
					int status;
					int n_user = RM_GetNthSelectedOutputUserNumber(id, &iso);
					if (n_user >= 0)
					{
						status = RM_SetCurrentSelectedOutputUserNumber(id, &n_user);
						if (status >= 0)
						{
							// open file
							char prefix[256];
							RM_GetFilePrefix(id, prefix, 256);
							std::ostringstream filename;
							filename << prefix << "_" << n_user;
							HDFInitialize(iso, filename.str().c_str(), (int) strlen(filename.str().c_str()));

							// Set HDF scalars
							std::vector < std::string > headings;
							int ncol = RM_GetSelectedOutputColumnCount(id);
							for (int icol = 0; icol < ncol; icol++)
							{
								char head[100];
								status = RM_GetSelectedOutputHeading(id, &icol, head, 100);
								headings.push_back(head);
							}
							HDFSetScalarNames(iso, headings);
						}
					}
				}
				FileWriter.SetHDFInitialized(true);
			}
		}
		//	
		// Write H5 file
		//
		if (*print_hdf != 0)
		{
			std::vector<double> local_selected_out;
			int status;
			for (int iso = 0; iso < nso; iso++)
			{
				int n_user = RM_GetNthSelectedOutputUserNumber(id, &iso);
				if (n_user >= 0)
				{
					status = RM_SetCurrentSelectedOutputUserNumber(id, &n_user);
					int ncol = RM_GetSelectedOutputColumnCount(id);
					if (status >= 0)
					{
						if (local_mpi_myself == 0)
						{
							local_selected_out.resize((size_t) (nxyz*ncol));
							int so_error = RM_GetSelectedOutput(id, local_selected_out.data());
							if ( !FileWriter.GetHDFInvariant())
							{
								HDF_WRITE_INVARIANT(&iso, &local_mpi_myself);
							}
							// Now write HDF file
							HDF_BEGIN_TIME_STEP(&iso);
							HDFBeginCTimeStep(iso);
							HDFFillHyperSlab(iso, local_selected_out, ncol);
							HDFEndCTimeStep(iso);
							HDF_END_TIME_STEP(&iso);
						}
						else
						{
							int so_error = RM_GetSelectedOutput(id, local_selected_out.data());
						}
					}
				}
			}
			*print_media = 0;
			FileWriter.SetHDFInvariant(true);
		}
	}
#endif
}

/* ---------------------------------------------------------------------- */
void
WriteXYZ(int *id, int *print_xyz, 
	double *x_node, double *y_node, double *z_node, int *xyz_mask,
	double *saturation, int *mapping)
/* ---------------------------------------------------------------------- */
{
	Reaction_module * Reaction_module_ptr = RM_interface::GetInstance(id);
	if (Reaction_module_ptr)
	{	
		int local_mpi_myself = RM_GetMpiMyself(id);
			
		int nso = RM_GetSelectedOutputCount(id);
		int nxyz = RM_GetSelectedOutputRowCount(id); 
		double current_time = RM_GetTimeConversion(id) * RM_GetTime(id);
		//
		// Initialize XYZ
		//
		if (!FileWriter.GetXYZInitialized() && nso > 0 && *print_xyz != 0)
		{
			if (local_mpi_myself == 0)
			{
				for (int iso = 0; iso < nso; iso++)
				{
					int status;
					int n_user = RM_GetNthSelectedOutputUserNumber(id, &iso);
					if (n_user >= 0)
					{
						status = RM_SetCurrentSelectedOutputUserNumber(id, &n_user);
						if (status >= 0)
						{
							// open file							
							char prefix[256];
							RM_GetFilePrefix(id, prefix, 256);
							std::ostringstream filename;
							filename << prefix << "_" << n_user << ".chem.xyz.tsv";
							if (!FileWriter.Get_io()->punch_open(filename.str().c_str()))
							{
								RM_ErrorMessage("Could not open xyz file.");
								RM_Error(id);
							}
							FileWriter.GetXYZOstreams().push_back(FileWriter.Get_io()->Get_punch_ostream());
							// write first headings
							char line_buff[132];
							sprintf(line_buff, "%15s\t%15s\t%15s\t%15s\t%2s\t", "x", "y",
								"z", "time", "in");
							FileWriter.Get_io()->punch_msg(line_buff);
							
							// create chemistry headings
							int ncol = RM_GetSelectedOutputColumnCount(id);
							std::ostringstream h;
							for (int icol = 0; icol < ncol; icol++)
							{
								char head[100];
								status = RM_GetSelectedOutputHeading(id, &icol, head, 100);
								std::string s(head);
								s.append("\t");
								h.width(20);
								h << s;
							}
							h << "\n";
							FileWriter.Get_io()->punch_msg(h.str().c_str());
						}
					}
				}
				FileWriter.SetXYZInitialized(true);
			}
		}
		//	
		// Write XYZ file
		//
		if (*print_xyz != 0)
		{
			std::vector<double> local_selected_out;
			int status;
			for (int iso = 0; iso < nso; iso++)
			{
				int n_user = RM_GetNthSelectedOutputUserNumber(id, &iso);
				if (n_user >= 0)
				{
					status = RM_SetCurrentSelectedOutputUserNumber(id, &n_user);
					int ncol = RM_GetSelectedOutputColumnCount(id);
					if (status >= 0)
					{
						if (local_mpi_myself == 0)
						{
							FileWriter.Get_io()->Set_punch_ostream(FileWriter.GetXYZOstreams()[iso]);
							local_selected_out.resize((size_t) (nxyz*ncol));
							int so_error = RM_GetSelectedOutput(id, local_selected_out.data());

							// write xyz file
							for (int irow = 0; irow < nxyz; irow++)
							{
								if (xyz_mask[irow] <= 0) continue;
								int active = 1;
								if (mapping[irow] < 0 || saturation[irow] <= 0)
								{
									active = 0;
								}

								// write x,y,z
								std::ostringstream ln;

								char line_buff[132];
								sprintf(line_buff, "%15g\t%15g\t%15g\t%15g\t%2d\t",
									x_node[irow], y_node[irow], z_node[irow], current_time,
									active);
								ln << line_buff;
								
								if (active)
								{
									// write chemistry values
									char token[21];
									for (int jcol = 0; jcol < ncol; jcol++)
									{		
										sprintf(token,"%19.10e\t", local_selected_out[jcol * nxyz + irow]);
										ln.width(20);
										ln << token;
									}
								}
								ln << "\n";
								FileWriter.Get_io()->punch_msg(ln.str().c_str());
							}

						}
						else
						{
							int so_error = RM_GetSelectedOutput(id, local_selected_out.data());
						}
					}
				}
			}
		}
	}
}
/* ---------------------------------------------------------------------- */
void
FinalizeFiles()
/* ---------------------------------------------------------------------- */
{
#ifdef HDF5_CREATE
		HDFFinalize();
#endif
	for (int iso = 0; iso < (int) FileWriter.GetXYZOstreams().size(); iso++)
	{
		FileWriter.GetXYZOstreams()[iso]->clear();
	}
	FileWriter.GetXYZOstreams().clear();
}
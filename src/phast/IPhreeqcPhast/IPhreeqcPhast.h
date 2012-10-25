#if !defined(PHAST_IPHREEQC_H_INCLUDED)
#define PHAST_IPHREEQC_H_INCLUDED
#include "PHRQ_base.h"
#include "StorageBin.h"
#include <vector>
#include <list>

const float INACTIVE_CELL_VALUE = 1.0e30f;
class PHRQ_io;
#include "IPhreeqc.hpp"
class IPhreeqcPhast: public IPhreeqc
{
public:
	IPhreeqcPhast(void);
	~IPhreeqcPhast(void);
	double Get_gfw(std::string);
	void Set_cell_volumes(int i, double pore_volume, double f, double v);
	void Selected_out_to_double();

	void Get_cell_from_storage_bin(cxxStorageBin & sb, int i);
	void Put_cell_in_storage_bin(cxxStorageBin & sb, int i);
	Phreeqc * Get_PhreeqcPtr(void) {return PhreeqcPtr;};
	size_t Get_Index() {return (int) this->Index;}
	cxxSolution *Get_solution(int n_user);
	void Set_out_stream(std::ostringstream *s) {this->out_stream = s;}
	void Set_punch_stream(std::ostringstream *s) {this->punch_stream = s;}	
	std::ostringstream & Get_out_stream(void) {return *this->out_stream;}
	std::ostringstream & Get_punch_stream(void) {return *this->punch_stream;}
	std::vector < double > & Get_punch_vector(void) {return this->punch_vector;}
	void Set_thread_clock_time(double t) {this->thread_clock_time = t;}
	double Get_thread_clock_time(void) {return this->thread_clock_time;}
	void Set_start_cell(int i) {this->start_cell = i;}
	int Get_start_cell(void) {return this->start_cell;}
	void Set_end_cell(int i) {this->end_cell = i;}
	int Get_end_cell(void) {return this->end_cell;}
protected:
	friend class IPhreeqcPhastLib;
	static std::map<size_t, IPhreeqcPhast*> PhastInstances;
	static size_t PhastInstancesIndex;

	// Data members
	int start_cell;
	int end_cell;
	std::ostringstream * out_stream;
	std::ostringstream * punch_stream;
	std::vector < double > punch_vector;
	std::vector<std::string> selected_output_names;
	double thread_clock_time;
};
#endif // !defined(PHAST_IPHREEQC_H_INCLUDED)

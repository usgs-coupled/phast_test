/* main.c */
#ifdef PHREEQC_IDENT
static char const svnid[] =
	"$Id$";
#endif
int clean_up(void);
int reset_transient_data(void);

/* accumulate.c */
int accumulate(void);
int accumulate_defaults(void);
void cells_with_faces(std::list < int >&list_of_numbers, Cell_Face face);

//int check_face(struct BC *bc_ptr, struct index_range *range_ptr);
int coord_to_cell(double x1, double *coord, int count_coord, int *i1);
int coords_to_range(double x1, double x2, double *coord, int count_coord,
					double eps, int uniform, int *i1, int *i2);
int coords_to_elt_range(double x1, double x2, double *coord, int count_coord,
						double eps, int uniform, int *i1, int *i2, const bool silent);
int coord_to_cell(double x1, double *coord, int count_coord, int *i1);
int distribute_property_to_cells(struct index_range *range_ptr,
								 struct property *mask,
								 struct property *property_ptr,
								 size_t offset, size_t offset_defined,
								 property_type p_type,
								 int match_bc_type,
								 int face, BC_info::BC_TYPE bc_type);
int distribute_property_to_list_of_cells(std::list < int >&pts,	// list of cell numbers in natural order
										 struct property *mask,
										 struct property *property_ptr,
										 size_t offset, size_t offset_defined,
										 property_type p_type,
										 int match_bc_type,
										 int face, BC_info::BC_TYPE bc_type);
int distribute_type_to_cells(struct index_range *range_ptr,
							 struct property *mask,
							 int integer_value,
							 size_t offset,
							 int match_bc_type, int face,
							 BC_info::BC_TYPE bc_type);
int distribute_property_to_elements(struct index_range *range_ptr,
									struct property *mask,
									struct property *property_ptr,
									size_t offset, size_t offset_defined,
									int integer);
int double_compare(const void *ptr1, const void *ptr2);
int get_double_property_for_cell(struct cell *cell_ptr,
								 struct property *property_ptr,
								 int node_sequence, double *value);
int get_mix_property_for_cell(struct cell *cell_ptr,
							  struct property *property_ptr,
							  int node_sequence, struct mix *mix_ptr);
int get_property_for_element(struct cell *cell_ptr,
							 struct property *property_ptr, int node_sequence,
							 double *value, int *integer_value);
double get_river_value(struct property *property_ptr, double x, double y);
Cell_Face guess_face(std::list < int >&list_of_numbers,
					 struct zone *zone_ptr);
int ijk_to_n(int i, int j, int k);
void n_to_ijk(int n, int &i, int &j, int &k);
void neighbors(int n, std::vector < int >&stencil);
void neighbors_active(int n, std::vector < int >&stencil);
void elt_neighbors(int n, std::vector < int >&stencil);
struct index_range *zone_to_range(struct zone *zone_ptr);
struct index_range *zone_to_elt_range(struct zone *zone_ptr, const bool silent);
int int_compare(const void *ptr1, const void *ptr2);
void range_to_list(struct index_range *range_ptr, std::list < int >&l);
void range_plus_one(struct index_range *range_ptr);
void set_exterior_cells(void);
int setup_bc(void);
int setup_chem_ic(bool forward);
int setup_head_ic(bool forward);
int setup_media(bool forward);
int setup_print_locations(struct print_zones_struct *print_zones_struct_ptr,
						  size_t offset1, size_t offset2);
int snap_out_to_range(double x1, double x2, double *coord, int count_coord,
					  int uniform, int *i1, int *i2);
struct index_range *vertex_to_range(gpc_vertex * v, int count_points);
int which_cell(double x1, double y1, double z1, int *i1, int *j1, int *k1);

/* check.c */
int check_properties(bool defaults);
int check_cells(void);
int check_dimensions(void);
int check_elements(bool defaults);
int check_hst_units(void);
int check_leaky_for_face(int i, int j);
int check_time_data(void);
int check_ss_time_units(struct time *time_ptr, char *errstr);
int check_time_units(struct time *time_ptr, int required, const char *errstr);
int check_time_series(struct time_series *ts_ptr, int start_at_zero,
					  const char *errorString);
int check_time_series_data(void);
int ijk_to_n_no_error(int i, int j, int k);
int set_active_cells(void);

/* read.c */
int read_file_doubles(char *next_char, double **d, int *count_d,
					  int *count_alloc);
int read_input(void);
int read_lines_times(char *next_char, struct time **times, int *count_times,
					 const char **opt_list, int count_opt_list, int *opt);
struct property *read_property(char *ptr, const char **opt_list,
							   int count_opt_list, int *opt, int delimited,
							   int allow_restart);
struct property *read_property_only(char *ptr, const char **opt_list, int count_opt_list, int *opt,
			  int delimited, int allow_restart);
int get_option(const char **opt_list, int count_opt_list, char **next_char);

/* rivers */

int bisector_points(gpc_vertex p0, gpc_vertex p1, gpc_vertex p2,
					River_Point * r_ptr);
int trapezoid_points(gpc_vertex p0, gpc_vertex p1, River_Point * r_ptr0,
					 double width1);
int build_rivers(void);
int check_quad(gpc_vertex * v);
int interpolate(River_Polygon * river_polygon_ptr);
double river_distance_grid(River_Point * river_ptr1, River_Point * river_ptr2);
void river_point_init(River_Point * rp_ptr);
int setup_rivers(void);
int tidy_rivers(void);
int update_rivers(void);


/* structures.c */
struct BC *bc_alloc(void);
int bc_free(struct BC *bc_ptr);
int bc_init(struct BC *bc_ptr);
struct cell *cell_alloc(void);
int cell_free(struct cell *cell_ptr);
int cell_init(struct cell *cell_ptr);
struct chem_ic *chem_ic_alloc(void);
int chem_ic_free(struct chem_ic *chem_ic_ptr);
int chem_ic_init(struct chem_ic *chem_ic_ptr);
int copy_time(struct time *time_source, struct time *time_destination);
struct Head_ic *head_ic_alloc(void);
int head_ic_free(struct Head_ic *head_ic_ptr);
int head_ic_init(struct Head_ic *head_ic_ptr);
struct grid_elt *grid_elt_alloc(void);
int grid_elt_free(struct grid_elt *grid_elt_ptr);
int grid_elt_init(struct grid_elt *grid_elt_ptr);
int mix_init(struct mix *mix_ptr);
struct property *property_alloc(void);
struct property *property_copy(struct property *source);
int property_free(struct property *property_ptr);
int river_free(River * river_ptr);
River *river_search(int n_user, int *n);
int well_free(Well * well_ptr);
Well *well_search(int n_user, int *n);
int well_interval_compare(const void *ptr1, const void *ptr2);
struct zone *zone_alloc(void);
int zone_check(struct zone *zone_ptr);
int print_zone_struct_init(struct print_zones_struct *print_zones_struct_ptr);
int print_zone_struct_free(struct print_zones_struct *print_zones_struct_ptr);

/* time.c */
int accumulate_time_series(struct time_series *ts_ptr);
int collate_simulation_periods(void);
int get_current_property_position(struct time_series *ts_ptr, double cur_time,
								  struct property_time **pt_ptr);
struct print_frequency *print_frequency_alloc(void);
int print_frequency_compare(const void *ptr1, const void *ptr2);
int print_frequency_init(struct print_frequency *print_frequency_ptr);
struct property_time *property_time_alloc(void);
int property_time_free(struct property_time *pt_ptr);
struct property_time *property_time_copy(struct property_time *source);
int property_time_compare(const void *ptr1, const void *ptr2);
int property_time_read(char *next_char,
					   struct property_time **property_time_ptr,
					   const char **opt_list, int count_opt_list, int *opt);
int read_lines_times(char *next_char, struct time **times, int *count_times,
					 const char **opt_list, int count_opt_list, int *opt);
int read_line_times(char *ptr, struct time **times, int *count_times);
struct property *read_property_limited(char *ptr);

/* time structure */
struct time *time_alloc();
int time_compare(const void *ptr1, const void *ptr2);
int time_copy(struct time *source, struct time *target);
int time_free(struct time *time_ptr);
int times_free(struct time *time_ptr, int count);
int time_init(struct time *time_ptr);

/* time_series */
int time_series_add(struct time_series *time_series_ptr,
					struct property_time *property_time_ptr);
struct time_series *time_series_alloc(void);
struct property_time *time_series_alloc_property_time(struct time_series
													  *time_series_ptr);

int time_series_realloc(struct time_series *time_series_ptr);
struct time_series *time_series_free(struct time_series *ts_ptr);
struct time_series *time_series_read_property(char *ptr,
											  const char **opt_list,
											  int count_opt_list, int *opt);
int time_series_sort(struct time_series *time_series_ptr);
int time_series_init(struct time_series *ts_ptr);

/* wells.c */
int wells_convert_coordinate_systems(void);
int setup_wells(void);
int tidy_wells(void);
int update_wells(void);

/* write.c */

int write_hst(void);
int write_thru(int thru);

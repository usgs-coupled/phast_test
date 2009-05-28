#define EXTERNAL extern
#include "hstinpt.h"
#include "PHAST_Transform.h"
static char const svnid[] = "$Id$";
//static bool well_convert_xy_coordinate_system(Well * well_ptr,
//										   PHAST_Transform::
//										   COORDINATE_SYSTEM target,
//										   PHAST_Transform * map2grid);
static bool well_convert_xy_to_grid(Well * well_ptr,
									PHAST_Transform * map2grid);
static bool well_elevations(Well * well_ptr,
							PHAST_Transform * map2grid);
/* ---------------------------------------------------------------------- */
int
tidy_wells(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Check well data 
 */
	int i, return_value;
	int max_n_user, n_user;
	Well *well_ptr;

	return_value = OK;
	if (count_wells <= 0)
		return (OK);
	if (simulation > 0)
		return (OK);

/*
 *    Make sure all wells are numbered
 */
	max_n_user = -1;
	for (i = 0; i < count_wells; i++)
	{
		if (wells[i].n_user > max_n_user)
			max_n_user = wells[i].n_user;
	}
	if (max_n_user < 0)
		max_n_user = 1;
	n_user = max_n_user + 1;
	for (i = 0; i < count_wells; i++)
	{
		if (wells[i].n_user < 0)
		{
			wells[i].n_user = n_user++;
		}
	}


	/*
	 *  Logical checks on well
	 */

	for (i = 0; i < count_wells; i++)
	{
		well_ptr = &wells[i];
		if (well_ptr->x_user_defined == FALSE || well_ptr->y_user_defined == FALSE)
		{
			sprintf(error_string,
					"X or Y not defined for well location, well %d.",
					well_ptr->n_user);
			error_msg(error_string, CONTINUE);
			input_error++;
			return_value = FALSE;
		}
		if (well_ptr->diameter_defined == FALSE &&
			well_ptr->radius_defined == FALSE)
		{
			sprintf(error_string,
					"Well bore diameter or radius not defined for well %d.",
					well_ptr->n_user);
			error_msg(error_string, CONTINUE);
			input_error++;
			return_value = FALSE;
		}
		if (well_ptr->solution_defined == FALSE && flow_only == FALSE)
		{
			if (well_ptr->q_defined == TRUE && well_ptr->current_q > 0)
			{
				sprintf(error_string,
						"Well is injection well, but associated solution number is not defined for well %d.",
						well_ptr->n_user);
				error_msg(error_string, CONTINUE);
				input_error++;
				return_value = FALSE;
			}
		}
		if (well_ptr->depth_user_defined == TRUE && well_ptr->lsd_user_defined == FALSE)
		{
			sprintf(error_string,
					"Screened interval defined by depth below land surface, but no land surface elevation defined for well %d.",
					well_ptr->n_user);
			error_msg(error_string, CONTINUE);
			input_error++;
			return_value = FALSE;
		}
		if ((well_ptr->depth_user_defined == FALSE &&
			 well_ptr->elevation_user_defined == FALSE) ||
			(well_ptr->count_depth_user <= 0 && well_ptr->count_elevation_user <= 0))
		{
			sprintf(error_string, "No screened interval defined for well %d.",
					well_ptr->n_user);
			error_msg(error_string, CONTINUE);
			input_error++;
			return_value = FALSE;
		}
		if (well_ptr->q_defined == FALSE)
		{
			sprintf(error_string, "No pumping rate defined for well %d.",
					well_ptr->n_user);
			error_msg(error_string, CONTINUE);
			input_error++;
			return_value = FALSE;
		}
	}

	return (return_value);
}

/* ---------------------------------------------------------------------- */
int
wells_convert_coordinate_systems(void)
/* ---------------------------------------------------------------------- */
{
	int j, return_code;
	Well *well_ptr;

	if (count_wells <= 0)
		return (OK);
	if (simulation > 0)
		return (OK);

	return_code = OK;

	for (j = 0; j < count_wells; j++)
	{
		if (!wells[j].in_region) continue;
		wells[j].new_def = FALSE;
		well_ptr = &(wells[j]);
		if (!well_convert_xy_to_grid(well_ptr, map_to_grid) ||
			!well_elevations(well_ptr, map_to_grid))
		{
			return_code = ERROR;
		}

	}
	return (return_code);
}

/* ---------------------------------------------------------------------- */
int
setup_wells(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Check well data 
 */
	int i, j, k, n, return_code;
	int k1, k2;
	int i_cell, j_cell, k_cell;
	Well *well_ptr;
	int count_coord;
	double *f, *du, *dl;
	Well_Interval *interval_ptr;
	double cell_bottom, cell_top, bottom, top, cell_node;
	return_code = OK;

	wells_update = FALSE;
	if (count_wells <= 0)
		return (OK);
	wells_update = TRUE;
	/* malloc scratch space */
	count_coord = grid[2].count_coord;
	f = (double *) malloc((size_t) count_coord * sizeof(double));
	if (f == NULL)
		malloc_error();
	dl = (double *) malloc((size_t) count_coord * sizeof(double));
	if (dl == NULL)
		malloc_error();
	du = (double *) malloc((size_t) count_coord * sizeof(double));
	if (du == NULL)
		malloc_error();


	// Check for wells in region
	count_wells_in_region = 0;
	for (j = 0; j < count_wells; j++)
	{
		well_ptr = &(wells[j]);
		/* find i, j, for well location */
		if (which_cell
			(well_ptr->x_grid, well_ptr->y_grid, grid[2].coord[grid[2].count_coord - 1],
			 &i_cell, &j_cell, &k_cell) == ERROR)
		{
			sprintf(error_string,
					"Well location is outside domain for well %d.",
					well_ptr->n_user);
			warning_msg(error_string);
			well_ptr->in_region = false;
		}
		else
		{
			count_wells_in_region++;
		}
	}
	for (j = 0; j < count_wells; j++)
	{
		if (!wells[j].in_region) continue;
		well_ptr = &(wells[j]);
		well_ptr->update = TRUE;
		for (i = 0; i < count_coord; i++)
		{
			f[i] = 0;
		}
		/* find i, j, for well location */
		which_cell
			(well_ptr->x_grid, well_ptr->y_grid, grid[2].coord[grid[2].count_coord - 1],
			 &i_cell, &j_cell, &k_cell);
		/*
		 *  Find cells for well intervals
		 */
		int count_well_cells = 0;
		bool screen_top(false), screen_bottom(false);
		well_ptr->screen_bottom = well_ptr->screen_top = 0.0;
		for (i = 0; i < well_ptr->count_elevation_grid; i++)
		{

			interval_ptr = &well_ptr->elevation_grid[i];

			double grid_bottom = grid[2].coord[0];
			double grid_top = grid[2].coord[grid[2].count_coord - 1];
			double interval_bottom = interval_ptr->bottom;
			double interval_top = interval_ptr->top;

			/* skip if bottom of well is outside grid */
			if (interval_bottom > grid_top || interval_top < grid_bottom)
			{
				continue;
			}
			/* reset bottom to bottom of grid if necessary */
			if (interval_bottom < grid_bottom)
			{
				interval_bottom = grid_bottom;
			}
			/* reset top to top of grid if necessary */
			if (interval_top > grid_top)
			{
				interval_top = grid_top;
			}
			if (screen_bottom)
			{
				if (interval_bottom < well_ptr->screen_bottom)
				{
					well_ptr->screen_bottom = interval_bottom;
				}
			}
			else
			{
				well_ptr->screen_bottom = interval_bottom;
				screen_bottom = true;
			}
			if (screen_top)
			{
				if (interval_top < well_ptr->screen_top)
				{
					well_ptr->screen_top = interval_top;
				}
			}
			else
			{
				well_ptr->screen_top = interval_top;
				screen_top = true;
			}

			if ((coord_to_cell(interval_bottom, grid[2].coord, grid[2].count_coord, &k1) == ERROR) ||
				(coord_to_cell(interval_top, grid[2].coord, grid[2].count_coord, &k2) == ERROR))
			{
				input_error++;
				sprintf(error_string,
						"Well screen extends outside domain for well %d.\n\tinterval:\t%g\t%g",
						well_ptr->n_user, interval_ptr->bottom,
						interval_ptr->top);
				error_msg(error_string, CONTINUE);
				continue;
			}
			for (k = k1; k <= k2; k++)
			{
				n = ijk_to_n(i_cell, j_cell, k);
				if (cells[n].cell_active == FALSE) continue;
				count_well_cells++;
				cell_bottom = cells[n].zone->z1;
				cell_node = cells[n].z;
				cell_top = cells[n].zone->z2;
				bottom = cell_bottom;
				top = cell_top;
				if (bottom < interval_bottom)
				{
					bottom = interval_bottom;
				}
				if (top > interval_top)
				{
					top = interval_top;
				}
				f[k] += (top - bottom) / (cell_top - cell_bottom);
				/*
				 * added logic to determine number of meters below node and above node 
				 */
				dl[k] = 0;
				if (bottom < cell_node)
				{
					if (top < cell_node)
					{
						dl[k] = top - bottom;
					}
					else
					{
						dl[k] = (cell_node - bottom);
					}
				}
				du[k] = 0;
				if (top > cell_node)
				{
					if (bottom > cell_node)
					{
						du[k] = top - bottom;
					}
					else
					{
						du[k] = (top - cell_node);
					}
				}
			}
		}

#ifdef DEBUG_WELLS
		output_msg(OUTPUT_STDERR, "Well %d:\n", well_ptr->n_user);
#endif
		if (count_well_cells <= 0)
		{
			count_wells_in_region--;
			well_ptr->in_region = false;
			sprintf(error_string,
				"Well %d does not intersect active region.",
				well_ptr->n_user);
			warning_msg(error_string);
			continue;
		}
		for (k = 0; k < count_coord; k++)
		{
			bool bottom_defined = false;
			bool top_defined = false;
			if (f[k] > 0)
			{
				well_ptr->cell_fraction =
					(Cell_Fraction *) realloc(well_ptr->cell_fraction,
											  (size_t) (well_ptr->
														count_cell_fraction +
														1) *
											  sizeof(Cell_Fraction));
				if (well_ptr->cell_fraction == NULL)
					malloc_error();
				n = ijk_to_n(i_cell, j_cell, k);
				well_ptr->cell_fraction[well_ptr->count_cell_fraction].cell =
					n;
				well_ptr->cell_fraction[well_ptr->count_cell_fraction].f =
					f[k];
				well_ptr->cell_fraction[well_ptr->count_cell_fraction].upper =
					du[k];
				well_ptr->cell_fraction[well_ptr->count_cell_fraction].lower =
					dl[k];
				well_ptr->count_cell_fraction++;
#ifdef DEBUG_WELLS
				output_msg(OUTPUT_STDERR, "\tijk: %d %d %d\tn: %d\tf:%g\n",
						   i_cell, j_cell, k, n, f[k]);
#endif
			}
		}
	}
	free_check_null(f);
	free_check_null(du);
	free_check_null(dl);
	return (return_code);
}

/* ---------------------------------------------------------------------- */
int
update_wells(void)
/* ---------------------------------------------------------------------- */
{
/*
 *   Check well data 
 */
	int j, n;
	Well *well_ptr;

	wells_update = FALSE;
	if (count_wells <= 0)
		return (OK);
	for (j = 0; j < count_wells; j++)
	{
		if (!wells[j].in_region) continue;
		if (wells[j].new_def == FALSE)
			continue;
		wells_update = TRUE;
		well_ptr = &(wells[j]);
		wells[j].new_def = UPDATE;
		/*
		 *   Find well
		 */
		for (n = 0; n < count_wells; n++)
		{
			if (wells[n].new_def == TRUE)
				continue;
			if (wells[n].new_def == UPDATE)
				continue;
			if (wells[n].n_user == wells[j].n_user)
			{
				break;
			}
		}
		if (n == count_wells)
		{
			sprintf(error_string, "Well %d %s not found for transient data",
					well_ptr->n_user, well_ptr->description);
			error_msg(error_string, CONTINUE);
			input_error++;
			continue;
		}
		/*
		 *   Update solution and/or flow rate
		 */
		if (wells[j].q_defined == TRUE)
		{
			wells[n].q = wells[j].q;
		}
		if (wells[j].solution_defined == TRUE)
		{
			wells[n].solution = wells[j].solution;
		}
		wells[j].update = TRUE;
	}
	return (OK);
}

/* ---------------------------------------------------------------------- */
bool
well_convert_xy_to_grid(Well * well_ptr,
						PHAST_Transform * map2grid)
/* ---------------------------------------------------------------------- */
{
	if (well_ptr->xy_coordinate_system_user == PHAST_Transform::NONE)
	{
		sprintf(error_string, "Error with coordinate system for well %d %s.",
				well_ptr->n_user, well_ptr->description);
		error_msg(error_string, CONTINUE);
		input_error++;
		return false;
	}
	switch (well_ptr->xy_coordinate_system_user)
	{
	case PHAST_Transform::GRID:
		well_ptr->x_grid = well_ptr->x_user;
		well_ptr->y_grid = well_ptr->y_user;
		break;
	case PHAST_Transform::MAP:
		{
			Point p(well_ptr->x_user, well_ptr->y_user, 0.0);
			map2grid->Transform(p);
			well_ptr->x_grid = p.x();
			well_ptr->y_grid = p.y();
		}
		break;
	default:
		sprintf(error_string, "Error with coordinate system for well %d %s.",
				well_ptr->n_user, well_ptr->description);
		error_msg(error_string, CONTINUE);
		input_error++;
		well_ptr->x_grid = well_ptr->x_user;
		well_ptr->y_grid = well_ptr->y_user;
		return false;
		break;
	}
	return true;
}
/* ---------------------------------------------------------------------- */
bool
well_elevations(Well * well_ptr,
				PHAST_Transform * map2grid)
/* ---------------------------------------------------------------------- */
{
/*
 *   Make list of z elevations in grid units for well
 *
 *   lsd and elevation units are given by well_ptr->z_coordinate_system
 *   depth units are given by well_ptr->depth_units
 */
	int i, k;
	bool return_code = true;

	/* Copy from user to working */
	free_check_null (well_ptr->elevation_grid);
	well_ptr->elevation_grid = NULL;
	well_ptr->elevation_grid = (Well_Interval *) malloc((size_t) ((well_ptr->count_elevation_user + 1) * sizeof(Well_Interval)));
	if (well_ptr->elevation_grid == NULL) malloc_error();
	memcpy(well_ptr->elevation_grid, well_ptr->elevation_user, (size_t) (well_ptr->count_elevation_user * sizeof(Well_Interval)));
	well_ptr->count_elevation_grid = well_ptr->count_elevation_user;

	/* convert elevations to grid units */
	/* set lsd to grid units */

	double lsd_grid = well_ptr->lsd_user;
	switch (well_ptr->z_coordinate_system_user)
	{
	case PHAST_Transform::GRID:
		break;
	case PHAST_Transform::NONE:
		sprintf(error_string,
			"Z coordinate system undefined for well: %d, %s",
			well_ptr->n_user, well_ptr->description);
		error_msg(error_string, CONTINUE);
		input_error++;
		return_code = false;
		break;
	case PHAST_Transform::MAP:
		{
			Point p(0, 0, well_ptr->lsd_user);
			map2grid->Transform(p);
			lsd_grid = p.z();
			double conversion_factor;
			conversion_factor = units.map_vertical.input_to_si / units.vertical.input_to_si;

			for (i = 0; i < well_ptr->count_elevation_grid; i++)
			{
				well_ptr->elevation_grid[i].top *= conversion_factor;
				well_ptr->elevation_grid[i].bottom *= conversion_factor;
			}
		}
		break;
	}

	/* convert depths to elevations in grid units */
	if (well_ptr->depth_user_defined == TRUE)
	{
		double conversion_factor = 1.0;
		if (units.well_depth.defined == TRUE)
		{
			conversion_factor = units.well_depth.input_to_si / units.vertical.input_to_si;
		}
#ifdef SKIP
		/* Not used, currently using well_depth in UNITS input */
		if (well_ptr->depth_units_user->defined == TRUE)
		{
			conversion_factor = well_ptr->depth_units_user->input_to_si / units.vertical.input_to_si;
		}
#endif
		for (i = 0; i < well_ptr->count_depth_user; i++)
		{
			well_ptr->elevation_grid =	(Well_Interval *) realloc(well_ptr->elevation_grid,
				(size_t) (well_ptr->count_elevation_grid + 1) * sizeof(Well_Interval));
			if (well_ptr->elevation_grid == NULL) malloc_error();
			well_ptr->elevation_grid[well_ptr->count_elevation_grid].top =
				lsd_grid - well_ptr->depth_user[i].top * conversion_factor;
			well_ptr->elevation_grid[well_ptr->count_elevation_grid].bottom =
				lsd_grid - well_ptr->depth_user[i].bottom  * conversion_factor;
			well_ptr->count_elevation_grid++;
		}
	}

	/* make sure top is greater than bottom */
	for (i = 0; i < well_ptr->count_elevation_grid; i++)
	{
		if (well_ptr->elevation_grid[i].top < well_ptr->elevation_grid[i].bottom)
		{
			double t = well_ptr->elevation_grid[i].top;
			well_ptr->elevation_grid[i].top = well_ptr->elevation_grid[i].bottom;
			well_ptr->elevation_grid[i].bottom = t;
		}
	}

	/* sort by tops from bottom of well to top of well */
	qsort(well_ptr->elevation_grid, (size_t) well_ptr->count_elevation_grid,
		(size_t) sizeof(Well_Interval), well_interval_compare);

	/* check for overlaps */
	for (i = 0; i < well_ptr->count_elevation_grid - 1; i++)
	{
		for (k = i + 1; k < well_ptr->count_elevation_grid; k++)
		{
			if (well_ptr->elevation_grid[i].top >
				well_ptr->elevation_grid[k].bottom)
			{
				return_code = false;
				input_error++;
				sprintf(error_string,
					"Well screen elevation intervals overlap (grid units).\n\tScreen 1:\n\t\tElevation:\t%g\t%g\n\tScreen 2:\n\t\tElevation:\t%g\t%g\n",
					well_ptr->elevation_grid[i].top,
					well_ptr->elevation_grid[i].bottom,
					well_ptr->elevation_grid[k].top,
					well_ptr->elevation_grid[k].bottom);
				error_msg(error_string, CONTINUE);
			}
		}
	}
	return (return_code);
	
}

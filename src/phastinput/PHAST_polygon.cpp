#include "PHAST_polygon.h"
#include "message.h"
#include <iostream>
#include <cassert>

// Note: No header files should follow the next three lines
#if defined(_WIN32) && defined(_DEBUG)
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

PHAST_polygon::PHAST_polygon(void)
{
}

PHAST_polygon::PHAST_polygon(gpc_polygon * poly,
							 PHAST_Transform::COORDINATE_SYSTEM cs)
{
	int i, j;
	this->coordinate_system = cs;
	for (i = 0; i < poly->num_contours; i++)
	{
		for (j = 0; j < poly->contour[i].num_vertices; j++)
		{
			Point p(poly->contour[i].vertex[j].x,
					poly->contour[i].vertex[j].y, 0.0);
			this->pts.push_back(p);
		}
	}

	std::vector < Point >::iterator it = this->pts.begin();
	for (i = 0; i < poly->num_contours; i++)
	{
		this->begin.push_back(it);
		for (j = 0; j < poly->contour[i].num_vertices; j++)
		{
			it++;
		}
		this->end.push_back(it);
	}
	zone z(Point(this->pts.begin(), this->pts.end(), Point::MIN), Point(this->pts.begin(), this->pts.end(), Point::MAX));
	this->box = z;
}
PHAST_polygon::PHAST_polygon(const std::vector < Point > &points,
							 PHAST_Transform::COORDINATE_SYSTEM cs):
pts(points)
{
	this->coordinate_system = cs;
	this->begin.push_back(this->pts.begin());
	this->end.push_back(this->pts.end());
}
PHAST_polygon::PHAST_polygon(const PHAST_polygon & poly):
box(poly.box),
pts(poly.pts),
coordinate_system(poly.coordinate_system)
{
	std::vector < std::vector < Point >::iterator >::const_iterator bi =
		poly.begin.begin();
	for (; bi != poly.begin.end(); ++bi)
	{
		std::vector < Point >::difference_type diff = *bi - poly.pts.begin();
		this->begin.push_back(this->pts.begin() + diff);
	}
	std::vector < std::vector < Point >::iterator >::const_iterator ei =
		poly.end.begin();
	for (; ei != poly.end.end(); ++ei)
	{
		std::vector < Point >::difference_type diff = *ei - poly.pts.begin();
		this->end.push_back(this->pts.begin() + diff);
	}
	assert(this->pts.size() == poly.pts.size());
	assert(this->begin.size() == poly.begin.size());
	assert(this->end.size() == poly.end.size());
}
PHAST_polygon & PHAST_polygon::operator=(const PHAST_polygon & rhs)
{
	if (this != &rhs)
	{
		this->pts = rhs.pts;
		this->box = rhs.box;
		this->begin.clear();
		this->end.clear();

		std::vector < std::vector < Point >::iterator >::const_iterator bi =
			rhs.begin.begin();
		for (; bi != rhs.begin.end(); ++bi)
		{
			std::vector < Point >::difference_type diff =
				*bi - rhs.pts.begin();
			this->begin.push_back(this->pts.begin() + diff);
		}
		std::vector < std::vector < Point >::iterator >::const_iterator ei =
			rhs.end.begin();
		for (; ei != rhs.end.end(); ++ei)
		{
			std::vector < Point >::difference_type diff =
				*ei - rhs.pts.begin();
			this->end.push_back(this->pts.begin() + diff);
		}
		assert(this->pts.size() == rhs.pts.size());
		assert(this->begin.size() == rhs.begin.size());
		assert(this->end.size() == rhs.end.size());
	}
	return *this;
}

PHAST_polygon::~PHAST_polygon(void)
{
}
void
PHAST_polygon::set_z(double z)
{

	// for each point, set z value
	std::vector < Point >::iterator it;
	for (it = this->pts.begin(); it != this->pts.end(); it++)
	{
		it->set_z(z);
	}
}
void
PHAST_polygon::set_z_to_v()
{

	// for each point, set z value
	std::vector < Point >::iterator it;
	for (it = this->pts.begin(); it != this->pts.end(); it++)
	{
		it->set_z(it->get_v());
	}
}

bool PHAST_polygon::Point_in_polygon(Point p)
{
//int pnpoly(int npol, float *xp, float *yp, float x, float y)

	int
		poly;
	for (poly = 0; poly < (int) this->begin.size(); poly++)
	{
		if (Point_in_simple_polygon(p, this->begin[poly], this->end[poly]))
			return true;
	}

	return (false);
}

bool
Point_in_simple_polygon(Point p, std::vector < Point >::iterator begin,
						std::vector < Point >::iterator end)
{
	bool
		in = false;
	double
		x = p.x();
	double
		y = p.y();
	std::vector < Point >::iterator i_it, j_it, npol_it;
	//int i, j;
	//int npol = pts.size();
	npol_it = end;
	//for (i = 0, j = npol-1; i < npol; j = i++) {
	j_it = npol_it - 1;
	for (i_it = begin; i_it != npol_it; j_it = i_it++)
	{
		//double xpi = pts[i].x();
		double
			xpi = i_it->x();
		//double xpj = pts[j].x();
		double
			xpj = j_it->x();
		//double ypi = pts[i].y();
		double
			ypi = i_it->y();
		//double ypj = pts[j].y();
		double
			ypj = j_it->y();
		if ((((ypi <= y) && (y < ypj)) ||
			 ((ypj <= y) && (y < ypi))) &&
			(x < (xpj - xpi) * (y - ypi) / (ypj - ypi) + xpi))
			in = !in;
	}
	if (in)
	{
		return (true);
	}
	// Also check if point is on an edge
	double
		z = 0.0, z1 = 0.0, z2 = 0.0;

	//j = pts.size() - 1;
	j_it = end - 1;
	//for (i = 0; i < (int) pts.size(); i++)
	for (i_it = begin; i_it != npol_it; i_it++)
	{
		double
			x1,
			y1,
			x2,
			y2;
		//x1 = pts[i].x();
		x1 = i_it->x();
		//y1 = pts[i].y();
		y1 = i_it->y();
		//x2 = pts[j].x();
		x2 = j_it->x();
		//y2 = pts[j].y();
		y2 = j_it->y();
		double
			xn,
			yn,
			zn,
			dist,
			t;
		Point::line_seg_point_near_3d(x1, y1, z1, x2, y2, z2, x, y, z, &xn,
									  &yn, &zn, &dist, &t);
		if (dist < 1e-6)
		{
			return (true);
		}
		//j = i;
		j_it = i_it;
	}
	return false;
}

bool PHAST_polygon::Line_intersect(Point lp1, Point lp2,
								   std::vector < Point > &intersect_pts)
{
	// lp1 is assumed to be outside bounding box of the polygon
	//int i, j;
	int
		poly;
	for (poly = 0; poly < (int) this->begin.size(); poly++)
	{
		if (Line_intersect_simple_polygon
			(lp1, lp2, this->begin[poly], this->end[poly], intersect_pts))
			return true;
	}
	return (false);
}

bool
Line_intersect_simple_polygon(Point lp1, Point lp2,
							  std::vector < Point >::iterator begin,
							  std::vector < Point >::iterator end,
							  std::vector < Point > &intersect)
{
	// lp1 is assumed to be outside bounding box of the polygon
	//int i, j;
	std::vector < Point >::iterator i_it, j_it, npol_it;
	std::vector < Point > intersect_pts;
	//int npol = pts.size();
	npol_it = end;
	//j = npol-1;
	j_it = npol_it - 1;
	//for (i = 0; i < npol; j = i++) {
	for (i_it = begin; i_it != npol_it; j_it = i_it++)
	{
		//line_and_segment_intersection(lp1, lp2, pts[i], pts[j], intersect_pts);
		line_and_segment_intersection(lp1, lp2, *i_it, *j_it, intersect_pts);
	}

	if (intersect_pts.size() == 0)
		return (false);
	// Need to check if midpoint of line segments is interior
	std::vector < Point >::iterator k_it, l_it, n_it;
	n_it = intersect_pts.end();
	l_it = n_it - 1;

	for (k_it = intersect_pts.begin(); k_it != intersect_pts.end() - 1;
		 k_it++)
	{
		l_it = k_it + 1;
		Point
			p = *l_it + 0.5 * (*k_it - *l_it);
		if (Point_in_simple_polygon(p, begin, end))
		{
			intersect.push_back(*l_it);
			intersect.push_back(*k_it);
		}
	}

	// Should be an even number of points
	if (intersect_pts.size() % 2 != 0)
		error_msg
			("Number of points of intersection of line with polygon should be even",
			 EA_STOP);

	if (intersect_pts.size() > 0)
	{
		return (true);
	}
	return (false);
}

void
PHAST_polygon::Set_bounding_box(void)
{
	Point min(this->pts.begin(), this->pts.end(), Point::MIN);
	Point max(this->pts.begin(), this->pts.end(), Point::MAX);
	this->box.x1 = min.x();
	this->box.y1 = min.y();
	this->box.z1 = min.z();
	this->box.x2 = max.x();
	this->box.y2 = max.y();
	this->box.z2 = max.z();
}

void
PHAST_polygon::Clear(void)
{
	this->pts.clear();
	this->begin.clear();
	this->end.clear();
	this->box = zone();
}
void PHAST_polygon::Tidy(void)
{
	PHAST_polygon pp;
	pp.pts.reserve(this->pts.size()+1);
	bool warning_identical = true;
	bool warning_too_few = true;
	bool warning_area = true;
	int i = 0;

	// check for three points in polygon and elimin
	size_t poly;

	for (poly = 0; poly < this->begin.size(); poly++)
	{
		std::vector < Point > simple_poly;
		std::vector < Point >::iterator i_it;

		i_it = this->begin[poly];

		// save last point
		double xlast, ylast;
		xlast = i_it->x();
		ylast = i_it->y();
		simple_poly.push_back(*i_it);
		i++;
		for (i_it = this->begin[poly] + 1; i_it != this->end[poly]; i_it++)
		{
			i++;
			if (i_it->x() != xlast ||
				i_it->y() != ylast)
			{
				simple_poly.push_back(*i_it);
			}
			else
			{
				if (warning_identical)
				{
					warning_msg	("Identical point removed from polygon. ");
					//warning_identical = false;
				}
			}
			xlast = i_it->x();
			ylast = i_it->y();
		}
		// check number of points
		if (simple_poly.size() < 3) 
		{
			if (warning_too_few)
			{
				warning_msg	("Simple polygon removed. Number of points is less than 3. ");
				//warning_too_few = false;
			}
			continue;
		}
		// check area
		gpc_polygon * gpc_poly = points_to_poly(simple_poly);
		double area = gpc_polygon_area(gpc_poly);
		gpc_free_polygon(gpc_poly);
		free(gpc_poly);
		if ( area <= 0)
		{
			if (warning_area)
			{
				warning_msg	("Simple polygon removed. Area is zero. ");
			}
			//warning_area = false;

			continue;
		}
		// add to pp

		i_it = simple_poly.begin();
		// save first point of simple polygon
		pp.pts.push_back(*i_it);
		// save start of simple polygon
		pp.begin.push_back(pp.pts.begin() + pp.pts.size() - 1);
		// save end of previous simple polygon
		if (poly > 0) 
		{
			pp.end.push_back(pp.pts.begin() + pp.pts.size() - 1);
		}
		// store rest of points for simple polygon
		for (i_it = simple_poly.begin() + 1; i_it != simple_poly.end(); i_it++)		
		{
			pp.pts.push_back(*i_it);
		}
	}
	// save end of last polygon
	pp.end.push_back(pp.pts.end());
	// copy points
	this->pts = pp.pts;

	// put in pointers
	std::vector < Point >::iterator jit;
	size_t j, count(0);
	std::vector < Point >::iterator kit = this->pts.begin();
	this->begin.clear();
	this->end.clear();

	for (j = 0; j < pp.begin.size(); j++)
	{
		this->begin.push_back(kit);
		for (jit = pp.begin[j]; jit != pp.end[j]; jit++)
		{
			count++;
			kit++;
		}
		this->end.push_back(kit);
	}


	count = 0;
	for (j = 0; j < this->begin.size(); j++)
	{
		for (jit = this->begin[j]; jit != this->end[j]; jit++)
		{
			count++;
		}
	}
	if (pp.pts.size() == 0)
	{
		error_msg("After processing, polygon has no points. ", EA_CONTINUE);
	}
	
	return;
}
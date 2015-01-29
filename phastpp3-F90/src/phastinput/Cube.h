#if !defined(CUBE_H_INCLUDED)
#define CUBE_H_INCLUDED
#include "Polyhedron.h"

class Cube:public Polyhedron
{
  public:
	// type enum
	enum PLANE_INTERSECTION
	{
		PI_NONE, PI_SEGMENT, PI_POINT
	};

	enum CUBE_INTERSECTION
	{
		CI_NONE, CI_INTERIOR, CI_FACE, CI_EDGE
	};

	// constructors
	  Cube(PHAST_Transform::COORDINATE_SYSTEM cs = PHAST_Transform::GRID);
	  Cube(const struct zone *zone_ptr, PHAST_Transform::COORDINATE_SYSTEM cs = PHAST_Transform::GRID);

	// destructor
	  virtual ~ Cube(void);

	// methods
	bool Point_in_polyhedron(const Point & t)const;
	void Points_in_polyhedron(std::list < int >&list,
							  std::vector < Point > &point_coord);
	//gpc_polygon *Face_polygon(Cell_Face face);
	gpc_polygon *Slice(Cell_Face face, double coord);
	virtual Cube *clone() const;
	virtual Cube *create() const;
	bool Segment_in_cube(Point & p1, Point & p2, Point & i1, Point & i2,
						 double &length,
						 Cube::CUBE_INTERSECTION & c_intersection);
	struct zone *Set_bounding_box();
	  PHAST_Transform::COORDINATE_SYSTEM Get_coordinate_system() const
	{
		return this->coordinate_system;
	}
	void Set_coordinate_system(PHAST_Transform::COORDINATE_SYSTEM c)
	{
		this->coordinate_system = c;
	}
	  PHAST_Transform::COORDINATE_SYSTEM Get_user_coordinate_system() const
	{
		return this->coordinate_system_user;
	}
	void Set_user_coordinate_system(PHAST_Transform::COORDINATE_SYSTEM c)
	{
		this->coordinate_system_user = c;
	}

	bool operator==(const Cube &other) const;
	bool operator!=(const Cube &other) const;

  protected:
	virtual void printOn(std::ostream & os) const;
	PHAST_Transform::COORDINATE_SYSTEM coordinate_system;
	PHAST_Transform::COORDINATE_SYSTEM coordinate_system_user;
};
Cube::PLANE_INTERSECTION Segment_intersect_plane(const double a,
												 const double b,
												 const double c,
												 const double d, Point & p1,
												 Point & diff, double &t);

#endif // !defined(CUBE_H_INCLUDED)

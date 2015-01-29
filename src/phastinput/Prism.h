#if !defined(PRISM_H_INCLUDED)
#define PRISM_H_INCLUDED
#include "Polyhedron.h"
#include "Data_source.h"
#include "PHAST_Transform.h"
class Wedge;
class Cube;
class Prism:public Polyhedron
{

  public:
	Prism(void);
	Prism(char * char_tag);
	Prism(Cube & c);
	Prism(Wedge & w);
	Prism(std::vector < Point > perimeter_pts, PHAST_Transform::COORDINATE_SYSTEM cs = PHAST_Transform::GRID);
	Prism(const Prism & c);
  public:
	 ~Prism(void);

	// type enum
	enum PRISM_OPTION
	{
		PERIMETER = 0,
//    DIP          = 1,
		TOP = 2,
		BOTTOM = 3,
		DESCRIPTION =4

	};

  public:
	// Virtual methods
	void Points_in_polyhedron(std::list < int >&list,
							  std::vector < Point > &point_xyz);
	Polyhedron *clone() const;
	Polyhedron *create() const;
	//gpc_polygon *Face_polygon(Cell_Face face);
	gpc_polygon *Slice(Cell_Face face, double coord);
	  PHAST_Transform::COORDINATE_SYSTEM What_coordinates();
	void Convert_coordinates(PHAST_Transform::COORDINATE_SYSTEM cs,
							 PHAST_Transform * map2grid);

	bool Is_homogeneous(void)const;
	PHAST_Transform::COORDINATE_SYSTEM Get_best_coordinate_system(void)const;

#if defined(__WPHAST__) && defined(_DEBUG)
	virtual void Dump(class CDumpContext& dc) const;
#endif

  protected:
	// Virtual methods
	struct zone *Set_bounding_box();
	virtual void printOn(std::ostream & os) const;

  public:
	// Methods
	  bool Read(PRISM_OPTION p_opt, std::istream & lines);
	bool Read(std::istream & lines);
	bool Project_point(Point & p, Cell_Face face, double coord);
	bool Project_points(std::vector < Point > &pts, Cell_Face face,
						double coord);
	bool Point_in_polyhedron(const Point & p);
	void Remove_top_bottom(gpc_polygon * polygon, Cell_Face face,
						   double coord);
	void Tidy();
	static bool Polygon_intersects_self(std::vector<Point> &vect);
	// data

	Data_source perimeter;
	Point prism_dip;
	Data_source bottom;
	Data_source top;

	// optimization
	bool last_defined;
	double last_x;
	double last_y;
	bool inside_perimeter;
	double last_bottom;
	bool last_top_defined;
	double last_top;

	static std::list < Prism * >prism_list;

	bool operator==(const Prism &other) const;
	bool operator!=(const Prism &other) const;

};
void Tidy_prisms(void);
void Convert_coordinates_prisms(PHAST_Transform::COORDINATE_SYSTEM cs,
						   PHAST_Transform * map2grid);
#endif // !defined(PRISM_H_INCLUDED)

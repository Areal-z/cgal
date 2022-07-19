// STL includes.
#include <string>
#include <vector>
#include <utility>
#include <fstream>
#include <iostream>
#include <iterator>
#include <cassert>

// CGAL includes.
#include <CGAL/assertions.h>
#include <CGAL/property_map.h>

#include <CGAL/Point_set_3.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/HalfedgeDS_vector.h>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>

#include <CGAL/Shape_detection/Region_growing/Region_growing.h>
#include <CGAL/Shape_detection/Region_growing/Point_set.h>
#include <CGAL/Shape_detection/Region_growing/Polygon_mesh.h>
#include <CGAL/Shape_detection/Region_growing/Polyline.h>
#include <CGAL/Shape_detection/Region_growing/Segment_set.h>
#include <CGAL/Shape_detection/Region_growing/free_functions.h>

template<class Kernel>
bool test_lines_points_with_normals() {

  using Point_2  = typename Kernel::Point_2;
  using Vector_2 = typename Kernel::Vector_2;
  using Item = typename std::vector< std::pair<Point_2, Vector_2> >::const_iterator;

  const std::vector< std::pair<Point_2, Vector_2> > points_with_normals = {
    std::make_pair(Point_2(0.1, 0.0), Vector_2(0.0, 1.0)),
    std::make_pair(Point_2(0.5, 0.0), Vector_2(0.0, 1.0)),
    std::make_pair(Point_2(0.9, 0.0), Vector_2(0.0, 1.0)),
    std::make_pair(Point_2(0.1, 2.0), Vector_2(0.0, 1.0)),
    std::make_pair(Point_2(0.5, 2.0), Vector_2(0.0, 1.0)),
    std::make_pair(Point_2(0.9, 2.0), Vector_2(0.0, 1.0)),
    std::make_pair(Point_2(0.1, 4.0), Vector_2(0.0, 1.0)),
    std::make_pair(Point_2(0.5, 4.0), Vector_2(0.0, 1.0)),
    std::make_pair(Point_2(0.9, 4.0), Vector_2(0.0, 1.0))
  };

  assert(points_with_normals.size() == 9);
  std::vector< std::pair< typename Kernel::Line_2, std::vector<Item> > > regions;
  CGAL::Shape_detection::region_growing_lines(
    points_with_normals,
    std::back_inserter(regions),
    CGAL::parameters::point_map(CGAL::First_of_pair_property_map<std::pair<Point_2, Vector_2> >()).
    normal_map(CGAL::Second_of_pair_property_map<std::pair<Point_2, Vector_2> >()));
  assert(regions.size() == 3);
  assert(regions[0].second.size() == 3);
  assert(regions[1].second.size() == 3);
  assert(regions[2].second.size() == 3);
  return true;
}

template<class Kernel>
bool test_lines_polylines_2() {

  using Point_2 = typename Kernel::Point_2;
  using Item = typename std::vector<Point_2>::const_iterator;
  const std::vector<Point_2> polyline_2 = {
    Point_2(0.10, 0.00), Point_2(0.50, 0.00), Point_2(0.90, 0.00),
    Point_2(0.13, 0.00), Point_2(0.17, 0.00), Point_2(0.21, 0.00),
    Point_2(0.21, 2.10), Point_2(0.21, 2.50), Point_2(0.21, 2.90),
    Point_2(0.21, 2.13), Point_2(0.21, 2.17), Point_2(0.21, 2.21)
  };

  assert(polyline_2.size() == 12);
  std::vector< std::pair< typename Kernel::Line_2, std::vector<Item> > > regions;
  CGAL::Shape_detection::region_growing_polylines(
    polyline_2, std::back_inserter(regions));
  assert(regions.size() == 2);
  assert(regions[0].second.size() == 6);
  assert(regions[1].second.size() == 6);

  return true;
}

template<class Kernel>
bool test_lines_polylines_3() {

  using Point_3 = typename Kernel::Point_3;
  using Item = typename std::vector<Point_3>::const_iterator;
  const std::vector<Point_3> polyline_3 = {
    Point_3(0.10, 0.0, 1.0), Point_3(0.50, 0.0, 1.0), Point_3(0.90, 0.0, 1.0),
    Point_3(0.13, 0.0, 1.0), Point_3(0.17, 0.0, 1.0), Point_3(0.21, 0.0, 1.0)
  };

  assert(polyline_3.size() == 6);
  std::vector< std::pair< typename Kernel::Line_3, std::vector<Item> > > regions;
  CGAL::Shape_detection::region_growing_polylines(
    polyline_3, std::back_inserter(regions));
  assert(regions.size() == 1);
  assert(regions[0].second.size() == 6);
  return true;
}

template<class Kernel>
bool test_polylines_equal_points() {

  using Point_2 = typename Kernel::Point_2;
  using Item = typename std::vector<Point_2>::const_iterator;
  const std::vector<Point_2> polyline_2 = {
    Point_2(0, 0), Point_2(1, 0), Point_2(2, 0), Point_2(3, 0),
    Point_2(7, 1), Point_2(8, 1), Point_2(9, 1), Point_2(10, 1),
    Point_2(14, 2), Point_2(15, 2), Point_2(16, 2), Point_2(17, 2),
    Point_2(19, 3), Point_2(20, 3), Point_2(21, 3), Point_2(22, 3)
  };

  assert(polyline_2.size() == 16);
  std::vector< std::pair< typename Kernel::Line_2, std::vector<Item> > > regions;
  CGAL::Shape_detection::region_growing_polylines(
    polyline_2, std::back_inserter(regions), CGAL::parameters::maximum_distance(0.01));
  assert(regions.size() == 4);

  assert(regions[0].second.size() == 4);
  assert(regions[1].second.size() == 4);
  assert(regions[2].second.size() == 4);
  assert(regions[3].second.size() == 4);
  return true;
}

template<class Kernel>
bool test_lines_segment_set_2() {

  using Point_2 = typename Kernel::Point_2;
  using Segment_2 = typename Kernel::Segment_2;

  using Segment_range = std::vector<Segment_2>;
  using Item = typename Segment_range::const_iterator;

  using Segment_map = CGAL::Dereference_property_map<const Segment_2, Item>;

  using Region_type = CGAL::Shape_detection::
    Segment_set::Least_squares_line_fit_region<Kernel, Item, Segment_map>;


  const Segment_range segments = {
    Segment_2(Point_2(0.1, 0.0), Point_2(0.5, 0.0)),
    Segment_2(Point_2(0.5, 0.0), Point_2(0.9, 0.0)),
    Segment_2(Point_2(0.9, 0.0), Point_2(0.9, 0.5)),
    Segment_2(Point_2(0.9, 0.5), Point_2(0.9, 0.9))
  };
  assert(segments.size() == 4);

  struct Neighbor_query {

    std::map<Item, std::vector<Item> > m_neighbors;
    Neighbor_query(const Segment_range& segments) {
      m_neighbors[segments.begin()] = { segments.begin() + 1 };
      m_neighbors[segments.begin() + 1] = { segments.begin(), segments.begin() + 2 };
      m_neighbors[segments.begin() + 2] = { segments.begin() + 1, segments.begin() + 3 };
      m_neighbors[segments.begin() + 3] = { segments.begin() + 2 };
    }

    void operator()(
      const Item query, std::vector<Item>& neighbors) const {
      neighbors.clear();
      const auto& data = m_neighbors.at(query);
      std::copy(data.begin(), data.end(), std::back_inserter(neighbors));
    }
  };
  using Region_growing = CGAL::Shape_detection::
    Region_growing<Neighbor_query, Region_type>;

  Neighbor_query neighbor_query(segments);
  Region_type region_type;

  std::vector< std::pair< typename Region_type::Primitive, std::vector<Item> > > regions;
  Region_growing region_growing(
    segments, neighbor_query, region_type);
  region_growing.detect(std::back_inserter(regions));
  assert(regions.size() == 2);
  assert(regions[0].second.size() == 2);
  assert(regions[1].second.size() == 2);
  return true;
}

template<class Kernel>
bool test_lines_segment_set_2_sorting() {

  using Point_2 = typename Kernel::Point_2;
  using Segment_2 = typename Kernel::Segment_2;

  using Segment_range = std::vector<Segment_2>;
  using Item = typename Segment_range::const_iterator;

  using Segment_map = CGAL::Dereference_property_map<const Segment_2, Item>;

  using Region_type = CGAL::Shape_detection::
    Segment_set::Least_squares_line_fit_region<Kernel, Item, Segment_map>;

  const Segment_range segments = {
    Segment_2(Point_2(0.1, 0.0), Point_2(0.5, 0.0)),
    Segment_2(Point_2(0.5, 0.0), Point_2(0.9, 0.0)),
    Segment_2(Point_2(0.9, 0.0), Point_2(0.9, 0.5)),
    Segment_2(Point_2(0.9, 0.5), Point_2(0.9, 0.9))
  };
  assert(segments.size() == 4);

  struct Neighbor_query {

    std::map<Item, std::vector<Item> > m_neighbors;
    Neighbor_query(const Segment_range& segments) {
      m_neighbors[segments.begin()] = { segments.begin() + 1 };
      m_neighbors[segments.begin() + 1] = { segments.begin(), segments.begin() + 2 };
      m_neighbors[segments.begin() + 2] = { segments.begin() + 1, segments.begin() + 3 };
      m_neighbors[segments.begin() + 3] = { segments.begin() + 2 };
    }

    void operator()(
      const Item query, std::vector<Item>& neighbors) const {
      neighbors.clear();
      const auto& data = m_neighbors.at(query);
      std::copy(data.begin(), data.end(), std::back_inserter(neighbors));
    }
  };

  using Region_growing = CGAL::Shape_detection::
    Region_growing<Neighbor_query, Region_type>;

  using Sorting = CGAL::Shape_detection::Segment_set::Least_squares_line_fit_sorting<Kernel, typename Segment_range::const_iterator, Neighbor_query, Segment_map>;

  Neighbor_query neighbor_query(segments);

  Sorting sorting(
    segments, neighbor_query);
  sorting.sort();

  Region_type region_type;

  std::vector<typename Region_growing::Primitive_and_region> regions;
  Region_growing region_growing(
    segments, neighbor_query, region_type, sorting.ordered());
  region_growing.detect(std::back_inserter(regions));
  assert(regions.size() == 2);
  assert(regions[0].second.size() == 2);
  assert(regions[1].second.size() == 2);
  return true;
}

template<class Kernel>
bool test_lines_segment_set_3() {

  using Point_3      = typename Kernel::Point_3;
  using Surface_mesh = CGAL::Surface_mesh<Point_3>;

  using Plane_region = CGAL::Shape_detection::
    Polygon_mesh::Least_squares_plane_fit_region<Kernel, Surface_mesh>;

  using One_ring_query = CGAL::Shape_detection::Polygon_mesh::One_ring_neighbor_query<Surface_mesh>;

  using Plane_sorting = CGAL::Shape_detection::Polygon_mesh::Least_squares_plane_fit_sorting<Kernel, Surface_mesh, One_ring_query>;

  using RG_planes = CGAL::Shape_detection::
    Region_growing<One_ring_query, Plane_region>;

  using Polyline_graph = CGAL::Shape_detection::
    Polygon_mesh::Polyline_graph<Surface_mesh>;
  using Segment_map = typename Polyline_graph::Segment_map;

  using Region_type = CGAL::Shape_detection::
    Segment_set::Least_squares_line_fit_region<Kernel, typename Surface_mesh::Edge_index, Segment_map>;
  using Sorting = CGAL::Shape_detection::
    Segment_set::Least_squares_line_fit_sorting<Kernel, typename Surface_mesh::Edge_index, Polyline_graph, Segment_map>;
  using RG_lines = CGAL::Shape_detection::
    Region_growing<Polyline_graph, Region_type>;

  std::ifstream in(CGAL::data_file_path("meshes/am.off"));
  CGAL::IO::set_ascii_mode(in);
  assert(in);

  Surface_mesh surface_mesh;
  in >> surface_mesh;
  in.close();

  const auto face_range = faces(surface_mesh);

  One_ring_query one_ring_query(surface_mesh);

  Plane_region plane_type(
    surface_mesh);

  // Sort face indices.
  Plane_sorting plane_sorting(
    surface_mesh, one_ring_query);
  plane_sorting.sort();

  // Create an instance of the region growing class.
  RG_planes rg_planes(
    face_range, one_ring_query, plane_type, plane_sorting.ordered());

  assert(surface_mesh.number_of_faces() == 7320);
  std::vector<typename RG_planes::Primitive_and_region> regions;
  rg_planes.detect(std::back_inserter(regions));
  assert(regions.size() == 9);

  assert(face_range.size() == 7320);

  Polyline_graph pgraph(surface_mesh, rg_planes.region_map());
  const auto& segment_range = pgraph.segment_range();

  Region_type region_type(CGAL::parameters::segment_map(pgraph.segment_map()));
  Sorting sorting(
    segment_range, pgraph, CGAL::parameters::segment_map(pgraph.segment_map()));
  sorting.sort();

  std::vector<typename RG_lines::Primitive_and_region> regions2;
  RG_lines region_growing(
    segment_range, pgraph, region_type, sorting.ordered());
  region_growing.detect(std::back_inserter(regions2));
  assert(regions2.size() == 21);
  return true;
}

template<class Kernel>
bool test_region_growing_lines() {
  assert(test_lines_points_with_normals<Kernel>());
  assert(test_lines_polylines_2<Kernel>());
  assert(test_lines_polylines_3<Kernel>());
  assert(test_polylines_equal_points<Kernel>());
  assert(test_lines_segment_set_2<Kernel>());
  assert(test_lines_segment_set_2_sorting<Kernel>());
  assert(test_lines_segment_set_3<Kernel>());

  return true;
}

template<class Kernel>
bool test_planes_points_with_normals() {

  using Point_3  = typename Kernel::Point_3;
  using Vector_3 = typename Kernel::Vector_3;

  const std::vector< std::pair<Point_3, Vector_3> > points_with_normals = {
    std::make_pair(Point_3(0.1, 0.0, 0.0), Vector_3(0.0, 0.0, 1.0)),
    std::make_pair(Point_3(0.5, 0.0, 0.0), Vector_3(0.0, 0.0, 1.0)),
    std::make_pair(Point_3(0.9, 0.0, 0.0), Vector_3(0.0, 0.0, 1.0)),
    std::make_pair(Point_3(0.1, 1.0, 0.0), Vector_3(0.0, 0.0, 1.0)),
    std::make_pair(Point_3(0.5, 1.0, 0.0), Vector_3(0.0, 0.0, 1.0)),
    std::make_pair(Point_3(0.9, 1.0, 0.0), Vector_3(0.0, 0.0, 1.0)),
    std::make_pair(Point_3(0.1, 2.0, 0.0), Vector_3(0.0, 0.0, 1.0)),
    std::make_pair(Point_3(0.5, 2.0, 0.0), Vector_3(0.0, 0.0, 1.0)),
    std::make_pair(Point_3(0.9, 2.0, 0.0), Vector_3(0.0, 0.0, 1.0))
  };

  assert(points_with_normals.size() == 9);
  std::vector<
    std::pair< typename Kernel::Plane_3, std::vector<typename std::vector<std::pair<Point_3, Vector_3> >::const_iterator> >
  > regions;

  CGAL::Shape_detection::region_growing_planes(
    points_with_normals,
    std::back_inserter(regions),
    CGAL::parameters::point_map(CGAL::First_of_pair_property_map<std::pair<Point_3, Vector_3> >()).
    normal_map(CGAL::Second_of_pair_property_map<std::pair<Point_3, Vector_3> >()));
  assert(regions.size() == 1);
  assert(regions[0].second.size() == 9);
  return true;
}

template<class Kernel>
bool test_planes_point_set() {

  using Point_3   = typename Kernel::Point_3;
  using Vector_3  = typename Kernel::Vector_3;
  using Point_set = CGAL::Point_set_3<Point_3, Vector_3>;

  std::vector< std::pair<Point_3, Vector_3> > points_with_normals = {
    std::make_pair(Point_3(0.1, 0.0, 1.0), Vector_3(0.0, 0.0, 1.0)),
    std::make_pair(Point_3(0.5, 0.0, 1.0), Vector_3(0.0, 0.0, 1.0)),
    std::make_pair(Point_3(0.9, 0.0, 1.0), Vector_3(0.0, 0.0, 1.0)),
    std::make_pair(Point_3(0.1, 1.0, 1.0), Vector_3(0.0, 0.0, 1.0)),
    std::make_pair(Point_3(0.5, 1.0, 1.0), Vector_3(0.0, 0.0, 1.0)),
    std::make_pair(Point_3(0.9, 1.0, 1.0), Vector_3(0.0, 0.0, 1.0)),
    std::make_pair(Point_3(0.1, 2.0, 1.0), Vector_3(0.0, 0.0, 1.0)),
    std::make_pair(Point_3(0.5, 2.0, 1.0), Vector_3(0.0, 0.0, 1.0)),
    std::make_pair(Point_3(0.9, 2.0, 1.0), Vector_3(0.0, 0.0, 1.0))
  };
  assert(points_with_normals.size() == 9);

  Point_set point_set(true);
  for (const auto& pwn : points_with_normals)
    point_set.insert(pwn.first, pwn.second);
  assert(point_set.size() == points_with_normals.size());

  points_with_normals.clear();
  assert(points_with_normals.size() == 0);

  std::vector< std::pair< typename Kernel::Plane_3, std::vector<typename Point_set::const_iterator> > > regions;
  CGAL::Shape_detection::region_growing_planes(
    point_set, std::back_inserter(regions));
  assert(regions.size() == 1);
  assert(regions[0].second.size() == 9);
  return true;
}

template<class Kernel>
bool test_planes_polyhedron() {

  using Point_3    = typename Kernel::Point_3;
  using Polyhedron = CGAL::Polyhedron_3<
    Kernel, CGAL::Polyhedron_items_3, CGAL::HalfedgeDS_vector>;

  Polyhedron polyhedron;
  const Point_3 p1(0, 0, 0);
  const Point_3 p2(1, 0, 0);
  const Point_3 p3(0, 1, 0);
  const Point_3 p4(0, 0, 1);
  const auto handle = polyhedron.make_tetrahedron(p1, p2, p3, p4);
  assert(polyhedron.is_tetrahedron(handle));

  assert(polyhedron.size_of_facets() == 4);
  std::vector< std::pair< typename Kernel::Plane_3, std::vector<typename boost::graph_traits<Polyhedron>::face_descriptor > > > regions;
  CGAL::Shape_detection::region_growing_planes_polygon_mesh(
    polyhedron, std::back_inserter(regions));
  assert(regions.size() == polyhedron.size_of_facets());
  return true;
}

template<class Kernel>
bool test_planes_surface_mesh() {

  using Point_3      = typename Kernel::Point_3;
  using Surface_mesh = CGAL::Surface_mesh<Point_3>;

  std::ifstream in(CGAL::data_file_path("meshes/am.off"));
  CGAL::IO::set_ascii_mode(in);
  assert(in);

  Surface_mesh surface_mesh;
  in >> surface_mesh;
  in.close();

  assert(surface_mesh.number_of_faces() == 7320);
  std::vector< std::pair< typename Kernel::Plane_3, std::vector<typename Surface_mesh::Face_index> > > regions;
  CGAL::Shape_detection::region_growing_planes_polygon_mesh(
    surface_mesh, std::back_inserter(regions));
  assert(regions.size() == 9);
  return true;
}

template<class Kernel>
bool test_region_growing_planes() {
  assert(test_planes_points_with_normals<Kernel>());
  assert(test_planes_point_set<Kernel>());
  assert(test_planes_polyhedron<Kernel>());
  assert(test_planes_surface_mesh<Kernel>());
  return true;
}

template<class Kernel>
bool test_region_growing_strict() {

  assert(test_region_growing_lines<Kernel>());
  assert(test_region_growing_planes<Kernel>());
  return true;
}

int main() {
  using SC = CGAL::Simple_cartesian<double>;
  using EPICK = CGAL::Exact_predicates_inexact_constructions_kernel;
  using EPECK = CGAL::Exact_predicates_exact_constructions_kernel;

  // ------>

  bool sc_test_success = true;
  if (!test_region_growing_strict<SC>())
    sc_test_success = false;
  std::cout << "rg_strict, sc_test_success: " << sc_test_success << std::endl;
  assert(sc_test_success);

  // ------>

  bool epick_test_success = true;
  if (!test_region_growing_strict<EPICK>())
    epick_test_success = false;
  std::cout << "rg_strict, epick_test_success: " << epick_test_success << std::endl;
  assert(epick_test_success);

  // ------>

  bool epeck_test_success = true;
  if (!test_region_growing_strict<EPECK>())
    epeck_test_success = false;
  std::cout << "rg_strict, epeck_test_success: " << epeck_test_success << std::endl;
  assert(epeck_test_success);

  const bool success = sc_test_success && epick_test_success && epeck_test_success;
  return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}

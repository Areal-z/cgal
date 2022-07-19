#include <CGAL/IO/PLY.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Iterator_range.h>
#include <CGAL/HalfedgeDS_vector.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Shape_detection/Region_growing/Region_growing.h>
#include <CGAL/Shape_detection/Region_growing/Polygon_mesh.h>
#include <CGAL/boost/graph/IO/polygon_mesh_io.h>
#include "include/utils.h"

// Typedefs.
using Kernel  = CGAL::Exact_predicates_exact_constructions_kernel;
using FT      = typename Kernel::FT;
using Point_3 = typename Kernel::Point_3;

// Choose the type of a container for a polygon mesh.
#define USE_SURFACE_MESH
#if defined(USE_SURFACE_MESH)
    using Polygon_mesh   = CGAL::Surface_mesh<Point_3>;
    using Face_range     = typename Polygon_mesh::Face_range;
    using Neighbor_query = CGAL::Shape_detection::Polygon_mesh::One_ring_neighbor_query<Polygon_mesh>;
    using Region_type    = CGAL::Shape_detection::Polygon_mesh::Least_squares_plane_fit_region<Kernel, Polygon_mesh>;
    using Sorting        = CGAL::Shape_detection::Polygon_mesh::Least_squares_plane_fit_sorting<Kernel, Polygon_mesh, Neighbor_query>;
#else
    using Polygon_mesh   = CGAL::Polyhedron_3<Kernel, CGAL::Polyhedron_items_3, CGAL::HalfedgeDS_vector>;
    using Face_range     = typename CGAL::Iterator_range<typename boost::graph_traits<Polygon_mesh>::face_iterator>;
    using Neighbor_query = CGAL::Shape_detection::Polygon_mesh::One_ring_neighbor_query<Polygon_mesh>;
    using Region_type    = CGAL::Shape_detection::Polygon_mesh::Least_squares_plane_fit_region<Kernel, Polygon_mesh>;
    using Sorting        = CGAL::Shape_detection::Polygon_mesh::Least_squares_plane_fit_sorting<Kernel, Polygon_mesh, Neighbor_query>;
#endif
using Region_growing = CGAL::Shape_detection::Region_growing<Neighbor_query, Region_type>;

int main(int argc, char *argv[]) {

  // Load data either from a local folder or a user-provided file.
  const bool is_default_input = argc > 1 ? false : true;
  const std::string filename = is_default_input ? CGAL::data_file_path("meshes/building.off") : argv[1];
  std::ifstream in(filename);
  CGAL::IO::set_ascii_mode(in);

  Polygon_mesh polygon_mesh;
  if (!CGAL::IO::read_polygon_mesh(filename, polygon_mesh)) {
    std::cerr << "ERROR: cannot read the input file!" << std::endl;
    return EXIT_FAILURE;
  }
  const Face_range face_range = faces(polygon_mesh);
  std::cout << "* number of input faces: " << face_range.size() << std::endl;
  assert(is_default_input && face_range.size() == 32245);

  // Default parameter values for the data file building.off.
  const FT          max_distance    = FT(1);
  const FT          max_angle       = FT(45);
  const std::size_t min_region_size = 5;

  // Create instances of the classes Neighbor_query and Region_type.
  Neighbor_query neighbor_query(polygon_mesh);

  Region_type region_type(
    polygon_mesh,
    CGAL::parameters::
    maximum_distance(max_distance).
    maximum_angle(max_angle).
    minimum_region_size(min_region_size));

  // Sort face indices.
  Sorting sorting(
    polygon_mesh, neighbor_query);
  sorting.sort();

  // Create an instance of the region growing class.
  Region_growing region_growing(
    face_range, neighbor_query, region_type, sorting.ordered());

  // Run the algorithm.
  std::vector<typename Region_growing::Primitive_and_region> regions;
  region_growing.detect(std::back_inserter(regions));
  std::cout << "* number of found planes: " << regions.size() << std::endl;
  assert(is_default_input && regions.size() == 355);

  const Region_growing::Region_map& map = region_growing.region_map();

  for (std::size_t i = 0; i < regions.size(); i++)
    for (auto& item : regions[i].second) {
      if (i != get(map, CGAL::Shape_detection::internal::conditional_deref<Region_growing::Item, typename Region_growing::Region_map::key_type>()(item))) {
        std::cout << "Region map incorrect" << std::endl;
      }
    }

  std::vector<typename Region_growing::Item> unassigned;
  region_growing.unassigned_items(face_range, std::back_inserter(unassigned));

  for (auto& item : unassigned) {
    if (std::size_t(-1) != get(map, CGAL::Shape_detection::internal::conditional_deref<Region_growing::Item, typename Region_growing::Region_map::key_type>()(item))) {
      std::cout << "Region map for unassigned incorrect" << std::endl;
    }
  }

  // Save regions to a file.
  const std::string fullpath = (argc > 2 ? argv[2] : "planes_polygon_mesh.ply");
  utils::save_polygon_mesh_regions(polygon_mesh, regions, fullpath);

  return EXIT_SUCCESS;
}

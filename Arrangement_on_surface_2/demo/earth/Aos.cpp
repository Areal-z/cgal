
#include "Aos.h"

#include <iostream>
#include <iterator>
#include <vector>

#include <qmath.h>
#include <qvector3d.h>

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Arrangement_on_surface_2.h>
#include <CGAL/Arr_extended_dcel.h>
#include <CGAL/Arr_geodesic_arc_on_sphere_traits_2.h>
#include <CGAL/Arr_spherical_topology_traits_2.h>
#include <CGAL/Vector_3.h>

#include "arr_print.h"
#include "Tools.h"

namespace {
  using Kernel = CGAL::Exact_predicates_exact_constructions_kernel;
  using Geom_traits = CGAL::Arr_geodesic_arc_on_sphere_traits_2<Kernel>;
  using Point = Geom_traits::Point_2;
  using Curve = Geom_traits::Curve_2;
  using Topol_traits = CGAL::Arr_spherical_topology_traits_2<Geom_traits>;
  using Arrangement = CGAL::Arrangement_on_surface_2<Geom_traits, Topol_traits>;

  // the following is from "arr_inexact_construction_segments.h":
  using Segment = Geom_traits::X_monotone_curve_2;
  using Vertex_handle = Arrangement::Vertex_handle;


  // Extended DCEL & Arrangement
  struct Flag
  {
    bool v;
    Flag() : v{ false } {}
    Flag(bool init) : v{ init } {}
  };

  using Ext_dcel = CGAL::Arr_extended_dcel<Geom_traits, Flag, Flag, Flag>;
  using Ext_topol_traits = CGAL::Arr_spherical_topology_traits_2<Geom_traits, 
                                                                      Ext_dcel>;
  using Ext_aos = CGAL::Arrangement_on_surface_2<Geom_traits, Ext_topol_traits>;



  using Dir3 = Kernel::Direction_3;
  std::ostream& operator << (std::ostream& os, const Dir3& d)
  {
    os << d.dx() << ", " << d.dy() << ", " << d.dz();
    return os;
  }

  using Approximate_point_2 = Geom_traits::Approximate_point_2;
  std::ostream& operator << (std::ostream& os, const Approximate_point_2& d)
  {
    os << d.dx() << ", " << d.dy() << ", " << d.dz();
    return os;
  }

  using Approximate_number_type = Geom_traits::Approximate_number_type;
  using Approximate_kernel = Geom_traits::Approximate_kernel;
  using Approximate_Vector_3 = CGAL::Vector_3<Approximate_kernel>;
  using Approximate_Direction_3 = Approximate_kernel::Direction_3;
  using Direction_3 = Kernel::Direction_3;


  std::ostream& operator << (std::ostream& os, const Approximate_Vector_3& v)
  {
    os << v.x() << ", " << v.y() << ", " << v.z();
    //os << v.hx() << ", " << v.hy() << ", " << v.hz() << ", " << v.hw();
    return os;
  }


  //---------------------------------------------------------------------------
  // below are the helper functions used to construct the arcs from KML data
  // TODO: Revisit handling of INNER & OUTER boundaries
  using Curves = std::vector<Curve>;

  // get curves for the given kml placemark
  // NOTE: this is defined here to keep the definitions local to this cpp file
  Curves  get_arcs(const Kml::Placemark& placemark)
  {
    Geom_traits traits;
    auto ctr_p = traits.construct_point_2_object();
    auto ctr_cv = traits.construct_curve_2_object();

    std::vector<Curve>  xcvs;
    for (const auto& polygon : placemark.polygons)
    {
      // colect all rings into a single list (FOR NOW!!!)
      // TO-DO: PROCESS OUTER & INNER BOUNDARIES SEPARATELY!!!
      Kml::LinearRings linear_rings;
      linear_rings.push_back(polygon.outer_boundary);
      for (const auto& inner_boundary : polygon.inner_boundaries)
        linear_rings.push_back(inner_boundary);


      // convert the nodes to points on unit-sphere
      for (const auto& lring : linear_rings)
      {
        std::vector<Approximate_Vector_3>  sphere_points;
        for (const auto& node : lring.nodes)
        {
          const auto p = node.get_coords_3d();
          Approximate_Vector_3 v(p.x, p.y, p.z);
          sphere_points.push_back(v);
        }

        // add geodesic arcs for the current LinearRing
        int num_points = sphere_points.size();
        for (int i = 0; i < num_points - 1; i++)
        {
          const auto p1 = sphere_points[i];
          const auto p2 = sphere_points[i + 1];
          xcvs.push_back(ctr_cv(ctr_p(p1.x(), p1.y(), p1.z()),
                                ctr_p(p2.x(), p2.y(), p2.z())));
        }
      }
    }

    return xcvs;
  }


  // this one is used by the Aos::check and Aos::ext_check functions
  int num_counted_nodes = 0;
  int num_counted_arcs = 0;
  int num_counted_polygons = 0;
  template<typename Arr_type>
  Curves  get_arcs(const Kml::Placemarks& placemarks, Arr_type& arr)
  {
    Geom_traits traits;
    auto ctr_p = traits.construct_point_2_object();
    auto ctr_cv = traits.construct_curve_2_object();

    num_counted_nodes = 0;
    num_counted_arcs = 0;
    num_counted_polygons = 0;
    std::vector<Curve>  xcvs;
    for (const auto& pm : placemarks)
    {
      for (const auto& polygon : pm.polygons)
      {
        num_counted_polygons++;

        // colect all rings into a single list (FOR NOW!!!)
        // TO-DO: PROCESS OUTER & INNER BOUNDARIES SEPARATELY!!!
        Kml::LinearRings linear_rings;
        linear_rings.push_back(polygon.outer_boundary);
        for (const auto& inner_boundary : polygon.inner_boundaries)
          linear_rings.push_back(inner_boundary);

        // loop on outer and inner boundaries 
        for (const auto& lring : linear_rings)
        {
          // convert the nodes to points on unit-sphere
          std::vector<Approximate_Vector_3>  sphere_points;
          for (const auto& node : lring.nodes)
          {
            num_counted_nodes++;
            const auto p = node.get_coords_3d();
            Approximate_Vector_3  v(p.x, p.y, p.z);
            sphere_points.push_back(v);
            CGAL::insert_point(arr, ctr_p(p.x, p.y, p.z));
          }

          // add curves
          int num_points = sphere_points.size();
          for (int i = 0; i < num_points - 1; i++)
          {
            num_counted_arcs++;
            const auto p1 = sphere_points[i];
            const auto p2 = sphere_points[i + 1];
            auto xcv = ctr_cv(ctr_p(p1.x(), p1.y(), p1.z()),
                              ctr_p(p2.x(), p2.y(), p2.z()));
            xcvs.push_back(xcv);
          }
        }
      }
    }
    return xcvs;
  }
}


Aos::Approx_arc Aos::get_approx_identification_curve(double error)
{
  Geom_traits traits;
  auto ctr_p = traits.construct_point_2_object();
  auto ctr_cv = traits.construct_curve_2_object();

  // identification curve (meridian pierced by NEGATIVE Y-AXIS)
  auto xcv = ctr_cv(ctr_p(0, 0, -1), ctr_p(0, 0, 1), Dir3(0,1,0));

  auto approx = traits.approximate_2_object();
  Approx_arc approx_arc;
  {
    std::vector<Approximate_point_2> v;
    auto oi2 = approx(xcv, error, std::back_insert_iterator(v));
    for (const auto& p : v)
    {
      const QVector3D arc_point(p.dx(), p.dy(), p.dz());
      approx_arc.push_back(arc_point);
    }
  }

  return approx_arc;
}

Aos::Approx_arcs Aos::get_approx_arcs(double error)
{
  Geom_traits traits;
  Arrangement arr(&traits);

  auto ctr_p = traits.construct_point_2_object();
  auto ctr_cv = traits.construct_curve_2_object();


  std::vector<Curve>  xcvs;
  xcvs.push_back(ctr_cv(ctr_p(1, 0, 0), ctr_p(0, 1, 0)));
  xcvs.push_back(ctr_cv(ctr_p(1, 0, 0), ctr_p(0, 0, 1)));
  xcvs.push_back(ctr_cv(ctr_p(0, 1, 0), ctr_p(0, 0, 1)));
  //xcvs.push_back(ctr_cv(ctr_p(1, 0, 0), ctr_p(0, 1, 0), Dir3(0, 0, -1)));
  //xcvs.push_back(ctr_cv(Dir3(0, 0, -1)));

  auto approx = traits.approximate_2_object();


  std::vector<std::vector<QVector3D>>  arcs;
  for (const auto& xcv : xcvs)
  {
    std::vector<Approximate_point_2> v;
    auto oi2 = approx(xcv, error, std::back_insert_iterator(v));

    std::vector<QVector3D> arc_points;
    for (const auto& p : v)
    {
      const QVector3D arc_point(p.dx(), p.dy(), p.dz());
      arc_points.push_back(arc_point);
    }
    arcs.push_back(std::move(arc_points));
  }
  //std::cout << "offset count = " << m_arc_offsets.size() << std::endl;

  return arcs;
}
Aos::Approx_arcs Aos::get_approx_arcs(const Kml::Placemark& placemark, double error)
{
  Geom_traits traits;
  auto ctr_p = traits.construct_point_2_object();
  auto ctr_cv = traits.construct_curve_2_object();

  auto xcvs = get_arcs(placemark);

  auto approx = traits.approximate_2_object();
  std::vector<std::vector<QVector3D>>  arcs;
  for (const auto& xcv : xcvs)
  {
    std::vector<Approximate_point_2> v;
    auto oi2 = approx(xcv, error, std::back_insert_iterator(v));

    std::vector<QVector3D> arc_points;
    for (const auto& p : v)
    {
      const QVector3D arc_point(p.dx(), p.dy(), p.dz());
      arc_points.push_back(arc_point);
    }
    arcs.push_back(std::move(arc_points));
  }
  //std::cout << "offset count = " << m_arc_offsets.size() << std::endl;

  return arcs;
}

void Aos::check(const Kml::Placemarks& placemarks)
{
  Geom_traits traits;
  Arrangement arr(&traits);

  auto xcvs = get_arcs(placemarks, arr);
  std::cout << "-------------------------------\n";
  std::cout << "num arr vertices (before adding arcs) = " << 
                                          arr.number_of_vertices() << std::endl;
  // add arcs
  for(auto xcv : xcvs)
    CGAL::insert_curve(arr, xcv);

  std::cout << "-------------------------------\n";
  std::cout << "num nodes = " << num_counted_nodes << std::endl;
  std::cout << "num arr vertices = " << arr.number_of_vertices() << std::endl;

  std::cout << "-------------------------------\n";
  std::cout << "num counted arcs = " << num_counted_arcs << std::endl;
  std::cout << "num arr edges = " << arr.number_of_edges() << std::endl;

  std::cout << "-------------------------------\n";
  std::cout << "num polygons = " << num_counted_polygons << std::endl;
  std::cout << "num arr faces = " << arr.number_of_faces() << std::endl;
}

std::vector<QVector3D> Aos::ext_check(const Kml::Placemarks& placemarks)
{
  // Construct the arrangement from 12 geodesic arcs.
  Geom_traits traits;
  Ext_aos arr(&traits);

  auto xcvs = get_arcs(placemarks, arr);

  // MARK all vertices as true
  for (auto vit = arr.vertices_begin(); vit != arr.vertices_end(); ++vit)
  {
    vit->set_data(Flag(true));
  }

  std::cout << "-------------------------------\n";
  std::cout << "num arr vertices (before adding arcs) = " <<
    arr.number_of_vertices() << std::endl;

  // add arcs
  for (auto& xcv : xcvs)
    CGAL::insert_curve(arr, xcv);

  // extract all vertices that are ADDED when inserting the arcs!
  int num_created_vertices = 0;
  std::vector<QVector3D> created_vertices;
  auto approx = traits.approximate_2_object();
  for (auto vit = arr.vertices_begin(); vit != arr.vertices_end(); ++vit)
  {
    auto& d = vit->data();
    if (vit->data().v == false)
    {
      std::cout << "-------------------------------------\n";
      std::cout << vit->point() << std::endl;

      if (2 == vit->degree())
        ;//continue;

      if (1 == vit->degree())
      {
        auto p = vit->point();
        auto p2 = p.location();
        std::cout << "   deg-1 vertex = " << p << std::endl;
        std::cout << "   deg-1 vertex: " << std::boolalpha << vit->incident_halfedges()->target()->data().v << std::endl;
      }


      num_created_vertices++;
      auto p = vit->point();
      auto ap = approx(p);
      QVector3D new_vertex(ap.dx(), ap.dy(), ap.dz());
      new_vertex.normalize();
      std::cout << new_vertex << std::endl;
      std::cout << "degree = " << vit->degree() << std::endl;
      
      created_vertices.push_back(new_vertex);

      // find the arcs that are adjacent to this vertex
      const auto first = vit->incident_halfedges();
      auto curr = first;
      do {

      } while (++curr != first);
    
      std::cout << "\n";
    }
  }
  Kml::Node n{ 180.0, -84.71338 };
  std::cout << "Node itself = " << n.get_coords_3d() << std::endl;
  std::cout << "*** num created vertices = " << num_created_vertices << std::endl;

  std::cout << "-------------------------------\n";
  std::cout << "num nodes = " << num_counted_nodes << std::endl;
  std::cout << "num arr vertices = " << arr.number_of_vertices() << std::endl;

  std::cout << "-------------------------------\n";
  std::cout << "num counted arcs = " << num_counted_arcs << std::endl;
  std::cout << "num arr edges = " << arr.number_of_edges() << std::endl;

  std::cout << "-------------------------------\n";
  std::cout << "num polygons = " << num_counted_polygons << std::endl;
  std::cout << "num arr faces = " << arr.number_of_faces() << std::endl;

  return created_vertices;
}


std::vector<QVector3D>  Aos::ext_check_id_based(Kml::Placemarks& placemarks)
{
  // Construct the arrangement from 12 geodesic arcs.
  Geom_traits traits, traits2;
  Ext_aos arr(&traits), arr2(&traits2);

  // 
  //auto nodes = Kml::generate_ids(placemarks);
  auto nodes = Kml::generate_ids_approx(placemarks, 0.001);

  //Segment s()
  auto ctr_p = traits.construct_point_2_object();
  auto ctr_cv = traits.construct_curve_2_object();

  int num_counted_arcs = 0;
  int num_counted_polygons = 0;
  // 
  std::vector<Point> points;
  using Vertex_handle = decltype(CGAL::insert_point(arr2, ctr_p(1, 0, 0)));
  std::vector<Vertex_handle> vertices;
  for (const auto& node : nodes)
  {
    auto n = node.get_coords_3d();
    auto p = ctr_p(n.x, n.y, n.z);
    auto v = CGAL::insert_point(arr, p);
    points.push_back(p);
    vertices.push_back(v);
    //arr.insert_at_vertices(Segment(p, p), v, v);
  }
  std::cout << "num nodes = " << nodes.size()   << std::endl;
  std::cout << "num points = " << points.size() << std::endl;
  // MARK all vertices as true
  for (auto vit = arr.vertices_begin(); vit != arr.vertices_end(); ++vit)
  {
    vit->set_data(Flag(true));
  }


  for (auto& placemark : placemarks)
  {
    for (auto& polygon : placemark.polygons)
    {
      num_counted_polygons++;

      //auto& pnodes = polygon.outer_boundary.nodes;
      //int num_nodes = pnodes.size();
      //for (int i = 0; i < num_nodes - 1; ++i)
      //{
      //  num_counted_arcs++;
      //  const auto node1 = pnodes[i];
      //  const auto node2 = pnodes[i + 1];
      //  auto n1 = node1.get_coords_3d();
      //  auto n2 = node2.get_coords_3d();
      //  auto p1 = ctr_p(n1.x, n1.y, n1.z);
      //  auto p2 = ctr_p(n2.x, n2.y, n2.z);
      //  
      //  //std::cout << p1 << std::endl;
      //  //std::cout << p2 << std::endl;
      //  //Segment s(p1, p2);
      //  //auto v1 = vertices[nid1];
      //  //auto v2 = vertices[nid2];
      //  //arr.insert_at_vertices(Segment(p1, p2), v1, v2);
      //  CGAL::insert(arr, ctr_cv(p1, p2));
      //}

      // TO DO : ADD the outer boundaries!
      auto& ids = polygon.outer_boundary.ids;
      int num_nodes = ids.size();
      for (int i = 0; i < num_nodes - 1; ++i)
      {
        num_counted_arcs++;
        const auto nid1 = ids[i];
        const auto nid2 = ids[i + 1];
        //assert(nodes[nid1] == polygon.outer_boundary.nodes[i]);
        //assert(nodes[nid2] == polygon.outer_boundary.nodes[i+1]);
        auto p1 = points[nid1];
        auto p2 = points[nid2];
        //std::cout << p1 << std::endl;
        //std::cout << p2 << std::endl;
        //Segment s(p1, p2);
        //auto v1 = vertices[nid1];
        //auto v2 = vertices[nid2];
        //Segment s(v1->point(), v2->point());
        //arr.insert_from_left_vertex()
        //arr.insert_at_vertices(s, v1, v2);
        //arr.insert_at_vertices(Segment(p1, p2), v1, v2);
        CGAL::insert(arr, ctr_cv(p1,p2));
      }
    }
  }


  std::cout << "-------------------------------\n";
  std::cout << "num arr vertices (before adding arcs) = " <<
    arr.number_of_vertices() << std::endl;


  // extract all vertices that are ADDED when inserting the arcs!
  int num_created_vertices = 0;
  std::vector<QVector3D> created_vertices;
  auto approx = traits.approximate_2_object();
  for (auto vit = arr.vertices_begin(); vit != arr.vertices_end(); ++vit)
  {
    auto& d = vit->data();
    if (vit->data().v == false)
    {
      std::cout << "-------------------------------------\n";
      std::cout << vit->point() << std::endl;

      if (2 == vit->degree())
        ;//continue;

      if (1 == vit->degree())
      {
        auto p = vit->point();
        auto p2 = p.location();
        std::cout << "deg-1 vertex = " << p << std::endl;
        std::cout << "deg-1 vertex: " << std::boolalpha << vit->incident_halfedges()->target()->data().v << std::endl;
      }


      num_created_vertices++;
      auto p = vit->point();
      auto ap = approx(p);
      QVector3D new_vertex(ap.dx(), ap.dy(), ap.dz());
      new_vertex.normalize();
      std::cout << new_vertex << std::endl;
      std::cout << "degree = " << vit->degree() << std::endl;

      created_vertices.push_back(new_vertex);

      //// find the arcs that are adjacent to this vertex
      //const auto first = vit->incident_halfedges();
      //auto curr = first;
      //do {

      //} while (++curr != first);
      std::cout << std::endl;
    }
  }
  std::cout << "*** num created vertices = " << num_created_vertices << std::endl;

  std::cout << "-------------------------------\n";
  std::cout << "num nodes = " << nodes.size() << std::endl;
  std::cout << "num arr vertices = " << arr.number_of_vertices() << std::endl;

  std::cout << "-------------------------------\n";
  std::cout << "num counted arcs = " << num_counted_arcs << std::endl;
  std::cout << "num arr edges = " << arr.number_of_edges() << std::endl;

  std::cout << "-------------------------------\n";
  std::cout << "num polygons = " << num_counted_polygons << std::endl;
  std::cout << "num arr faces = " << arr.number_of_faces() << std::endl;

  return created_vertices;
}
// Copyright (c) 2018 INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Florent Lafarge, Simon Giraudot, Thien Hoang, Dmitry Anisimov
//

#ifndef CGAL_SHAPE_DETECTION_REGION_GROWING_INTERNAL_UTILS_H
#define CGAL_SHAPE_DETECTION_REGION_GROWING_INTERNAL_UTILS_H

#include <CGAL/license/Shape_detection.h>

// STL includes.
#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <typeinfo>

// Boost headers.
#include <boost/mpl/has_xxx.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/graph_traits.hpp>

// Named parameters.
#include <CGAL/boost/graph/named_params_helper.h>
#include <CGAL/boost/graph/Named_function_parameters.h>

// Face graph includes.
#include <CGAL/Iterator_range.h>
#include <CGAL/HalfedgeDS_vector.h>
#include <CGAL/boost/graph/iterator.h>
#include <CGAL/boost/graph/graph_traits_Surface_mesh.h>
#include <CGAL/boost/graph/graph_traits_Polyhedron_3.h>

// CGAL includes.
#include <CGAL/assertions.h>
#include <CGAL/number_utils.h>
#include <CGAL/Cartesian_converter.h>
#include <CGAL/Eigen_diagonalize_traits.h>
#include <CGAL/linear_least_squares_fitting_2.h>
#include <CGAL/linear_least_squares_fitting_3.h>
#include <CGAL/boost/iterator/counting_iterator.hpp>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

namespace CGAL {
namespace Shape_detection {
namespace internal {

  template<typename GeomTraits>
  class Default_sqrt {

  private:
    using Traits = GeomTraits;
    using FT = typename Traits::FT;

  public:
    const FT operator()(const FT value) const {
      CGAL_precondition(value >= FT(0));
      return static_cast<FT>(CGAL::sqrt(CGAL::to_double(value)));
    }
  };

  BOOST_MPL_HAS_XXX_TRAIT_NAMED_DEF(Has_nested_type_Sqrt, Sqrt, false)

  // Case: do_not_use_default = false.
  template<typename GeomTraits,
  bool do_not_use_default = Has_nested_type_Sqrt<GeomTraits>::value>
  class Get_sqrt {

  public:
    using Traits = GeomTraits;
    using Sqrt = Default_sqrt<Traits>;

    static Sqrt sqrt_object(const Traits& ) {
      return Sqrt();
    }
  };

  // Case: do_not_use_default = true.
  template<typename GeomTraits>
  class Get_sqrt<GeomTraits, true> {

  public:
    using Traits = GeomTraits;
    using Sqrt = typename Traits::Sqrt;

    static Sqrt sqrt_object(const Traits& traits) {
      return traits.sqrt_object();
    }
  };

  template<typename FT>
  struct Compare_scores {

    const std::vector<FT>& m_scores;
    Compare_scores(const std::vector<FT>& scores) :
    m_scores(scores)
    { }

    bool operator()(const std::size_t i, const std::size_t j) const {
      CGAL_precondition(i < m_scores.size());
      CGAL_precondition(j < m_scores.size());
      return m_scores[i] > m_scores[j];
    }
  };

  template<
  typename Traits,
  typename InputRange,
  typename ItemMap>
  std::pair<typename Traits::Line_2, typename Traits::FT>
  create_line_2(
    const InputRange& input_range, const ItemMap item_map,
    const std::vector<std::size_t>& region, const Traits&) {

    using FT = typename Traits::FT;
    using Line_2 = typename Traits::Line_2;

    using ITraits = CGAL::Exact_predicates_inexact_constructions_kernel;
    using IConverter = Cartesian_converter<Traits, ITraits>;

    using IFT = typename ITraits::FT;
    using IPoint_2 = typename ITraits::Point_2;
    using ILine_2 = typename ITraits::Line_2;

    using Input_type = typename ItemMap::value_type;
    using Item = typename std::conditional<
      std::is_same<typename Traits::Point_2, Input_type>::value,
      typename ITraits::Point_2, typename ITraits::Segment_2 >::type;

    std::vector<Item> items;
    CGAL_precondition(region.size() > 0);
    items.reserve(region.size());
    const IConverter iconverter = IConverter();

    for (const std::size_t item_index : region) {
      CGAL_precondition(item_index < input_range.size());
      const auto& key = *(input_range.begin() + item_index);
      const auto& item = get(item_map, key);
      items.push_back(iconverter(item));
    }
    CGAL_postcondition(items.size() == region.size());

    ILine_2 fitted_line;
    IPoint_2 fitted_centroid;
    const IFT score = CGAL::linear_least_squares_fitting_2(
      items.begin(), items.end(),
      fitted_line, fitted_centroid,
      CGAL::Dimension_tag<0>(), ITraits(),
      CGAL::Eigen_diagonalize_traits<IFT, 2>());

    const Line_2 line(
      static_cast<FT>(fitted_line.a()),
      static_cast<FT>(fitted_line.b()),
      static_cast<FT>(fitted_line.c()));
    return std::make_pair(line, static_cast<FT>(score));
  }

  template<
  typename Traits,
  typename InputRange,
  typename ItemMap>
  std::pair<typename Traits::Line_3, typename Traits::FT>
  create_line_3(
    const InputRange& input_range, const ItemMap item_map,
    const std::vector<std::size_t>& region, const Traits&) {

    using FT = typename Traits::FT;
    using Line_3 = typename Traits::Line_3;
    using Point_3 = typename Traits::Point_3;
    using Direction_3 = typename Traits::Direction_3;

    using ITraits = CGAL::Exact_predicates_inexact_constructions_kernel;
    using IConverter = Cartesian_converter<Traits, ITraits>;

    using IFT = typename ITraits::FT;
    using IPoint_3 = typename ITraits::Point_3;
    using ILine_3 = typename ITraits::Line_3;

    using Input_type = typename ItemMap::value_type;
    using Item = typename std::conditional<
      std::is_same<typename Traits::Point_3, Input_type>::value,
      typename ITraits::Point_3, typename ITraits::Segment_3 >::type;

    std::vector<Item> items;
    CGAL_precondition(region.size() > 0);
    items.reserve(region.size());
    const IConverter iconverter = IConverter();

    for (const std::size_t item_index : region) {
      CGAL_precondition(item_index < input_range.size());
      const auto& key = *(input_range.begin() + item_index);
      const auto& item = get(item_map, key);
      items.push_back(iconverter(item));
    }
    CGAL_postcondition(items.size() == region.size());

    ILine_3 fitted_line;
    IPoint_3 fitted_centroid;
    const IFT score = CGAL::linear_least_squares_fitting_3(
      items.begin(), items.end(),
      fitted_line, fitted_centroid,
      CGAL::Dimension_tag<0>(), ITraits(),
      CGAL::Eigen_diagonalize_traits<IFT, 3>());

    const auto p = fitted_line.point(0);
    const auto d = fitted_line.direction();
    const Point_3 init(
      static_cast<FT>(p.x()),
      static_cast<FT>(p.y()),
      static_cast<FT>(p.z()));
    const Direction_3 direction(
      static_cast<FT>(d.dx()),
      static_cast<FT>(d.dy()),
      static_cast<FT>(d.dz()));
    const Line_3 line(init, direction);
    return std::make_pair(line, static_cast<FT>(score));
  }

  template<
  typename Traits,
  typename InputRange,
  typename ItemMap>
  std::pair<typename Traits::Plane_3, typename Traits::FT>
  create_plane(
    const InputRange& input_range, const ItemMap item_map,
    const std::vector<std::size_t>& region, const Traits&) {

    using FT = typename Traits::FT;
    using Plane_3 = typename Traits::Plane_3;

    using ITraits = CGAL::Exact_predicates_inexact_constructions_kernel;
    using IConverter = Cartesian_converter<Traits, ITraits>;

    using IFT = typename ITraits::FT;
    using IPoint_3 = typename ITraits::Point_3;
    using IPlane_3 = typename ITraits::Plane_3;

    using Input_type = typename ItemMap::value_type;
    using Item = typename std::conditional<
      std::is_same<typename Traits::Point_3, Input_type>::value,
      typename ITraits::Point_3, typename ITraits::Segment_3 >::type;

    std::vector<Item> items;
    CGAL_precondition(region.size() > 0);
    items.reserve(region.size());
    const IConverter iconverter = IConverter();

    for (const std::size_t item_index : region) {
      CGAL_precondition(item_index < input_range.size());
      const auto& key = *(input_range.begin() + item_index);
      const auto& item = get(item_map, key);
      items.push_back(iconverter(item));
    }
    CGAL_postcondition(items.size() == region.size());

    IPlane_3 fitted_plane;
    IPoint_3 fitted_centroid;
    const IFT score = CGAL::linear_least_squares_fitting_3(
      items.begin(), items.end(),
      fitted_plane, fitted_centroid,
      CGAL::Dimension_tag<0>(), ITraits(),
      CGAL::Eigen_diagonalize_traits<IFT, 3>());

    const Plane_3 plane(
      static_cast<FT>(fitted_plane.a()),
      static_cast<FT>(fitted_plane.b()),
      static_cast<FT>(fitted_plane.c()),
      static_cast<FT>(fitted_plane.d()));
    return std::make_pair(plane, static_cast<FT>(score));
  }

  template<
  typename Traits,
  typename FaceGraph,
  typename FaceRange,
  typename VertexToPointMap>
  std::pair<typename Traits::Plane_3, typename Traits::FT>
  create_plane_from_faces(
    const FaceGraph& face_graph,
    const FaceRange& face_range,
    const VertexToPointMap vertex_to_point_map,
    const std::vector<std::size_t>& region, const Traits&) {

    using FT = typename Traits::FT;
    using Plane_3 = typename Traits::Plane_3;

    using ITraits = CGAL::Exact_predicates_inexact_constructions_kernel;
    using IConverter = Cartesian_converter<Traits, ITraits>;

    using IFT = typename ITraits::FT;
    using IPoint_3 = typename ITraits::Point_3;
    using IPlane_3 = typename ITraits::Plane_3;

    std::vector<IPoint_3> points;
    CGAL_precondition(region.size() > 0);
    points.reserve(region.size());
    const IConverter iconverter = IConverter();

    for (const std::size_t face_index : region) {
      CGAL_precondition(face_index < face_range.size());
      const auto face = *(face_range.begin() + face_index);

      const auto hedge = halfedge(face, face_graph);
      const auto vertices = vertices_around_face(hedge, face_graph);
      CGAL_postcondition(vertices.size() > 0);

      for (const auto vertex : vertices) {
        const auto& point = get(vertex_to_point_map, vertex);
        points.push_back(iconverter(point));
      }
    }
    CGAL_postcondition(points.size() >= region.size());

    IPlane_3 fitted_plane;
    IPoint_3 fitted_centroid;
    const IFT score = CGAL::linear_least_squares_fitting_3(
      points.begin(), points.end(),
      fitted_plane, fitted_centroid,
      CGAL::Dimension_tag<0>(), ITraits(),
      CGAL::Eigen_diagonalize_traits<IFT, 3>());

    const Plane_3 plane(
      static_cast<FT>(fitted_plane.a()),
      static_cast<FT>(fitted_plane.b()),
      static_cast<FT>(fitted_plane.c()),
      static_cast<FT>(fitted_plane.d()));
    return std::make_pair(plane, static_cast<FT>(score));
  }

} // namespace internal
} // namespace Shape_detection
} // namespace CGAL

#endif // CGAL_SHAPE_DETECTION_REGION_GROWING_INTERNAL_UTILS_H

// Copyright (c) 2020 GeometryFactory (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Simon Giraudot
//

#ifndef CGAL_SHAPE_DETECTION_REGION_GROWING_POINT_SET_LEAST_SQUARES_CYLINDER_FIT_REGION_H
#define CGAL_SHAPE_DETECTION_REGION_GROWING_POINT_SET_LEAST_SQUARES_CYLINDER_FIT_REGION_H

#include <CGAL/license/Shape_detection.h>

// Boost includes.
#include <boost/unordered_map.hpp>

// Internal includes.
#include <CGAL/Shape_detection/Region_growing/internal/utils.h>

namespace CGAL {
namespace Shape_detection {
namespace Point_set {

  /*!
    \ingroup PkgShapeDetectionRGOnPoints

    \brief Region type based on the quality of the least squares cylinder
    fit applied to 3D points.

    This class fits an infinite cylinder to chunks of points in a 3D
    point set and controls the quality of this fit.  If all quality
    conditions are satisfied, the chunk is accepted as a valid region,
    otherwise rejected.

    \tparam GeomTraits
    a model of `Kernel`

    \tparam InputRange
    a model of `ConstRange` whose iterator type is `RandomAccessIterator`

    \tparam PointMap
    a model of `ReadablePropertyMap` whose key type is the value type of the input
    range and value type is `Kernel::Point_3`

    \tparam NormalMap
    a model of `ReadablePropertyMap` whose key type is the value type of the input
    range and value type is `Kernel::Vector_3`

    \cgalModels `RegionType`
  */
  template<
  typename GeomTraits,
  typename InputRange,
  typename PointMap,
  typename NormalMap>
  class Least_squares_cylinder_fit_region {

  public:
    /// \name Types
    /// @{

    /// \cond SKIP_IN_MANUAL
    using Traits = GeomTraits;
    using Input_range = InputRange;
    using Point_map = PointMap;
    using Normal_map = NormalMap;
    /// \endcond

    /// Number type.
    typedef typename GeomTraits::FT FT;

    /// Item type.
    using Item = typename InputRange::const_iterator;
    using Region = std::vector<Item>;

    /// Primitive
    using Primitive = struct P {
      P(const typename Traits::Line_3& a, const typename Traits::FT r) : axis(a), radius(r) {}
      typename Traits::Line_3 axis;
      typename Traits::FT radius;
    };
    using Result_type = std::vector<std::pair<Primitive, Region> >;

    /// Region map
    using Region_unordered_map = boost::unordered_map<Item, std::size_t, internal::hash_item<Item> >;
    using Region_index_map = boost::associative_property_map<Region_unordered_map>;
    /// @}

  private:
    using Point_3 = typename Traits::Point_3;
    using Vector_3 = typename Traits::Vector_3;
    using Line_3 = typename Traits::Line_3;

    using Squared_distance_3 = typename Traits::Compute_squared_distance_3;
    using Get_sqrt = internal::Get_sqrt<Traits>;
    using Sqrt = typename Get_sqrt::Sqrt;

  public:
    /// \name Initialization
    /// @{

    /*!
      \brief initializes all internal data structures.

      \tparam NamedParameters
      a sequence of \ref bgl_namedparameters "Named Parameters"

      \param input_range
      an instance of `InputRange` with 3D points and
      corresponding 3D normal vectors

      \param np
      a sequence of \ref bgl_namedparameters "Named Parameters"
      among the ones listed below

      \cgalNamedParamsBegin
        \cgalParamNBegin{maximum_distance}
          \cgalParamDescription{the maximum distance from a point to a cylinder}
          \cgalParamType{`GeomTraits::FT`}
          \cgalParamDefault{1}
        \cgalParamNEnd
        \cgalParamNBegin{maximum_angle}
          \cgalParamDescription{the maximum angle in degrees between
          the normal of a point and the radius of a cylinder}
          \cgalParamType{`GeomTraits::FT`}
          \cgalParamDefault{25 degrees}
        \cgalParamNEnd
        \cgalParamNBegin{cosine_value}
          \cgalParamDescription{the cos value computed as `cos(maximum_angle * PI / 180)`,
          this parameter can be used instead of the `maximum_angle`}
          \cgalParamType{`GeomTraits::FT`}
          \cgalParamDefault{`cos(25 * PI / 180)`}
        \cgalParamNEnd
        \cgalParamNBegin{minimum_region_size}
          \cgalParamDescription{the minimum number of 3D points a region must have}
          \cgalParamType{`std::size_t`}
          \cgalParamDefault{3}
        \cgalParamNEnd
        \cgalParamNBegin{minimum_radius}
          \cgalParamDescription{the radius below which an estimated cylinder
          is considered as invalid and discarded}
          \cgalParamType{`GeomTraits::FT`}
          \cgalParamDefault{0, no limit}
        \cgalParamNEnd
        \cgalParamNBegin{maximum_radius}
          \cgalParamDescription{the radius above which an estimated cylinder
          is considered as invalid and discarded.}
          \cgalParamType{`GeomTraits::FT`}
          \cgalParamDefault{+infinity, no limit}
        \cgalParamNEnd
        \cgalParamNBegin{point_map}
          \cgalParamDescription{an instance of `PointMap` that maps an item from `input_range`
          to `Kernel::Point_3`}
          \cgalParamDefault{`PointMap()`}
        \cgalParamNEnd
        \cgalParamNBegin{normal_map}
          \cgalParamDescription{ an instance of `NormalMap` that maps an item from `input_range`
          to `Kernel::Vector_3`}
          \cgalParamDefault{`NormalMap()`}
        \cgalParamNEnd
        \cgalParamNBegin{geom_traits}
          \cgalParamDescription{an instance of `GeomTraits`}
          \cgalParamDefault{`GeomTraits()`}
        \cgalParamNEnd
      \cgalNamedParamsEnd

      \pre `input_range.size() > 0`
      \pre `maximum_distance >= 0`
      \pre `maximum_angle >= 0 && maximum_angle <= 90`
      \pre `cosine_value >= 0 && cosine_value <= 1`
      \pre `minimum_region_size > 0`
      \pre `minimum_radius >= 0`
      \pre `maximum_radius >= minimum_radius`
    */
    template<typename CGAL_NP_TEMPLATE_PARAMETERS>
    Least_squares_cylinder_fit_region(
      const InputRange& input_range,
      const CGAL_NP_CLASS& np = parameters::default_values()) :
      m_input_range(input_range),
      m_point_map(Point_set_processing_3_np_helper<InputRange, CGAL_NP_CLASS,PointMap,NormalMap>::get_const_point_map(input_range, np)),
      m_normal_map(Point_set_processing_3_np_helper<InputRange, CGAL_NP_CLASS,PointMap,NormalMap>::get_normal_map(input_range, np)),
      m_traits(parameters::choose_parameter(parameters::get_parameter(
        np, internal_np::geom_traits), GeomTraits())),
      m_sqrt(Get_sqrt::sqrt_object(m_traits)),
      m_squared_distance_3(m_traits.compute_squared_distance_3_object()) {

      CGAL_precondition(input_range.size() > 0);
      const FT max_distance = parameters::choose_parameter(
        parameters::get_parameter(np, internal_np::maximum_distance), FT(1));
      CGAL_precondition(max_distance >= FT(0));
      m_distance_threshold = max_distance;

      const FT max_angle = parameters::choose_parameter(
        parameters::get_parameter(np, internal_np::maximum_angle), FT(25));
      CGAL_precondition(max_angle >= FT(0) && max_angle <= FT(90));

      m_min_region_size = parameters::choose_parameter(
        parameters::get_parameter(np, internal_np::minimum_region_size), 3);
      CGAL_precondition(m_min_region_size > 0);

      const FT default_cos_value = static_cast<FT>(std::cos(CGAL::to_double(
        (max_angle * static_cast<FT>(CGAL_PI)) / FT(180))));
      const FT cos_value = parameters::choose_parameter(
        parameters::get_parameter(np, internal_np::cosine_value), default_cos_value);
      CGAL_precondition(cos_value >= FT(0) && cos_value <= FT(1));
      m_cos_value_threshold = cos_value;

      m_min_radius = parameters::choose_parameter(
        parameters::get_parameter(np, internal_np::minimum_radius), FT(0));
      CGAL_precondition(m_min_radius >= FT(0));

      m_max_radius = parameters::choose_parameter(
        parameters::get_parameter(np, internal_np::maximum_radius),
        FT((std::numeric_limits<double>::max)()));
      CGAL_precondition(m_max_radius >= m_min_radius);
    }

    /// @}

    /// \name Access
    /// @{

    /*!
      \brief implements `RegionType::region_index_map()`.

      This function creates an empty property map that maps iterators on the input range `Item` to std::size_t
    */
    Region_index_map region_index_map() {
      return Region_index_map(m_region_map);
    }

    /*!
      \brief implements `RegionType::primitive()`.

      This function provides the last primitive that has been fitted with the region.

      \return Primitive parameters that fits the region

      \pre `successful fitted primitive via successful call of update(region) with a sufficient large region`
    */
    Primitive primitive() const {
      return Primitive(m_axis, m_radius);
    }

    /*!
      \brief implements `RegionType::is_part_of_region()`.

      This function controls if a point with the index `query_index` is within
      the `maximum_distance` from the corresponding cylinder and if the angle between
      its normal and the cylinder radius is within the `maximum_angle`. If both conditions
      are satisfied, it returns `true`, otherwise `false`.

      \param query_index
      index of the query point

      \param indices
      indices of the inliers of the region

      The first parameter is not used in this implementation.

      \return Boolean `true` or `false`

      \pre `query_index < input_range.size()`
    */
    bool is_part_of_region(
      const Item,
      const Item query,
      const Region& region) const {

      // First, we need to integrate at least 6 points so that the
      // computed cylinder means something.
      if (region.size() < 6) {
        return true;
      }

      // TODO: Why do we get so many nan in this class?
      if (std::isnan(CGAL::to_double(m_radius))) {
        return false;
      }

      // If radius is out of bound, nothing fits, early ending.
      if (m_radius < m_min_radius || m_radius > m_max_radius) {
        return false;
      }

      const Point_3& query_point = get(m_point_map, *query);
      Vector_3 normal = get(m_normal_map, *query);

      // TODO: Why do we have m_axis = 0 here sometimes?
      // Should it ever happen?
      if (m_axis.to_vector() == Vector_3(0, 0, 0)) return false;
      const FT sq_dist = m_squared_distance_3(query_point, m_axis);
      if (std::isnan(CGAL::to_double(sq_dist))) return false;
      const FT distance_to_center = m_sqrt(sq_dist);
      const FT distance_to_cylinder = CGAL::abs(distance_to_center - m_radius);

      if (distance_to_cylinder > m_distance_threshold) {
        return false;
      }

      const FT sq_norm = normal * normal;
      if (std::isnan(CGAL::to_double(sq_norm))) return false;
      normal = normal / m_sqrt(sq_norm);

      const Point_3 proj = m_axis.projection(query_point);
      Vector_3 ray(proj, query_point);
      const FT sq_ray = ray * ray;
      if (std::isnan(CGAL::to_double(sq_ray))) return false;
      ray = ray / m_sqrt(sq_ray);

      if (CGAL::abs(normal * ray) < m_cos_value_threshold) {
        return false;
      }
      return true;
    }

    /*!
      \brief implements `RegionType::is_valid_region()`.

      This function controls if the estimated radius is between `minimum_radius`
      and `maximum_radius` and if the `region` contains at least `min_region_size` points.

      \param region
      indices of points included in the region

      \return Boolean `true` or `false`
    */
    inline bool is_valid_region(const Region& region) const {
      return (
        (m_min_radius <= m_radius && m_radius <= m_max_radius) &&
        (region.size() >= m_min_region_size)
      );
    }

    /*!
      \brief implements `RegionType::update()`.

      This function fits the least squares cylinder to all points from the `region`.

      \param region
      indices of points included in the region

      \return Boolean `true` if the cylinder fitting succeeded and `false` otherwise

      \pre `region.size() > 0`
    */
    bool update(const Region& region) {
      if (region.size() < 6)
        return true;

      // Fit a cylinder.
      CGAL_precondition(region.size() >= 6);
      FT radius; Line_3 axis;
      std::tie(radius, axis) = internal::create_cylinder(
        region, m_point_map, m_normal_map,
        m_traits, false).first;

      if (radius >= FT(0)) {
        m_radius = radius;
        m_axis = axis;
      }
      else return false;

      return true;
    }

    /// @}

    /// \cond SKIP_IN_MANUAL
    std::pair<FT, Line_3> get_cylinder(
      const Region& region) const {
      return internal::create_cylinder(
        region, m_point_map, m_normal_map,
        m_traits, false).first;
    }
    /// \endcond

  private:
    const Input_range& m_input_range;
    const Point_map m_point_map;
    const Normal_map m_normal_map;
    const Traits m_traits;
    Region_unordered_map m_region_map;

    FT m_distance_threshold;
    FT m_cos_value_threshold;
    std::size_t m_min_region_size;
    FT m_min_radius;
    FT m_max_radius;

    const Sqrt m_sqrt;
    const Squared_distance_3 m_squared_distance_3;

    FT m_radius;
    Line_3 m_axis;
  };

} // namespace Point_set
} // namespace Shape_detection
} // namespace CGAL

#endif // CGAL_SHAPE_DETECTION_REGION_GROWING_POINT_SET_LEAST_SQUARES_CYLINDER_FIT_REGION_H

// Copyright (c) 2020 GeometryFactory SARL (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Dmitry Anisimov
//

#ifndef CGAL_SHAPE_DETECTION_REGION_GROWING_SEGMENT_SET_LEAST_SQUARES_LINE_FIT_SORTING_H
#define CGAL_SHAPE_DETECTION_REGION_GROWING_SEGMENT_SET_LEAST_SQUARES_LINE_FIT_SORTING_H

#include <CGAL/license/Shape_detection.h>

// Internal includes.
#include <CGAL/Shape_detection/Region_growing/internal/property_map.h>

namespace CGAL {
namespace Shape_detection {
namespace Segment_set {

  template<
  typename GeomTraits,
  typename InputRange,
  typename NeighborQuery,
  typename SegmentMap>
  class Least_squares_line_fit_sorting {

  public:
    /// \name Types
    /// @{

    /// \cond SKIP_IN_MANUAL
    using Traits = GeomTraits;
    using Input_range = InputRange;
    using Neighbor_query = NeighborQuery;
    using Segment_map = SegmentMap;
    using Segment_type = typename Segment_map::value_type;
    using Seed_map = internal::Seed_property_map;
    /// \endcond

    #ifdef DOXYGEN_RUNNING
      /*!
        a model of `ReadablePropertyMap` whose key and value type is `std::size_t`.
        This map provides an access to the ordered indices of input segments.
      */
      typedef unspecified_type Seed_map;
    #endif

    /// @}

  private:
    using FT = typename Traits::FT;
    using Segment_set_traits = typename std::conditional<
      std::is_same<typename Traits::Segment_2, Segment_type>::value,
      internal::Region_growing_traits_2<Traits>,
      internal::Region_growing_traits_3<Traits> >::type;
    using Compare_scores = internal::Compare_scores<FT>;

  public:
    /// \name Initialization
    /// @{

    Least_squares_line_fit_sorting(
      const InputRange& input_range,
      NeighborQuery& neighbor_query,
      const SegmentMap segment_map = SegmentMap(),
      const GeomTraits traits = GeomTraits()) :
    m_input_range(input_range),
    m_neighbor_query(neighbor_query),
    m_segment_map(segment_map),
    m_segment_set_traits(traits) {

      CGAL_precondition(input_range.size() > 0);
      m_order.resize(m_input_range.size());
      std::iota(m_order.begin(), m_order.end(), 0);
      m_scores.resize(m_input_range.size());
    }

    /// @}

    /// \name Sorting
    /// @{

    void sort() {

      compute_scores();
      CGAL_precondition(m_scores.size() > 0);
      Compare_scores cmp(m_scores);
      std::sort(m_order.begin(), m_order.end(), cmp);
    }

    /// @}

    /// \name Access
    /// @{

    Seed_map seed_map() {
      return Seed_map(m_order);
    }

    /// @}

  private:
    const Input_range& m_input_range;
    Neighbor_query& m_neighbor_query;
    const Segment_map m_segment_map;
    const Segment_set_traits m_segment_set_traits;
    std::vector<std::size_t> m_order;
    std::vector<FT> m_scores;

    void compute_scores() {

      std::vector<std::size_t> neighbors;
      for (std::size_t i = 0; i < m_input_range.size(); ++i) {
        neighbors.clear();
        m_neighbor_query(i, neighbors);
        neighbors.push_back(i);
        const auto& key = *(m_input_range.begin() + i);
        const auto& segment = get(m_segment_map, key);
        const auto& source = segment.source();
        const auto& target = segment.target();
        if (source == target) m_scores[i] = FT(0); // put it at the very back
        else m_scores[i] = m_segment_set_traits.create_line(
          m_input_range, m_segment_map, neighbors).second;
      }
    }
  };

} // namespace Segment_set
} // namespace Shape_detection
} // namespace CGAL

#endif // #define CGAL_SHAPE_DETECTION_REGION_GROWING_SEGMENT_SET_LEAST_SQUARES_LINE_FIT_SORTING_H

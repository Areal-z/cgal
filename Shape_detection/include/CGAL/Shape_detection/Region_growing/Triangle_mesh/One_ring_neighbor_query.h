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

#ifndef CGAL_SHAPE_DETECTION_REGION_GROWING_TRIANGLE_MESH_ONE_RING_NEIGHBOR_QUERY_H
#define CGAL_SHAPE_DETECTION_REGION_GROWING_TRIANGLE_MESH_ONE_RING_NEIGHBOR_QUERY_H

#include <CGAL/license/Shape_detection.h>

// Internal includes.
#include <CGAL/Shape_detection/Region_growing/internal/property_map.h>

namespace CGAL {
namespace Shape_detection {
namespace Triangle_mesh {

  /*!
    \ingroup PkgShapeDetectionRGOnMesh

    \brief Edge-adjacent faces connectivity in a triangle mesh.

    This class returns all faces, which are edge-adjacent to a query face in a
    triangle mesh being a `TriangleMesh`.

    \tparam TriangleMesh
    a model of `FaceListGraph`

    \tparam FaceRange
    a model of `ConstRange` whose iterator type is `RandomAccessIterator` and
    value type is the face type of a triangle mesh

    \cgalModels `NeighborQuery`
  */
  template<typename TriangleMesh
#ifndef CGAL_NO_DEPRECATED_CODE
  , typename FaceRange = void
#endif
  >
  class One_ring_neighbor_query
  {
    using Face_to_index_map = typename boost::property_map<TriangleMesh, CGAL::dynamic_face_property_t<std::size_t> >::const_type;
  public:
    /// \cond SKIP_IN_MANUAL
    using face_descriptor = typename boost::graph_traits<TriangleMesh>::face_descriptor;
    using Face_graph = TriangleMesh;
    /// \endcond

    /// Item type.
    using Item = face_descriptor;
    using Region = std::vector<Item>;

    /// \name Initialization
    /// @{

    /*!
      \brief initializes all internal data structures.

      \param pmesh
      an instance of a `TriangleMesh` that represents a triangle mesh

      \pre `faces(pmesh).size() > 0`
    */
    One_ring_neighbor_query(
      const TriangleMesh& tmesh) :
    m_face_graph(tmesh),
    m_face_to_index_map(get(CGAL::dynamic_face_property_t<std::size_t>(), tmesh))
    {
      m_face_range.reserve(num_faces(tmesh)); // a bit larger if has garbage
      for (face_descriptor f : faces(tmesh))
      {
        put(m_face_to_index_map, f, m_face_range.size());
        m_face_range.push_back(f);
      }
      CGAL_precondition(m_face_range.size() > 0);
    }

    /// @}

    /// \name Access
    /// @{

    /*!
      \brief implements `NeighborQuery::operator()()`.

      This operator retrieves all faces,
      which are edge-adjacent to the face `query`.
      These indices are returned in `neighbors`.

      \param query
      `Item` of the query face

      \param neighbors
      `Items` of faces, which are neighbors of the query face

      \pre `query_index < faces(tmesh).size()`
    */
    void operator()(
      const Item query,
      std::vector<Item>& neighbors) const {

      neighbors.clear();
      const auto query_hedge = halfedge(query, m_face_graph);

      for (face_descriptor face : faces_around_face(query_hedge, m_face_graph))
      {
        if (face != boost::graph_traits<TriangleMesh>::null_face())
        {
          neighbors.push_back(face);
        }
      }
    }

    /// @}

    /// \cond SKIP_IN_MANUAL
    // A property map that can be used to access indices of the input faces.
    const Face_to_index_map& face_to_index_map() const {
      return m_face_to_index_map;
    }
    /// \endcond

  private:
    const Face_graph& m_face_graph;
    std::vector<face_descriptor> m_face_range;
    Face_to_index_map m_face_to_index_map;
  };

} // namespace Triangle_mesh
} // namespace Shape_detection
} // namespace CGAL

#endif // CGAL_SHAPE_DETECTION_REGION_GROWING_TRIANGLE_MESH_ONE_RING_NEIGHBOR_QUERY_H

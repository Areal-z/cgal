// Copyright (c) 2019 GeometryFactory Sarl (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
// You can redistribute it and/or modify it under the terms of the GNU
// General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0+
//
// Author(s)     : Simon Giraudot

#ifndef CGAL_KINETIC_SHAPE_RECONSTRUCTION_3_H
#define CGAL_KINETIC_SHAPE_RECONSTRUCTION_3_H

//#include <CGAL/license/Kinetic_shape_reconstruction.h>

#include <CGAL/box_intersection_d.h>

#include <CGAL/KSR_3/Data_structure.h>
#include <CGAL/KSR_3/Polygon_splitter.h>

#include <CGAL/KSR_3/Event.h>
#include <CGAL/KSR_3/Event_queue.h>

#include <CGAL/KSR/debug.h>

#include <unordered_set>

namespace CGAL
{

template <typename GeomTraits>
class Kinetic_shape_reconstruction_3
{
public:

  typedef GeomTraits Kernel;
  typedef typename Kernel::FT FT;
  typedef typename Kernel::Point_2 Point_2;
  typedef typename Kernel::Segment_2 Segment_2;
  typedef typename Kernel::Direction_2 Direction_2;
  typedef typename Kernel::Line_2 Line_2;
  typedef typename Kernel::Ray_2 Ray_2;
  typedef typename Kernel::Vector_2 Vector_2;
  typedef typename Kernel::Point_3 Point_3;
  typedef typename Kernel::Segment_3 Segment_3;
  typedef typename Kernel::Direction_3 Direction_3;
  typedef typename Kernel::Line_3 Line_3;
  typedef typename Kernel::Ray_3 Ray_3;
  typedef typename Kernel::Vector_3 Vector_3;

  typedef KSR_3::Data_structure<Kernel> Data;
  typedef typename Data::PVertex PVertex;
  typedef typename Data::PEdge PEdge;
  typedef typename Data::PFace PFace;
  typedef typename Data::IEdge IEdge;
  typedef typename Data::IVertex IVertex;

  typedef KSR_3::Event<Data> Event;
  typedef KSR_3::Event_queue<Data> Event_queue;

private:

  Data m_data;
  Event_queue m_queue;
  FT m_min_time;
  FT m_max_time;

public:

  Kinetic_shape_reconstruction_3()
  {

  }

  // TODO: enlarge=1.0 does not work!
  template <typename PolygonRange, typename PolygonMap>
  bool partition (const PolygonRange& polygons, PolygonMap polygon_map,
                  unsigned int k = 2, FT enlarge_bbox_ratio = 1.1)
  {
    CGAL::Bbox_3 bbox;
    for (const auto& poly : polygons)
    {
      const std::vector<Point_3>& polygon = get (polygon_map, poly);
      bbox += CGAL::bbox_3 (polygon.begin(), polygon.end());
    }

    m_data.init (polygons.size());

    std::cout << "Adding bbox as polygons" << std::endl;
    add_bbox_as_polygons (bbox, enlarge_bbox_ratio);

    std::cout << "Adding input as polygons" << std::endl;

    KSR::size_t input_idx = 0;
    for (const typename PolygonRange::const_iterator::value_type& poly : polygons)
      m_data.add_polygon (get (polygon_map, poly), input_idx ++);

    FT time_step = CGAL::approximate_sqrt(CGAL::squared_distance(Point_3 (bbox.xmin(), bbox.ymin(), bbox.zmin()),
                                                                 Point_3 (bbox.xmax(), bbox.ymax(), bbox.zmax())));

    time_step /= 50.0;
    std::cout.precision(20);
    std::cout << "time_step " << time_step << std::endl;

    std::cout << "Making input polygons intersection free" << std::endl;

    KSR_3::dump (m_data, "init");

    CGAL_assertion(check_integrity(true));
    make_polygons_intersection_free(k);
    CGAL_assertion(check_integrity(true));

    for (KSR::size_t i = 6; i < m_data.number_of_support_planes(); ++i) {
      const auto& sp = m_data.support_plane(i);
      std::cout << "plane index: " << i << std::endl;
      std::cout << "plane: " <<
      sp.plane().a() << ", " <<
      sp.plane().b() << ", " <<
      sp.plane().c() << ", " <<
      sp.plane().d() << std::endl;
    }

    std::cout << "Polygons are splitted" << std::endl;

    // KSR_3::dump_segmented_edges (m_data, "init");

    KSR_3::dump (m_data, "intersected");

    std::size_t iter = 0;
    m_min_time = 0;
    m_max_time = time_step;
    while (initialize_queue())
    {
      run(k);
      m_min_time = m_max_time;
      m_max_time += time_step;

      // dump (m_data, "iter_100-" + std::to_string(iter));
      // CGAL_assertion(check_integrity(true));
      ++iter;
      // if (iter > 100) {
      //   std::cout << "Handling iteration # " << std::to_string(iter) << std::endl;
      //   CGAL_assertion_msg(false, "WHY SO MANY ITERATIONS?");
      // }
    }
    std::cout << "Checking final mesh integrity!" << std::endl;
    CGAL_assertion(check_integrity(true));
    std::cout << "Finalizing KSR!" << std::endl;
    dump (m_data, "iter_1000-final-result");
    // m_data.create_polyhedrons();
    return true;
  }

  template<typename OutputIterator>
  OutputIterator output_partition_edges_to_segment_soup(
    OutputIterator edges) const {

    CGAL_assertion_msg(false, "TODO: IMPLEMENT OUTPUT PARTITION EDGES!");
    return edges;
  }

  template<typename VertexOutputIterator, typename FaceOutputIterator>
  void output_partition_faces_to_polygon_soup(
    VertexOutputIterator vertices, FaceOutputIterator faces,
    const bool with_bbox = false) const {

    CGAL_assertion_msg(false, "TODO: IMPLEMENT OUTPUT PARTITION FACES!");
  }

  template<typename PolyhedronOutputIterator>
  PolyhedronOutputIterator output_partition_polyhedrons(
    PolyhedronOutputIterator polyhedrons) const {

    CGAL_assertion_msg(false, "TODO: IMPLEMENT OUTPUT PARTITION POLYHEDRONS!");
    return polyhedrons;
  }

  template <typename PointRange, typename PointMap, typename VectorMap>
  void reconstruct (const PointRange& points, PointMap point_map, VectorMap normal_map)
  {
    // TODO
  }

  bool check_integrity(bool verbose = false) const
  {
    for (KSR::size_t i = 0; i < m_data.number_of_support_planes(); ++ i)
    {
      if (!m_data.mesh_is_valid(i))
      {
        if (verbose)
          std::cerr << "ERROR: Mesh " << i << " is invalid" << std::endl;
        return false;
      }

      for (const IEdge& iedge : m_data.iedges(i))
        if (m_data.intersected_planes(iedge).find (i)
            == m_data.intersected_planes(iedge).end())
        {
          if (verbose)
            std::cerr << "ERROR: Support_plane[" << i
                      << "] is intersected by " << m_data.str(iedge)
                      << " which claims it does not intersect it" << std::endl;
          return false;
        }
    }

    for (const IEdge iedge : m_data.iedges())
    {
      for (KSR::size_t support_plane_idx : m_data.intersected_planes (iedge))
      {
        if (m_data.iedges(support_plane_idx).find (iedge)
            == m_data.iedges(support_plane_idx).end())
        {
          if (verbose)
            std::cerr << "ERROR: " << m_data.str(iedge)
                      << " intersects Support_plane[" << support_plane_idx
                      << "] which claims it's not intersected by it" << std::endl;
          return false;
        }
      }
    }

    return true;
  }

private:

  void add_bbox_as_polygons (const CGAL::Bbox_3& bbox, FT ratio)
  {
    const FT xmed = (bbox.xmin() + bbox.xmax()) / FT(2);
    const FT ymed = (bbox.ymin() + bbox.ymax()) / FT(2);
    const FT zmed = (bbox.zmin() + bbox.zmax()) / FT(2);
    const FT dx = (bbox.xmax() - bbox.xmin()) / FT(2);
    const FT dy = (bbox.ymax() - bbox.ymin()) / FT(2);
    const FT dz = (bbox.zmax() - bbox.zmin()) / FT(2);

    FT xmin = xmed - ratio * dx;
    FT xmax = xmed + ratio * dx;
    FT ymin = ymed - ratio * dy;
    FT ymax = ymed + ratio * dy;
    FT zmin = zmed - ratio * dz;
    FT zmax = zmed + ratio * dz;

		std::cout.precision(20);
		std::cout << "x_min = " << xmin << ";" << std::endl;
		std::cout << "y_min = " << ymin << ";" << std::endl;
		std::cout << "z_min = " << zmin << ";" << std::endl;
		std::cout << "x_max = " << xmax << ";" << std::endl;
		std::cout << "y_max = " << ymax << ";" << std::endl;
		std::cout << "z_max = " << zmax << ";" << std::endl;

    std::array<Point_3, 8> bbox_points
      = { Point_3 (xmin, ymin, zmin),
          Point_3 (xmin, ymin, zmax),
          Point_3 (xmin, ymax, zmin),
          Point_3 (xmin, ymax, zmax),
          Point_3 (xmax, ymin, zmin),
          Point_3 (xmax, ymin, zmax),
          Point_3 (xmax, ymax, zmin),
          Point_3 (xmax, ymax, zmax) };

    std::array<Point_3, 4> facet_points;

    facet_points = { bbox_points[0], bbox_points[1], bbox_points[3], bbox_points[2] };
    m_data.add_bbox_polygon (facet_points);

    facet_points = { bbox_points[4], bbox_points[5], bbox_points[7], bbox_points[6] };
    m_data.add_bbox_polygon (facet_points);

    facet_points = { bbox_points[0], bbox_points[1], bbox_points[5], bbox_points[4] };
    m_data.add_bbox_polygon (facet_points);

    facet_points = { bbox_points[2], bbox_points[3], bbox_points[7], bbox_points[6] };
    m_data.add_bbox_polygon (facet_points);

    facet_points = { bbox_points[1], bbox_points[5], bbox_points[7], bbox_points[3] };
    m_data.add_bbox_polygon (facet_points);

    facet_points = { bbox_points[0], bbox_points[4], bbox_points[6], bbox_points[2] };
    m_data.add_bbox_polygon (facet_points);

    CGAL_assertion (m_data.ivertices().size() == 8);
    CGAL_assertion (m_data.iedges().size() == 12);
  }

  void make_polygons_intersection_free (unsigned int k)
  {
    // TODO: FIX IT AND MAKE IT WORK FOR ANY NUMBER OF SUPPORT PLANES!
    std::cout << "num support planes: " << m_data.number_of_support_planes() << std::endl;
    if (m_data.number_of_support_planes() < 8) {
      return;
    }

    // First, generate all transverse intersection lines
    typedef std::map<KSR::Idx_set, std::pair<IVertex, IVertex> > Map;
    Map map_p2vv;

    for (const IVertex ivertex : m_data.ivertices())
    {
      KSR::Idx_set key = m_data.intersected_planes (ivertex, false);
      if (key.size() < 2)
        continue;

      typename Map::iterator iter;
      bool inserted;
      std::tie (iter, inserted) = map_p2vv.insert (std::make_pair (key,
                                                                   std::make_pair (ivertex,
                                                                                   IVertex())));
      if (!inserted)
        iter->second.second = ivertex;
    }


    // Then, intersect these lines to find internal intersection vertices
    KSR::vector<std::pair<KSR::Idx_set, KSR::vector<IVertex> > > todo;
    for (typename Map::iterator it_a = map_p2vv.begin(); it_a != map_p2vv.end(); ++ it_a)
    {
      const KSR::Idx_set& set_a = it_a->first;

      todo.push_back (std::make_pair (set_a, KSR::vector<IVertex>()));

      KSR::vector<IVertex>& crossed_vertices = todo.back().second;
      crossed_vertices.push_back (it_a->second.first);

      std::set<KSR::Idx_set> done;

      for (typename Map::iterator it_b = map_p2vv.begin() ; it_b != map_p2vv.end(); ++ it_b)
      {
        const KSR::Idx_set& set_b = it_b->first;
        KSR::size_t common_plane_idx = KSR::no_element();
        std::set_intersection (set_a.begin(), set_a.end(), set_b.begin(), set_b.end(),
                               boost::make_function_output_iterator
                               ([&](const KSR::size_t& idx) -> void { common_plane_idx = idx; }));

        if (common_plane_idx != KSR::no_element())
        {
          KSR::Idx_set union_set = set_a;
          union_set.insert (set_b.begin(), set_b.end());
          if (!done.insert (union_set).second)
            continue;

          Point_2 inter;
          if (!KSR::intersection_2 (m_data.to_2d(common_plane_idx,
                                                 Segment_3 (m_data.point_3 (it_a->second.first),
                                                            m_data.point_3 (it_a->second.second))),
                                    m_data.to_2d(common_plane_idx,
                                                 (Segment_3 (m_data.point_3 (it_b->second.first),
                                                             m_data.point_3 (it_b->second.second)))),
                                    inter))
            continue;

          crossed_vertices.push_back (m_data.add_ivertex
                                      (m_data.to_3d (common_plane_idx, inter), union_set));
        }
      }
      crossed_vertices.push_back (it_a->second.second);
    }

    for (auto& t : todo)
      m_data.add_iedge (t.first, t.second);

    // Refine polygons
    for (KSR::size_t i = 0; i < m_data.number_of_support_planes(); ++ i)
    {
      KSR_3::Polygon_splitter<GeomTraits> splitter (m_data);
      splitter.split_support_plane (i, k);
    }
  }

  bool initialize_queue()
  {
    std::cout << "Initializing queue for events in [" << m_min_time << ";" << m_max_time << "]" << std::endl;

    m_data.update_positions(m_max_time);

    bool still_running = false;

    for (KSR::size_t i = 0; i < m_data.number_of_support_planes(); ++ i)
    {
      KSR::vector<IEdge> iedges;
      KSR::vector<Segment_2> segments_2;
      KSR::vector<CGAL::Bbox_2> segment_bboxes;
      init_search_structures (i, iedges, segments_2, segment_bboxes);

      for (const PVertex pvertex : m_data.pvertices(i))
        if (compute_events_of_vertex (pvertex, iedges, segments_2, segment_bboxes))
          still_running = true;
    }

    m_data.update_positions(m_min_time);

    return still_running;
  }

  void init_search_structures (KSR::size_t i,
                               KSR::vector<IEdge>& iedges,
                               KSR::vector<Segment_2>& segments_2,
                               KSR::vector<CGAL::Bbox_2>& segment_bboxes)
  {
    // To get random access, copy in vector (suboptimal to do this
    // all the time, maybe this should be done once and for all and
    // replace the set)
    iedges.reserve (m_data.iedges(i).size());
    std::copy (m_data.iedges(i).begin(),
               m_data.iedges(i).end(),
               std::back_inserter(iedges));

    // Precompute segments and bboxes
    segments_2.reserve (iedges.size());
    segment_bboxes.reserve (iedges.size());
    for (const IEdge& iedge : iedges)
    {
      segments_2.push_back (m_data.segment_2 (i, iedge));
      segment_bboxes.push_back (segments_2.back().bbox());
    }
  }

  bool compute_events_of_vertex (const PVertex& pvertex,
                                 const KSR::vector<IEdge>& iedges,
                                 const KSR::vector<Segment_2>& segments_2,
                                 const KSR::vector<CGAL::Bbox_2>& segment_bboxes)
  {
    std::cout.precision(20);
    if (m_data.is_frozen(pvertex))
      return false;

    Segment_2 sv (m_data.point_2 (pvertex, m_min_time),
                  m_data.point_2 (pvertex, m_max_time));
    CGAL::Bbox_2 sv_bbox = sv.bbox();

    if (m_data.has_iedge(pvertex)) // Constrained vertex
    {
      // const auto cutime = m_data.current_time();
      // m_data.update_positions(m_min_time);
      // std::cout << "Computing events for the constrained pvertex: " << m_data.str(pvertex) << ": " << m_data.point_3(pvertex) << std::endl;
      // m_data.update_positions(cutime);

      // Test left and right vertices on mesh face
      PVertex prev;
      PVertex next;
      std::tie (prev, next) = m_data.prev_and_next (pvertex);

      for (const PVertex& pother : { prev, next })
      {
        if (pother == Data::null_pvertex()
            || !m_data.is_active(pother)
            || m_data.has_iedge (pother))
          continue;

        Segment_2 so (m_data.point_2 (pother, m_min_time),
                      m_data.point_2 (pother, m_max_time));
        CGAL::Bbox_2 so_bbox = so.bbox();

        if (!do_overlap (sv_bbox, so_bbox))
          continue;

        Point_2 point;
        if (!KSR::intersection_2 (sv, so, point))
          continue;

        FT dist = CGAL::approximate_sqrt (CGAL::squared_distance (sv.source(), point));
        FT time = dist / m_data.speed(pvertex);

        m_queue.push (Event (true, pvertex, pother, m_min_time + time));

        // std::cout << "pvertex: " << m_data.point_3(pvertex) << std::endl;
        // std::cout << "pother: " << m_data.point_3(pother) << std::endl;
      }

      // Test end-vertices of intersection edge
      IEdge iedge = m_data.iedge(pvertex);
      for (const IVertex& ivertex : { m_data.source(iedge), m_data.target(iedge) })
      {
        if (!m_data.is_active(ivertex))
          continue;
        Point_2 pi = m_data.to_2d (pvertex.first, ivertex);
        if (sv.to_vector() * Vector_2 (sv.source(), pi) < 0)
          continue;

        FT dist = CGAL::approximate_sqrt(CGAL::squared_distance (sv.source(), pi));
        FT time = dist / m_data.speed(pvertex);

        if (time < m_max_time - m_min_time) {
          m_queue.push (Event (true, pvertex, ivertex, m_min_time + time));

          // std::cout << "pvertex: " << m_data.point_3(pvertex) << std::endl;
          // std::cout << "ivertex: " << m_data.point_3(ivertex) << std::endl;
        }
      }
    }
    else // Unconstrained vertex
    {
      PVertex prev = m_data.prev(pvertex);
      PVertex next = m_data.next(pvertex);

      // Test all intersection edges
      for (std::size_t j = 0; j < iedges.size(); ++ j)
      {
        const IEdge& iedge = iedges[j];

        if (m_data.iedge(prev) == iedge ||
            m_data.iedge(next) == iedge)
          continue;
        if (!m_data.is_active(iedge))
          continue;

        if (!CGAL::do_overlap (sv_bbox, segment_bboxes[j]))
          continue;

        Point_2 point;
        if (!KSR::intersection_2 (sv, segments_2[j], point))
          continue;

        FT dist = CGAL::approximate_sqrt (CGAL::squared_distance (m_data.point_2 (pvertex, m_min_time), point));
        FT time = dist / m_data.speed(pvertex);

        m_queue.push (Event (false, pvertex, iedge, m_min_time + time));

        // std::cout << "pvertex: " << m_data.point_3(pvertex) << std::endl;
        // std::cout << "iedge: " << m_data.segment_3(iedge) << std::endl;
      }
    }
    return true;
  }

  const bool are_parallel(
    const Segment_2& seg1, const Segment_2& seg2) {

    const FT tol = FT(1) / FT(100000);
    FT m1 = FT(100000), m2 = FT(100000);

    const FT d1 = (seg1.target().x() - seg1.source().x());
    const FT d2 = (seg2.target().x() - seg2.source().x());

    if (CGAL::abs(d1) > tol)
      m1 = (seg1.target().y() - seg1.source().y()) / d1;
    if (CGAL::abs(d2) > tol)
      m2 = (seg2.target().y() - seg2.source().y()) / d2;

    // return CGAL::parallel(seg1, seg2); // exact version

    if (CGAL::abs(m1 - m2) < tol)
      return true;
    return false;
  }

  void run(const unsigned int k)
  {
    std::cout << "Unstacking queue size: " << m_queue.size() << std::endl;

    KSR::size_t iterations = 0;

    static int iter = 0;

    while (!m_queue.empty())
    {
      Event ev = m_queue.pop();

      FT current_time = ev.time();

      if (iter < 10)
      {
        dump (m_data, "iter_0" + std::to_string(iter));
        dump_event (m_data, ev, "iter_0" + std::to_string(iter));
      }
      else
      {
        dump (m_data, "iter_" + std::to_string(iter));
        dump_event (m_data, ev, "iter_" + std::to_string(iter));
      }

      m_data.update_positions (current_time);

      std::cout << "* APPLYING " << iter << ": " << ev << std::endl << std::endl;

      ++ iter;

      // if (iter == 10) {
      //   exit(0);
      // }

      apply(k, ev);

      CGAL_assertion(check_integrity(true));

      // m_data.update_positions (0.5 * (current_time + m_queue.next().time()));
      // dump (m_data, "after_" + std::to_string(iter - 1));
      // m_data.update_positions (current_time);
      ++ iterations;
    }
    std::cout << "Finished!" << std::endl;
  }

  void apply (
    const unsigned int k,
    const Event& ev)
  {
    PVertex pvertex = ev.pvertex();

    if (ev.is_pvertex_to_pvertex())
    {
      PVertex pother = ev.pother();

      remove_events (pvertex);
      remove_events (pother);

      CGAL_assertion (m_data.has_iedge(pvertex));

      if (m_data.has_iedge(pother)) // Two constrained vertices meet
      {
        CGAL_assertion_msg (false, "TODO: two constrained");
      }
      else // One constrained vertex meets a free vertex
      {
        if (m_data.transfer_vertex (pvertex, pother))
        {
          compute_events_of_vertices (std::array<PVertex,2>{pvertex, pother});

          PVertex prev, next;
          std::tie (prev, next) = m_data.border_prev_and_next(pvertex);

          PVertex pthird = prev;
          if (pthird == pother)
            pthird = next;
          else
            CGAL_assertion (next == pother);

          remove_events (pthird);
          // TODO: Should we remove here any events related to the crossed iedges?
          compute_events_of_vertices (std::array<PVertex,1>{pthird});
        }
        else
          compute_events_of_vertices (std::array<PVertex,1>{pvertex});
      }
    }
    else if (ev.is_pvertex_to_iedge())
    {
      PVertex prev = m_data.prev(pvertex);
      PVertex next = m_data.next(pvertex);
      IEdge iedge = ev.iedge();
      PFace pface = m_data.pface_of_pvertex (pvertex);

      Segment_2 seg_edge = m_data.segment_2 (pvertex.first, iedge);

      bool done = false;
      for (const PVertex& pother : { prev, next })
      {
        Segment_2 seg (m_data.point_2(pother, ev.time()),
                       m_data.point_2(pvertex, ev.time()));
        CGAL_assertion (seg.squared_length() != 0);

        bool both_are_free = true;
        if (
          m_data.iedge(pvertex) != m_data.null_iedge() ||
          m_data.iedge(pother)  != m_data.null_iedge()) {
          both_are_free = false;
        }

        if (both_are_free && are_parallel(seg, seg_edge)) {

          remove_events(pvertex);
          remove_events(pother);

          bool collision, bbox_reached;
          // std::tie(collision, bbox_reached) = m_data.collision_occured(pvertex, iedge);
          std::tie(collision, bbox_reached) = m_data.is_occupied(pvertex, iedge);
          std::cout << "collision/bbox: " << collision << "/" << bbox_reached << std::endl;

          bool collision_other, bbox_reached_other;
          // collision_other = m_data.collision_occured(pother, iedge).first;
          std::tie(collision_other, bbox_reached_other) = m_data.is_occupied(pvertex, iedge);
          std::cout << "other/bbox: " << collision_other << "/" << bbox_reached_other << std::endl;

          if ((collision || collision_other) && m_data.k(pface) > 1)
            m_data.k(pface) --;

          bool stop = false;
          if (bbox_reached) {
            m_data.k(pface) = 1; stop = true;
          }
          if ((collision || collision_other) && m_data.k(pface) == 1) {
            stop = true;
          }
          if ((collision || collision_other) && m_data.k(pface) > 1)
            m_data.k(pface)--;
          CGAL_assertion(m_data.k(pface) >= 1);

          if (stop) // polygon stops
          {
            m_data.crop_polygon(pvertex, pother, iedge);
            remove_events(iedge, pvertex.first);
            compute_events_of_vertices(std::array<PVertex,2>{pvertex, pother});
          }
          else // polygon continues beyond the edge
          {
            PVertex pv0, pv1;
            std::tie(pv0, pv1) = m_data.propagate_polygon(k, pvertex, pother, iedge);
            remove_events(iedge, pvertex.first);
            compute_events_of_vertices(std::array<PVertex, 4>{pvertex, pother, pv0, pv1});
          }

          done = true;
          break;
        }
      }

      if (!done) {

        remove_events(pvertex);

        bool collision, bbox_reached;
        // std::tie(collision, bbox_reached) = m_data.collision_occured(pvertex, iedge);
        std::tie(collision, bbox_reached) = m_data.is_occupied(pvertex, iedge);
        std::cout << "collision/bbox: " << collision << "/" << bbox_reached << std::endl;

        bool stop = false;
        if (bbox_reached) {
          m_data.k(pface) = 1; stop = true;
        }
        if (collision && m_data.k(pface) == 1) {
          stop = true;
        }
        if (collision && m_data.k(pface) > 1)
          m_data.k(pface)--;
        CGAL_assertion(m_data.k(pface) >= 1);

        if (stop) // polygon stops
        {
          const PVertex pvnew = m_data.crop_polygon(pvertex, iedge);
          remove_events(iedge, pvertex.first);
          compute_events_of_vertices(std::array<PVertex,2>{pvertex, pvnew});
        }
        else // polygon continues beyond the edge
        {
          const std::array<PVertex, 3> pvnew = m_data.propagate_polygon(k, pvertex, iedge);
          remove_events(iedge, pvertex.first);
          compute_events_of_vertices(pvnew);
        }
      }
    }
    else if (ev.is_pvertex_to_ivertex())
    {
      // first, let's gather all vertices that will get merged
      std::vector<PVertex> pvertices
        = m_data.pvertices_around_ivertex (ev.pvertex(), ev.ivertex());

      for (auto& pv: pvertices)
        std::cerr << m_data.point_3(pv) << std::endl;
      std::cerr << std::endl;

      std::cerr << "Found " << pvertices.size() << " pvertices ready to be merged" << std::endl;

      // Remove associated events.
      // for (const PVertex& pvertex : pvertices)
      //   remove_events (pvertex);
      for (std::size_t i = 1; i < pvertices.size() - 1; ++i)
        remove_events (pvertices[i]);

      // Merge them and get the newly created vertices.
      std::vector<IEdge> crossed;
      std::vector<PVertex> new_pvertices
        = m_data.merge_pvertices_on_ivertex(m_min_time, m_max_time, k, pvertices, ev.ivertex(), crossed);

      // Remove all events of the crossed iedges.
      for (const auto& iedge : crossed)
        remove_events(iedge, pvertex.first);

      // And compute new events.
      CGAL_assertion(new_pvertices.size() > 0);
      compute_events_of_vertices (new_pvertices);
    }
    else
    {
      CGAL_assertion_msg (false, "Event is invalid");
    }
  }

  void remove_events (
    const IEdge& iedge,
    const KSR::size_t support_plane_idx) {

    m_queue.erase_vertex_events(iedge, support_plane_idx);
    // std::cout << "erasing events for iedge " << m_data.str(iedge) << std::endl;
    // std::cout << m_data.segment_3(iedge) << std::endl;
  }

  void remove_events (const PVertex& pvertex) {
    m_queue.erase_vertex_events (pvertex);
    // std::cout << "erasing events for pvertex " << m_data.str(pvertex) << " : " << m_data.point_3(pvertex) << std::endl;
  }

  template <typename PVertexRange>
  void compute_events_of_vertices (const PVertexRange& pvertices)
  {
    m_min_time = m_data.current_time();

    m_data.update_positions(m_max_time);

    KSR::vector<IEdge> iedges;
    KSR::vector<Segment_2> segments_2;
    KSR::vector<CGAL::Bbox_2> segment_bboxes;
    init_search_structures (pvertices.front().first, iedges, segments_2, segment_bboxes);

    for (const PVertex& pvertex : pvertices)
      m_data.deactivate(pvertex);

    for (const PVertex& pvertex : pvertices)
      compute_events_of_vertex (pvertex, iedges, segments_2, segment_bboxes);

    for (const PVertex& pvertex : pvertices)
      m_data.activate(pvertex);

    m_data.update_positions(m_min_time);
  }

};



} // namespace CGAL


#endif // CGAL_KINETIC_SHAPE_RECONSTRUCTION_3_H

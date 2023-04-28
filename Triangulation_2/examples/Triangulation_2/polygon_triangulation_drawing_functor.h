// Copyright(c) 2022  GeometryFactory (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Andreas Fabri

#ifndef CGAL_POLYGON_TRIANGULATION_DRAWING_FUNCTOR_H
#define CGAL_POLYGON_TRIANGULATION_DRAWING_FUNCTOR_H

#include <CGAL/license/Triangulation_2.h>
#include <CGAL/Drawing_functor.h>
#include <CGAL/Random.h>
#include <CGAL/draw_triangulation_2.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>

namespace CGAL
{

template<class PT>
struct Polygon_triangulation_drawing_functor :
  public CGAL::Drawing_functor<PT,
                               typename PT::Vertex_handle,
                               typename PT::Finite_edges_iterator,
                               typename PT::Finite_faces_iterator>
{
  template<class IPM>
  Polygon_triangulation_drawing_functor(IPM ipm)
  {
    this->colored_face =
      [](const PT&, const typename PT::Finite_faces_iterator) -> bool
      { return true; };

    this->face_color =
      [](const PT&, const typename PT::Finite_faces_iterator fh) -> CGAL::IO::Color
      {
        CGAL::Random random((unsigned int)(std::size_t)(&*fh));
        return get_random_color(random);
      };

    this->draw_face=
      [ipm](const PT&, const typename PT::Finite_faces_iterator fh) -> bool
      { return get(ipm, fh); };

    this->draw_edge=
      [ipm](const PT& pt, const typename PT::Finite_edges_iterator eh) -> bool
      {
        typename PT::Face_handle fh1=eh->first;
        typename PT::Face_handle fh2=pt.mirror_edge(*eh).first;
        return get(ipm, fh1) || get(ipm, fh2);
      };
  }
};

#define CGAL_CT2_TYPE CGAL::Constrained_Delaunay_triangulation_2<K, TDS, Itag>

template <class K, class TDS, typename Itag,
          typename BufferType=float, class DrawingFunctor>
void add_in_graphic_buffer(const CGAL_CT2_TYPE& ct2,
                           CGAL::Graphic_storage<BufferType>& graphic_buffer,
                           const DrawingFunctor& drawing_functor)
{
  draw_function_for_t2::compute_elements(ct2, graphic_buffer, drawing_functor);
}

#ifdef CGAL_USE_BASIC_VIEWER

// Specialization of draw function.
template <class K, class TDS, typename Itag, class DrawingFunctor>
void draw(const CGAL_CT2_TYPE &ct2, const DrawingFunctor &drawingfunctor,
          const char *title="Constrained Triangulation_2 Basic Viewer")
{
  CGAL::Graphic_storage<float> buffer;
  add_in_graphic_buffer(ct2, buffer, drawingfunctor);
  draw_graphic_storage(buffer, title);
}

#endif // CGAL_USE_BASIC_VIEWER

#undef CGAL_CT2_TYPE

}; // end namespace CGAL

#endif // CGAL_POLYGON_TRIANGULATION_DRAWING_FUNCTOR_H

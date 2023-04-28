// Copyright (c) 1997
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL$
// $Id$
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Guillaume Damiand <guillaume.damiand@liris.cnrs.fr>
//                 Mostafa Ashraf <mostaphaashraf1996@gmail.com>

#ifndef CGAL_DRAW_POLYGON_WITH_HOLES_2_H
#define CGAL_DRAW_POLYGON_WITH_HOLES_2_H

#include <CGAL/Qt/Basic_viewer_qt.h>
#include <CGAL/Graphic_storage.h>
#include <CGAL/Drawing_functor.h>
#include <CGAL/Polygon_with_holes_2.h>

#ifdef DOXYGEN_RUNNING
namespace CGAL {

/*!
\ingroup PkgDrawPolygonWithHoles2

opens a new window and draws `aph`, an instance of the `CGAL::Polygon_with_holes_2` class. A call to this function is blocking, that is the program continues as soon as the user closes the window. This function requires `CGAL_Qt5`, and is only available if the macro `CGAL_USE_BASIC_VIEWER` is defined.
Linking with the cmake target `CGAL::CGAL_Basic_viewer` will link with `CGAL_Qt5` and add the definition `CGAL_USE_BASIC_VIEWER`.
\tparam PH an instance of the `CGAL::Polygon_with_holes_2` class.
\param aph the polygon with holes to draw.

*/
template<class PH>
void draw(const PH& aph);

} /* namespace CGAL */
#endif

namespace CGAL
{

namespace draw_function_for_ph2_with_holes {

template <typename BufferType=float, class P2, class DrawingFunctor>
void compute_one_loop_elements(const P2& ap2,
                               const typename P2::General_polygon_2& aloop,
                               Graphic_storage<BufferType> &graphic_buffer,
                               bool hole,
                               const DrawingFunctor& drawing_functor)
{
  if (hole && drawing_functor.are_faces_enabled())
  { graphic_buffer.add_point_in_face(aloop.vertex(aloop.size()-1)); }

  typename P2::General_polygon_2::Vertex_const_iterator prev;
  for(typename P2::General_polygon_2::Vertex_const_iterator i=aloop.vertices_begin();
      i!=aloop.vertices_end(); ++i)
  {
    if(drawing_functor.are_vertices_enabled() &&
       drawing_functor.draw_vertex(ap2, i))
     { // Add vertex
        if(drawing_functor.colored_vertex(ap2, i))
      { graphic_buffer.add_point(*i, drawing_functor.vertex_color(ap2, i)); }
      else
      { graphic_buffer.add_point(*i); }
     }

    if(i!=aloop.vertices_begin() &&
       drawing_functor.are_edges_enabled() &&
       drawing_functor.draw_edge(ap2, i))
    { // Add segment with previous point
          if(drawing_functor.colored_vertex(ap2, i))
      { graphic_buffer.add_segment(*prev, *i, drawing_functor.edge_color(ap2, i)); }
      else
      { graphic_buffer.add_segment(*prev, *i); }
    }

    if(drawing_functor.are_faces_enabled())
    { graphic_buffer.add_point_in_face(*i); } // Add point in face

    prev=i;
  }

  // Add the last segment between the last point and the first one
  if(drawing_functor.are_edges_enabled() &&
     drawing_functor.draw_edge(ap2, aloop.vertices_begin()))
  { graphic_buffer.add_segment(*prev, *(aloop.vertices_begin())); }
}

template <typename BufferType=float, class P2, class DrawingFunctor>
void compute_elements(const P2& p2, Graphic_storage<BufferType> &graphic_buffer,
                      const DrawingFunctor& drawing_functor)
{
  if (p2.outer_boundary().is_empty()) return;

  if (drawing_functor.are_faces_enabled())
  {
    if(drawing_functor.colored_face(p2, nullptr))
    { graphic_buffer.face_begin(drawing_functor.face_color(p2, nullptr)); }
    else
    { graphic_buffer.face_begin(); }
  }

  compute_one_loop_elements<BufferType, P2>(p2, p2.outer_boundary(), graphic_buffer,
                                            false, drawing_functor);

  for (typename P2::Hole_const_iterator it=p2.holes_begin(); it!=p2.holes_end(); ++it)
  {
    compute_one_loop_elements<BufferType, P2>(p2, *it, graphic_buffer,
                                              true, drawing_functor);
    if (drawing_functor.are_faces_enabled())
    { graphic_buffer.add_point_in_face(p2.outer_boundary().vertex
                                       (p2.outer_boundary().size()-1));
    }
  }

 if (drawing_functor.are_faces_enabled())
 { graphic_buffer.face_end(); }
}

} // draw_function_for_ph2

#define CGAL_P2_WITH_HOLES_TYPE CGAL::Polygon_with_holes_2<T, C>

template <class T, class C, typename BufferType=float, class DrawingFunctor>
void add_in_graphic_buffer(const CGAL_P2_WITH_HOLES_TYPE& p2,
                           CGAL::Graphic_storage<BufferType>& graphic_buffer,
                           const DrawingFunctor &drawing_functor)
{
  draw_function_for_ph2_with_holes::compute_elements(p2, graphic_buffer,
                                                     drawing_functor);
}

template <class T, class C, typename BufferType=float>
void add_in_graphic_buffer(const CGAL_P2_WITH_HOLES_TYPE& p2,
                           CGAL::Graphic_storage<BufferType>& graphic_buffer)
{
  Drawing_functor<CGAL_P2_WITH_HOLES_TYPE,
                  typename CGAL_P2_WITH_HOLES_TYPE::General_polygon_2::Vertex_const_iterator,
                  typename CGAL_P2_WITH_HOLES_TYPE::General_polygon_2::Vertex_const_iterator,
                  void*> drawing_functor;

  add_in_graphic_buffer(p2, graphic_buffer, drawing_functor);
}

#ifdef CGAL_USE_BASIC_VIEWER

// Specialization of draw function.
template<class T, class C, typename BufferType=float, class DrawingFunctor>
void draw(const CGAL_P2_WITH_HOLES_TYPE& ap2, const DrawingFunctor &drawing_functor,
          const char* title="Polygon with Holes Basic Viewer")
{
  CGAL::Graphic_storage<BufferType> buffer;
  add_in_graphic_buffer(ap2, buffer, drawing_functor);
  draw_graphic_storage(buffer, title);
}

template<class T, class C, typename BufferType=float>
void draw(const CGAL_P2_WITH_HOLES_TYPE& ap2,
          const char* title="Polygon with Holes Basic Viewer")
{
  CGAL::Graphic_storage<BufferType> buffer;
  add_in_graphic_buffer(ap2, buffer);
  draw_graphic_storage(buffer, title);
}

#endif // CGAL_USE_BASIC_VIEWER

} // End namespace CGAL

#endif // CGAL_DRAW_POLYGON_WITH_HOLES_2_H

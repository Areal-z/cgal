// Copyright (c) 2018 CNRS and LIRIS' Establishments (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL$
// $Id$
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Guillaume Damiand <guillaume.damiand@liris.cnrs.fr>
//                 Mostafa Ashraf <mostaphaashraf1996@gmail.com>

#ifndef CGAL_DRAW_LCC_H
#define CGAL_DRAW_LCC_H

#include <CGAL/Qt/Basic_viewer_qt.h>
#include <CGAL/Graphic_buffer.h>
#include <CGAL/Drawing_functor.h>
#include <CGAL/Linear_cell_complex_operations.h>
#include <CGAL/Random.h>

namespace CGAL {

namespace draw_function_for_lcc
{

typedef CGAL::Exact_predicates_inexact_constructions_kernel Local_kernel;
typedef Local_kernel::Point_3 Local_point;
typedef Local_kernel::Vector_3 Local_vector;

template <class LCC, class Local_kernel, int dim = LCC::ambient_dimension>
struct LCC_geom_utils;

template <class LCC, class Local_kernel>
struct LCC_geom_utils<LCC, Local_kernel, 3>
{
  static typename Local_kernel::Vector_3
  get_vertex_normal(const LCC& lcc, typename LCC::Dart_const_descriptor dh)
  {
    typename Local_kernel::Vector_3 n =
        internal::Geom_utils<typename LCC::Traits, Local_kernel>::
            get_local_vector(CGAL::compute_normal_of_cell_0<LCC>(lcc, dh));
    n = n / (CGAL::sqrt(n * n));
    return n;
  }
};

template <class LCC, class Local_kernel>
struct LCC_geom_utils<LCC, Local_kernel, 2>
{
  static typename Local_kernel::Vector_3
  get_vertex_normal(const LCC&, typename LCC::Dart_const_descriptor)
  {
    typename Local_kernel::Vector_3 n=CGAL::NULL_VECTOR;
    return n;
  }
};

template <typename BufferType=float, class LCC, class DrawingFunctorLCC>
void compute_face(typename LCC::Dart_const_handle dh,
                  typename LCC::Dart_const_handle voldh, const LCC *lcc,
                  const DrawingFunctorLCC &m_drawing_functor,
                  CGAL::Graphic_buffer<BufferType> &graphic_buffer)
{
  if (!m_drawing_functor.are_faces_enabled() ||
      !m_drawing_functor.draw_face(*lcc, dh))
  { return; }

  // We fill only closed faces.
  typename LCC::Dart_const_handle cur=dh;
  do
  {
    if (!lcc->is_next_exist(cur))
    { return; } // open face=>not filled
    cur = lcc->next(cur);
  }
  while (cur!=dh);

  if (m_drawing_functor.colored_volume(*lcc, voldh))
  {
    graphic_buffer.face_begin(m_drawing_functor.volume_color(*lcc, voldh));
  }
  else if (m_drawing_functor.colored_face(*lcc, dh))
  {
    graphic_buffer.face_begin(m_drawing_functor.face_color(*lcc, dh));
  }
  else
  { graphic_buffer.face_begin(); }

  cur=dh;
  do
  {
    graphic_buffer.add_point_in_face
      (lcc->point(cur),
       LCC_geom_utils<LCC, Local_kernel>::get_vertex_normal(*lcc, cur));
    cur=lcc->next(cur);
  }
  while (cur!=dh);

  graphic_buffer.face_end();
}

template <typename BufferType=float, class LCC, class DrawingFunctor>
void compute_edge(typename LCC::Dart_const_handle dh, const LCC *lcc,
                  const DrawingFunctor &m_drawing_functor,
                  CGAL::Graphic_buffer<BufferType> &graphic_buffer)
{
  if (!m_drawing_functor.are_edges_enabled() ||
      !m_drawing_functor.draw_edge(*lcc, dh))
  { return; }

  const typename LCC::Point& p1=lcc->point(dh);
  typename LCC::Dart_const_handle d2=lcc->other_extremity(dh);
  if (d2!=LCC::null_descriptor)
  {
    if (m_drawing_functor.colored_edge(*lcc, dh))
    {
      graphic_buffer.add_segment(p1, lcc->point(d2),
                                 m_drawing_functor.edge_color(*lcc, dh));
    }
    else
    { graphic_buffer.add_segment(p1, lcc->point(d2)); }
  }
}

template <typename BufferType=float, class LCC, class DrawingFunctorLCC>
void compute_vertex(typename LCC::Dart_const_handle dh, const LCC *lcc,
                    const DrawingFunctorLCC &m_drawing_functor,
                    CGAL::Graphic_buffer<BufferType> &graphic_buffer)
{
  if (!m_drawing_functor.are_vertices_enabled() ||
      !m_drawing_functor.draw_vertex(*lcc, dh))
  { return; }

  if (m_drawing_functor.colored_vertex(*lcc, dh))
  {
    graphic_buffer.add_point(lcc->point(dh),
                             m_drawing_functor.vertex_color(*lcc, dh));
  }
  else
  { graphic_buffer.add_point(lcc->point(dh)); }
}

template <typename BufferType=float, class LCC, class DrawingFunctor>
void compute_elements(const LCC *lcc,
                      CGAL::Graphic_buffer<BufferType> &graphic_buffer,
                      const DrawingFunctor &m_drawing_functor)
{
  if (lcc==nullptr)
  { return; }

  typename LCC::size_type markvolumes = lcc->get_new_mark();
  typename LCC::size_type markfaces = lcc->get_new_mark();
  typename LCC::size_type markedges = lcc->get_new_mark();
  typename LCC::size_type markvertices = lcc->get_new_mark();
  typename LCC::size_type oriented_mark = lcc->get_new_mark();

  lcc->orient(oriented_mark);

  for(typename LCC::Dart_range::const_iterator it=lcc->darts().begin(),
        itend=lcc->darts().end(); it!=itend; ++it)
  {
    if (!lcc->is_marked(it, markvolumes) &&
        m_drawing_functor.draw_volume(*lcc, it))
    {
      for(typename LCC::template Dart_of_cell_basic_range<3>::const_iterator
            itv=lcc->template darts_of_cell_basic<3>(it, markvolumes).begin(),
            itvend=lcc->template darts_of_cell_basic<3>(it, markvolumes).end();
          itv!=itvend; ++itv)
      {
        lcc->mark(itv, markvolumes);
        if (!lcc->is_marked(itv, markfaces) &&
            lcc->is_marked(itv, oriented_mark) &&
            m_drawing_functor.draw_face(*lcc, itv))
        {
          if ((!m_drawing_functor.volume_wireframe(*lcc, itv) ||
               (!lcc->template is_free<3>(itv) &&
                !m_drawing_functor.volume_wireframe(*lcc, lcc->template beta<3>(itv)))) &&
              !m_drawing_functor.face_wireframe(*lcc, itv))
          { compute_face(itv, it, lcc, m_drawing_functor, graphic_buffer); }
          for(typename LCC::template Dart_of_cell_basic_range<2>::const_iterator
                itf=lcc->template darts_of_cell_basic<2>(itv, markfaces).begin(),
                itfend=lcc->template darts_of_cell_basic<2>(itv, markfaces).end();
              itf!=itfend; ++itf)
          {
            lcc->mark(itf, markfaces);
            if (!lcc->is_marked(itf, markedges) &&
                m_drawing_functor.draw_edge(*lcc, itf))
            {
              compute_edge(itf, lcc, m_drawing_functor, graphic_buffer);
              for(typename LCC::template Dart_of_cell_basic_range<1>::const_iterator
                    ite=lcc->template darts_of_cell_basic<1>(itf, markedges).begin(),
                    iteend=lcc->template darts_of_cell_basic<1>(itf, markedges).end();
                  ite!=iteend; ++ite)
              {
                lcc->mark(ite, markedges);
                if (!lcc->is_marked(ite, markvertices) &&
                    m_drawing_functor.draw_vertex(*lcc, ite))
                {
                  compute_vertex(ite, lcc, m_drawing_functor, graphic_buffer);
                  CGAL::mark_cell<LCC, 0>(*lcc, ite, markvertices);
                }
              }
            }
          }
        }
      }
    }
  }

  for (typename LCC::Dart_range::const_iterator it = lcc->darts().begin(),
         itend = lcc->darts().end(); it != itend; ++it)
  {
    lcc->unmark(it, markvertices);
    lcc->unmark(it, markedges);
    lcc->unmark(it, markfaces);
    lcc->unmark(it, markvolumes);
    lcc->unmark(it, oriented_mark);
  }

  lcc->free_mark(markvolumes);
  lcc->free_mark(markfaces);
  lcc->free_mark(markedges);
  lcc->free_mark(markvertices);
  lcc->free_mark(oriented_mark);
}

} // namespace draw_function_for_lcc

#define CGAL_LCC_TYPE                                                          \
  CGAL::Linear_cell_complex_base<d_, ambient_dim, Traits_, Items_, Alloc_,     \
                                 Map, Refs, Storage_>

// add_in_graphic_buffer: to add a LCC in the given graphic buffer, with a
// drawing functor.
template<unsigned int d_, unsigned int ambient_dim, class Traits_,
         class Items_, class Alloc_,
         template <unsigned int, class, class, class, class> class Map,
         class Refs, class Storage_,
         typename BufferType=float, class DrawingFunctor>
void add_in_graphic_buffer(const CGAL_LCC_TYPE &alcc,
                           CGAL::Graphic_buffer<BufferType> &graphic_buffer,
                           const DrawingFunctor &m_drawing_functor)
{
  draw_function_for_lcc::compute_elements(static_cast<const Refs*>(&alcc),
                                          graphic_buffer, m_drawing_functor);
}

// add_in_graphic_buffer: to add a LCC in the given graphic buffer, without a
// drawing functor. Use default drawing values.
template<unsigned int d_, unsigned int ambient_dim, class Traits_,
         class Items_, class Alloc_,
         template <unsigned int, class, class, class, class> class Map,
         class Refs, class Storage_, typename BufferType=float>
void add_in_graphic_buffer(const CGAL_LCC_TYPE &alcc,
                           CGAL::Graphic_buffer<BufferType> &graphic_buffer)
{
  CGAL::Drawing_functor_with_volume<CGAL_LCC_TYPE,
                                    typename CGAL_LCC_TYPE::Dart_const_handle,
                                    typename CGAL_LCC_TYPE::Dart_const_handle,
                                    typename CGAL_LCC_TYPE::Dart_const_handle,
                                    typename CGAL_LCC_TYPE::Dart_const_handle>
    drawing_functor_with_volume;

  drawing_functor_with_volume.colored_volume = [](const CGAL_LCC_TYPE&,
                      typename CGAL_LCC_TYPE::Dart_const_handle) -> bool
  { return true; };

  drawing_functor_with_volume.volume_color =  [] (const CGAL_LCC_TYPE& alcc,
                           typename CGAL_LCC_TYPE::Dart_const_handle dh) -> CGAL::IO::Color
  {
    CGAL::Random random((unsigned int)(alcc.darts().index(dh)));
    return get_random_color(random);
  };

  add_in_graphic_buffer(alcc, graphic_buffer, drawing_functor_with_volume);
}

#ifdef CGAL_USE_BASIC_VIEWER

// Specialization of draw function for a LCC, with a drawing functor.
template<unsigned int d_, unsigned int ambient_dim, class Traits_,
         class Items_, class Alloc_,
         template <unsigned int, class, class, class, class> class Map,
         class Refs, class Storage_,
         class DrawingFunctor>
void draw(const CGAL_LCC_TYPE &alcc, const DrawingFunctor &drawing_functor,
          const char *title="LCC Basic Viewer")
{
  CGAL::Graphic_buffer<float> buffer;
  add_in_graphic_buffer(alcc, buffer, drawing_functor);
  draw_buffer(buffer, title);
}

// Specialization of draw function for a LCC, without a drawing functor.
template<unsigned int d_, unsigned int ambient_dim, class Traits_,
         class Items_, class Alloc_,
         template <unsigned int, class, class, class, class> class Map,
         class Refs, class Storage_>
void draw(const CGAL_LCC_TYPE &alcc, const char *title="LCC Basic Viewer")
{
  CGAL::Graphic_buffer<float> buffer;
  add_in_graphic_buffer(alcc, buffer);
  draw_buffer(buffer, title);
}

#endif // CGAL_USE_BASIC_VIEWER

#undef CGAL_LCC_TYPE

} // End namespace CGAL

#endif // CGAL_DRAW_LCC_H

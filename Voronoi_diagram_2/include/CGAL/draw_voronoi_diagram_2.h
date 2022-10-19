// Copyright(c) 2019  Foundation for Research and Technology-Hellas (Greece).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Jasmeet Singh <jasmeet.singh.mec11@iitbhu.ac.in>
//                 Mostafa Ashraf <mostaphaashraf1996@gmail.com>

#ifndef CGAL_DRAW_VORONOI_DIAGRAM_2_H
#define CGAL_DRAW_VORONOI_DIAGRAM_2_H

#include <CGAL/license/Voronoi_diagram_2.h>
#include <CGAL/Qt/Basic_viewer_qt.h>
#include <CGAL/Graphic_buffer.h>
#include <CGAL/Drawing_functor.h>
#include <CGAL/Triangulation_utils_2.h>
#include <CGAL/Voronoi_diagram_2/Face.h>
#include <CGAL/Voronoi_diagram_2/Handle_adaptor.h>
#include <CGAL/Voronoi_diagram_2/Vertex.h>
#include <CGAL/Voronoi_diagram_2/basic.h>
#include <CGAL/Voronoi_diagram_2/Accessor.h>

namespace CGAL {

namespace draw_function_for_v2
{

// We need a specific functor for voronoi2 in order to allow to differentiate
// voronoi and dual vertices, and to manage rays.
template <typename DS,
          typename vertex_handle,
          typename edge_handle,
          typename face_handle>
struct Drawing_functor_voronoi :
    public CGAL::Drawing_functor<DS, vertex_handle, edge_handle, face_handle>
{
  Drawing_functor_voronoi() : m_draw_voronoi_vertices(true),
                              m_draw_dual_vertices(true),
                              dual_vertex_color(CGAL::IO::Color(50, 100, 180)),
                              ray_color(CGAL::IO::Color(100, 0, 0)),
                              bisector_color(CGAL::IO::Color(0, 100, 0))
  {}

  void disable_voronoi_vertices() { m_draw_voronoi_vertices=false; }
  void enable_voronoi_vertices() { m_draw_voronoi_vertices=true; }
  bool are_voronoi_vertices_enabled() const { return m_draw_voronoi_vertices; }
  void negate_draw_voronoi_vertices() { m_draw_voronoi_vertices=!m_draw_voronoi_vertices; }

  void disable_dual_vertices() { m_draw_dual_vertices=false; }
  void enable_dual_vertices() { m_draw_dual_vertices=true; }
  bool are_dual_vertices_enabled() const { return m_draw_dual_vertices; }
  void negate_draw_dual_vertices() { m_draw_dual_vertices=!m_draw_dual_vertices; }

  CGAL::IO::Color dual_vertex_color;
  CGAL::IO::Color ray_color;
  CGAL::IO::Color bisector_color;

protected:
  bool m_draw_voronoi_vertices;
  bool m_draw_dual_vertices;
};

typedef CGAL::Exact_predicates_inexact_constructions_kernel Local_kernel;
typedef Local_kernel::Point_3  Local_point;
typedef Local_kernel::Vector_3 Local_vector;

template <typename BufferType=float, class V2, class DrawingFunctor>
void compute_vertex(const V2& v2,
                    typename V2::Vertex_iterator vh,
                    CGAL::Graphic_buffer<BufferType>& graphic_buffer,
                    const DrawingFunctor& drawing_functor)
{
  if(!drawing_functor.draw_vertex(v2, vh))
  { return; }

  if(drawing_functor.colored_vertex(v2, vh))
  { graphic_buffer.add_point(vh->point(), drawing_functor.vertex_color(v2, vh)); }
  else
  { graphic_buffer.add_point(vh->point()); }
}

template <typename BufferType=float, class V2, class DrawingFunctor>
void compute_dual_vertex(const V2& /*v2*/,
                         typename V2::Delaunay_graph::Finite_vertices_iterator vi,
                         CGAL::Graphic_buffer<BufferType> &graphic_buffer,
                         const DrawingFunctor& drawing_functor)
{ graphic_buffer.add_point(vi->point(), drawing_functor.dual_vertex_color); }

template <typename BufferType=float, class V2, class DrawingFunctor>
void add_segments_and_update_bounding_box(const V2& v2,
                                          typename V2::Halfedge_iterator he,
                                          CGAL::Graphic_buffer<BufferType>& graphic_buffer,
                                          DrawingFunctor& drawing_functor)
{
  typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
  typedef typename V2::Delaunay_vertex_handle Delaunay_vertex_const_handle;

  if (he->is_segment())
  {
    if(drawing_functor.colored_edge(v2, he))
    {
      graphic_buffer.add_segment(he->source()->point(), he->target()->point(),
                                 drawing_functor.edge_color(v2, he));
    }
    else
    {
      graphic_buffer.add_segment(he->source()->point(), he->target()->point());
    }
  }
  else
  {
    Delaunay_vertex_const_handle v1 = he->up();
    Delaunay_vertex_const_handle v2 = he->down();

    Kernel::Vector_2 direction(v1->point().y() - v2->point().y(),
                               v2->point().x() - v1->point().x());
    if (he->is_ray())
    {
      Kernel::Point_2 end_point;
      if (he->has_source())
      {
        end_point = he->source()->point();

        // update_bounding_box_for_ray(end_point, direction);
        Local_point lp = Basic_viewer_qt<>::get_local_point(end_point);
        Local_vector lv = Basic_viewer_qt<>::get_local_vector(direction);
        CGAL::Bbox_3 b = (lp + lv).bbox();
        graphic_buffer.update_bounding_box(b);
      }
    }
    else if (he->is_bisector())
    {
      Kernel::Point_2 pointOnLine((v1->point().x() + v2->point().x()) / 2,
                                  (v1->point().y() + v2->point().y()) / 2);
      Kernel::Vector_2 perpendicularDirection(
          v2->point().x() - v1->point().x(),
          v2->point().y() - v1->point().y());

      // update_bounding_box_for_line(pointOnLine, direction,
      //                               perpendicularDirection);
      Local_point lp = Basic_viewer_qt<>::get_local_point(pointOnLine);
      Local_vector lv = Basic_viewer_qt<>::get_local_vector(direction);
      Local_vector lpv = Basic_viewer_qt<>::get_local_vector(perpendicularDirection);

      CGAL::Bbox_3 b = lp.bbox() + (lp + lv).bbox() + (lp + lpv).bbox();
      graphic_buffer.update_bounding_box(b);
    }
  }
}

template <class V2>
Local_kernel::Point_2 get_second_point(typename V2::Halfedge_handle ray,
                                       const CGAL::Bbox_3 & m_bounding_box)
{
  typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
  typedef typename V2::Delaunay_vertex_handle Delaunay_vertex_const_handle;

  Delaunay_vertex_const_handle v1 = ray->up();
  Delaunay_vertex_const_handle v2 = ray->down();

  // calculate direction of ray and its inverse
  Kernel::Vector_2 v(v1->point().y() - v2->point().y(),
                     v2->point().x() - v1->point().x());
  Local_kernel::Vector_2 inv(1 / v.x(), 1 / v.y());

  // origin of the ray
  Kernel::Point_2 p;
  if (ray->has_source())
  { p = ray->source()->point(); }
  else
  { p = ray->target()->point(); }

  // get the bounding box of the viewer
  Local_kernel::Vector_2 boundsMin(m_bounding_box.xmin(),
                                   m_bounding_box.zmin());
  Local_kernel::Vector_2 boundsMax(m_bounding_box.xmax(),
                                   m_bounding_box.zmax());
  // calculate intersection
  double txmax, txmin, tymax, tymin;

  if (inv.x() >= 0)
  {
    txmax = (boundsMax.x() - p.x()) * inv.x();
    txmin = (boundsMin.x() - p.x()) * inv.x();
  }
  else
  {
    txmax = (boundsMin.x() - p.x()) * inv.x();
    txmin = (boundsMax.x() - p.x()) * inv.x();
  }

  if (inv.y() >= 0)
  {
    tymax = (boundsMax.y() - p.y()) * inv.y();
    tymin = (boundsMin.y() - p.y()) * inv.y();
  }
  else
  {
    tymax = (boundsMin.y() - p.y()) * inv.y();
    tymin = (boundsMax.y() - p.y()) * inv.y();
  }

  if (tymin > txmin)
    txmin = tymin;
  if (tymax < txmax)
    txmax = tymax;

  Local_kernel::Point_2 p1;
  if (v.x() == 0)
  { p1 = Local_kernel::Point_2(p.x(), p.y() + tymax * v.y()); }
  else if (v.y() == 0)
  { p1 = Local_kernel::Point_2(p.x() + txmax * v.x(), p.y()); }
  else
  { p1 = Local_kernel::Point_2(p.x() + txmax * v.x(), p.y() + tymax * v.y()); }
  return p1;
}

// Halfedge_const_handle
template <typename BufferType=float, class V2, class DrawingFunctor>
void compute_rays_and_bisectors(const V2&,
                                typename V2::Halfedge_iterator he,
                                CGAL::Graphic_buffer<BufferType>& graphic_buffer,
                                const DrawingFunctor& drawing_functor)
{
  typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
  typedef typename V2::Delaunay_vertex_handle Delaunay_vertex_const_handle;

  Delaunay_vertex_const_handle v1 = he->up();
  Delaunay_vertex_const_handle v2 = he->down();

  Kernel::Vector_2 direction(v1->point().y() - v2->point().y(),
                             v2->point().x() - v1->point().x());
  if (he->is_ray())
  {
    if (he->has_source())
    {
      // add_ray_segment(he->source()->point(), get_second_point(he, graphic_buffer.get_bounding_box()));
      graphic_buffer.add_ray(he->source()->point(), direction, drawing_functor.ray_color);
    }
  }
  else if (he->is_bisector())
  {
    Kernel::Point_2 pointOnLine((v1->point().x() + v2->point().x()) / 2,
                                (v1->point().y() + v2->point().y()) / 2);
    graphic_buffer.add_line(pointOnLine, direction, drawing_functor.bisector_color);
  }
}

template <typename BufferType=float, class V2, class DrawingFunctor>
void compute_face(const V2& v2,
                  typename V2::Face_iterator fh,
                  CGAL::Graphic_buffer<BufferType>& graphic_buffer,
                  const DrawingFunctor& m_drawing_functor)
{
  if(fh->is_unbounded() || !m_drawing_functor.draw_face(v2, fh))
  { return; }

  if(m_drawing_functor.colored_face(v2, fh))
  { graphic_buffer.face_begin(m_drawing_functor.face_color(v2, fh)); }
  else { graphic_buffer.face_begin(); }

  typename V2::Ccb_halfedge_circulator ec_start=fh->ccb();
  typename V2::Ccb_halfedge_circulator ec=ec_start;
  do
  {
    graphic_buffer.add_point_in_face(ec->source()->point());
  }
  while (++ec!=ec_start);
  graphic_buffer.face_end();

  // Test: for unbounded faces (??)
  //      else {
  //            do{
  //                if( ec->has_source() ){
  //                    add_point_in_face(ec->source()->point());
  //                }
  //                else{
  //                    add_point_in_face(get_second_point(ec->twin(), graphic_buffer.get_bounding_box()));
  //                }
  //            } while(++ec != ec_start);
  //        }
}

template <typename BufferType=float, class V2, class DrawingFunctor>
void compute_elements(const V2& v2,
                      CGAL::Graphic_buffer<BufferType>& graphic_buffer,
                      const DrawingFunctor& drawing_functor)
{
  if(drawing_functor.are_vertices_enabled())
  {
    // Draw the voronoi vertices
    if (drawing_functor.are_voronoi_vertices_enabled())
    {
      for (typename V2::Vertex_iterator it=v2.vertices_begin();
           it!=v2.vertices_end(); ++it)
      { compute_vertex(v2, it, graphic_buffer, drawing_functor); }
    }

    // Draw the dual vertices
    if (drawing_functor.are_dual_vertices_enabled())
    {
      for (typename V2::Delaunay_graph::Finite_vertices_iterator
             it=v2.dual().finite_vertices_begin();
           it!=v2.dual().finite_vertices_end(); ++it)
      { compute_dual_vertex(v2, it, graphic_buffer, drawing_functor); }
    }
  }

  if(drawing_functor.are_edges_enabled())
  {
    // Add segments and update bounding box
    for (typename V2::Halfedge_iterator it=v2.halfedges_begin();
         it!=v2.halfedges_end(); ++it)
    { add_segments_and_update_bounding_box(v2, it,
                                           graphic_buffer, drawing_functor); }
  }

  for (typename V2::Halfedge_iterator it=v2.halfedges_begin();
       it!=v2.halfedges_end(); ++it)
  { compute_rays_and_bisectors(v2, it, graphic_buffer, drawing_functor); }

  if (drawing_functor.are_faces_enabled())
  {
    for (typename V2::Face_iterator it=v2.faces_begin(); it!=v2.faces_end(); ++it)
    { compute_face(v2, it, graphic_buffer, drawing_functor); }
  }
}

} // namespace draw_function_for_v2

#define CGAL_VORONOI_TYPE CGAL::Voronoi_diagram_2 <DG, AT, AP>

template <class DG, class AT, class AP,
          typename BufferType=float, class DrawingFunctor>
void add_in_graphic_buffer(const CGAL_VORONOI_TYPE &v2,
                           CGAL::Graphic_buffer<BufferType>& graphic_buffer,
                           const DrawingFunctor& m_drawing_functor)
{
  draw_function_for_v2::compute_elements(v2, graphic_buffer, m_drawing_functor);
}

template <class DG, class AT, class AP, typename BufferType=float>
void add_in_graphic_buffer(const CGAL_VORONOI_TYPE& v2,
                           CGAL::Graphic_buffer<BufferType>& graphic_buffer)
{
  // Default functor; user can add his own functor.
  CGAL::draw_function_for_v2::Drawing_functor_voronoi<CGAL_VORONOI_TYPE,
                                                      typename CGAL_VORONOI_TYPE::Vertex_iterator,
                                                      typename CGAL_VORONOI_TYPE::Halfedge_iterator,
                                                      typename CGAL_VORONOI_TYPE::Face_iterator>
    drawing_functor;

  add_in_graphic_buffer(v2, graphic_buffer, drawing_functor);
}

#ifdef CGAL_USE_BASIC_VIEWER

// Specialization of draw function.
template<class DG, class AT, class AP,
         typename BufferType=float, class DrawingFunctor>
void draw(const CGAL_VORONOI_TYPE& av2,
          const DrawingFunctor& drawing_functor,
          const char *title="2D Voronoi Diagram Basic Viewer")
{
  CGAL::Graphic_buffer<BufferType> buffer;
  add_in_graphic_buffer(av2, buffer, drawing_functor);
  draw_buffer(buffer, title);
}

template<class DG, class AT, class AP, typename BufferType=float>
void draw(const CGAL_VORONOI_TYPE& av2,
          const char *title="2D Voronoi Diagram Basic Viewer")
{
  CGAL::Graphic_buffer<BufferType> buffer;

  CGAL::draw_function_for_v2::Drawing_functor_voronoi<CGAL_VORONOI_TYPE,
              typename CGAL_VORONOI_TYPE::Vertex_iterator,
              typename CGAL_VORONOI_TYPE::Halfedge_iterator,
              typename CGAL_VORONOI_TYPE::Face_iterator>
  drawing_functor;

  add_in_graphic_buffer(av2, buffer, drawing_functor);

  QApplication_and_basic_viewer app(buffer, title);
  if(app)
  {
    // Here we define the std::function to capture key pressed.
    app.basic_viewer().on_key_pressed=
      [&av2, &drawing_functor] (QKeyEvent* e, CGAL::Basic_viewer_qt<float>* basic_viewer) -> bool
      {
        const ::Qt::KeyboardModifiers modifiers = e->modifiers();
        if ((e->key() == ::Qt::Key_R) && (modifiers == ::Qt::NoButton))
        {
          basic_viewer->negate_draw_rays();
          basic_viewer->displayMessage
            (QString("Draw rays=%1.").arg(basic_viewer->get_draw_rays()?"true":"false"));

          basic_viewer->redraw();
        }
        else if ((e->key() == ::Qt::Key_V) && (modifiers == ::Qt::ShiftModifier))
        {
          if(drawing_functor.are_voronoi_vertices_enabled())
          { drawing_functor.disable_voronoi_vertices(); }
          else { drawing_functor.enable_voronoi_vertices(); }

          basic_viewer->displayMessage
            (QString("Voronoi vertices=%1.").
             arg(drawing_functor.are_voronoi_vertices_enabled()?"true":"false"));

          basic_viewer->clear();
          draw_function_for_v2::compute_elements(av2,
                                                 basic_viewer->get_graphic_buffer(),
                                                 drawing_functor);
          basic_viewer->redraw();
        }
        else if ((e->key() == ::Qt::Key_D) && (modifiers == ::Qt::NoButton))
        {
          if(drawing_functor.are_dual_vertices_enabled())
          { drawing_functor.disable_dual_vertices(); }
          else { drawing_functor.enable_dual_vertices(); }

          basic_viewer->displayMessage(QString("Dual vertices=%1.").
                                       arg(drawing_functor.are_dual_vertices_enabled()?"true":"false"));

          basic_viewer->clear();
          draw_function_for_v2::compute_elements(av2,
                                                 basic_viewer->get_graphic_buffer(),
                                                 drawing_functor);
          basic_viewer->redraw();
        }
        else
        {
          // Return false will call the base method to process others/classicals key
          return false;
        }
        return true; // the key was captured
      };

    // Here we add shortcut descriptions
    app.basic_viewer().setKeyDescription(::Qt::Key_R, "Toggles rays display");
    app.basic_viewer().setKeyDescription(::Qt::Key_D, "Toggles dual vertices display");
    app.basic_viewer().setKeyDescription(::Qt::ShiftModifier, ::Qt::Key_V, "Toggles voronoi vertices display");

    // Then we run the app
    app.run();
  }
}

#endif // CGAL_USE_BASIC_VIEWER

#undef CGAL_VORONOI_TYPE

} // End namespace CGAL

#endif // CGAL_DRAW_VORONOI_DIAGRAM_2_H

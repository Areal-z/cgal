// Copyright (c) 2022 GeometryFactory Sarl (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s):   Guillaume Damiand <guillaume.damiand@liris.cnrs.fr>
//              Mostafa Ashraf <mostaphaashraf1996@gmail.com>

#ifndef CGAL_GENERIC_FUNCTORS_H
#define CGAL_GENERIC_FUNCTORS_H

namespace CGAL {

template <typename DS,
          typename vertex_handle,
          typename edge_handle,
          typename face_handle>  
struct GenericFunctor
{
  GenericFunctor(): m_enabled_vertices(true),
                            m_enabled_edges(true),
                            m_enabled_faces(true)
  {
    draw_vertex=[](const DS &, vertex_handle)->bool { return true; };
    draw_edge=[](const DS &, edge_handle)->bool { return true; };
    draw_face=[](const DS &, face_handle)->bool { return true; };
    
    colored_vertex=[](const DS &, vertex_handle)->bool { return false; };
    colored_edge=[](const DS &, edge_handle)->bool { return false; };
    colored_face=[](const DS &, face_handle)->bool { return false; };

    face_wireframe=[](const DS &, face_handle)->bool { return false; };
  }
    
  std::function<bool(const DS &, vertex_handle)> draw_vertex;
  std::function<bool(const DS &, edge_handle)>   draw_edge;
  std::function<bool(const DS &, face_handle)>   draw_face;
  
  std::function<bool(const DS &, vertex_handle)> colored_vertex;
  std::function<bool(const DS &, edge_handle)>   colored_edge;
  std::function<bool(const DS &, face_handle)>   colored_face;
  
  std::function<bool(const DS &, face_handle)> face_wireframe;
  
  std::function<CGAL::IO::Color(const DS &, vertex_handle)> vertex_color;
  std::function<CGAL::IO::Color(const DS &, edge_handle)>   edge_color;
  std::function<CGAL::IO::Color(const DS &, face_handle)>   face_color;

  void disable_vertices() { m_enabled_vertices=false; }
  void enable_vertices() { m_enabled_vertices=true; }
  bool are_vertices_enabled() const { return m_enabled_vertices; }

  void disable_edges() { m_enabled_edges=false; }
  void enable_edges() { m_enabled_edges=true; }
  bool are_edges_enabled() const { return m_enabled_edges; }

  void disable_faces() { m_enabled_faces=false; }
  void enable_faces() { m_enabled_faces=true; }
  bool are_faces_enabled() const { return m_enabled_faces; }
  
protected:
  bool m_enabled_vertices, m_enabled_edges, m_enabled_faces;
};

template <typename DS, 
          typename vertex_handle, 
          typename edge_handle,
          typename face_handle,
          typename volume_handle>
struct GenericFunctorWithVolume :
    public GenericFunctor<DS, vertex_handle, edge_handle, face_handle>
{
  GenericFunctorWithVolume() : m_enabled_volumes(true)
  {
    draw_volume=[](const DS &, volume_handle)->bool { return true; };
    colored_volume=[](const DS &, volume_handle)->bool { return false; };
    volume_wireframe=[](const DS &, volume_handle)->bool { return false; };
  }

  std::function<bool(const DS &, volume_handle)>            draw_volume;
  std::function<bool(const DS &, volume_handle)>            colored_volume;
  std::function<bool(const DS &, volume_handle)>            volume_wireframe;
  std::function<CGAL::IO::Color(const DS &, volume_handle)> volume_color;

  void disable_volumes() { m_enabled_volumes=false; }
  void enable_volumes() { m_enabled_volumes=true; }
  bool are_volumes_enabled() const { return m_enabled_volumes; }

protected:
  bool m_enabled_volumes;
};

} // End namespace CGAL

#endif // CGAL_GENERIC_FUNCTORS_H
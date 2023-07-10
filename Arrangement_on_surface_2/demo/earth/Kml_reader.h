
#ifndef KML_READER_H
#define KML_READER_H

#include <iostream>
#include <string>
#include <vector>

#include <qvector3d.h>


class Kml
{
public:
  // double precision 3D-point (QVector3D has float coordinates)
  struct Vec3d
  {
    double x, y, z;

    friend std::ostream& operator << (std::ostream& os, const Vec3d& v)
    {
      os << v.x << ", " << v.y << ", " << v.z;
      return os;
    }
  };

  struct Node
  {
    double lon, lat;

    Node() : lon(-1111), lat(-1111) {};
    Node(double longitude, double latitude) : lon(longitude), lat(latitude) {};

    bool operator == (const Node& r) const;
    Vec3d get_coords_3d(const double r = 1.0) const;
    QVector3D get_coords_3f(const double r=1.0) const;
    friend std::ostream& operator << (std::ostream& os, const Node& n);
  };
  using Nodes = std::vector<Node>;

  struct Arc
  {
    Node from, to;
  };
  using Arcs = std::vector<Arc>;

  struct LinearRing
  {
    std::vector<Node> nodes;
    std::vector<int>  ids;

    Arcs get_arcs() const;
    void get_arcs(Arcs& arcs) const;
  };
  using LinearRings = std::vector<LinearRing>;


  struct Polygon
  {
    LinearRing outer_boundary;
    LinearRings inner_boundaries;

    // when collecting nodes start from the outer boundary and then get nodes
    // from individual inner boundaries in the order
    Nodes get_all_nodes() const;
  
    std::vector<LinearRing*> get_all_boundaries();
  };
 

  struct Placemark
  {
    std::vector<Polygon> polygons;
    std::string name;

    // collects all nodes from all polygons
    Nodes get_all_nodes() const;

    Arcs get_all_arcs() const;
  };
  using Placemarks = std::vector<Placemark>;


  static Placemarks read(const std::string& file_name);

  static Nodes get_duplicates(const Placemarks& placemarks);


  // Outputs all used nodes without duplications!
  // NOTE: this function modifies Placemarks data-structure!
  static Nodes generate_ids(Placemarks& placemarks);
};


#endif

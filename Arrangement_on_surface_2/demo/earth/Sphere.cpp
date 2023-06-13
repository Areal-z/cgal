
#include "Sphere.h"

#include <qvector3d.h>

#include <cmath>
#include <vector>


Sphere::Sphere(int num_slices, int num_stacks, float r)
{
  initializeOpenGLFunctions();

  num_stacks = std::max<int>(2, num_stacks);
  std::vector<QVector3D> vertices, normals;

  // NORTH POLE
  vertices.push_back(QVector3D(0, 0, r));
  normals.push_back(QVector3D(0, 0, 1));

  // SOUTH POLE
  vertices.push_back(QVector3D(0, 0, -r));
  normals.push_back(QVector3D(0, 0, -1));
  int starting_index_of_middle_vertices = vertices.size();

  for (int j = 1; j < num_stacks; ++j)
  {
    // Calculate the latitude (vertical angle) for the current stack
    float lat = M_PI * j / num_stacks;
    float rxy = r * std::sin(lat);
    float z = r * std::cos(lat);

    for (int i = 0; i < num_slices; ++i)
    {
      // Calculate the longitude (horizontal angle) for the current slice
      float lon = 2 * M_PI * i / num_slices;

      // Convert spherical coordinates to Cartesian coordinates
      float x = rxy * std::cos(lon);
      float y = rxy * std::sin(lon);

      auto p = QVector3D(x, y, z);
      auto n = p / p.length();
      vertices.push_back(p);
      normals.push_back(n);
    }
  }

  // strided vertex-data
  std::vector<QVector3D> vertex_data;
  for (int i = 0; i < vertices.size(); ++i)
  {
    vertex_data.push_back(vertices[i]);
    vertex_data.push_back(normals[i]);
  }


  // add the indices for all triangles
  std::vector<GLuint> indices;

  // NORTH CAP
  const int north_vertex_index = 0;
  const int north_cap_vertex_index_start = starting_index_of_middle_vertices;
  for (int i = 0; i < num_slices; i++)
  {
    indices.push_back(north_vertex_index);
    indices.push_back(north_cap_vertex_index_start + i);
    indices.push_back(north_cap_vertex_index_start + (i + 1) % num_slices);
  }

  // 0 = NORTH VERTEX
  // 1 = SOUTH VERTEX
  // [2, 2 + (numSlices-1)] = bottom vertices of the stack #1
  // [2+numSlices, 2 + (2*numSlices - 1)] = bottom vertices of the stack #2
  // ...
  // [2+(k-1)*numSlices, 2 + (k*numSlices -1)] = bottom vertices of the stack #k
  // ..
  // [2+(numStacks-1)*numSlices, 2+(numStacks*numSlices-1)] = bottom vertices of
  //                                                the last stack (# numStacks)

  // SOUTH CAP
  const int south_vertex_index = 1;
  const int south_cap_index_start = starting_index_of_middle_vertices +
    (num_stacks - 2) * num_slices;
  for (int i = 0; i < num_slices; ++i)
  {
    const auto vi0 = south_vertex_index;
    const auto vi1 = south_cap_index_start + i;
    const auto vi2 = south_cap_index_start + (i + 1) % num_slices;
    indices.push_back(vi2);
    indices.push_back(vi1);
    indices.push_back(vi0);
  }

  // MIDDLE TRIANGLES
  for (int k = 0; k < num_stacks - 2; ++k)
  {
    const int stack_start_index = starting_index_of_middle_vertices +
      k * num_slices;
    const int next_stack_start_index = stack_start_index + num_slices;
    for (int i = 0; i < num_slices; ++i)
    {
      // check why the following code snippet does not work (winding order?)
      //int vi0 = stackStartIndex + i;
      //int vi1 = nextStackStartIndex + i;
      //int vi2 = nextStackStartIndex + (i + 1) % numSlices;
      //int vi3 = stackStartIndex + (i + 1) % numSlices;
      int vi0 = stack_start_index + i;
      int vi1 = stack_start_index + (i + 1) % num_slices;
      int vi2 = next_stack_start_index + i;
      int vi3 = next_stack_start_index + (i + 1) % num_slices;

      indices.push_back(vi0);
      indices.push_back(vi2);
      indices.push_back(vi1);
      //
      indices.push_back(vi2);
      indices.push_back(vi3);
      indices.push_back(vi1);
    }
  }
  m_num_indices = indices.size();


  // DEFINE OPENGL BUFFERS
  glGenVertexArrays(1, &m_vao);
  glBindVertexArray(m_vao);

  // Index buffer
  glGenBuffers(1, &m_ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
  auto indices_size = sizeof(GLuint) * indices.size();
  auto indices_data = reinterpret_cast<const void*>(indices.data());
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
    indices_size,
    indices_data,
    GL_STATIC_DRAW);

  // Vertex Buffer
  glGenBuffers(1, &m_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  auto vertex_buffer_size = sizeof(QVector3D) * vertex_data.size();
  auto vertex_buffer_data = reinterpret_cast<const void*>(vertex_data.data());
  glBufferData(GL_ARRAY_BUFFER,
    vertex_buffer_size,
    vertex_buffer_data,
    GL_STATIC_DRAW);

  // Position Vertex-Attribute
  GLint position_attrib_index = 0;
  const void* position_offset = 0;
  GLsizei stride = 6 * sizeof(float);
  glVertexAttribPointer(position_attrib_index,
    3,
    GL_FLOAT, GL_FALSE,
    stride,
    position_offset);
  glEnableVertexAttribArray(position_attrib_index);

  // Normal Vertex-Attribute
  GLint normal_attrib_index = 1;
  auto* normal_offset = reinterpret_cast<const void*>(3 * sizeof(float));
  glVertexAttribPointer(normal_attrib_index,
    3,
    GL_FLOAT,
    GL_FALSE,
    stride,
    normal_offset);
  glEnableVertexAttribArray(normal_attrib_index);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  // Note: calling this before glBindVertexArray(0) results in no output!
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void Sphere::draw()
{
  // DRAW TRIANGLES
  glBindVertexArray(m_vao);
  {
    //glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glDrawElements(GL_TRIANGLES, m_num_indices, GL_UNSIGNED_INT, 0);
  }
  glBindVertexArray(0);
}
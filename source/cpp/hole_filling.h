#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/boost/graph/graph_traits_Surface_mesh.h>

#include <CGAL/Polygon_mesh_processing/triangulate_hole.h>
#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <boost/foreach.hpp>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef Kernel::Point_3 Point;
typedef CGAL::Surface_mesh<Point> Mesh;

typedef boost::graph_traits<Mesh>::halfedge_descriptor    halfedge_descriptor;
typedef boost::graph_traits<Mesh>::face_descriptor       face_descriptor;
typedef boost::graph_traits<Mesh>::vertex_descriptor      vertex_descriptor;

int fill_holes(Mesh& mesh)
{
  unsigned int nb_holes = 0;
  BOOST_FOREACH(halfedge_descriptor h, halfedges(mesh))
  {
    if(is_border(h,mesh))
    {
      std::vector<face_descriptor>  patch_facets;
      std::vector<vertex_descriptor> patch_vertices;
      bool success = CGAL::cpp11::get<0>(
        CGAL::Polygon_mesh_processing::triangulate_refine_and_fair_hole(
                  mesh,
                  h,
                  std::back_inserter(patch_facets),
                  std::back_inserter(patch_vertices),
     CGAL::Polygon_mesh_processing::parameters::vertex_point_map(get(CGAL::vertex_point, mesh)).
                  geom_traits(Kernel())) );

      std::cout << "* Number of facets in constructed patch: " << patch_facets.size() << std::endl;
      std::cout << "  Number of vertices in constructed patch: " << patch_vertices.size() << std::endl;
      std::cout << "  Is fairing successful: " << success << std::endl;
      nb_holes++;
    }
  }

  std::cout << std::endl;
  std::cout << nb_holes << " holes have been filled" << std::endl;
  
  return nb_holes;
}


int triangulate_faces(Mesh& mesh)
{
  CGAL::Polygon_mesh_processing::triangulate_faces(mesh);

  // Confirm that all faces are triangles.
  BOOST_FOREACH(boost::graph_traits<Mesh>::face_descriptor fit, faces(mesh))
    if (next(next(halfedge(fit, mesh), mesh), mesh)
        !=   prev(halfedge(fit, mesh), mesh))
      std::cerr << "Error: non-triangular face left in mesh." << std::endl;

  return 0;
}
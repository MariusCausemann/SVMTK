#include "CGALMeshCreator.h"
//#include "remove_isolated_vertices.h"

// move from h other stuff
#include <CGAL/IO/File_medit.h>
#include <CGAL/make_mesh_3.h>
#include <CGAL/refine_mesh_3.h>
#include <CGAL/IO/Polyhedron_iostream.h>

#include <CGAL/Mesh_3/Detect_polylines_in_polyhedra.h>
#include <CGAL/Mesh_3/polylines_to_protect.h>
// Current problem simple geometries with sharp edges 90 degree 
// CLEAN : add egdes and insert edges
// insert_edges -> insert points, add_edges -> insert polylines



// --------------------------------
//  Constructor for multiple CGALSurfaces and uses binary mapping to set subdomain indices 
//--------------------------------
CGALMeshCreator::CGALMeshCreator( std::vector<CGALSurface> surfaces )
{
    Function_vector v;
    Polyhedron polyhedron;
    for(std::vector<CGALSurface>::iterator sit= surfaces.begin() ;sit!= surfaces.end();sit++)
    {
       sit->get_polyhedron(polyhedron);
       min_sphere.add_polyhedron(polyhedron);
       Polyhedral_mesh_domain_3 *polyhedral_domain = new Polyhedral_mesh_domain_3(polyhedron);
       v.push_back(polyhedral_domain);
    }
    Function_wrapper wrapper(v);

    domain_ptr = std::unique_ptr<Mesh_domain> (new Mesh_domain(wrapper,wrapper.bbox()));
    default_parameters();
}

// --------------------------------
//  Constructor for multiple CGALSurfaces and a AbstractMap i.e. 
//  user-defined mapping of binary states of overlapping CGALSurfaces. "00101"-> 2 instead of 20  
//--------------------------------
CGALMeshCreator::CGALMeshCreator( std::vector<CGALSurface> surfaces , AbstractMap& map )
{
    Function_vector v;
    Polyhedron polyhedron;
    for(std::vector<CGALSurface>::iterator sit= surfaces.begin() ;sit!= surfaces.end();sit++)
    {
       sit->get_polyhedron(polyhedron);
       min_sphere.add_polyhedron(polyhedron);
       Polyhedral_mesh_domain_3 *polyhedral_domain = new Polyhedral_mesh_domain_3(polyhedron);
       v.push_back(polyhedral_domain);
      
    }


    Function_wrapper wrapper(v,map);


    domain_ptr=std::unique_ptr<Mesh_domain> (new Mesh_domain(wrapper, wrapper.bbox()));
    default_parameters();
}



// --------------------------------
//  Constructor for CGALSurfaces
// --------------------------------
CGALMeshCreator::CGALMeshCreator(CGALSurface &surface) 
{
    Function_vector v;
    Polyhedron polyhedron;
    surface.get_polyhedron(polyhedron);

    min_sphere.add_polyhedron(polyhedron);
    // add_sharp_border_edges(polyhedron); standalone 

    Polyhedral_mesh_domain_3 *polyhedral_domain = new Polyhedral_mesh_domain_3(polyhedron);

    v.push_back(polyhedral_domain);
    Function_wrapper wrapper(v);



    domain_ptr=std::unique_ptr<Mesh_domain> (new Mesh_domain(wrapper,wrapper.bbox())); 

    default_parameters();
    // compute default parameters -> FT CGAL::Polygon_mesh_processing::volume and  resolution gives cell_size 
}

//----------------------------------------------------
//----------------------------------------------------
//----------------------------------------------------
//----------------------------------------------------

void CGALMeshCreator::lipschitz_size_field(int subdomain_id, int k,double min_size,double max_size)
{

    // lipschitz_size_field  
    // TODO : Implement

    if (lip_sizing_ptr)
    {
        
       lip_sizing_ptr->add_parameters_for_subdomain(subdomain_id,k,min_size,max_size);
    }
    else
    {
       lip_sizing_ptr = std::unique_ptr<Lip_sizing> (new Lip_sizing(*domain_ptr.get()));
       lip_sizing_ptr.get()->add_parameters_for_subdomain(subdomain_id,k,min_size,max_size);

    }
}

void CGALMeshCreator::set_parameters(Parameters new_parameters)
{
    for (Parameters::iterator pit= new_parameters.begin(); pit!=new_parameters.end(); ++pit )
    {
        parameters[pit->first] = static_cast<double>(pit->second);
    }
}
void CGALMeshCreator::set_parameter(std::string key , double value )
{
   parameters[key] = value;
}



void CGALMeshCreator::default_creating_mesh()
{

    Mesh_criteria criteria(CGAL::parameters::facet_angle   =25.0, 
                           CGAL::parameters::edge_size     =0.025,
                           CGAL::parameters::facet_size    =0.05,
                           CGAL::parameters::facet_distance=0.005,
                           CGAL::parameters::cell_radius_edge_ratio=3,
                           CGAL::parameters::cell_size=0.05);

    c3t3 = CGAL::make_mesh_3<C3t3>(*domain_ptr.get(), criteria);

    remove_isolated_vertices(c3t3);

}
void CGALMeshCreator::create_mesh()
{
    std::cout << "begin_meshing" << std::endl;


    
    Mesh_criteria criteria(CGAL::parameters::facet_angle=parameters["facet_angle"],
                           CGAL::parameters::facet_size =parameters["facet_size"],
                           CGAL::parameters::facet_distance=parameters["facet_distance"],
                           CGAL::parameters::cell_radius_edge_ratio=parameters["cell_radius_edge_ratio"],
                           CGAL::parameters::cell_size=parameters["cell_size"] );

    c3t3 = CGAL::make_mesh_3<C3t3>(*domain_ptr.get(), criteria);

    remove_isolated_vertices(c3t3);

}

void CGALMeshCreator::create_mesh(const double mesh_resolution )
{

    // r=
    double r = min_sphere.get_bounding_sphere_radius(); 
    const double cell_size = r/mesh_resolution*2.0;

    Mesh_criteria criteria(CGAL::parameters::edge_size = cell_size,
                                       CGAL::parameters::facet_angle = 30.0,
                                       CGAL::parameters::facet_size = cell_size,
                                       CGAL::parameters::facet_distance = cell_size/10.0, 
                                       CGAL::parameters::cell_radius_edge_ratio = 3.0,
                                       CGAL::parameters::cell_size = cell_size);

    c3t3 = CGAL::make_mesh_3<C3t3>(*domain_ptr.get(), criteria);

    remove_isolated_vertices(c3t3);

}




void CGALMeshCreator::save_mesh(std::string OutPath)
{
    std::ofstream  medit_file(OutPath);
    c3t3.output_to_medit(medit_file);
    medit_file.close();
}


void CGALMeshCreator::refine_mesh()
{
   Mesh_criteria criteria( CGAL::parameters::facet_angle=parameters["facet_angle"],
                           CGAL::parameters::facet_size =parameters["facet_size"],
                           CGAL::parameters::facet_distance=parameters["facet_distance"],
                           CGAL::parameters::cell_radius_edge_ratio=parameters["cell_radius_edge_ratio"],
                           CGAL::parameters::cell_size=parameters["cell_size"] );

   refine_mesh_3(c3t3, *domain_ptr.get(), criteria,CGAL::parameters::no_reset_c3t3());


}


void CGALMeshCreator::label_boundary_cells(int btag , int ntag ) // workaround to mark boundary cells of for example lateral ventircles. this allows for easy marking of Facetfunction in FEniCS 
{
  Subdomain_index subdomain_index(btag);
  Subdomain_index subdomain_index_bis(ntag);
  for(C3t3::Cells_in_complex_iterator cit = c3t3.cells_in_complex_begin(subdomain_index);cit != c3t3.cells_in_complex_end(); ++cit)
  {
     for (std::size_t i = 0; i < 4; i++)
     { if (c3t3.subdomain_index(cit->neighbor(i))!=subdomain_index){c3t3.set_subdomain_index(cit->neighbor(i), subdomain_index_bis);}    }      
  }

} 

void CGALMeshCreator::remove_label_cells(int tag) //rename
{
  Subdomain_index subdomain_index(tag);
  for(C3t3::Cells_in_complex_iterator cit = c3t3.cells_in_complex_begin(subdomain_index);cit != c3t3.cells_in_complex_end(); ++cit){ c3t3.remove_from_complex(cit);}
}


//----------------------   Have overloaded functions or just one????????????????
void CGALMeshCreator::add_sharp_border_edges(Polyhedron& polyhedron) // no need to expose ?  
{ 

  Polylines polylinput; 
  typedef boost::property_map<Polyhedron, CGAL::edge_is_feature_t>::type EIF_map;
  EIF_map eif = get(CGAL::edge_is_feature, polyhedron);

  CGAL::Polygon_mesh_processing::detect_sharp_edges(polyhedron,80, eif); // -> threshold ?? 
   for( Polyhedron::Edge_iterator he = polyhedron.edges_begin(); he != polyhedron.edges_end() ; ++he)
   {
      if(he->is_feature_edge() ) 
      {
         Polyline_3 polyline;
         polyline.push_back(he->vertex()->point());
         polyline.push_back(he->opposite()->vertex()->point());     
         polylinput.push_back(polyline);
      }    
  }   
  polylines_to_protect(this->borders, polylinput.begin(),  polylinput.end() ); // borders 
}



void CGALMeshCreator::add_sharp_border_edges(CGALSurface& surface) 
{ 

  Polyhedron polyhedron;
  surface.get_polyhedron(polyhedron);
  Polylines polylinput; 
  typedef boost::property_map<Polyhedron, CGAL::edge_is_feature_t>::type EIF_map;
  EIF_map eif = get(CGAL::edge_is_feature, polyhedron);

  CGAL::Polygon_mesh_processing::detect_sharp_edges(polyhedron,80, eif);
   for( Polyhedron::Edge_iterator he = polyhedron.edges_begin(); he != polyhedron.edges_end() ; ++he)
   {
      if(he->is_feature_edge() ) 
      {
         Polyline_3 polyline;
         polyline.push_back(he->vertex()->point());
         polyline.push_back(he->opposite()->vertex()->point());     
         polylinput.push_back(polyline);
      }    
  }   
  polylines_to_protect(this->borders, polylinput.begin(),  polylinput.end() );


}

// 
template < typename InputIterator>
void CGALMeshCreator::insert_edge(InputIterator begin, InputIterator end) // Used in conjuction with refine mesh -> will insert points directly into mesh TODO: Remove
{
  Tr& tr = c3t3.triangulation();
  Corner_index corner_index (1);
  Curve_index curve_index (1);
  std::vector<Vertex_handle> vertex_map;

  for ( ; begin != end; ++begin)
  {

      Vertex_handle *vh =  new Vertex_handle();
      *vh =  tr.insert(*begin);
      vertex_map.push_back(&vh);

  }

  for (typename std::vector<Vertex_handle>::const_iterator it = vertex_map.begin();it != vertex_map.end(); it++)
  {
      if (  std::next(it, 1) != vertex_map.end() )
      {
          c3t3.add_to_complex(*it,*std::next(it, 1), curve_index);  // split ??
          
      }

  } 


}
void CGALMeshCreator::set_features()
{ 
  Polylines polylines;
  polylines_to_protect(polylines, features.begin() , features.end()  );
  set_features(polylines.begin(), polylines.end());
}


void CGALMeshCreator::lloyd(double time_limit, int max_iteration_number, double convergence,double freeze_bound, bool do_freeze )
{CGAL::lloyd_optimize_mesh_3(c3t3, *domain_ptr.get(), time_limit=time_limit, max_iteration_number=max_iteration_number,convergence=convergence, freeze_bound  = freeze_bound, do_freeze = do_freeze); } 

void CGALMeshCreator::odt(double time_limit, int max_iteration_number, double convergence,double freeze_bound, bool do_freeze) 
{CGAL::odt_optimize_mesh_3(c3t3, *domain_ptr.get(), time_limit=time_limit, max_iteration_number=max_iteration_number,convergence=convergence, freeze_bound  = freeze_bound, do_freeze = do_freeze); } 



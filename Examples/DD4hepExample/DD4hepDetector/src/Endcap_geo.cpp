///////////////////////////////////////////////////////////////////
// Endcap_geo.cpp, ACTS project
///////////////////////////////////////////////////////////////////

#include "DD4hep/DetFactoryHelper.h"
#include "ACTS/Plugins/DD4hepPlugins/IDetExtension.hpp"
#include "ACTS/Plugins/DD4hepPlugins/DetExtension.hpp"

using namespace std;
using namespace DD4hep;
using namespace DD4hep::Geometry;

/**
 Constructor for a disc like endcap volume, possibly containing layers and the layers possibly containing modules. Both endcaps, the positive and negative can be build with this constructor. Atlas like style
 */

static Ref_t create_element(LCDD& lcdd, xml_h xml, SensitiveDetector sens)
{
    xml_det_t x_det = xml;
    string det_name = x_det.nameStr();
    //Make DetElement
    DetElement cylinderVolume(det_name, x_det.id());
    //add Extension to Detlement for the RecoGeometry
    Acts::DetExtension* detvolume = new Acts::DetExtension(Acts::ShapeType::Disc);
    cylinderVolume.addExtension<Acts::IDetExtension>(detvolume);
    //make Volume
    DD4hep::XML::Dimension x_det_dim(x_det.dimensions());
    Tube tube_shape(x_det_dim.rmin(),x_det_dim.rmax(),x_det_dim.dz());
    Volume tube_vol(det_name,tube_shape,lcdd.air()); //air at the moment change later
    tube_vol.setVisAttributes(lcdd, x_det_dim.visStr());
    //go trough possible layers
        int module_num_num = 0;
    size_t layer_num = 0;
    for (xml_coll_t j(xml,_U(layer));j;++j)
    {
        xml_comp_t x_layer  = j;
        double l_rmin       = x_layer.inner_r();
        double l_rmax       = x_layer.outer_r();
        double l_length     = x_layer.dz();
        //the vector for the modules
        std::vector<DD4hep::Geometry::DetElement> mod;
        //Create Volume and DetElement for Layer
        string layer_name  = det_name + _toString(layer_num,"layer%d");
        Volume layer_vol(layer_name,Tube(l_rmin,l_rmax,l_length), lcdd.material(x_layer.materialStr()));
        DetElement lay_det (cylinderVolume,layer_name,layer_num);
        //Visualization
        layer_vol.setVisAttributes(lcdd, x_layer.visStr());
        //go trough possible modules
        if(x_layer.hasChild(_U(module))){
            for (xml_coll_t i(x_layer,_U(module)); i; i++) {
                xml_comp_t x_module = i;
            int repeat = x_module.repeat();
            double deltaphi = 2.*M_PI/repeat;
            double radius   = x_module.radius();
            double slicedz  = x_module.dz();
            //Create the module volume
            Volume mod_vol("module", Trapezoid(x_module.x1(),x_module.x2(), x_module.thickness(), x_module.thickness(),x_module.length()), lcdd.material(x_module.materialStr()));//changed
            size_t module_num = 0;
            //Place the Modules
            for (int k=0;k < repeat;k++)
            {
                string zname = _toString(k,"z%d");
                //Visualization
                mod_vol.setVisAttributes(lcdd, x_module.visStr());
                double phi = deltaphi/dd4hep::rad * k;
                string module_name = zname + _toString(repeat*module_num_num+module_num,"module%d");
                Position trans(radius*cos(phi),
                               radius*sin(phi),
                               slicedz);
                //Create the module DetElement
                DetElement mod_det(lay_det,module_name,repeat*module_num_num+module_num);
                //Set Sensitive Volmes sensitive
                if (x_module.isSensitive()) {
                    mod_vol.setSensitiveDetector(sens);
                    //add Extension for sensitive component
                    Acts::DetExtension* detSensModule = new Acts::DetExtension(sens.readout().segmentation());
                    mod_det.addExtension<Acts::IDetExtension>(detSensModule);
                }
                
                // go through possible components
                int comp_num = 0.;
                for (xml_coll_t comp(x_module,_U(module_component)); comp; comp++) {
                    xml_comp_t x_comp = comp;
                    // create the component volume
                    string comp_name = _toString(comp_num,"component%d") + x_comp.materialStr();
                    Volume comp_vol(comp_name, Trapezoid(x_comp.x1(),x_comp.x2(),x_comp.thickness(),x_comp.thickness(),x_comp.length()),lcdd.material(x_comp.materialStr()));
                    comp_vol.setVisAttributes(lcdd, x_comp.visStr());
                    // Create DetElement
                    DetElement comp_det(mod_det,comp_name,comp_num);
                    // Set Sensitive Volumes sensitive
/*                    if(x_comp.isSensitive()) {
                        comp_vol.setSensitiveDetector(sens);
                        //add Extension for sensitive component
                        Acts::DetExtension* detSensComponent = new Acts::DetExtension(sens.readout().segmentation());
                        mod_det.addExtension<Acts::IDetExtension>(detSensComponent);
                    }*/
                    // place component in module
                    Position translation (0., x_comp.z(), 0.);
                    PlacedVolume placed_comp = mod_vol.placeVolume(comp_vol,translation);
                    // assign the placed Volume to the DetElement
                    comp_det.setPlacement(placed_comp);
                    placed_comp.addPhysVolID("component",comp_num);
                    ++comp_num;
                }
                
                mod.push_back(mod_det);
                //Place Module Box Volumes in layer
                PlacedVolume placedmodule = layer_vol.placeVolume(mod_vol,Transform3D(RotationX(0.5*M_PI)*RotationY(phi+0.5*M_PI)*RotationZ(0.1*M_PI),trans)); //RotationX(0.5*M_PI)*RotationY(phi+0.5*M_PI)*RotationZ(0.1*M_PI),trans)
                placedmodule.addPhysVolID("module", repeat*module_num_num+module_num);
                //assign module DetElement to the placed module volume
                mod_det.setPlacement(placedmodule);
                ++module_num;
            }
            ++module_num_num;
          }
        }
        //set granularity of layer material mapping and where material should be mapped
        // hand over modules to ACTS
        Acts::DetExtension* detlayer = new Acts::DetExtension(100,100,Acts::LayerMaterialPos::inner,mod,"XzY");
        lay_det.addExtension<Acts::IDetExtension>(detlayer);
        double layerZpos = x_layer.z();
        if (x_det_dim.z() < 0.) layerZpos = -x_layer.z();
        //Placed Layer Volume
        Position layer_pos(0.,0.,layerZpos);
        PlacedVolume placedLayer = tube_vol.placeVolume(layer_vol, layer_pos);
        placedLayer.addPhysVolID("layer",layer_num);
        lay_det.setPlacement(placedLayer);
        ++layer_num;
    }
    //Place Volume
    Position endcap_translation(0.,0.,x_det_dim.z());
    Transform3D endcap_transform(endcap_translation);
    Volume mother_vol = lcdd.pickMotherVolume(cylinderVolume);
    PlacedVolume placedTube = mother_vol.placeVolume(tube_vol, endcap_transform);
    placedTube.addPhysVolID("system",cylinderVolume.id());
    cylinderVolume.setPlacement(placedTube);
    
    return cylinderVolume;
}

DECLARE_DETELEMENT(ACTS_Endcap, create_element)

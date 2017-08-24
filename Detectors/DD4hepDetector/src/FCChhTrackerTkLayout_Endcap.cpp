
#include "DD4hep/DetFactoryHelper.h"
#include "DetUtils.h"

#include "ACTS/Plugins/DD4hepPlugins/ActsExtension.hpp"
#include "ACTS/Plugins/DD4hepPlugins/IActsExtension.hpp"

using dd4hep::Volume;
using dd4hep::DetElement;
using dd4hep::xml::Dimension;
using dd4hep::xml::Component;
using dd4hep::PlacedVolume;

namespace det {
static dd4hep::Ref_t
createTkLayoutTrackerEndcap(dd4hep::Detector&             lcdd,
                            dd4hep::xml::Handle_t               xmlElement,
                            dd4hep::SensitiveDetector sensDet)
{
  // shorthands
  dd4hep::xml::DetElement xmlDet
      = static_cast<dd4hep::xml::DetElement>(xmlElement);
  Dimension dimensions(xmlDet.dimensions());
  double    l_overlapMargin = 0.0001;

  // get sensitive detector type from xml
  dd4hep::xml::Dimension sdTyp
      = xmlElement.child(_Unicode(sensitive));  // retrieve the type
  sensDet.setType(sdTyp.typeStr());             // set for the whole detector

  // definition of top volume
  std::string                 detName = xmlDet.nameStr();
  DetElement                  worldDetElement(detName, xmlDet.id());
  Acts::ActsExtension::Config actsBarrelConfig;
  actsBarrelConfig.isEndcap        = true;
  Acts::ActsExtension* worldDetExt = new Acts::ActsExtension(actsBarrelConfig);
  worldDetElement.addExtension<Acts::IActsExtension>(worldDetExt);

  // envelope volume for one of the endcaps, either forward or backward
  double envelopeThickness = 0.5 * (dimensions.zmax() - dimensions.zmin());
  dd4hep::Tube envelopeShape(dimensions.rmin() - l_overlapMargin,
                                       dimensions.rmax() + l_overlapMargin,
                                       envelopeThickness + l_overlapMargin);
  Volume envelopeVolume(detName, envelopeShape, lcdd.air());
  envelopeVolume.setVisAttributes(lcdd.invisible());

  Component xDiscs          = xmlElement.child(_Unicode(discs));
  Component xFirstDisc      = xDiscs.child(_Unicode(discZPls));
  Component xFirstDiscRings = xFirstDisc.child(_Unicode(rings));

  // create disc volume
  l_overlapMargin *= 0.99;
  double discThickness = 0.5 * (xFirstDisc.zmax() - xFirstDisc.zmin());
  dd4hep::Tube discShape(dimensions.rmin() - l_overlapMargin,
                                   dimensions.rmax() + l_overlapMargin,
                                   discThickness + l_overlapMargin);
  Volume discVolume("disc", discShape, lcdd.air());
  discVolume.setVisAttributes(lcdd.invisible());

  unsigned int discCounter = 0;
  unsigned int compCounter = 0;
  double       currentZ;
  for (dd4hep::xml::Collection_t xDiscColl(xDiscs, "discZPls");
       nullptr != xDiscColl;
       ++xDiscColl) {
    Component xDisc = static_cast<Component>(xDiscColl);
    currentZ        = xDisc.z() - dimensions.zmin() - envelopeThickness;
    DetElement disc_det(
        worldDetElement, "disc" + std::to_string(discCounter), discCounter);
    // iterate over rings
    for (dd4hep::xml::Collection_t xRingColl(xFirstDiscRings, _U(ring));
         nullptr != xRingColl;
         ++xRingColl) {
      Component xRing             = static_cast<Component>(xRingColl);
      Component xRingModules      = xRing.child(_Unicode(modules));
      Component xModuleOdd        = xRingModules.child(_Unicode(moduleOdd));
      Component xModuleEven       = xRingModules.child(_Unicode(moduleEven));
      Component xModuleProperties = xRing.child(_Unicode(moduleProperties));
      Component xModulePropertiesComp
          = xModuleProperties.child(_Unicode(components));
      Component xSensorProperties = xRing.child(_Unicode(sensorProperties));

      // place components in module
      double integratedCompThickness = 0.;
      for (dd4hep::xml::Collection_t xCompColl(xModulePropertiesComp,
                                               _U(component));
           nullptr != xCompColl;
           ++xCompColl) {
        Component xComp = static_cast<Component>(xCompColl);
        Volume    componentVolume(
            "component",
            dd4hep::Trapezoid(
                0.5 * xModuleProperties.attr<double>("modWidthMin"),
                0.5 * xModuleProperties.attr<double>("modWidthMax"),
                0.5 * xComp.thickness(),
                0.5 * xComp.thickness(),
                0.5 * xSensorProperties.attr<double>("sensorLength")),
            lcdd.material(xComp.materialStr()));

        // create the Acts::DigitizationModule (needed to do geometric
        // digitization) for all modules which have the same segmentation
        auto digiModule = Acts::trapezoidalDigiModule(
            0.5 * xModuleProperties.attr<double>("modWidthMin"),
            0.5 * xModuleProperties.attr<double>("modWidthMax"),
            0.5 * xSensorProperties.attr<double>("sensorLength"),
            0.5 * xComp.thickness(),
            sensDet.readout().segmentation());

        unsigned int nPhi = xRing.attr<int>("nModules");
        double       lX, lY, lZ;
        double       phi = 0;
        for (unsigned int phiIndex = 0; phiIndex < nPhi; ++phiIndex) {
          double phiTilt   = 0;
          double thetaTilt = 0;
          if (0 == phiIndex % 2) {
            // the rotation for the odd module is already taken care
            // of by the position in tklayout xml
            phi = 2 * dd4hep::pi * static_cast<double>(phiIndex)
                / static_cast<double>(nPhi);
            lX        = xModuleEven.X();
            lY        = xModuleEven.Y();
            lZ        = xModuleEven.Z() - dimensions.zmin() - discThickness;
            phiTilt   = xModuleEven.attr<double>("phiTilt");
            thetaTilt = xModuleEven.attr<double>("thetaTilt");
          } else {
            lX        = xModuleOdd.X();
            lY        = xModuleOdd.Y();
            lZ        = xModuleOdd.Z() - dimensions.zmin() - discThickness;
            phiTilt   = xModuleOdd.attr<double>("phiTilt");
            thetaTilt = xModuleOdd.attr<double>("thetaTilt");
          }
          // position module in the x-y plane, smaller end inward
          // and incorporate phi tilt if any
          dd4hep::RotationY lRotation1(M_PI * 0.5);
          dd4hep::RotationX lRotation2(M_PI * 0.5 + phiTilt);
          // align radially
          double componentOffset = integratedCompThickness
              - 0.5 * xModuleProperties.attr<double>("modThickness")
              + 0.5 * xComp.thickness();
          dd4hep::RotationZ lRotation3(atan2(lY, lX));
          // theta tilt, if any -- note the different convention between
          // tklayout and here, thus the subtraction of pi / 2
          dd4hep::RotationY lRotation4(thetaTilt - M_PI * 0.5);
          dd4hep::RotationZ lRotation_PhiPos(phi);
          // position in  disk
          dd4hep::Translation3D lTranslation(
              lX, lY, lZ + componentOffset);
          dd4hep::Transform3D myTrafo(
              lRotation4 * lRotation3 * lRotation2 * lRotation1, lTranslation);
          PlacedVolume placedComponentVolume = discVolume.placeVolume(
              componentVolume, lRotation_PhiPos * myTrafo);
          if (xComp.isSensitive()) {
            placedComponentVolume.addPhysVolID("component", compCounter);
            componentVolume.setSensitiveDetector(sensDet);
            DetElement comp_det(
                disc_det, "comp" + std::to_string(compCounter), compCounter);
            comp_det.setPlacement(placedComponentVolume);

            // create and attach the extension with the shared digitzation
            // module
            Acts::ActsExtension* moduleExtension
                = new Acts::ActsExtension(digiModule);
            comp_det.addExtension<Acts::IActsExtension>(moduleExtension);

            ++compCounter;
          }
        }
        integratedCompThickness += xComp.thickness();
      }
    }
    PlacedVolume placedDiscVolume = envelopeVolume.placeVolume(
        discVolume, dd4hep::Position(0, 0, currentZ));
    placedDiscVolume.addPhysVolID("disc", discCounter);
    ++discCounter;
    Acts::ActsExtension::Config layConfig;
    // the local coordinate systems of modules in dd4hep and acts differ
    // see http://acts.web.cern.ch/ACTS/latest/doc/group__DD4hepPlugins.html
    layConfig.axes = "XZy";  // correct translation of local x axis in dd4hep to
                             // local x axis in acts
    layConfig.isLayer             = true;
    Acts::ActsExtension* detlayer = new Acts::ActsExtension(layConfig);
    disc_det.addExtension<Acts::IActsExtension>(detlayer);
    disc_det.setPlacement(placedDiscVolume);
  }

  // top of the hierarchy
  Volume motherVol = lcdd.pickMotherVolume(worldDetElement);
  dd4hep::Translation3D envelopeTranslation(
      0, 0, dimensions.zmin() + envelopeThickness);
  double envelopeNegRotAngle = 0;
  if (dimensions.reflect()) {
    envelopeNegRotAngle = dd4hep::pi;
  }
  dd4hep::RotationX envelopeNegRotation(envelopeNegRotAngle);
  PlacedVolume                placedEnvelopeVolume = motherVol.placeVolume(
      envelopeVolume, envelopeNegRotation * envelopeTranslation);
  placedEnvelopeVolume.addPhysVolID("system", xmlDet.id());
  worldDetElement.setPlacement(placedEnvelopeVolume);
  return worldDetElement;
}
}  // namespace det

DECLARE_DETELEMENT(TkLayoutEcapTracker, det::createTkLayoutTrackerEndcap)

#include "ACTFW/DD4hepG4/DD4hepToG4Svc.hpp"
#include "ACTFW/DD4hepG4/GeoConstruction.hpp"

DD4hepG4::DD4hepToG4Svc::DD4hepToG4Svc(const DD4hepG4::DD4hepToG4Svc::Config& cfg) :
m_cfg(cfg),
m_geant4Geometry(nullptr)
{}

DD4hepG4::DD4hepToG4Svc::~DD4hepToG4Svc()
{
    //delete m_geant4Geometry;
}

FW::ProcessCode DD4hepG4::DD4hepToG4Svc::initialize()
{
    return FW::ProcessCode::SUCCESS;
}
FW::ProcessCode DD4hepG4::DD4hepToG4Svc::finalize()
{
    return FW::ProcessCode::SUCCESS;
}

FW::ProcessCode DD4hepG4::DD4hepToG4Svc::buildGeant4Geometry()
{
    if(m_cfg.dd4hepService->lcdd()) {
        G4VUserDetectorConstruction* detector(new DD4hepG4::GeoConstruction(*(m_cfg.dd4hepService->lcdd())));
        m_geant4Geometry = detector;
    }
 //   if (!m_geant4Geometry) FW::ProcessCode::ERROR;
    return FW::ProcessCode::SUCCESS;
}

G4VUserDetectorConstruction*  DD4hepG4::DD4hepToG4Svc::geant4Geometry()
{
    if (!m_geant4Geometry) buildGeant4Geometry();
    return m_geant4Geometry;
}


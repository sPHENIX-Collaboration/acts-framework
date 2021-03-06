// This file is part of the Acts project.
//
// Copyright (C) 2019 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ACTFW/Options/Pythia8Options.hpp"

#include <boost/program_options.hpp>

#include "ACTFW/Generators/MultiplicityGenerators.hpp"
#include "ACTFW/Generators/Pythia8ProcessGenerator.hpp"
#include "ACTFW/Generators/VertexGenerators.hpp"

void
FW::Options::addPythia8Options(boost::program_options::options_description& opt)
{
  using namespace boost::program_options;

  opt.add_options()("evg-cmsEnergy",
                    value<double>()->default_value(14000.),
                    "CMS value of the beam in [GeV].")(
      "evg-pdgBeam0",
      value<int>()->default_value(2212),
      "PDG number of beam 0 particles.")("evg-pdgBeam1",
                                         value<int>()->default_value(2212),
                                         "PDG number of beam 1 particles.")(
      "evg-hsProcess",
      value<std::string>()->default_value("HardQCD:all = on"),
      "The process string for the hard scatter event.")(
      "evg-puProcess",
      value<std::string>()->default_value("SoftQCD:all = on"),
      "The process string for the pile-up events.")(
      "evg-pileup",
      value<int>()->default_value(200),
      "Number of instantaneous pile-up events.")(
      "evg-vertex-xy-std",
      value<double>()->default_value(0.015),
      "Transverse vertex standard deviation in [mm].")(
      "evg-vertex-z-std",
      value<double>()->default_value(55.5),
      "Longitudinal vertex standard deviation in [mm].")(
      "evg-vertex-t-std",
      value<double>()->default_value(0.08),
      "Timestamp vertex standard deviation in [ns].")(
      "evg-shuffle", bool_switch(), "Randomnly shuffle the vertex order.");
}

FW::EventGenerator::Config
FW::Options::readPythia8Options(const boost::program_options::variables_map& vm,
                                Acts::Logging::Level lvl)
{
  Pythia8Generator::Config hardCfg;
  hardCfg.pdgBeam0  = vm["evg-pdgBeam0"].template as<int>();
  hardCfg.pdgBeam1  = vm["evg-pdgBeam1"].template as<int>();
  hardCfg.cmsEnergy = vm["evg-cmsEnergy"].template as<double>();
  hardCfg.settings  = {vm["evg-hsProcess"].template as<std::string>()};
  Pythia8Generator::Config pileupCfg;
  pileupCfg.pdgBeam0  = vm["evg-pdgBeam0"].template as<int>();
  pileupCfg.pdgBeam1  = vm["evg-pdgBeam1"].template as<int>();
  pileupCfg.cmsEnergy = vm["evg-cmsEnergy"].template as<double>();
  pileupCfg.settings  = {vm["evg-puProcess"].template as<std::string>()};

  auto vtxStdXY
      = vm["evg-vertex-xy-std"].template as<double>() * Acts::UnitConstants::mm;
  auto vtxStdZ
      = vm["evg-vertex-z-std"].template as<double>() * Acts::UnitConstants::mm;
  auto vtxStdT
      = vm["evg-vertex-t-std"].template as<double>() * Acts::UnitConstants::ns;
  auto mu = static_cast<size_t>(vm["evg-pileup"].template as<int>());

  EventGenerator::Config cfg;
  cfg.generators = {
      {FixedMultiplicityGenerator{1},
       GaussianVertexGenerator{vtxStdXY, vtxStdXY, vtxStdZ, vtxStdT},
       Pythia8Generator::makeFunction(hardCfg, lvl)},
      {PoissonMultiplicityGenerator{mu},
       GaussianVertexGenerator{vtxStdXY, vtxStdXY, vtxStdZ, vtxStdT},
       Pythia8Generator::makeFunction(pileupCfg, lvl)},
  };
  cfg.shuffle = vm["evg-shuffle"].template as<bool>();

  return cfg;
}

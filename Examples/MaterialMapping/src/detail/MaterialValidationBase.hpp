// This file is part of the Acts project.
//
// Copyright (C) 2017-2018 Acts project team
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/program_options.hpp>
#include <memory>
#include "ACTFW/Common/CommonOptions.hpp"
#include "ACTFW/Common/GeometryOptions.hpp"
#include "ACTFW/Common/MaterialOptions.hpp"
#include "ACTFW/Common/OutputOptions.hpp"
#include "ACTFW/Framework/Sequencer.hpp"
#include "ACTFW/Plugins/BField/BFieldOptions.hpp"
#include "ACTFW/Plugins/Root/RootIndexedMaterialReader.hpp"
#include "ACTFW/Plugins/Root/RootMaterialTrackWriter.hpp"
#include "ACTFW/Propagation/PropagationAlgorithm.hpp"
#include "ACTFW/Propagation/PropagationOptions.hpp"
#include "ACTFW/Random/RandomNumbersOptions.hpp"
#include "ACTFW/Random/RandomNumbersSvc.hpp"
#include "ACTFW/Utilities/Paths.hpp"
#include "Acts/Detector/TrackingGeometry.hpp"
#include "Acts/Extrapolator/Navigator.hpp"
#include "Acts/MagneticField/ConstantBField.hpp"
#include "Acts/MagneticField/InterpolatedBFieldMap.hpp"
#include "Acts/MagneticField/SharedBField.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/StraightLineStepper.hpp"

namespace po = boost::program_options;

/// @brief Propagation setup
///
/// @tparam sequencer_t Type of the sequencer of the framework
/// @tparam bfield_t Type of the magnetic field
///
/// @param sequencer The framework sequencer, Propagation algorithm to be added
/// @param bfield The bfield object needed for the Stepper & propagagor
/// @param vm The program options for the log file
/// @param randomNumberSvc The framework random number engine
/// @param tGeometry The TrackingGeometry object
///
/// @return a process code
template <typename sequencer_t, typename bfield_t>
FW::ProcessCode
setupPropagation(sequencer_t&                                  sequencer,
                 bfield_t                                      bfield,
                 po::variables_map&                            vm,
                 std::shared_ptr<FW::RandomNumbersSvc>         randomNumberSvc,
                 std::shared_ptr<const Acts::TrackingGeometry> tGeometry)
{
  // Get the log level
  auto logLevel = FW::Options::readLogLevel<po::variables_map>(vm);

  // Get a Navigator
  Acts::Navigator navigator(tGeometry);

  // Resolve the bfield map template and create the propgator
  using Stepper    = Acts::EigenStepper<bfield_t>;
  using Propagator = Acts::Propagator<Stepper, Acts::Navigator>;
  Stepper    stepper(std::move(bfield));
  Propagator propagator(std::move(stepper), std::move(navigator));

  // Read the propagation config and create the algorithms
  auto pAlgConfig = FW::Options::readPropagationConfig(vm, propagator);
  pAlgConfig.randomNumberSvc = randomNumberSvc;
  auto propagationAlg = std::make_shared<FW::PropagationAlgorithm<Propagator>>(
      pAlgConfig, logLevel);

  // Add the propagation algorithm
  sequencer.appendEventAlgorithms({propagationAlg});

  return FW::ProcessCode::SUCCESS;
}

/// @brief Straight Line Propagation setup
///
/// @tparam sequencer_t Type of the sequencer of the framework
///
/// @param sequencer The framework sequencer, Propagation algorithm to be added
/// @param vm The program options for the log file
/// @param randomNumberSvc The framework random number engine
/// @param tGeometry The TrackingGeometry object
///
/// @return a process code
template <typename sequencer_t>
FW::ProcessCode
setupStraightLinePropagation(
    sequencer_t&                                  sequencer,
    po::variables_map&                            vm,
    std::shared_ptr<FW::RandomNumbersSvc>         randomNumberSvc,
    std::shared_ptr<const Acts::TrackingGeometry> tGeometry)
{
  // Get the log level
  auto logLevel = FW::Options::readLogLevel<po::variables_map>(vm);

  // Get a Navigator
  Acts::Navigator navigator(tGeometry);

  // Straight line stepper
  using SlStepper  = Acts::StraightLineStepper;
  using Propagator = Acts::Propagator<SlStepper, Acts::Navigator>;
  // Make stepper and propagator
  SlStepper  stepper;
  Propagator propagator(std::move(stepper), std::move(navigator));

  // Read the propagation config and create the algorithms
  auto pAlgConfig = FW::Options::readPropagationConfig(vm, propagator);
  pAlgConfig.randomNumberSvc = randomNumberSvc;
  auto propagationAlg = std::make_shared<FW::PropagationAlgorithm<Propagator>>(
      pAlgConfig, logLevel);

  // Add the propagation algorithm
  sequencer.appendEventAlgorithms({propagationAlg});

  return FW::ProcessCode::SUCCESS;
}

/// @brief The material validation example, it runs a propagation
/// and then writes out the material information
///
/// @tparam options_setup_t Type of the options generator
/// @tparam geometry_setup_t Type of the geometry generator
///
/// @param argc the number of argumetns of the call
/// @param atgv the argument list
///
///
/// @param optionsSetup is a struct to generate example specific options
/// @param geometrySteup is a struct to generate the geometry
///
template <typename options_setup_t, typename geometry_setup_t>
int
materialValidationExample(int              argc,
                          char*            argv[],
                          options_setup_t  optionsSetup,
                          geometry_setup_t geometrySetup)
{

  // Create the config object for the sequencer
  FW::Sequencer::Config seqConfig;
  // Now create the sequencer
  FW::Sequencer sequencer(seqConfig);
  // Declare the supported program options.
  po::options_description desc("Allowed options");
  // Add the common options
  FW::Options::addCommonOptions<po::options_description>(desc);
  // Add the geometry options
  FW::Options::addGeometryOptions<po::options_description>(desc);
  // Add the material options
  FW::Options::addMaterialOptions<po::options_description>(desc);
  // Add the bfield options
  FW::Options::addBFieldOptions<po::options_description>(desc);
  // Add the random number options
  FW::Options::addRandomNumbersOptions<po::options_description>(desc);
  // Add the fatras options
  FW::Options::addPropagationOptions<po::options_description>(desc);
  // Add the output options
  FW::Options::addOutputOptions<po::options_description>(desc);

  // Add specific options for this geometry
  optionsSetup(desc);

  // Map to store the given program options
  po::variables_map vm;
  // Get all options from contain line and store it into the map
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);
  // Print help if requested
  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 1;
  }

  // The Log level
  auto nEvents  = FW::Options::readNumberOfEvents<po::variables_map>(vm);
  auto logLevel = FW::Options::readLogLevel<po::variables_map>(vm);

  // Create the random number engine
  auto randomNumberSvcCfg
      = FW::Options::readRandomNumbersConfig<po::variables_map>(vm);
  auto randomNumberSvc
      = std::make_shared<FW::RandomNumbersSvc>(randomNumberSvcCfg);
  // Add it to the sequencer
  sequencer.addServices({randomNumberSvc});

  // Material loading from external source
  auto mDecorator = FW::Options::readMaterialDecorator<po::variables_map>(vm);

  // Set up the geometry
  auto geometry  = geometrySetup(vm, mDecorator);
  auto tGeometry = geometry.first;

  // Create BField service
  auto bField  = FW::Options::readBField<po::variables_map>(vm);
  auto field2D = std::get<std::shared_ptr<InterpolatedBFieldMap2D>>(bField);
  auto field3D = std::get<std::shared_ptr<InterpolatedBFieldMap3D>>(bField);
  auto fieldC  = std::get<std::shared_ptr<Acts::ConstantBField>>(bField);

  if (vm["prop-stepper"].template as<int>() == 0) {
    // Straight line stepper was chosen
    setupStraightLinePropagation(sequencer, vm, randomNumberSvc, tGeometry);
  } else if (field2D) {
    // Define the interpolated b-field: 2D
    using BField = Acts::SharedBField<InterpolatedBFieldMap2D>;
    BField fieldMap(field2D);
    setupPropagation(sequencer, fieldMap, vm, randomNumberSvc, tGeometry);
  } else if (field3D) {
    // Define the interpolated b-field: 3D
    using BField = Acts::SharedBField<InterpolatedBFieldMap3D>;
    BField fieldMap(field3D);
    setupPropagation(sequencer, fieldMap, vm, randomNumberSvc, tGeometry);
  } else {
    // Create the constant  field
    using CField = Acts::ConstantBField;
    CField fieldMap(*std::get<std::shared_ptr<CField>>(bField));
    // Create the constant  field
    setupPropagation(sequencer, fieldMap, vm, randomNumberSvc, tGeometry);
  }

  // ---------------------------------------------------------------------------------
  // Output directory
  std::string outputDir     = vm["output-dir"].template as<std::string>();
  auto        matCollection = vm["prop-material-collection"].as<std::string>();

  if (vm["output-root"].template as<bool>()) {
    // Write the propagation steps as ROOT TTree
    FW::Root::RootMaterialTrackWriter::Config matTrackWriterRootConfig;
    matTrackWriterRootConfig.collection = matCollection;
    matTrackWriterRootConfig.filePath
        = FW::joinPaths(outputDir, matCollection + ".root");
    auto matTrackWriterRoot
        = std::make_shared<FW::Root::RootMaterialTrackWriter>(
            matTrackWriterRootConfig);
    if (sequencer.addWriters({matTrackWriterRoot}) != FW::ProcessCode::SUCCESS)
      return -1;
  }

  // Initiate the run
  sequencer.run(nEvents);
  // Return success code
  return 0;
}

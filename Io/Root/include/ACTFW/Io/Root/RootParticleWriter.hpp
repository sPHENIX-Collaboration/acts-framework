// This file is part of the Acts project.
//
// Copyright (C) 2017-2018 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <mutex>

#include "ACTFW/EventData/SimParticle.hpp"
#include "ACTFW/EventData/SimVertex.hpp"
#include "ACTFW/Framework/WriterT.hpp"

class TFile;
class TTree;

namespace FW {

/// Write out a particles associated to process vertices into a TTree
///
/// Safe to use from multiple writer threads - uses a std::mutex lock.
///
/// Each entry in the TTree corresponds to one particle for optimum writing
/// speed. The event number is part of the written data.
///
/// A common file can be provided for to the writer to attach his TTree,
/// this is done by setting the Config::rootFile pointer to an existing file
///
/// Safe to use from multiple writer threads - uses a std::mutex lock.
class RootParticleWriter final : public WriterT<std::vector<Data::SimVertex>>
{
public:
  struct Config
  {
    std::string collection;              ///< particle collection to write
    std::string filePath;                ///< path of the output file
    std::string fileMode = "RECREATE";   ///< file access mode
    std::string treeName = "particles";  ///< name of the output tree
    TFile*      rootFile = nullptr;      ///< common root file
  };

  /// Constructor
  ///
  /// @param cfg Configuration struct
  /// @param level Message level declaration
  RootParticleWriter(const Config&        cfg,
                     Acts::Logging::Level level = Acts::Logging::INFO);

  /// Virtual destructor
  ~RootParticleWriter() override;

  /// End-of-run hook
  ProcessCode
  endRun() final override;

protected:
  /// @brief Write method called by the base class
  /// @param [in] context is the algorithm context for event information
  /// @param [in] vertices is the process vertex collection for the
  /// particles to be attached
  ProcessCode
  writeT(const AlgorithmContext&             context,
         const std::vector<Data::SimVertex>& vertices) final override;

private:
  Config     m_cfg;         ///< The config class
  std::mutex m_writeMutex;  ///< Mutex used to protect multi-threaded writes
  TFile*     m_outputFile{nullptr};  ///< The output file
  TTree*     m_outputTree{nullptr};  ///< The output tree
  int        m_eventNr{0};           ///< the event number of
  float      m_vx{0.};               ///< Vertex position x
  float      m_vy{0.};               ///< Vertex position y
  float      m_vz{0.};               ///< Vertex position z
  float      m_vt{0.};               ///< Vertex time t
  float      m_px{0.};               ///< Momentum position x
  float      m_py{0.};               ///< Momentum position y
  float      m_pz{0.};               ///< Momentum position z
  float      m_pT{0.};               ///< Momentum position transverse component
  float      m_eta{0.};              ///< Momentum direction eta
  float      m_phi{0.};              ///< Momentum direction phi
  float      m_mass{0.};             ///< Particle mass
  int        m_charge{0};            ///< Particle charge
  int        m_pdgCode{0};           ///< Particle pdg code
  uint64_t   m_barcode{0};           ///< Particle barcode
  uint32_t   m_vertexPrimary{0};     ///< Barcode primary vertex id
  uint32_t   m_vertexSecondary{0};   ///< Barcode secondary vertex id
  uint32_t   m_particle{0};          ///< Barcode particle id
  uint32_t   m_parentParticle{0};    ///< Barcode parent particle id
  uint32_t   m_process{0};           ///< Barcode process id
};

}  // namespace FW

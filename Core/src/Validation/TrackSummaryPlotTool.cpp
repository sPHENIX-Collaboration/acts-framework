// This file is part of the Acts project.
//
// Copyright (C) 2019 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ACTFW/Validation/TrackSummaryPlotTool.hpp"

using Acts::VectorHelpers::eta;
using Acts::VectorHelpers::perp;

FW::TrackSummaryPlotTool::TrackSummaryPlotTool(
    const FW::TrackSummaryPlotTool::Config& cfg,
    Acts::Logging::Level                    level)
  : m_cfg(cfg), m_logger(Acts::getDefaultLogger("TrackSummaryPlotTool", level))
{
}

void
FW::TrackSummaryPlotTool::book(
    TrackSummaryPlotTool::TrackSummaryPlotCache& trackSummaryPlotCache) const
{
  PlotHelpers::Binning bEta = m_cfg.varBinning.at("Eta");
  PlotHelpers::Binning bPt  = m_cfg.varBinning.at("Pt");
  PlotHelpers::Binning bNum = m_cfg.varBinning.at("Num");
  ACTS_DEBUG("Initialize the histograms for track info plots");
  // number of track states versus eta
  trackSummaryPlotCache.nStates_vs_eta = PlotHelpers::bookProf(
      "nStates_vs_eta", "Number of total states vs. #eta", bEta, bNum);
  // number of measurements versus eta
  trackSummaryPlotCache.nMeasurements_vs_eta = PlotHelpers::bookProf(
      "nMeasurements_vs_eta", "Number of measurements vs. #eta", bEta, bNum);
  // number of holes versus eta
  trackSummaryPlotCache.nHoles_vs_eta = PlotHelpers::bookProf(
      "nHoles_vs_eta", "Number of holes vs. #eta", bEta, bNum);
  // number of outliers versus eta
  trackSummaryPlotCache.nOutliers_vs_eta = PlotHelpers::bookProf(
      "nOutliers_vs_eta", "Number of outliers vs. #eta", bEta, bNum);
  // number of track states versus pt
  trackSummaryPlotCache.nStates_vs_pt = PlotHelpers::bookProf(
      "nStates_vs_pT", "Number of total states vs. pT", bPt, bNum);
  // number of measurements versus pt
  trackSummaryPlotCache.nMeasurements_vs_pt = PlotHelpers::bookProf(
      "nMeasurements_vs_pT", "Number of measurements vs. pT", bPt, bNum);
  // number of holes versus pt
  trackSummaryPlotCache.nHoles_vs_pt = PlotHelpers::bookProf(
      "nHoles_vs_pT", "Number of holes vs. pT", bPt, bNum);
  // number of outliers versus pt
  trackSummaryPlotCache.nOutliers_vs_pt = PlotHelpers::bookProf(
      "nOutliers_vs_pT", "Number of outliers vs. pT", bPt, bNum);
}

void
FW::TrackSummaryPlotTool::clear(
    TrackSummaryPlotCache& trackSummaryPlotCache) const
{
  delete trackSummaryPlotCache.nStates_vs_eta;
  delete trackSummaryPlotCache.nMeasurements_vs_eta;
  delete trackSummaryPlotCache.nOutliers_vs_eta;
  delete trackSummaryPlotCache.nHoles_vs_eta;
  delete trackSummaryPlotCache.nStates_vs_pt;
  delete trackSummaryPlotCache.nMeasurements_vs_pt;
  delete trackSummaryPlotCache.nOutliers_vs_pt;
  delete trackSummaryPlotCache.nHoles_vs_pt;
}

void
FW::TrackSummaryPlotTool::write(
    const TrackSummaryPlotTool::TrackSummaryPlotCache& trackSummaryPlotCache)
    const
{
  ACTS_DEBUG("Write the plots to output file.");
  trackSummaryPlotCache.nStates_vs_eta->Write();
  trackSummaryPlotCache.nMeasurements_vs_eta->Write();
  trackSummaryPlotCache.nOutliers_vs_eta->Write();
  trackSummaryPlotCache.nHoles_vs_eta->Write();
  trackSummaryPlotCache.nStates_vs_pt->Write();
  trackSummaryPlotCache.nMeasurements_vs_pt->Write();
  trackSummaryPlotCache.nOutliers_vs_pt->Write();
  trackSummaryPlotCache.nHoles_vs_pt->Write();
}

void
FW::TrackSummaryPlotTool::fill(
    TrackSummaryPlotTool::TrackSummaryPlotCache& trackSummaryPlotCache,
    const Data::SimParticle&                     truthParticle,
    const size_t&                                nStates,
    const size_t&                                nMeasurements,
    const size_t&                                nOutliers,
    const size_t&                                nHoles) const
{
  Acts::Vector3D truthMom = truthParticle.momentum();

  double t_eta = eta(truthMom);
  double t_pT  = perp(truthMom);

  PlotHelpers::fillProf(trackSummaryPlotCache.nStates_vs_eta, t_eta, nStates);
  PlotHelpers::fillProf(
      trackSummaryPlotCache.nMeasurements_vs_eta, t_eta, nMeasurements);
  PlotHelpers::fillProf(
      trackSummaryPlotCache.nOutliers_vs_eta, t_eta, nOutliers);
  PlotHelpers::fillProf(trackSummaryPlotCache.nHoles_vs_eta, t_eta, nHoles);
  PlotHelpers::fillProf(trackSummaryPlotCache.nStates_vs_pt, t_pT, nStates);
  PlotHelpers::fillProf(
      trackSummaryPlotCache.nMeasurements_vs_pt, t_pT, nMeasurements);
  PlotHelpers::fillProf(trackSummaryPlotCache.nOutliers_vs_pt, t_pT, nOutliers);
  PlotHelpers::fillProf(trackSummaryPlotCache.nHoles_vs_pt, t_pT, nHoles);
}

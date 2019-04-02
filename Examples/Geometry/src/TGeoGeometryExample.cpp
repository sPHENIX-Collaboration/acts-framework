// This file is part of the Acts project.
//
// Copyright (C) 2018 Acts project team
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ACTFW/TGeoDetector/TGeoDetector.hpp"
#include "detail/GeometryExampleBase.hpp"

/// @brief main executable
///
/// @param argc The argument count
/// @param argv The argument list
int
main(int argc, char* argv[])
{
  // --------------------------------------------------------------------------------
  TGeoOptions  tgeoOptions;
  TGeoGeometry tgeoGeometry;

  // now process it
  return processGeometry(argc, argv, tgeoOptions, tgeoGeometry);
}
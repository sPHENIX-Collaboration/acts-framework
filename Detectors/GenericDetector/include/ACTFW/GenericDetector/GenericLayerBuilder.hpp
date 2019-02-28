// This file is part of the Acts project.
//
// Copyright (C) 2016-2018 Acts project team
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <iostream>

#include "ACTFW/GenericDetector/GenericDetectorElement.hpp"
#include "ACTFW/GenericDetector/LayerBuilderT.hpp"

namespace FW {
namespace Generic {
  using GenericLayerBuilder = LayerBuilderT<GenericDetectorElement>;

}  // end of namespace Generic
}  // end of namespace FW

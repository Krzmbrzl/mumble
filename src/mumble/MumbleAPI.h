// Copyright 2005-2019 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#ifndef MUMBLE_MUMBLE_API_H_
#define MUMBLE_MUMBLE_API_H_

// In here the MumbleAPI struct is defined
#include "PluginComponents.h"

namespace API {
	/// @param apiVersion The API version to get the function struct for
	/// @returns The struct containing the function pointers to the respective API functions
	///
	/// @throws std::invalid_argument if there is no set of API functions for the requested API version
	MumbleAPI getMumbleAPI(const Version_t& apiVersion);
};

#endif

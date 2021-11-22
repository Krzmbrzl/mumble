// Copyright 2021 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "VolumeAdjustment.h"

#include <cassert>
#include <cmath>

VolumeAdjustment::VolumeAdjustment(float factor, int dbAdjustment) : factor(factor), dbAdjustment(dbAdjustment) {
	assert(dbAdjustment == InvalidDBAdjustment || std::abs(std::pow(2.0f, dbAdjustment / 6.0f) - factor) < 0.01f);
}

VolumeAdjustment VolumeAdjustment::fromFactor(float factor) {
	if (factor > 0) {
		float dB = std::log2(factor) * 6;

		if (std::abs(dB - static_cast< int >(dB)) < 0.1f) {
			// Close-enough
			return VolumeAdjustment(factor, std::round(dB));
		} else {
			return VolumeAdjustment(factor, InvalidDBAdjustment);
		}
	} else {
		return VolumeAdjustment(factor, InvalidDBAdjustment);
	}
}

VolumeAdjustment VolumeAdjustment::fromDBAdjustment(int dbAdjustment) {
	float factor = std::pow(2.0f, dbAdjustment / 6.0f);

	return VolumeAdjustment(factor, dbAdjustment);
}

bool operator==(const VolumeAdjustment &lhs, const VolumeAdjustment &rhs) {
	return lhs.dbAdjustment == rhs.dbAdjustment && std::abs(lhs.factor - rhs.factor) < 0.1f;
}

bool operator!=(const VolumeAdjustment &lhs, const VolumeAdjustment &rhs) {
	return !(lhs == rhs);
}

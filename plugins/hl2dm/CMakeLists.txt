# Copyright The Mumble Developers. All rights reserved.
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file at the root of the
# Mumble source tree or at <https://www.mumble.info/LICENSE>.

set(PLUGIN_RETRACTED ON PARENT_SCOPE)
if(retracted-plugins)
	add_library(hl2dm SHARED "../null_plugin.cpp")

	target_compile_definitions(hl2dm PRIVATE "NULL_DESC=L\"Half-Life 2: Deathmatch (retracted, now using Link)\"")
endif()

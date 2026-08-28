// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file PluginInitializer.h
 * \author jwall64
 * \brief Contains a prototype for a function with C linkage for initializing plugins. This file should only be included by plugins.
 */

#pragma once

#include <Plugin/PluginInitContext.hpp>

extern "C" i32 initPlugin(nbl::PluginInitContext* context);


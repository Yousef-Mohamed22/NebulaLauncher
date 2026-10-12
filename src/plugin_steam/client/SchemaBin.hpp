#pragma once

#include <Core/Result.hpp>
#include <Codec/Json.hpp>
#include <Platform/Allocator.hpp>

nbl::Result parseSchemaBin(nbl::StringView path, nbl::JsonObject* out, nbl::LinearAllocator* allocator);

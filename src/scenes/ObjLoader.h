#pragma once

#include <filesystem>

#include "Shapes.h"

MeshData loadObj(const std::filesystem::path& path);

#pragma once
#include <string>

[[nodiscard]] extern std::vector<std::string> dump_sav(const void* sav, size_t sav_size, bool bitpacked);
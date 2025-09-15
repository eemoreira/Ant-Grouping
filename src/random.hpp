#pragma once

std::mt19937 rng((uint32_t)std::chrono::steady_clock::now().time_since_epoch().count());
int uniform(int l, int r) { return std::uniform_int_distribution<int>(l, r)(rng); }

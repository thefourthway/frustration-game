#include <game/seed_rng.h>
#include <chrono>

namespace g
{
    std::mt19937 make_seeded_rng()
    {
        // Gather multiple entropy sources
        std::random_device rd;
        auto time_ns = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        auto time_s  = std::chrono::system_clock::now().time_since_epoch().count();

        std::seed_seq seed{
            rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd(), // hardware entropy
            (uint32_t)(time_ns & 0xFFFFFFFF),               // low bits of nanoseconds
            (uint32_t)(time_ns >> 32),                       // high bits
            (uint32_t)(time_s  & 0xFFFFFFFF),
            (uint32_t)(time_s  >> 32),
        };

        return std::mt19937(seed);
    }
}
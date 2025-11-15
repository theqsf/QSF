// Copyright (c) 2014-2022, The QSF Project
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors
//    may be used to endorse or promote products derived from this software without
//    specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Parts originally copyright (c) 2012-2013 The Cryptonote developers

#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include <boost/multiprecision/cpp_int.hpp>
#include "crypto/hash.h"

namespace cryptonote
{
    //
    // Correct 128-bit difficulty type (must match difficulty.cpp)
    //
    using difficulty_type =
        boost::multiprecision::number<
            boost::multiprecision::cpp_int_backend<
                128, 128,
                boost::multiprecision::unsigned_magnitude,
                boost::multiprecision::unchecked,
                void
            >
        >;

    //
    // 64-bit difficulty API
    //
    bool check_hash_64(const crypto::hash &hash, uint64_t difficulty);
    uint64_t next_difficulty_64(
        std::vector<uint64_t> timestamps,
        std::vector<uint64_t> cumulative_difficulties,
        size_t target_seconds
    );

    //
    // 128-bit difficulty API
    //
    bool check_hash_128(const crypto::hash &hash, difficulty_type difficulty);

    /**
     * @brief check_hash (dispatcher)
     *
     * REQUIRED inline function.  
     * Many parts of the daemon, miner, and wallet assume this inline exists
     * in difficulty.h — NOT in difficulty.cpp.
     *
     * This was missing before, causing:
     *
     *   undefined reference to cryptonote::check_hash(...)
     *
     * This fixes the entire linking chain.
     */
    inline bool check_hash(const crypto::hash &hash, difficulty_type difficulty)
    {
        // Use fast 64-bit branch if possible
        if (difficulty <= std::numeric_limits<uint64_t>::max())
        {
            return check_hash_64(hash, difficulty.convert_to<uint64_t>());
        }

        // Otherwise use full 128-bit depth
        return check_hash_128(hash, difficulty);
    }

    //
    // Legacy difficulty algorithm
    //
    difficulty_type next_difficulty(
        std::vector<uint64_t> timestamps,
        std::vector<difficulty_type> cumulative_difficulties,
        size_t target_seconds
    );

    //
    // LWMA3 difficulty algorithm
    //
    difficulty_type next_difficulty_lwma(
        std::vector<uint64_t> timestamps,
        std::vector<difficulty_type> cumulative_difficulties,
        size_t target_seconds,
        bool enable_hf18_features = false,
        size_t lwma_window = 0
    );

    //
    // Hex formatting helper
    //
    std::string hex(difficulty_type v);
}


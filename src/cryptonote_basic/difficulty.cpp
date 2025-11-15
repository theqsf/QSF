// Copyright (c) 2014-2022, The QSF Project
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Parts of this file are originally copyright (c) 2012-2013 The Cryptonote developers

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <iostream>
#include <limits>

#include "int-util.h"
#include "crypto/hash.h"
#include "cryptonote_config.h"
#include "difficulty.h"

#undef qsf_DEFAULT_LOG_CATEGORY
#define qsf_DEFAULT_LOG_CATEGORY "difficulty"


namespace cryptonote {

  using std::size_t;
  using std::uint64_t;
  using std::vector;

#if defined(__x86_64__)
  static inline void mul(uint64_t a, uint64_t b, uint64_t &low, uint64_t &high) {
    low = mul128(a, b, &high);
  }
#else
  static inline void mul(uint64_t a, uint64_t b, uint64_t &low, uint64_t &high) {
    // Portable 64x64 -> 128 multiply for non-x86_64 platforms

    uint64_t aLow = a & 0xFFFFFFFF;
    uint64_t aHigh = a >> 32;
    uint64_t bLow = b & 0xFFFFFFFF;
    uint64_t bHigh = b >> 32;

    uint64_t res = aLow * bLow;
    uint64_t lowRes1 = res & 0xFFFFFFFF;
    uint64_t carry = res >> 32;

    res = aHigh * bLow + carry;
    uint64_t highResHigh1 = res >> 32;
    uint64_t highResLow1 = res & 0xFFFFFFFF;

    res = aLow * bHigh;
    uint64_t lowRes2 = res & 0xFFFFFFFF;
    carry = res >> 32;

    res = aHigh * bHigh + carry;
    uint64_t highResHigh2 = res >> 32;
    uint64_t highResLow2 = res & 0xFFFFFFFF;

    uint64_t r = highResLow1 + lowRes2;
    carry = r >> 32;
    low = (r << 32) | lowRes1;
    r = highResHigh1 + highResLow2 + carry;
    uint64_t d3 = r & 0xFFFFFFFF;
    carry = r >> 32;
    r = highResHigh2 + carry;
    high = d3 | (r << 32);
  }
#endif

  static inline bool cadd(uint64_t a, uint64_t b) {
    return a + b < a;
  }

  static inline bool cadc(uint64_t a, uint64_t b, bool c) {
    return a + b < a || (c && a + b == (uint64_t)-1);
  }

  // ==========================================================
  //  64-bit difficulty path
  // ==========================================================

  bool check_hash_64(const crypto::hash &hash, uint64_t difficulty) {
    uint64_t low, high, top, cur;

    // First check the highest word, this will most likely fail for a random hash.
    mul(swap64le(((const uint64_t *) &hash)[3]), difficulty, top, high);
    if (high != 0)
      return false;

    mul(swap64le(((const uint64_t *) &hash)[0]), difficulty, low, cur);
    mul(swap64le(((const uint64_t *) &hash)[1]), difficulty, low, high);
    bool carry = cadd(cur, low);
    cur = high;
    mul(swap64le(((const uint64_t *) &hash)[2]), difficulty, low, high);
    carry = cadc(cur, low, carry);
    carry = cadc(high, top, carry);
    return !carry;
  }

  uint64_t next_difficulty_64(std::vector<uint64_t> timestamps,
                              std::vector<uint64_t> cumulative_difficulties,
                              size_t target_seconds)
  {
    if (timestamps.size() > DIFFICULTY_WINDOW)
    {
      timestamps.resize(DIFFICULTY_WINDOW);
      cumulative_difficulties.resize(DIFFICULTY_WINDOW);
    }

    size_t length = timestamps.size();
    assert(length == cumulative_difficulties.size());
    if (length <= 1)
      return 1;

    std::sort(timestamps.begin(), timestamps.end());

    size_t cut_begin, cut_end;
    if (length <= DIFFICULTY_WINDOW - 2 * DIFFICULTY_CUT) {
      cut_begin = 0;
      cut_end = length;
    } else {
      cut_begin = (length - (DIFFICULTY_WINDOW - 2 * DIFFICULTY_CUT) + 1) / 2;
      cut_end = cut_begin + (DIFFICULTY_WINDOW - 2 * DIFFICULTY_CUT);
    }

    uint64_t time_span = timestamps[cut_end - 1] - timestamps[cut_begin];
    if (time_span == 0)
      time_span = 1;

    uint64_t total_work = cumulative_difficulties[cut_end - 1] -
                          cumulative_difficulties[cut_begin];
    assert(total_work > 0);

    uint64_t low, high;
    mul(total_work, target_seconds, low, high);
    if (high != 0 || low + time_span - 1 < low)
      return 0;

    return (low + time_span - 1) / time_span;
  }

  // ==========================================================
  //  Shared constants & 128-bit difficulty path
  // ==========================================================

  const difficulty_type max64bit(std::numeric_limits<std::uint64_t>::max());
  const boost::multiprecision::uint256_t max128bit(std::numeric_limits<boost::multiprecision::uint128_t>::max());
  const difficulty_type max128bit_difficulty(std::numeric_limits<difficulty_type>::max());
  const boost::multiprecision::uint512_t max256bit(std::numeric_limits<boost::multiprecision::uint256_t>::max());

#define FORCE_FULL_128_BITS

  bool check_hash_128(const crypto::hash &hash, difficulty_type difficulty) {
#ifndef FORCE_FULL_128_BITS
    // Fast check: if difficulty is large and the high word of the hash is non-zero, fail quickly
    if (difficulty >= max64bit && ((const uint64_t *)&hash)[3] > 0)
      return false;
#endif
    // Full 256-bit compare using 512-bit accumulator
    boost::multiprecision::uint512_t hashVal = 0;
#ifdef FORCE_FULL_128_BITS
    for (int i = 0; i < 4; i++) { // highest word is zero
#else
    for (int i = 1; i < 4; i++) { // highest word is zero
#endif
      hashVal <<= 64;
      hashVal |= swap64le(((const uint64_t *)&hash)[3 - i]);
    }
    return hashVal * difficulty <= max256bit;
  }

  // NOTE:
  // check_hash(const crypto::hash&, difficulty_type) is now defined
  // inline in difficulty.h and delegates to check_hash_64 / check_hash_128.
  // Do NOT add a non-inline definition here, or you'll get ODR/link issues.

  // ==========================================================
  //  Legacy cut-based difficulty (pre-fork)
  // ==========================================================

  difficulty_type next_difficulty(std::vector<uint64_t> timestamps,
                                  std::vector<difficulty_type> cumulative_difficulties,
                                  size_t target_seconds)
  {
    if (timestamps.size() > DIFFICULTY_WINDOW)
    {
      timestamps.resize(DIFFICULTY_WINDOW);
      cumulative_difficulties.resize(DIFFICULTY_WINDOW);
    }

    size_t length = timestamps.size();
    assert(length == cumulative_difficulties.size());
    if (length <= 1)
      return 1;

    std::sort(timestamps.begin(), timestamps.end());

    size_t cut_begin, cut_end;
    if (length <= DIFFICULTY_WINDOW - 2 * DIFFICULTY_CUT) {
      cut_begin = 0;
      cut_end = length;
    } else {
      cut_begin = (length - (DIFFICULTY_WINDOW - 2 * DIFFICULTY_CUT) + 1) / 2;
      cut_end = cut_begin + (DIFFICULTY_WINDOW - 2 * DIFFICULTY_CUT);
    }

    uint64_t time_span = timestamps[cut_end - 1] - timestamps[cut_begin];
    if (time_span == 0)
      time_span = 1;

    difficulty_type total_work =
      cumulative_difficulties[cut_end - 1] -
      cumulative_difficulties[cut_begin];

    boost::multiprecision::uint256_t res =
      (boost::multiprecision::uint256_t(total_work) * target_seconds +
       time_span - 1) / time_span;

    if (res > max128bit)
      return 0;

    return res.convert_to<difficulty_type>();
  }

  // ==========================================================
  //  LWMA3 (RandomX) difficulty with diagnostics
  // ==========================================================

  difficulty_type next_difficulty_lwma(std::vector<uint64_t> timestamps,
                                       std::vector<difficulty_type> cumulative_difficulties,
                                       size_t target_seconds,
                                       bool enable_hf18_features,
                                       size_t lwma_window)
  {
    // LWMA3 requires CHRONOLOGICAL order (block order).
    // Verify timestamps are monotonically non-decreasing.
    for (size_t i = 1; i < timestamps.size(); ++i)
    {
      if (timestamps[i] < timestamps[i - 1])
      {
        return 1; // fail safe
      }
    }

    // Window size: Use provided window if given, otherwise use default
    // Before HF18: always 90 (mainnet default)
    // After HF18: 30 for testnet, 90 for mainnet/stagenet (passed from call site)
    const size_t N = (lwma_window > 0) ? lwma_window : ::config::POW_LWMA_WINDOW;

    if (timestamps.size() <= 1 || cumulative_difficulties.size() <= 1)
    {
      return 1;
    }

    // Use at most N last blocks (or fewer if height < N)
    const size_t n = std::min(N, timestamps.size() - 1);
    if (n < 2)
    {
      return 1;
    }

    const size_t start_idx = timestamps.size() - (n + 1);

    int64_t       sum_weighted_solve = 0;
    difficulty_type sum_diff         = 0;

    const int64_t min_solve = 1;
    const int64_t max_solve = (int64_t)target_seconds * 6;

    for (size_t i = 1; i <= n; ++i)
    {
      const size_t idx  = start_idx + i;
      const size_t prev = start_idx + i - 1;

      int64_t solvetime =
        (int64_t)timestamps[idx] - (int64_t)timestamps[prev];

      if (solvetime < min_solve) solvetime = min_solve;
      if (solvetime > max_solve) solvetime = max_solve;

      sum_weighted_solve += solvetime * (int64_t)i;

      const difficulty_type block_diff =
        cumulative_difficulties[idx] - cumulative_difficulties[prev];

      sum_diff += block_diff;
    }

    if (sum_weighted_solve <= 0)
    {
      sum_weighted_solve =
        (int64_t)target_seconds * (int64_t)(n * (n + 1)) / 2;
    }

    boost::multiprecision::uint256_t numerator =
        boost::multiprecision::uint256_t(sum_diff) *
        target_seconds *
        n * (n + 1) / 2;

    boost::multiprecision::uint256_t next =
        numerator / sum_weighted_solve;

    if (next > max128bit)
      return max128bit_difficulty;

    difficulty_type result = next.convert_to<difficulty_type>();

    // ---------------------------------------------
    // HF18 SAFETY CLAMP: Prevent difficulty from stalling
    // ---------------------------------------------
    //
    // If the next difficulty is too high relative to the previous block's
    // difficulty, allow it to decay faster (up to 3× reduction).
    //
    // This protects low-hashrate testnets & small mainnets from stalls
    // when a high-hash miner briefly joins, then leaves.
    //
    if (enable_hf18_features && !cumulative_difficulties.empty() && cumulative_difficulties.size() >= 2)
    {
      difficulty_type prev_diff =
          cumulative_difficulties.back() - cumulative_difficulties[cumulative_difficulties.size() - 2];
      if (prev_diff > 0)
      {
        difficulty_type min_allowed = prev_diff / 3;  // Allow difficulty to fall by 3×
        if (result < min_allowed)
        {
          result = min_allowed;
        }
      }
    }

    return result == 0 ? 1 : result;
  }

  std::string hex(difficulty_type v)
  {
    static const char chars[] = "0123456789abcdef";
    std::string s;
    while (v > 0)
    {
      s.push_back(chars[(v & 0xf).convert_to<unsigned>()]);
      v >>= 4;
    }
    if (s.empty())
      s += "0";

    std::reverse(s.begin(), s.end());
    return "0x" + s;
  }

} // namespace cryptonote


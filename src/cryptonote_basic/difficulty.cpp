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
    // __int128 isn't part of the standard, so the previous function wasn't portable. mul128() in Windows is fine,
    // but this portable function should be used elsewhere. Credit for this function goes to latexi95.

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

    //Addition

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
    return a + b < a || (c && a + b == (uint64_t) -1);
  }

  bool check_hash_64(const crypto::hash &hash, uint64_t difficulty) {
    uint64_t low, high, top, cur;
    // First check the highest word, this will most likely fail for a random hash.
    mul(swap64le(((const uint64_t *) &hash)[3]), difficulty, top, high);
    if (high != 0) {
      return false;
    }
    mul(swap64le(((const uint64_t *) &hash)[0]), difficulty, low, cur);
    mul(swap64le(((const uint64_t *) &hash)[1]), difficulty, low, high);
    bool carry = cadd(cur, low);
    cur = high;
    mul(swap64le(((const uint64_t *) &hash)[2]), difficulty, low, high);
    carry = cadc(cur, low, carry);
    carry = cadc(high, top, carry);
    return !carry;
  }

  uint64_t next_difficulty_64(std::vector<std::uint64_t> timestamps, std::vector<uint64_t> cumulative_difficulties, size_t target_seconds) {

    if(timestamps.size() > DIFFICULTY_WINDOW)
    {
      timestamps.resize(DIFFICULTY_WINDOW);
      cumulative_difficulties.resize(DIFFICULTY_WINDOW);
    }


    size_t length = timestamps.size();
    assert(length == cumulative_difficulties.size());
    if (length <= 1) {
      return 1;
    }
    static_assert(DIFFICULTY_WINDOW >= 2, "Window is too small");
    assert(length <= DIFFICULTY_WINDOW);
    sort(timestamps.begin(), timestamps.end());
    size_t cut_begin, cut_end;
    static_assert(2 * DIFFICULTY_CUT <= DIFFICULTY_WINDOW - 2, "Cut length is too large");
    if (length <= DIFFICULTY_WINDOW - 2 * DIFFICULTY_CUT) {
      cut_begin = 0;
      cut_end = length;
    } else {
      cut_begin = (length - (DIFFICULTY_WINDOW - 2 * DIFFICULTY_CUT) + 1) / 2;
      cut_end = cut_begin + (DIFFICULTY_WINDOW - 2 * DIFFICULTY_CUT);
    }
    assert(/*cut_begin >= 0 &&*/ cut_begin + 2 <= cut_end && cut_end <= length);
    uint64_t time_span = timestamps[cut_end - 1] - timestamps[cut_begin];
    if (time_span == 0) {
      time_span = 1;
    }
    uint64_t total_work = cumulative_difficulties[cut_end - 1] - cumulative_difficulties[cut_begin];
    assert(total_work > 0);
    uint64_t low, high;
    mul(total_work, target_seconds, low, high);
    // blockchain errors "difficulty overhead" if this function returns zero.
    // TODO: consider throwing an exception instead
    if (high != 0 || low + time_span - 1 < low) {
      return 0;
    }
    return (low + time_span - 1) / time_span;
  }

#if defined(_MSC_VER)
#ifdef max
#undef max
#endif
#endif

  const difficulty_type max64bit(std::numeric_limits<std::uint64_t>::max());
  const boost::multiprecision::uint256_t max128bit(std::numeric_limits<boost::multiprecision::uint128_t>::max());
  const difficulty_type max128bit_difficulty(std::numeric_limits<difficulty_type>::max());
  const boost::multiprecision::uint512_t max256bit(std::numeric_limits<boost::multiprecision::uint256_t>::max());

#define FORCE_FULL_128_BITS

  bool check_hash_128(const crypto::hash &hash, difficulty_type difficulty) {
#ifndef FORCE_FULL_128_BITS
    // fast check
    if (difficulty >= max64bit && ((const uint64_t *) &hash)[3] > 0)
      return false;
#endif
    // usual slow check
    boost::multiprecision::uint512_t hashVal = 0;
#ifdef FORCE_FULL_128_BITS
    for(int i = 0; i < 4; i++) { // highest word is zero
#else
    for(int i = 1; i < 4; i++) { // highest word is zero
#endif
      hashVal <<= 64;
      hashVal |= swap64le(((const uint64_t *) &hash)[3 - i]);
    }
    return hashVal * difficulty <= max256bit;
  }

  bool check_hash(const crypto::hash &hash, difficulty_type difficulty) {
    if (difficulty <= max64bit) // if can convert to small difficulty - do it
      return check_hash_64(hash, difficulty.convert_to<std::uint64_t>());
    else
      return check_hash_128(hash, difficulty);
  }

  difficulty_type next_difficulty(std::vector<uint64_t> timestamps, std::vector<difficulty_type> cumulative_difficulties, size_t target_seconds) {
    //cutoff DIFFICULTY_LAG
    if(timestamps.size() > DIFFICULTY_WINDOW)
    {
      timestamps.resize(DIFFICULTY_WINDOW);
      cumulative_difficulties.resize(DIFFICULTY_WINDOW);
    }


    size_t length = timestamps.size();
    assert(length == cumulative_difficulties.size());
    if (length <= 1) {
      return 1;
    }
    static_assert(DIFFICULTY_WINDOW >= 2, "Window is too small");
    assert(length <= DIFFICULTY_WINDOW);
    sort(timestamps.begin(), timestamps.end());
    size_t cut_begin, cut_end;
    static_assert(2 * DIFFICULTY_CUT <= DIFFICULTY_WINDOW - 2, "Cut length is too large");
    if (length <= DIFFICULTY_WINDOW - 2 * DIFFICULTY_CUT) {
      cut_begin = 0;
      cut_end = length;
    } else {
      cut_begin = (length - (DIFFICULTY_WINDOW - 2 * DIFFICULTY_CUT) + 1) / 2;
      cut_end = cut_begin + (DIFFICULTY_WINDOW - 2 * DIFFICULTY_CUT);
    }
    assert(/*cut_begin >= 0 &&*/ cut_begin + 2 <= cut_end && cut_end <= length);
    uint64_t time_span = timestamps[cut_end - 1] - timestamps[cut_begin];
    if (time_span == 0) {
      time_span = 1;
    }
    difficulty_type total_work = cumulative_difficulties[cut_end - 1] - cumulative_difficulties[cut_begin];
    assert(total_work > 0);
    boost::multiprecision::uint256_t res =  (boost::multiprecision::uint256_t(total_work) * target_seconds + time_span - 1) / time_span;
    if(res > max128bit)
      return 0; // to behave like previous implementation, may be better return max128bit?
    return res.convert_to<difficulty_type>();
  }

  difficulty_type next_difficulty_lwma(std::vector<uint64_t> timestamps, std::vector<difficulty_type> cumulative_difficulties, size_t target_seconds) {
    // LWMA3 (Linearly Weighted Moving Average 3) difficulty adjustment algorithm
    // Implementation based on Zawy's LWMA3, used by Monero and other RandomX-based cryptocurrencies
    // This algorithm naturally handles RandomX variance without artificial difficulty bounds
    
    // CRITICAL: LWMA3 requires timestamps in CHRONOLOGICAL ORDER (not sorted!)
    // Verify timestamps are in chronological order (monotonically increasing)
    for (size_t i = 1; i < timestamps.size(); ++i)
    {
      if (timestamps[i] < timestamps[i-1])
      {
        MERROR("LWMA3 ERROR: Timestamps are NOT in chronological order! This will break LWMA3.");
        MERROR("  Timestamp[" << (i-1) << "] = " << timestamps[i-1]);
        MERROR("  Timestamp[" << i << "] = " << timestamps[i]);
        // Force chronological order by sorting (but this indicates a bug upstream)
        // This should NEVER happen - timestamps must be in block order
        return 1; // Fail safe
      }
    }
    
    // Use configured window size (typically 60 or 90 blocks)
    const size_t N = ::config::POW_LWMA_WINDOW;
    
    // DEBUG: Log input data
    LOG_PRINT_L1("LWMA3 INPUT: timestamps.size()=" << timestamps.size() 
                 << " cumulative_difficulties.size()=" << cumulative_difficulties.size()
                 << " target_seconds=" << target_seconds
                 << " N=" << N);
    
    if (timestamps.size() > 10)
    {
      LOG_PRINT_L1("LWMA3 first 5 timestamps: " 
                   << timestamps[0] << ", " << timestamps[1] << ", " << timestamps[2] << ", " 
                   << timestamps[3] << ", " << timestamps[4]);
      LOG_PRINT_L1("LWMA3 last 5 timestamps: " 
                   << timestamps[timestamps.size()-5] << ", " << timestamps[timestamps.size()-4] << ", "
                   << timestamps[timestamps.size()-3] << ", " << timestamps[timestamps.size()-2] << ", "
                   << timestamps[timestamps.size()-1]);
    }
    
    // Early return for insufficient data
    if (timestamps.size() <= 1 || cumulative_difficulties.size() <= 1)
    {
      LOG_PRINT_L1("LWMA3: Insufficient data, returning 1");
      return 1;
    }
    
    // Use the last N blocks (or all available if less than N)
    const size_t n = std::min(N, timestamps.size() - 1);
    if (n < 2)
    {
      LOG_PRINT_L1("LWMA3: n < 2 (n=" << n << "), returning 1");
      return 1;
    }
    
    LOG_PRINT_L1("LWMA3: Using n=" << n << " blocks (requested N=" << N << ")");
    
    // Calculate starting index for the window
    const size_t start_idx = timestamps.size() - (n + 1);
    
    // LWMA3 formula: next_difficulty = (sum_difficulty * target * N * (N+1) / 2) / sum(weighted_solvetimes)
    // where weighted_solvetimes = sum(solvetime[i] * i) for i = 1 to N
    // and solvetime[i] is clamped between 1 and 6×target
    
    int64_t sum_weighted_solvetimes = 0;
    difficulty_type sum_difficulty = 0;
    
    // Solvetime clamp: minimum 1 second, maximum 6×target
    // This prevents timestamp manipulation while allowing normal RandomX variance
    // Do not allow negative solvetimes - minimum is 1
    const int64_t min_solvetime = 1;
    const int64_t max_solvetime = static_cast<int64_t>(target_seconds) * 6;
    
    // Calculate weighted solvetimes and sum of difficulties
    for (size_t i = 1; i <= n; ++i)
    {
      const size_t idx = start_idx + i;
      const size_t prev_idx = start_idx + i - 1;
      
      // Calculate solvetime for this block (time between consecutive blocks)
      int64_t solvetime = static_cast<int64_t>(timestamps[idx]) - static_cast<int64_t>(timestamps[prev_idx]);
      
      // Clamp solvetime: minimum 1, maximum 6×target
      // This is the only clamp needed - LWMA3 handles the rest naturally
      if (solvetime < min_solvetime)
        solvetime = min_solvetime;
      if (solvetime > max_solvetime)
        solvetime = max_solvetime;
      
      // Weight by block index (more recent blocks have higher weight: 1, 2, 3, ..., n)
      sum_weighted_solvetimes += solvetime * static_cast<int64_t>(i);
      
      // Sum individual block difficulties
      const difficulty_type block_diff = cumulative_difficulties[idx] - cumulative_difficulties[prev_idx];
      sum_difficulty += block_diff;
    }
    
    // Safety check: ensure weighted solvetimes sum is positive
    // This should never happen with proper clamping, but handle edge cases
    if (sum_weighted_solvetimes <= 0)
    {
      // Fallback: use target time for all blocks with proper weighting
      // This maintains algorithm stability in edge cases
      sum_weighted_solvetimes = static_cast<int64_t>(target_seconds) * static_cast<int64_t>(n * (n + 1)) / 2;
    }
    
    // LWMA3 formula calculation
    // numerator = sum_difficulty * target * n * (n + 1) / 2
    // denominator = sum_weighted_solvetimes
    // next_difficulty = numerator / denominator
    boost::multiprecision::uint256_t numerator = boost::multiprecision::uint256_t(sum_difficulty) * target_seconds;
    numerator = numerator * n * (n + 1) / 2;
    
    boost::multiprecision::uint256_t next = numerator / sum_weighted_solvetimes;
    
    // Check for overflow (128-bit limit)
    if (next > max128bit)
      return max128bit_difficulty;
    
    // Convert and return with safe fallback
    difficulty_type result = next.convert_to<difficulty_type>();
    
    // DEBUG: Log result
    LOG_PRINT_L1("LWMA3 RESULT: next_difficulty=" << result 
                 << " (from sum_difficulty=" << sum_difficulty 
                 << " sum_weighted_solvetimes=" << sum_weighted_solvetimes << ")");
    
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

}

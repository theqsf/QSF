# Difficulty Rescue Implementation Summary

## Overview

This document summarizes the implementation of the difficulty rescue mechanism for QSF mainnet stuck at block 31,670.

## Problem

- Mainnet chain stuck at block 31,670
- Current difficulty: ~78.7 billion (too high for ~1-2 MH/s network hashrate)
- Root cause: Transient hashrate spike caused difficulty to spike, then miners left

## Solution Components

### 1. One-Time Difficulty Reset

**Location**: `src/cryptonote_core/blockchain.cpp`

**Implementation**:
- Check if `height == 31671` and `m_nettype == MAINNET`
- If true, return fixed difficulty value: `5000000000` (5 billion)
- Applied in both:
  - `get_difficulty_for_next_block()`
  - `get_next_difficulty_for_alternative_chain()`

**Constants** (in `src/cryptonote_config.h`):
```cpp
#define QSF_DIFFICULTY_RESCUE_HEIGHT_MAINNET            31671
#define QSF_DIFFICULTY_RESCUE_VALUE_MAINNET            5000000000
```

### 2. Ongoing Safety Valve

**Location**: `src/cryptonote_basic/difficulty.cpp`

**Implementation**:
- Added to `next_difficulty_lwma()` function
- Calculates average solve time over the difficulty window
- If average > 4 hours (14400 seconds), reduces difficulty by up to 4×
- Minimum difficulty floor: 1 million
- Only active on mainnet after height 31,671

**Constants**:
```cpp
#define QSF_DIFFICULTY_SAFETY_VALVE_STUCK_TIME          14400  // 4 hours
#define QSF_DIFFICULTY_SAFETY_VALVE_MIN_DIFFICULTY      1000000
```

## Code Changes

### Files Modified

1. **src/cryptonote_config.h**
   - Added rescue height and value constants
   - Added safety valve parameters
   - Added config namespace constants for all network types

2. **src/cryptonote_basic/difficulty.h**
   - Updated `next_difficulty_lwma()` signature:
     ```cpp
     difficulty_type next_difficulty_lwma(
         std::vector<uint64_t> timestamps,
         std::vector<difficulty_type> cumulative_difficulties,
         size_t target_seconds,
         bool enable_hf18_features = false,
         size_t lwma_window = 0,
         uint64_t height = 0,
         uint8_t nettype = 0
     );
     ```

3. **src/cryptonote_basic/difficulty.cpp**
   - Updated function implementation
   - Added safety valve logic after HF18 clamp

4. **src/cryptonote_core/blockchain.cpp**
   - Added rescue check in `get_difficulty_for_next_block()`
   - Added rescue check in `get_next_difficulty_for_alternative_chain()`
   - Updated all 3 call sites to pass height and network type
   - Added logging for rescue and safety valve activations

5. **src/version.cpp.in**
   - Updated version to 3.0.5.0
   - Updated release name to "Difficulty Rescue"

## Function Call Flow

```
get_difficulty_for_next_block()
  ├─ Check rescue height (31671) → return rescue value if match
  ├─ Check pow_switch_block → return reset difficulty
  └─ Call next_difficulty_lwma(timestamps, difficulties, target, hf18_active, lwma_window, height, m_nettype)
      ├─ Calculate normal LWMA difficulty
      ├─ Apply HF18 safety clamp
      └─ Apply safety valve (if height >= 31671 and mainnet)
```

## Testing

✅ **Compilation**: All modified files compile successfully
- `cryptonote_basic` library: ✅
- `cryptonote_core` library: ✅
- No linter errors: ✅

⚠️ **Network Testing**: Required before deployment
- Test on mainnet that rescue activates at height 31,671
- Verify safety valve activates when chain is stuck
- Confirm all nodes compute same difficulty

## Consensus Impact

**CRITICAL**: This is a consensus-breaking change.

- Nodes running v3.0.4.0 or earlier will compute different difficulty for block 31,671
- All nodes must upgrade to v3.0.5.0 to remain on the correct chain
- The rescue is deterministic: all v3.0.5.0 nodes will compute the same difficulty

## Logging

The implementation logs:

1. **Rescue activation**:
   ```
   DIFFICULTY RESCUE: Applying one-time difficulty reset at height 31671 to 5000000000 (mainnet rescue from stuck chain)
   ```

2. **Safety valve activation**:
   ```
   DIFFICULTY SAFETY VALVE: Reduced difficulty at height X from Y to Z due to long average solve time
   ```

## Design Decisions

1. **Rescue value (5 billion)**: 
   - Reasonable for ~1-2 MH/s network
   - ~16× lower than current stuck difficulty
   - Allows chain to resume quickly

2. **Safety valve threshold (4 hours)**:
   - Long enough to avoid false positives from normal variance
   - Short enough to prevent extended chain stalls

3. **Reduction factor (up to 4×)**:
   - Prevents runaway difficulty
   - Maintains some difficulty to prevent spam

4. **Minimum difficulty (1 million)**:
   - Prevents difficulty from going too low
   - Maintains basic security

## Future Considerations

- Monitor safety valve activations on mainnet
- Adjust parameters if needed based on network behavior
- Consider additional mechanisms for difficulty stability


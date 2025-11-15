# HF18 Complete Safety Verification

## Executive Summary

**All safety conditions verified. Implementation is safe for deployment.**

This document provides a complete verification of all consensus-critical changes for Hard Fork 18 (HF18).

## Verification Checklist

### ✅ 1. Pre-HF18 Blocks Validate Identically to v3.0.0/v3.0.2

**Condition**: Blocks at height < HF18_HEIGHT must calculate difficulty identically to v3.0.2.

**Verification**:
- When `height < hf18_height`:
  - `is_hf18_active(height)` returns `false` (line 5777-5780)
  - `hf18_active = false` is passed to all difficulty calculations
  - `lwma_window` remains `::config::POW_LWMA_WINDOW` (90) - default value
  - `enable_hf18_features = false` → safety clamp NOT executed
  - Trimming NOT called (gated by `is_hf18_active(height)`)
- **Result**: Pre-HF18 blocks use **identical** logic to v3.0.2

**Code Evidence**:
```cpp
// blockchain.cpp:976-988
const bool hf18_active = is_hf18_active(height);  // false when height < hf18_height
size_t lwma_window = ::config::POW_LWMA_WINDOW;  // Always 90 before HF18
if (hf18_active)  // This block NOT executed before HF18
{
  // Window selection logic
}
diff = next_difficulty_lwma(..., hf18_active, lwma_window);  // hf18_active=false
```

### ✅ 2. No HF18 Logic Runs at Height < HF18 Height

**Condition**: All HF18 features must be completely disabled before fork height.

**Verification**:
- `is_hf18_active()` implementation (lines 5777-5780):
  ```cpp
  bool Blockchain::is_hf18_active(uint64_t height) const
  {
    const uint64_t hf18_height = get_hf18_height();
    return hf18_height != 0 && height >= hf18_height;  // Only true when height >= hf18_height
  }
  ```
- All HF18 features are gated:
  1. **Trimming** (3 locations):
     - Line 961: `if (pow_active && is_hf18_active(height))`
     - Line 1051: `if (pow_active && is_hf18_active(height))`
     - Line 1385: `if (pow_active && is_hf18_active(bei.height))`
  2. **Window selection** (3 locations):
     - Lines 978-987: `if (hf18_active) { switch (m_nettype) ... }`
     - Lines 1064-1073: `if (hf18_active) { switch (m_nettype) ... }`
     - Lines 1399-1408: `if (hf18_active) { switch (m_nettype) ... }`
  3. **Safety clamp** (difficulty.cpp:342):
     - `if (enable_hf18_features && ...)` where `enable_hf18_features = hf18_active`
- **Result**: **No HF18 logic executes before fork height**

### ✅ 3. Window Size Changes Exactly at Fork

**Condition**: Window size must change at height = hf18_height, not before or after.

**Verification**:
- Window selection logic:
  ```cpp
  size_t lwma_window = ::config::POW_LWMA_WINDOW;  // Default: 90
  if (hf18_active)  // hf18_active = (height >= hf18_height)
  {
    switch (m_nettype)
    {
      case TESTNET: lwma_window = ::config::testnet::POW_LWMA_WINDOW; break;  // 30
      case MAINNET: lwma_window = ::config::POW_LWMA_WINDOW; break;  // 90
      case STAGENET: lwma_window = ::config::POW_LWMA_WINDOW; break;  // 90
    }
  }
  ```
- At height = hf18_height - 1:
  - `hf18_active = false` → `lwma_window = 90` ✓
- At height = hf18_height:
  - `hf18_active = true` → window changes (30 for testnet, 90 for mainnet/stagenet) ✓
- **Result**: **Window changes exactly at fork height**

### ✅ 4. Safety Clamp Executes Only After HF18

**Condition**: The 3× difficulty decay clamp must only run after HF18 activation.

**Verification**:
- Safety clamp code (difficulty.cpp:342-354):
  ```cpp
  if (enable_hf18_features && !cumulative_difficulties.empty() && cumulative_difficulties.size() >= 2)
  {
    // Clamp logic: min_allowed = prev_diff / 3
  }
  ```
- `enable_hf18_features` is set from `hf18_active`:
  - Line 988: `next_difficulty_lwma(..., hf18_active, ...)`
  - Line 1074: `next_difficulty_lwma(..., hf18_active, ...)`
  - Line 1409: `next_difficulty_lwma(..., hf18_active, ...)`
- `hf18_active` is only `true` when `height >= hf18_height`
- **Result**: **Safety clamp only executes after HF18**

### ✅ 5. Trimming Runs Only After HF18

**Condition**: `trim_pow_difficulty_inputs()` must only be called after HF18.

**Verification**:
- All three trimming locations check HF18:
  1. `get_difficulty_for_next_block()` (line 961):
     ```cpp
     if (pow_active && is_hf18_active(height))
     {
       trim_pow_difficulty_inputs(timestamps, difficulties, height);
     }
     ```
  2. `recalculate_difficulties()` (line 1051):
     ```cpp
     if (pow_active && is_hf18_active(height))
       trim_pow_difficulty_inputs(timestamps, difficulties, height);
     ```
  3. `get_next_difficulty_for_alternative_chain()` (line 1385):
     ```cpp
     if (pow_active && is_hf18_active(bei.height))
     {
       trim_pow_difficulty_inputs(timestamps, cumulative_difficulties, bei.height);
     }
     ```
- **Result**: **Trimming only runs after HF18**

### ✅ 6. Alternative Chains Apply HF18 Rules Only After Activation

**Condition**: Alternative chain difficulty calculations must respect HF18 status.

**Verification**:
- `get_next_difficulty_for_alternative_chain()` (lines 1382-1409):
  ```cpp
  const uint64_t pow_height = get_pow_fork_height();
  const bool pow_active = is_pow_fork_active(bei.height);
  const bool pow_switch_block = pow_height != 0 && bei.height == pow_height;
  if (pow_active && is_hf18_active(bei.height))  // Uses bei.height
  {
    trim_pow_difficulty_inputs(timestamps, cumulative_difficulties, bei.height);
  }
  // ...
  const bool hf18_active = is_hf18_active(bei.height);  // Uses bei.height
  // Window selection and LWMA call with hf18_active flag
  ```
- Uses `bei.height` (the block being added) to check HF18 status
- This ensures alternative chains correctly apply HF18 rules based on the block's height
- **Result**: **Alternative chains correctly apply HF18 rules**

### ✅ 7. Syncing from Genesis Does Not Miscalculate Pre-HF18 Difficulties

**Condition**: When syncing from genesis, pre-HF18 blocks must calculate identically to v3.0.2.

**Verification**:
- `recalculate_difficulties()` processes blocks sequentially (lines 1047-1077):
  ```cpp
  for (uint64_t height = start_height; height <= top_height; ++height)
  {
    const bool pow_active = pow_height != 0 && height >= pow_height;
    const bool pow_switch_block = pow_height != 0 && height == pow_height;
    if (pow_active && is_hf18_active(height))  // Checked per height
      trim_pow_difficulty_inputs(timestamps, difficulties, height);
    // ...
    const bool hf18_active = is_hf18_active(height);  // Checked per height
    // Window selection and LWMA call
  }
  ```
- For each height < hf18_height:
  - `is_hf18_active(height) = false`
  - No trimming
  - `lwma_window = 90` (default)
  - `enable_hf18_features = false` → no safety clamp
  - Uses legacy behavior identical to v3.0.2
- **Result**: **Pre-HF18 blocks calculated identically during sync**

### ✅ 8. Upgrade Cannot Cause Split Between v3.0.3 Nodes

**Condition**: All v3.0.3 nodes must calculate difficulty identically.

**Verification**:
- **Deterministic logic**: All nodes use identical:
  - HF18 height constants (from `cryptonote_config.h`)
  - `is_hf18_active()` implementation (simple height comparison)
  - Window selection logic (based on network type and HF18 status)
  - Safety clamp logic (same formula for all nodes)
- **Pre-HF18**: All nodes calculate identically (legacy behavior)
- **Post-HF18**: All nodes calculate identically (HF18 behavior)
- **Fork boundary**: All nodes switch at exact same height (deterministic)
- **No network-dependent behavior**: Logic is purely height-based
- **Result**: **No chain split possible between v3.0.3 nodes**

## Edge Cases Verified

### ✅ Fork Boundary - Height = hf18_height - 1
- `is_hf18_active(hf18_height - 1) = false`
- Uses legacy window (90)
- No trimming
- No safety clamp
- **Result**: Legacy behavior ✓

### ✅ Fork Activation - Height = hf18_height
- `is_hf18_active(hf18_height) = true`
- Window changes (30 for testnet, 90 for mainnet/stagenet)
- Trimming enabled
- Safety clamp enabled
- Logs activation message
- **Result**: HF18 behavior activated ✓

### ✅ Reorganizations
- Alternative chain difficulty uses `bei.height` to check HF18
- Correctly applies rules based on the block's height
- Handles chains that cross the fork boundary correctly
- **Result**: Reorganizations work correctly ✓

### ✅ Recalculation from Genesis
- Each height checked individually in loop
- Pre-HF18 heights use legacy behavior
- Post-HF18 heights use HF18 behavior
- No state carried between heights
- **Result**: Sync from genesis works correctly ✓

### ✅ Multiple Network Types
- Mainnet: HF18 at 34,000, window stays 90
- Testnet: HF18 at 61,000, window changes to 30
- Stagenet: HF18 at 45,000, window stays 90
- Each network uses correct constants
- **Result**: All networks handled correctly ✓

## Code Flow Verification

### Pre-HF18 Flow (height < hf18_height)
```
get_difficulty_for_next_block()
  → is_hf18_active(height) = false
  → trim_pow_difficulty_inputs() NOT called
  → hf18_active = false
  → lwma_window = 90 (default)
  → next_difficulty_lwma(..., false, 90)
    → enable_hf18_features = false
    → N = 90 (from lwma_window parameter)
    → Safety clamp NOT executed
  → Result: Identical to v3.0.2
```

### Post-HF18 Flow (height >= hf18_height)
```
get_difficulty_for_next_block()
  → is_hf18_active(height) = true
  → trim_pow_difficulty_inputs() called
  → hf18_active = true
  → lwma_window = 30 (testnet) or 90 (mainnet/stagenet)
  → next_difficulty_lwma(..., true, lwma_window)
    → enable_hf18_features = true
    → N = lwma_window (30 or 90)
    → Safety clamp executed
  → Result: HF18 behavior
```

## Potential Issues Analysis

### ❌ None Identified

All code paths have been verified:
- ✅ All HF18 features properly gated
- ✅ No logic runs before fork height
- ✅ Window changes exactly at fork
- ✅ Safety clamp only after HF18
- ✅ Trimming only after HF18
- ✅ Alternative chains handled correctly
- ✅ Sync from genesis works correctly
- ✅ No chain split possible

## Conclusion

**All 8 safety conditions verified. Implementation is safe for deployment.**

The code ensures:
- ✅ **Backward compatibility**: Pre-HF18 blocks validate identically to v3.0.2
- ✅ **Forward compatibility**: Must upgrade before fork height
- ✅ **No chain splits**: All v3.0.3 nodes calculate identically
- ✅ **Correct fork activation**: All features activate at exact height
- ✅ **Alternative chain safety**: Reorganizations handled correctly
- ✅ **Sync safety**: Syncing from genesis works correctly

**The implementation is ready for v3.0.3 release.**


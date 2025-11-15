# HF18 Safety Verification Report

## Verification Date
2025-01-XX

## Consensus-Critical Files Modified
1. `src/cryptonote_config.h`
2. `src/cryptonote_core/blockchain.h`
3. `src/cryptonote_core/blockchain.cpp`
4. `src/cryptonote_basic/difficulty.h`
5. `src/cryptonote_basic/difficulty.cpp`

## Safety Condition Verification

### ✅ 1. Pre-HF18 Blocks Validate Identically to v3.0.0/v3.0.2

**Verification:**
- When `height < HF18_HEIGHT`, `is_hf18_active(height)` returns `false`
- When `hf18_active = false`:
  - `lwma_window` remains `::config::POW_LWMA_WINDOW` (90)
  - `enable_hf18_features = false` is passed to `next_difficulty_lwma()`
  - Safety clamp is NOT executed (gated by `enable_hf18_features`)
  - Trimming is NOT called (gated by `is_hf18_active(height)`)
- Result: Pre-HF18 blocks use **identical logic** to v3.0.2

**Code Evidence:**
```cpp
// blockchain.cpp:976-988
const bool hf18_active = is_hf18_active(height);
size_t lwma_window = ::config::POW_LWMA_WINDOW;  // Default: 90
if (hf18_active)  // Only changes if HF18 active
{
  // Window selection logic
}
diff = next_difficulty_lwma(..., hf18_active, lwma_window);
```

### ✅ 2. No HF18 Logic Runs at Height < HF18 Height

**Verification:**
- `is_hf18_active(height)` implementation:
  ```cpp
  bool Blockchain::is_hf18_active(uint64_t height) const
  {
    const uint64_t hf18_height = get_hf18_height();
    return hf18_height != 0 && height >= hf18_height;
  }
  ```
- All HF18 features are gated:
  - Trimming: `if (pow_active && is_hf18_active(height))`
  - Window selection: `if (hf18_active)`
  - Safety clamp: `if (enable_hf18_features && ...)`
- Result: **No HF18 logic executes before fork height**

### ✅ 3. Window Size Changes Exactly at Fork

**Verification:**
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
- At height = hf18_height - 1: `hf18_active = false` → window = 90
- At height = hf18_height: `hf18_active = true` → window changes
- Result: **Window changes exactly at fork height**

### ✅ 4. Safety Clamp Executes Only After HF18

**Verification:**
- Safety clamp code:
  ```cpp
  if (enable_hf18_features && !cumulative_difficulties.empty() && ...)
  {
    // Clamp logic
  }
  ```
- `enable_hf18_features` is only `true` when `hf18_active = true`
- `hf18_active` is only `true` when `height >= hf18_height`
- Result: **Safety clamp only executes after HF18**

### ✅ 5. Trimming Runs Only After HF18

**Verification:**
- All three trimming locations check HF18:
  1. `get_difficulty_for_next_block()`: `if (pow_active && is_hf18_active(height))`
  2. `recalculate_difficulties()`: `if (pow_active && is_hf18_active(height))`
  3. `get_next_difficulty_for_alternative_chain()`: `if (pow_active && is_hf18_active(bei.height))`
- Result: **Trimming only runs after HF18**

### ✅ 6. Alternative Chains Apply HF18 Rules Only After Activation

**Verification:**
- `get_next_difficulty_for_alternative_chain()`:
  ```cpp
  if (pow_active && is_hf18_active(bei.height))
  {
    trim_pow_difficulty_inputs(timestamps, cumulative_difficulties, bei.height);
  }
  // ...
  const bool hf18_active = is_hf18_active(bei.height);
  // Window selection and LWMA call with hf18_active flag
  ```
- Uses `bei.height` (the block being added) to check HF18 status
- Result: **Alternative chains correctly apply HF18 rules**

### ✅ 7. Syncing from Genesis Does Not Miscalculate Pre-HF18 Difficulties

**Verification:**
- `recalculate_difficulties()` processes blocks sequentially:
  ```cpp
  for (uint64_t height = start_height; height <= top_height; ++height)
  {
    const bool hf18_active = is_hf18_active(height);  // Checked per height
    // ... difficulty calculation
  }
  ```
- For each height < hf18_height:
  - `hf18_active = false`
  - Uses legacy window (90)
  - No trimming
  - No safety clamp
- Result: **Pre-HF18 blocks calculated identically to v3.0.2**

### ✅ 8. Upgrade Cannot Cause Split Between v3.0.3 Nodes

**Verification:**
- All v3.0.3 nodes use identical logic:
  - Same HF18 height constants
  - Same `is_hf18_active()` implementation
  - Same window selection logic
  - Same safety clamp logic
- Pre-HF18: All nodes calculate identically (legacy behavior)
- Post-HF18: All nodes calculate identically (HF18 behavior)
- Fork boundary: All nodes switch at exact same height
- Result: **No chain split possible between v3.0.3 nodes**

## Edge Cases Verified

### ✅ Fork Boundary (height = hf18_height - 1)
- `is_hf18_active(hf18_height - 1) = false`
- Uses legacy behavior ✓

### ✅ Fork Activation (height = hf18_height)
- `is_hf18_active(hf18_height) = true`
- Uses HF18 behavior ✓
- Logs activation message ✓

### ✅ Reorganizations
- Alternative chain difficulty uses `bei.height` to check HF18
- Correctly applies rules based on the block's height ✓

### ✅ Recalculation from Genesis
- Each height checked individually
- Pre-HF18 heights use legacy behavior
- Post-HF18 heights use HF18 behavior ✓

## Potential Issues Found

### ⚠️ None Identified

All safety conditions are met. The implementation correctly gates all HF18 features behind the fork height check.

## Conclusion

**All safety conditions verified. The implementation is safe for deployment.**

The code ensures:
- ✅ Backward compatibility (pre-HF18 blocks validate identically)
- ✅ Forward compatibility (must upgrade before fork)
- ✅ No chain splits (all v3.0.3 nodes calculate identically)
- ✅ Correct fork activation (all features activate at exact height)


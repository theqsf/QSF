# QSF v3.0.3 - Difficulty Stability (HF18)

**Release Name**: Difficulty Stability  
**Release Date**: TBD  
**Hard Fork**: HF18

## Overview

QSF v3.0.3 introduces **Hard Fork 18 (HF18)**, a critical consensus upgrade that fixes difficulty algorithm issues and prevents chain stalls. This release implements three major improvements to the LWMA3 difficulty algorithm:

1. **Safety Clamp**: Prevents difficulty from dropping too fast (max 3× per block)
2. **Testnet Window Reduction**: Faster recovery with 30-block window (vs 90)
3. **Improved Trimming**: Better handling of fork boundaries

## Hard Fork Information

### Fork Heights
- **Mainnet**: Block 34,000
- **Testnet**: Block 61,000
- **Stagenet**: Block 45,000

### What Changes at HF18

**Before HF18:**
- All networks use 90-block LWMA window
- No difficulty decay safety clamp
- Legacy trimming behavior

**After HF18:**
- **Testnet**: 30-block window + safety clamp + improved trimming
- **Mainnet/Stagenet**: 90-block window + safety clamp + improved trimming

## Key Features

### 1. Difficulty Decay Safety Clamp

**Problem**: When a high-hashrate miner joins and then leaves, difficulty can stay too high, causing chain stalls (90+ minute block times).

**Solution**: After HF18, difficulty can decay by up to 3× per block, ensuring the chain never freezes.

**Impact**:
- Prevents chain stalls on low-hashrate networks
- Maintains LWMA3 mathematical correctness
- Allows difficulty to recover quickly when hash rate drops

### 2. Testnet Window Reduction

**Problem**: Testnet with 90-block window was too slow to recover from hash rate changes.

**Solution**: Testnet uses 30-block window after HF18 for faster adjustments.

**Impact**:
- Testnet recovers 3× faster from difficulty spikes
- Better for testing and development
- Mainnet keeps full 90-block stability

### 3. Improved Trimming Logic

**Problem**: Pre-fork blocks could contaminate difficulty calculations.

**Solution**: Better anchoring and window limiting ensures only post-fork blocks are used.

**Impact**:
- Prevents difficulty calculation errors
- Ensures consistent behavior across all networks
- Fixes edge cases around fork boundaries

## Upgrade Instructions

### Critical: Upgrade Before Fork Height

**You must upgrade before the fork height for your network:**

- **Mainnet users**: Upgrade before block 34,000
- **Testnet users**: Upgrade before block 61,000
- **Stagenet users**: Upgrade before block 45,000

### Step-by-Step Upgrade

1. **Download v3.0.3**:
   ```bash
   git clone https://github.com/quantumsafefoundation/qsf.git
   cd qsf
   git checkout release/v3.0.3
   ```

2. **Build** (see README.md for full instructions):
   ```bash
   mkdir build && cd build
   cmake ..
   make -j$(nproc)
   ```

3. **Stop your daemon**:
   ```bash
   # If running as systemd service
   sudo systemctl stop qsf-daemon
   
   # Or if running manually
   # Press Ctrl+C or kill the process
   ```

4. **Replace binaries**:
   ```bash
   sudo cp build/bin/qsfd /usr/local/bin/
   # Or wherever your binaries are installed
   ```

5. **Start daemon**:
   ```bash
   sudo systemctl start qsf-daemon
   # Or run manually
   ```

6. **Verify**:
   - Check logs for "Hard Fork 18 scheduled at height XXXX"
   - At fork height, you'll see "HF18 activated: new difficulty rules enabled"

## What to Expect

### At Startup
You'll see a message like:
```
Hard Fork 18 scheduled at height 34000 (Difficulty Stability Upgrade)
```

### At Fork Activation
When the fork activates, you'll see:
```
HF18 activated: new difficulty rules enabled
```

### After Fork
- **Testnet**: Faster difficulty adjustments (30-block window)
- **All networks**: Difficulty can decay up to 3× per block if needed
- **Smoother operation**: No more chain stalls from difficulty spikes

## Testing

This release has been extensively tested on:
- Testnet with various hash rate scenarios
- Mainnet compatibility (pre-HF18 blocks remain valid)
- Difficulty calculation correctness
- Fork activation and logging

## Bug Fixes

- Fixed difficulty explosion on testnet (10-700× increases)
- Corrected cumulative difficulty handling around fork boundaries
- Improved trimming to prevent pre-fork block contamination
- Removed debug logging from production builds

## Technical Details

### Files Changed
- `src/cryptonote_config.h` - HF18 height constants
- `src/cryptonote_core/blockchain.*` - HF18 implementation
- `src/cryptonote_basic/difficulty.*` - Algorithm improvements
- `src/version.cpp.in` - Version bump to 3.0.3.0

### API Changes
- `next_difficulty_lwma()` now accepts `enable_hf18_features` and `lwma_window` parameters
- New methods: `get_hf18_height()`, `is_hf18_active()`

## Compatibility

### Backward Compatibility
- ✅ All blocks before HF18 remain valid
- ✅ Pre-HF18 difficulty calculations unchanged
- ✅ No wallet changes required

### Forward Compatibility
- ⚠️ Nodes running v3.0.2 or earlier will not accept blocks after HF18
- ⚠️ Must upgrade before fork height

## Support

- **GitHub Issues**: https://github.com/quantumsafefoundation/qsf/issues
- **Documentation**: See CHANGELOG.md for detailed changes
- **Community**: Join our Discord/Telegram for support

## Acknowledgments

Thanks to the community for reporting difficulty issues and helping test this release.

---

**Important**: This is a **consensus-critical** upgrade. All nodes must upgrade before the fork height to avoid being left behind.


# QSF v3.0.3 - Difficulty Stability (HF18)

## Release Date
TBD

## Hard Fork 18 - Difficulty Stability Upgrade

This release introduces **Hard Fork 18 (HF18)**, a consensus upgrade that implements critical difficulty algorithm improvements to prevent chain stalls and ensure stable block production.

### Fork Heights
- **Mainnet**: Height 34,000
- **Testnet**: Height 61,000  
- **Stagenet**: Height 45,000

## Major Changes

### Difficulty Algorithm Improvements

#### 1. LWMA3 Safety Clamp (HF18)
- **Prevents chain stalls**: Difficulty can now decay by up to 3× per block when hash rate drops
- **Protects small networks**: Prevents difficulty from staying too high after a large miner leaves
- **Maintains LWMA3 correctness**: Algorithm remains mathematically sound while adding safety bounds

#### 2. Testnet Window Reduction (HF18)
- **Faster recovery**: Testnet uses 30-block window (reduced from 90) for quicker difficulty adjustments
- **Better for testing**: Allows testnet to recover faster from hash rate changes
- **Mainnet unchanged**: Mainnet and stagenet continue using 90-block window for stability

#### 3. Improved Trimming Logic (HF18)
- **Post-fork anchoring**: Difficulty calculation properly anchors at fork boundary
- **Window limiting**: Ensures only the last 91 blocks (POW_LWMA_WINDOW + 1) are used
- **Prevents pre-fork contamination**: Blocks before the RandomX fork are correctly excluded

### Technical Details

#### Before HF18
- All networks use 90-block LWMA window
- No difficulty decay safety clamp
- Legacy trimming behavior

#### After HF18
- **Testnet**: 30-block window + safety clamp + improved trimming
- **Mainnet/Stagenet**: 90-block window + safety clamp + improved trimming

### Code Changes

#### Files Modified
- `src/cryptonote_config.h` - Added HF18 height constants
- `src/cryptonote_core/blockchain.h` - Added `get_hf18_height()` and `is_hf18_active()` methods
- `src/cryptonote_core/blockchain.cpp` - HF18 gating logic, startup messages, activation logging
- `src/cryptonote_basic/difficulty.h` - Updated `next_difficulty_lwma()` signature
- `src/cryptonote_basic/difficulty.cpp` - HF18 features, removed debug logs
- `src/version.cpp.in` - Updated to v3.0.3.0

#### New Features
- HF18 activation detection and logging
- Startup message showing HF18 schedule
- Network-specific LWMA window selection
- Difficulty decay safety clamp

## Bug Fixes

- Fixed difficulty explosion issue on testnet after RandomX fork
- Corrected cumulative difficulty handling around fork boundaries
- Improved trimming logic to prevent pre-fork block contamination
- Removed debug logging artifacts from production builds

## Upgrade Instructions

### For Node Operators

1. **Upgrade before HF18 height**:
   - Mainnet: Before block 34,000
   - Testnet: Before block 61,000
   - Stagenet: Before block 45,000

2. **Download and install v3.0.3**:
   ```bash
   git clone https://github.com/quantumsafefoundation/qsf.git
   cd qsf
   git checkout release/v3.0.3
   # Build instructions in README.md
   ```

3. **Restart your daemon**:
   - The daemon will automatically activate HF18 at the scheduled height
   - You'll see "HF18 activated: new difficulty rules enabled" in logs

### For Miners

- **No action required**: Mining continues normally
- **Difficulty will adjust**: Expect smoother difficulty transitions after HF18
- **Testnet miners**: Will see faster difficulty recovery with 30-block window

### For Exchanges

- **Monitor HF18 activation**: Ensure your nodes are upgraded before fork height
- **Test on testnet first**: Recommended to test v3.0.3 on testnet before mainnet fork
- **No wallet changes**: This is a daemon-only upgrade

## Testing

This release has been tested on:
- Testnet with various hash rate scenarios
- Mainnet compatibility (pre-HF18 blocks remain valid)
- Difficulty calculation correctness
- Fork activation and logging

## Known Issues

None at this time.

## Contributors

- Difficulty algorithm improvements
- HF18 implementation
- Testing and validation

## Previous Releases

- **v3.0.2**: Entropy Reset (RandomX fork)
- **v3.0.1**: Bug fixes
- **v3.0.0**: Major RandomX integration

---

For detailed technical information, see the source code and commit history.


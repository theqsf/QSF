# QSF v3.0.5.0 Release Notes - Difficulty Rescue

## Release Date
TBD

## Tag
v3.0.5-difficulty-rescue

## Summary

This release implements a **consensus-critical difficulty rescue mechanism** to unstick the QSF mainnet chain at block 31,670 and prevent future difficulty-lock scenarios.

## Critical Consensus Change

⚠️ **MANDATORY UPGRADE**: This release contains consensus-impacting changes. All nodes must upgrade to v3.0.5.0 or later to remain on the correct chain.

Nodes running older versions will compute different difficulty for block 31,671 and will diverge from the network.

## Problem Statement

The QSF mainnet chain became stuck at block 31,670 due to:

1. A temporary hashrate spike (hundreds of MH/s) that occurred shortly before/at height 31,670
2. Difficulty retarget logic (modified in v3.0.2) that reacted too aggressively to the spike
3. After miners left, remaining hashrate (~1-2 MH/s) was insufficient to find block 31,671 at the elevated difficulty (~78.7 billion)

The chain has been unable to progress since block 31,670 was mined.

## Solution

This release implements two mechanisms:

### 1. One-Time Difficulty Reset at Height 31,671

- **Location**: Mainnet only, height 31,671
- **Action**: Override normal difficulty calculation with a fixed reset value (5 billion)
- **Purpose**: Immediately unstick the chain
- **Implementation**: 
  - Constants: `QSF_DIFFICULTY_RESCUE_HEIGHT_MAINNET = 31671`
  - Constants: `QSF_DIFFICULTY_RESCUE_VALUE_MAINNET = 5000000000`
  - Applied in `get_difficulty_for_next_block()` and `get_next_difficulty_for_alternative_chain()`

### 2. Ongoing Difficulty Safety Valve

- **Activation**: Height >= 31,671 on mainnet
- **Trigger**: If average solve time over the difficulty window exceeds 4 hours
- **Action**: Automatically reduce difficulty (up to 4× reduction, minimum floor: 1 million)
- **Purpose**: Prevent future difficulty-lock scenarios
- **Implementation**: 
  - Constants: `QSF_DIFFICULTY_SAFETY_VALVE_STUCK_TIME = 14400` (4 hours)
  - Constants: `QSF_DIFFICULTY_SAFETY_VALVE_MIN_DIFFICULTY = 1000000`
  - Applied in `next_difficulty_lwma()`

## Technical Details

### Files Modified

1. **src/cryptonote_config.h**
   - Added rescue height and difficulty constants
   - Added safety valve parameters
   - Added config namespace constants for all network types

2. **src/cryptonote_basic/difficulty.cpp**
   - Updated `next_difficulty_lwma()` signature to accept height and network type
   - Implemented safety valve logic based on average solve time

3. **src/cryptonote_basic/difficulty.h**
   - Updated function signature for `next_difficulty_lwma()`

4. **src/cryptonote_core/blockchain.cpp**
   - Added rescue check in `get_difficulty_for_next_block()`
   - Added rescue check in `get_next_difficulty_for_alternative_chain()`
   - Updated all `next_difficulty_lwma()` call sites
   - Added logging for rescue and safety valve activations

5. **src/version.cpp.in**
   - Updated version to 3.0.5.0
   - Updated release name to "Difficulty Rescue"

### Design Principles

- ✅ **No rollback**: Block 31,670 remains valid and unchanged
- ✅ **No special advantage**: All miners treated equally
- ✅ **Deterministic**: All nodes compute the same difficulty
- ✅ **Mainnet-only**: Testnet and stagenet unaffected
- ✅ **One-time rescue**: Reset applies only at height 31,671
- ✅ **Ongoing protection**: Safety valve active from height 31,671 onwards

## Build Instructions

Standard QSF build process:

```bash
cd build
cmake ..
make -j$(nproc)
```

No special flags required.

## Upgrade Instructions

1. **Stop the daemon** (if running)
2. **Backup your blockchain data** (recommended)
3. **Build or download v3.0.5.0**
4. **Start the daemon**

The daemon will automatically:
- Apply the rescue difficulty at height 31,671
- Enable the safety valve from height 31,671 onwards
- Log rescue and safety valve activations

## Logging

The daemon will log:

- `DIFFICULTY RESCUE: Applying one-time difficulty reset at height 31671 to 5000000000`
- `DIFFICULTY SAFETY VALVE: Reduced difficulty at height X from Y to Z due to long average solve time`

## Testing

- ✅ Code compiles without errors
- ✅ No linter errors
- ✅ Function signatures updated consistently
- ⚠️ **Network testing required**: This is a consensus change and must be tested on mainnet

## Future Work

(Not included in this release)

- Explore PoW tweaks that maintain RandomX CPU fairness
- Investigate mining rules that discourage hash farm advantages
- Encourage small miner participation without breaking general-purpose PoW compatibility

## Credits

Implementation by Cursor AI based on QSF developer specifications.

## References

- Issue: Mainnet stuck at block 31,670
- Root cause: Difficulty runaway from transient hashrate spike
- Previous releases:
  - v3.0.0.0: RandomX activation at height 31,000
  - v3.0.2.0: Difficulty/mining parameter changes (likely contributed to issue)
  - v3.0.4.0: Sync/wallet fixes (chain already stuck)


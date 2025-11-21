# QSF v3.0.7.0 Release Notes - Difficulty Override (Fixed)

## Critical Fix

This release fixes v3.0.6.0 where the difficulty rescue was not activating correctly.

## Problem with v3.0.6.0

The rescue logic used `height == rescue_height` which only matched exactly at height 31671. However, when the chain is stuck at height 31670, the difficulty calculation for block 31671 wasn't being properly overridden.

## Solution in v3.0.7.0

Changed the rescue condition from:
```cpp
if (height == rescue_height)
```

to:
```cpp
if (height >= rescue_height)
```

This ensures that:
- Block 31671 (and all subsequent blocks) get the 16× difficulty reduction
- The rescue activates immediately when computing difficulty for the next block
- All three difficulty calculation paths are updated:
  - `get_difficulty_for_next_block()` - main chain
  - `get_next_difficulty_for_alternative_chain()` - alternative chains
  - `recalculate_difficulties()` - database recalculation

## Expected Behavior

After upgrading to v3.0.7.0:
- `get_info` RPC will show difficulty ≈ 4.9 billion (78686542538 / 16) for block 31671
- Block 31671 becomes immediately mineable with current hashrate
- All blocks from 31671 onwards will have reduced difficulty until the safety valve adjusts

## Upgrade Instructions

**MANDATORY**: All nodes must upgrade from v3.0.6.0 to v3.0.7.0

1. Stop the daemon
2. Rebuild or download v3.0.7.0 binaries
3. Restart the daemon
4. Verify difficulty is reduced:
   ```bash
   curl -s http://127.0.0.1:18071/json_rpc \
     -d '{"jsonrpc":"2.0","id":"0","method":"get_info"}' \
     -H 'Content-Type: application/json' | jq '.result.difficulty'
   ```
   Should show ~4.9 billion instead of ~78.7 billion

## Version

- Version: 3.0.7.0
- Release name: "Difficulty Override (Fixed)"


# QSF v3.0.6.0 Release Notes – Difficulty Override

## Summary

- **Critical consensus fix**: Block 31,671 difficulty is explicitly divided by 16 on mainnet.
- **Immediate effect**: Block 31,671 becomes mineable with the existing community hashrate (≈1–2 MH/s).
- **Post-rescue behavior**: The v3.0.5 safety-valve logic continues to govern blocks ≥ 31,672.

## Technical Details

1. **One-time override**
   - When computing difficulty for block **31,671** on **mainnet**, the daemon rescales the calculated difficulty by `1/16`.
   - Applies to the canonical chain, alternative chains, and database difficulty replays (`recalculate_difficulties`).
   - Deterministic across all upgraded nodes; incompatible with ≤v3.0.5 at height 31,671 (intentional consensus split).

2. **Configuration constants**
   - `QSF_DIFFICULTY_RESCUE_HEIGHT_MAINNET = 31671`
   - `QSF_DIFFICULTY_RESCUE_DIVISOR_MAINNET = 16`
   - `QSF_DIFFICULTY_RESCUE_VALUE_MAINNET = 0` (0 = “derive from divisor”)

3. **Version bump**
   - `DEF_QSF_VERSION = 3.0.6.0`
   - Release codename: **“Difficulty Override”**

4. **Upgrade instructions**
   - Mandatory for **all** nodes and miners.
   - Stop the daemon, rebuild or download v3.0.6 binaries, restart.
   - Expect block 31,671 to appear shortly after majority upgrade adoption.

5. **Verification tips**
   - Monitor progress:
     ```bash
     curl -s http://127.0.0.1:18071/json_rpc \
       -d '{"jsonrpc":"2.0","id":"0","method":"get_info"}' \
       -H 'Content-Type: application/json' | jq '.result.height, .result.difficulty'
     ```
   - After block 31,671 is mined, check its recorded difficulty:
     ```bash
     curl -s http://127.0.0.1:18071/json_rpc \
       -d '{
             "jsonrpc": "2.0",
             "id": "0",
             "method": "get_block_header_by_height",
             "params": { "height": 31671 }
           }' \
       -H 'Content-Type: application/json' | jq '.result.difficulty'
     ```

## Testing

- `cryptonote_basic` and `cryptonote_core` targets compile cleanly.
- No runtime flag changes required.

## Important

This release is the **only** path forward for the mainnet chain stuck at block 31,670. Upgrade immediately so the network converges on the lowered difficulty for block 31,671.


# QSF v3.0.9 — Difficulty Rescue /64

**Codename:** "Difficulty Rescue /64"  
**Type:** Mandatory consensus update  
**Activation:** Mainnet at block height 31,670+

## What changed

- **Reduced difficulty divisor: 16 → 64**  
  - `QSF_DIFFICULTY_RESCUE_DIVISOR_MAINNET` changed from **16** to **64**  
  - First affected block: height **31,670**  
  - The effective difficulty at this height is **scaled down by 64×** (previously 16×)

- **Result:**
  - Old difficulty: `~4.9e9` (with divisor 16)
  - New effective difficulty at rescue height: `~1.2e9` (with divisor 64)
  - Expected difficulty ≈ **1.2B** instead of 4.9B
  - Expected block time **~20–40 min** at current hashrate

- **Safety valve preserved:**
  - The 4-hour "stuck" detector and minimum difficulty floor remain active for future blocks.

## Why this is mandatory

Nodes ≤ v3.0.8 will continue to target the higher difficulty (with divisor 16) and will not agree with the corrected chain using divisor 64.

All **nodes, pools, and explorers must upgrade** to v3.0.9 to remain on the canonical mainnet chain.

## Upgrade instructions (mainnet)

1. Stop the daemon:
   ```bash
   systemctl stop qsf-daemon-mainnet.service
   # or ./qsf exit in a shell
   ```

2. Backup your blockchain data (recommended):
   ```bash
   cp -r ~/.qsf/lmdb ~/.qsf/lmdb.backup
   ```

3. Build or download v3.0.9.0:
   ```bash
   cd ~/QSF
   git pull origin main
   cd build
   cmake ..
   make -j$(nproc)
   ```

4. Start the daemon:
   ```bash
   systemctl start qsf-daemon-mainnet.service
   # or ./build/bin/qsf --config-file ~/.qsf/qsf.conf
   ```

5. Verify the rescue is active:
   ```bash
   curl -s http://127.0.0.1:18071/json_rpc \
     -d '{"jsonrpc":"2.0","id":"0","method":"get_info"}' \
     -H 'Content-Type: application/json' | jq '.result.difficulty, .result.height'
   ```
   
   Expected output:
   - `difficulty`: Should show ~1.2 billion instead of ~4.9 billion
   - `height`: Should still be 31670 until the block is mined

## Technical Details

### Code Changes

1. **src/cryptonote_config.h**
   - `QSF_DIFFICULTY_RESCUE_DIVISOR_MAINNET`: Changed from 16 to **64**
   - `QSF_DIFFICULTY_RESCUE_HEIGHT_MAINNET`: Remains **31670** (unchanged)
   - Safety valve constants: Unchanged

2. **src/version.cpp.in**
   - Version bumped to 3.0.9.0
   - Release name: "Difficulty Rescue /64"

### Consensus Impact

- **Breaking change**: Nodes running v3.0.8 or earlier will compute different difficulty
- **Deterministic**: All v3.0.9.0 nodes will compute the same reduced difficulty (64× reduction)
- **Immediate effect**: Block 31671 becomes more easily mineable as soon as majority upgrades

## Expected Timeline

Once the majority of network hashrate upgrades to v3.0.9.0:
- Block 31671 should be found within **20–40 minutes** at current hashrate
- Chain will resume normal operation
- Safety valve will continue to protect against future difficulty spikes

## Support

For issues or questions:
- GitHub: https://github.com/theqsf/QSF
- Discord: [QSF Community Discord]

## Credits

Implementation by Cursor AI based on community feedback and network analysis.


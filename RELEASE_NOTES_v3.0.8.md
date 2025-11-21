# QSF v3.0.8 — Direct Difficulty Rescue

**Codename:** "Direct Difficulty Rescue"  
**Type:** Mandatory consensus update  
**Activation:** Mainnet at block height 31,670+

## What changed

- **Difficulty rescue now activates directly at the stuck block**  
  - `QSF_DIFFICULTY_RESCUE_HEIGHT_MAINNET` moved to **31,670**  
  - Rescue logic applies for **all heights ≥ 31,670**  
  - The effective difficulty at this height is **scaled down by 16×**

- **Result:**
  - Old difficulty: `~7.8686542538e10`
  - New effective difficulty at rescue height: `~4.9e9`
  - This makes the next block realistically mineable with the hashrate we actually have.

- **Safety valve preserved:**
  - The 4-hour "stuck" detector and minimum difficulty floor introduced in v3.0.5+ remain active for future blocks.

## Why this is mandatory

Nodes ≤ v3.0.7 will continue to target the old, extremely high difficulty and will not agree with the corrected chain.

All **nodes, pools, and explorers must upgrade** to v3.0.8 to remain on the canonical mainnet chain.

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

3. Build or download v3.0.8.0:
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
   - `difficulty`: Should show ~4.9 billion (4,917,908,908) instead of ~78.7 billion
   - `height`: Should still be 31670 until the block is mined

## Technical Details

### Code Changes

1. **src/cryptonote_config.h**
   - `QSF_DIFFICULTY_RESCUE_HEIGHT_MAINNET`: Changed from 31671 to **31670**

2. **src/cryptonote_core/blockchain.cpp**
   - All three difficulty calculation paths use `height >= rescue_height` (already fixed in v3.0.7)
   - Rescue applies immediately when computing difficulty for block 31671

3. **src/version.cpp.in**
   - Version bumped to 3.0.8.0
   - Release name: "Direct Difficulty Rescue"

### Consensus Impact

- **Breaking change**: Nodes running v3.0.7 or earlier will compute different difficulty
- **Deterministic**: All v3.0.8.0 nodes will compute the same reduced difficulty
- **Immediate effect**: Block 31671 becomes mineable as soon as majority upgrades

## Expected Timeline

Once the majority of network hashrate upgrades to v3.0.8.0:
- Block 31671 should be found within **minutes to a few hours**
- Chain will resume normal operation
- Safety valve will continue to protect against future difficulty spikes

## Support

For issues or questions:
- GitHub: https://github.com/theqsf/QSF
- Discord: [QSF Community Discord]

## Credits

Implementation by Cursor AI based on community feedback and network analysis.


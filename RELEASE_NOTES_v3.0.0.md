# QSF v3.0.0 – Entropy Reset

## TL;DR
- Planned hard fork at height **31,000** to restore sane difficulty
- **LWMA-3** difficulty algorithm replaces legacy EMA
- Difficulty reset to **1,000,000** at the fork block
- Enforced **60s minimum block time** to eliminate timestamp spam
- RandomX seed path tweaked at fork to invalidate private miners’ caches

## Activation
- **Mainnet:** height 31,000  
- **Stagenet:** height 47,000  
- **Testnet:** height 60,800  
- Nodes must upgrade to v3.0.0 before their network’s activation height. Non-upgraded nodes will follow an incompatible chain due to the new difficulty and timestamp rules.

## Consensus Changes
1. **Difficulty Reset**
   - Block 31,000 difficulty forcibly set to 1,000,000.
   - Difficulty history cache cleared so the next window starts fresh.

2. **LWMA-3 Difficulty**
   - Uses a 90-block window targeting 120s per block.
   - Rapidly adapts to hashrate swings while resisting timestamp gaming.

3. **Minimum Timestamp Rule**
   - A block’s timestamp must be at least `previous_timestamp + 60`.
   - Applies to main chain and alternative chains, and miners clamp template timestamps accordingly.

4. **RandomX Tweak**
   - Seed hash is rehashed with `QSF-RX-TWEAK` and the seed height once the fork activates.
   - Forces miners to refresh datasets and invalidates bespoke optimizations.

## RPC / Observability
- Reported target now reflects the fork target (120s) after activation.
- `getinfo`/daemon status automatically show the LWMA-based difficulty.

## Upgrade Checklist
1. Stop your node.
2. Install v3.0.0 binaries.
3. Start the node and allow it to sync before height 31,000.
4. Pool ops: ensure miners refresh RandomX datasets at the fork.

## Backward Compatibility
- No database format changes.
- Old nodes will stall on the legacy chain once block 31,000 is mined.

## Source Highlights
- `cryptonote_basic/difficulty.*`: LWMA implementation.
- `cryptonote_core/blockchain.*`: Fork gating, timestamp enforcement, RandomX seed tweaks.
- `cryptonote_config.h`: Network-specific activation heights and constants.

Stay safe, keep your node patched, and happy solo mining!


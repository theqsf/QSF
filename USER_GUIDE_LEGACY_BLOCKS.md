# User Guide: Legacy Blocks and Wallet Recovery

## Quick Answer

**Users do NOT need to do anything special.** Here's what happens:

1. **Nodes (Daemons)**: Update to the new code version. They will automatically:
   - Accept legacy blocks during blockchain sync (backward compatibility)
   - Continue accepting old blocks that are already in the chain
   - Use secure signatures for all newly mined blocks

2. **Wallets**: No changes needed. Wallets continue working normally:
   - Connect to updated daemons
   - Query blockchain history (including legacy blocks)
   - Wallets don't validate quantum-safe signatures themselves

## Detailed Explanation

### How Blockchain Sync Works

When nodes sync with each other:

1. **Node A (with updated code)** connects to **Node B (may have legacy blocks)**
2. Node A requests missing blocks from Node B
3. Node B sends blocks (which may include legacy blocks with old signatures)
4. Node A validates each block:
   - **Legacy blocks**: Accepted because backward compatibility code recognizes old signature format
   - **New blocks**: Must have secure signatures (with nonces and HMAC)
5. All blocks are added to the chain

### What Happens to Legacy Blocks

**Legacy blocks remain in the blockchain:**
- ✅ They are accepted during sync (backward compatibility)
- ✅ They continue to exist in the chain
- ⚠️ They remain vulnerable to forgery (by design - can't reject existing blocks)
- ✅ New blocks use secure signatures

### Wallet Behavior

**Wallets don't validate blocks:**
- Wallets connect to daemons via RPC
- Daemons handle all block validation
- Wallets just query transaction history and balances
- No wallet changes needed

### What Users Need to Do

#### For Node Operators (Miners/Full Nodes):

1. **Update your daemon** to the new version
2. **Restart your daemon** - it will:
   - Automatically sync with the network
   - Accept legacy blocks during sync
   - Use secure signatures for new blocks you mine
   - Auto-upgrade your mining keys if they're in old format

#### For Wallet Users:

**Nothing!** Just:
- Continue using your existing wallet
- Connect to updated daemons (they'll automatically use secure signatures)
- No wallet recovery or restoration needed
- Your existing wallet file works as-is

### Wallet Recovery Scenarios

You only need wallet recovery if:
- You lost your wallet file
- You want to restore from seed phrase
- You're setting up a new wallet

**Recovery process:**
1. Use your seed phrase to restore wallet
2. Connect to any updated daemon
3. Wallet syncs blockchain (including legacy blocks)
4. Your funds appear (they're in the blockchain, not affected by signature format)

### Automatic Key Migration

When miners start the new version:

1. **Old keys detected**: Mining keys in old format (32 or 64 bytes)
2. **Automatic upgrade**: Keys automatically regenerated to new secure format (96 bytes)
3. **No user action**: Happens automatically when daemon starts
4. **Log message**: "Auto-migrated quantum-safe keys from old format to new secure format"

### Security Timeline

```
Before Update:
├── All blocks: Vulnerable signatures
└── Status: All blocks forgeable

After Update:
├── Legacy blocks (existing): Still vulnerable (accepted for backward compatibility)
├── New blocks: Secure signatures (unforgeable)
└── Status: New blocks secure, legacy blocks remain (but can't be changed)

Future:
└── Over time, all active blocks will be new secure blocks
```

### FAQ

**Q: Do I need to resync my blockchain?**
A: No. Your existing blockchain database is fine. The updated daemon will accept your existing blocks.

**Q: Do I need to recover my wallet?**
A: No. Your existing wallet file works as-is. Just connect to updated daemons.

**Q: Will my old blocks be replaced?**
A: No. Legacy blocks remain in the chain. They're accepted but remain vulnerable. Only new blocks use secure signatures.

**Q: What if I'm a miner?**
A: Update your daemon. Your mining keys will automatically upgrade to the new format when you start mining.

**Q: What if I'm running a node but not mining?**
A: Just update your daemon. It will sync with the network and accept both legacy and new blocks.

**Q: Do I need to send my blocks anywhere?**
A: No. Blocks are automatically synced between nodes. Just update your daemon and it will sync normally.

**Q: What if my daemon is offline?**
A: When you update and restart, it will sync from the network and catch up, accepting legacy blocks during sync.

### Summary

- ✅ **Nodes**: Update daemon, sync automatically handles legacy blocks
- ✅ **Wallets**: No changes needed, work as-is
- ✅ **Recovery**: Only needed if you lost wallet file (not related to signature fix)
- ✅ **Miners**: Keys auto-upgrade, no manual action needed

**The fix is transparent to users - just update your daemon and everything continues working.**


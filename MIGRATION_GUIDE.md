# QSF v2.1.1 Migration Guide

## Quick Migration Checklist

- [ ] Backup current wallet files
- [ ] Download QSF v2.1.1
- [ ] Stop current QSF daemon/wallet
- [ ] Install QSF v2.1.1
- [ ] Generate NEW quantum-safe keys
- [ ] Test new keys work correctly
- [ ] Update all network nodes
- [ ] Verify network connectivity

---

## Detailed Migration Steps

### For GUI Users

#### 1. Backup Your Current Wallet
```
1. Open QSF GUI Miner
2. Go to File → Export Wallet
3. Save backup to a secure location
```

#### 2. Download and Install v2.1.1
```
1. Download QSF v2.1.1 from GitHub releases
2. Extract the archive
3. Run the installer (or build from source)
4. Launch QSF GUI Miner v2.1.1
```

#### 3. Generate New Keys
```
1. Open QSF GUI Miner
2. Navigate to "Quantum-Safe" tab
3. Click "Generate New XMSS Keys"
4. Click "Generate New SPHINCS+ Keys"
5. Save keys securely (you'll be prompted)
6. Verify keys are displayed in the tab
```

#### 4. Verify Everything Works
```
1. Try creating a test transaction
2. Check that signatures are generated
3. Verify the transaction is accepted
```

---

### For CLI Users

#### 1. Backup Current Files
```bash
# Create backup directory
mkdir -p ~/qsf-backup-$(date +%Y%m%d)

# Backup wallet
cp ~/.quantumsafefoundation/qsf-wallet ~/qsf-backup-$(date +%Y%m%d)/

# Backup keys
cp ~/.quantumsafefoundation/qsf.keys ~/qsf-backup-$(date +%Y%m%d)/

# Backup config
cp ~/.quantumsafefoundation/qsf.local.conf ~/qsf-backup-$(date +%Y%m%d)/
```

#### 2. Stop Current Services
```bash
# Stop daemon
sudo systemctl stop qsf-daemon

# Or if running manually
pkill -f qsf
```

#### 3. Download and Build v2.1.1
```bash
# Navigate to QSF directory
cd /path/to/QSF

# Pull latest code (if using git)
git pull origin main
git checkout v2.1.1

# Or download release
wget https://github.com/theqsf/QSF/releases/download/v2.1.1/QSF-v2.1.1.tar.gz
tar -xzf QSF-v2.1.1.tar.gz
cd QSF-v2.1.1

# Build
./build.sh
```

#### 4. Generate New Keys
```bash
# Option 1: Generate dual keys (recommended)
./build/bin/qsf-wallet-cli --generate-dual-keys

# Option 2: Generate keys separately
./build/bin/qsf-wallet-cli --generate-xmss-keys
./build/bin/qsf-wallet-cli --generate-sphincs-keys
```

#### 5. Start New Services
```bash
# Start daemon
./build/bin/qsf --daemon

# Or with systemd
sudo systemctl start qsf-daemon

# Start wallet
./build/bin/qsf-wallet-cli
```

#### 6. Verify Keys
```bash
# Check keys are loaded
./build/bin/qsf-wallet-cli --show-keys

# Test signature generation
./build/bin/qsf-wallet-cli --test-signatures
```

---

### For Node Operators

#### 1. Schedule Maintenance Window
- Notify users of planned downtime
- Coordinate with other node operators
- Plan for 30-60 minutes of downtime

#### 2. Update Node
```bash
# Stop node
sudo systemctl stop qsf-daemon

# Backup blockchain data (optional but recommended)
cp -r ~/.quantumsafefoundation/lmdb ~/.quantumsafefoundation/lmdb.backup

# Install new version (see CLI steps above)
# Generate new keys
# Start node
sudo systemctl start qsf-daemon
```

#### 3. Verify Node Health
```bash
# Check node is running
./build/bin/qsf status

# Check connections
./build/bin/qsf status | grep connections

# Check blockchain sync
./build/bin/qsf status | grep height
```

#### 4. Monitor Network
- Ensure node is syncing properly
- Check for any errors in logs
- Verify connectivity with other nodes

---

## Troubleshooting

### Problem: Old signatures are rejected

**Solution:** This is expected behavior. Old signatures are incompatible with the new format. You must:
1. Generate new keys
2. Create new signatures with the new keys
3. Old signatures will not verify (this is correct)

### Problem: Cannot verify signatures

**Solution:** 
1. Ensure you're using v2.1.1 on both signing and verifying sides
2. Verify keys are properly generated
3. Check that both XMSS and SPHINCS+ keys are present (for dual signatures)

### Problem: Keys won't generate

**Solution:**
1. Check file permissions on `~/.quantumsafefoundation/`
2. Ensure sufficient disk space
3. Try generating keys one at a time (XMSS first, then SPHINCS+)
4. Check logs for specific error messages

### Problem: Node won't start after upgrade

**Solution:**
1. Check logs: `tail -f ~/.quantumsafefoundation/qsf.log`
2. Verify blockchain data is compatible
3. Try starting with `--data-dir` pointing to backup
4. If issues persist, resync from scratch

### Problem: Network connectivity issues

**Solution:**
1. Ensure all nodes are upgraded to v2.1.1
2. Check firewall rules allow P2P port (default 18070)
3. Verify seed nodes are updated
4. Check network connectivity: `./build/bin/qsf status`

---

## Rollback Procedure

**⚠️ WARNING: Rolling back is NOT recommended due to security vulnerability.**

If you absolutely must rollback:

1. **Understand the risk:** You'll be vulnerable to the private key exposure bug
2. **Stop using the network:** Don't sign anything with old version
3. **Restore backup:** Use your backup files
4. **Restart old version:** Start v2.1.0 or earlier
5. **Plan immediate upgrade:** Schedule upgrade to v2.1.1 ASAP

---

## Post-Migration Verification

### Verify Keys Are New
```bash
# Compare key hashes (should be different)
sha256sum ~/.quantumsafefoundation/qsf.keys.backup
sha256sum ~/.quantumsafefoundation/qsf.keys
```

### Verify Signatures Work
```bash
# Test signature generation
./build/bin/qsf-wallet-cli --test-signatures

# Expected output:
# ✓ XMSS signature test passed
# ✓ SPHINCS+ signature test passed
# ✓ Dual signature test passed
```

### Verify Network Connectivity
```bash
# Check peer connections
./build/bin/qsf status

# Expected:
# Connections: 8/12
# Height: [current height]
# Status: synchronized
```

---

## Security Checklist After Migration

- [ ] Old keys are deleted or securely archived
- [ ] New keys are backed up securely
- [ ] New keys are tested and working
- [ ] All nodes are upgraded to v2.1.1
- [ ] Network connectivity is restored
- [ ] No old signatures are being used
- [ ] Monitoring is in place for unauthorized activity

---

## Need Help?

- **GitHub Issues:** https://github.com/theqsf/QSF/issues
- **Discord:** https://discord.com/channels/1435343401502376027/1435343403360587940
- **BitcoinTalk:** https://bitcointalk.org/index.php?topic=5564519.new#new

---

**Last Updated:** November 4, 2024  
**Version:** v2.1.1


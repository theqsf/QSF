# QSF v3.0.4 — “Network Recovery Release”

**Protocol Version: 17**  
**Release Version: 3.0.4.0-release**

## Summary

QSF v3.0.4 is a stability-focused recovery update. It does **not** introduce any new consensus rules or PoW algorithm changes. Instead, it restores full network consistency by aligning all nodes, wallets, and pool software on a unified release version following earlier experimental difficulty adjustments.

This release ensures:

- Stable chain progression  
- Unified versioning across all builds  
- Compatibility with the last known good consensus (v3.0.1)  
- Full RandomX block production  
- No more divergence between node operators  

If you run **any node, pool, or wallet**, this upgrade is **mandatory**.

---

## 🛠 What Changed in v3.0.4

### ✔ Version Alignment Fix

The internal version references were updated to:

- `3.0.4.0` (core)
- `3.0.4.0-release` (full banner)

This resolves mismatches where nodes reported older versions even when compiled from newer code.

### ✔ Rebased to Last Known Good Consensus

This release intentionally **restores the stable consensus state from v3.0.1**, before the LWMA/difficulty experiments that caused:

- Rapid difficulty spikes  
- Zero block production  
- Chain stall at height **31665**  
- Fork-like behaviour due to inconsistent nodes  

v3.0.4 ensures all nodes run on the same proven consensus path.

### ✔ No Consensus or Difficulty Algorithm Changes

No changes were made to:

- RandomX PoW implementation  
- Block reward logic  
- Difficulty calculation  
- Hard fork rules  
- Consensus flags  
- Block validation  
- Network protocol  

This is a *pure recovery* release.

---

## ⚙️ Why This Update Was Necessary

Between v3.0.2–v3.0.3, multiple pools and nodes were running mismatched builds, causing:

- Exponential difficulty (x700 increase in 9 blocks)
- Missing block production  
- Stalled blockchain  
- RPC errors
- Outdated peers overwhelming good peers  

v3.0.4 unifies everyone under a **single correct build** based on the last documented stable state.

---

## ⬆️ Upgrade Instructions

### Linux Node / Daemon

```bash
git clone https://github.com/theqsf/QSF.git
cd QSF
git checkout v3.0.4.0-release
./build.sh   # or ./build_linux.sh depending on your environment
cd build/bin/
./qsf
```

Make sure all services restart with the freshly built binaries.


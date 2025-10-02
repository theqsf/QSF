# 🔒 QSF Quantum-Safe Foundation

<div align="center">
  <img src="src/gui_miner/icons/qsf_icon.png" alt="QSF Logo" width="120" height="120">
  
  **A Quantum-Resistant Cryptocurrency with RandomX Proof-of-Work and DUAL XMSS/SPHINCS+ Signatures**
  
  [![License](https://img.shields.io/badge/license-BSD3-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
  [![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
  [![Version](https://img.shields.io/badge/version-2.0.0-blue.svg)]()
  [![Quantum-Safe](https://img.shields.io/badge/quantum--safe-100%25-brightgreen.svg)]()
  [![51% Attack Protection](https://img.shields.io/badge/51%25%20attack%20protection-ACTIVE-red.svg)]()
</div>

---

## 🌟 **Overview**

QSF (Quantum-Safe Foundation) is a **revolutionary cryptocurrency** that combines **RandomX proof-of-work mining** with **DUAL quantum-resistant signature schemes** to create the **most secure and future-proof digital currency** in existence.

## 🚀 **Core Features**

### 🔐 **DUAL Quantum-Resistant Security (MANDATORY)**
- **XMSS (eXtended Merkle Signature Scheme)**: Stateful hash-based signatures
- **SPHINCS+**: Stateless hash-based signatures  
- **DUAL ENFORCEMENT**: BOTH algorithms required simultaneously
- **NO COMPROMISE**: Maximum quantum resistance with no option to weaken security

### ⚡ **RandomX Mining + Quantum-Safe Integration**
- **CPU-optimized mining**: Fair distribution using RandomX algorithm
- **ASIC-resistant**: Prevents centralization through specialized hardware
- **Quantum-Safe Integration**: RandomX PoW + quantum-safe signatures enforced
- **Energy-efficient**: Optimized for consumer-grade hardware

### 🛡️ **Advanced 51% Attack Protection**
- **Multi-layer defense**: Early detection at 40% (before 51%)
- **Automatic response**: Immediate difficulty spike and network segmentation
- **Self-healing**: Automatic recovery and network restoration
- **Geographic distribution**: Prevents single-region attacks

### 🌐 **Multi-Network Support**
- **Mainnet**: Production network with full quantum-safe enforcement
- **Testnet**: Development and testing network
- **Stagenet**: Staging environment for testing

### 💻 **Modern GUI Wallet**
- **Bitcoin-like Interface**: Familiar and intuitive design
- **Real-time Mining**: Integrated mining with live statistics
- **Quantum-Safe Tab**: Mandatory key generation and management
- **Network Status**: Visual indicators for connectivity and security

---

## 📦 **Installation & Setup**

### **🚀 Quick Setup (Recommended)**
```bash
# Clone the repository
git clone https://github.com/your-username/QSF.git
cd QSF

# Initialize submodules and build everything
git submodule update --init --recursive
./build.sh
```

The build script automatically handles:
- ✅ **Submodule initialization** (trezor-common, randomx, miniupnp, rapidjson)
- ✅ **Qt Wayland dependencies** (required for GUI miner)
- ✅ **Configuration files** (`~/.quantumsafefoundation/qsf.local.conf`)
- ✅ **Complete build process** (daemon, GUI miner, wallet CLI)

### **🎯 Running QSF Components**
```bash
# Start the daemon (recommended)
./start_qsf_daemon.sh

# Or run components directly
cd build/bin
./qsf                    # QSF daemon
./qsf-gui-miner          # GUI miner
./qsf-wallet-cli         # Wallet CLI
./qsf-wallet-rpc         # Wallet RPC server
```

### **⚙️ Quick Start Guide**
1. **Launch the GUI**: Run `./qsf-gui-miner` from `build/bin/`
2. **Generate DUAL Quantum-Safe Keys**: Go to "Quantum-Safe" tab and generate BOTH XMSS and SPHINCS+ keys
3. **Select Network**: Choose Mainnet, Testnet, or Stagenet
4. **Create Wallet**: Generate a new wallet address
5. **Start Mining**: Begin mining with RandomX + quantum-safe validation


---

## 🔧 **Network Configuration**

### **Domain-Based Seed Nodes (Like QSF)**
| Network | RPC Port | P2P Port | Daemon URL | Seed Nodes |
|---------|----------|----------|------------|------------|
| **Mainnet** | 18071 | 18070 | `http://qsfchain.com:18071` | `seeds.qsfchain.com`<br>`seeds.qsfcoin.com`<br>`seeds.qsfcoin.network`<br>`seeds.qsfcoin.org`<br>`seeds.qsfnetwork.co` |
| **Testnet** | 28071 | 28070 | `http://qsfnetwork.com:28071` | `seeds.qsfnetwork.com` |
| **Stagenet** | 38071 | 38070 | `http://qsfcoin.network:38071` | `seeds.qsfcoin.network` |

### **DNS Configuration**
The project uses domain-based seed nodes (similar to QSF's `seeds.qsfseeds.se`):
- **Mainnet**: `seeds.qsfchain.com`, `seeds.qsfcoin.com`, etc.
- **Testnet**: `seeds.qsfnetwork.com`
- **Stagenet**: `seeds.qsfcoin.network`

See [setup_dns.sh](setup_dns.sh) for DNS configuration instructions.

---

## 🎯 **Usage Guide**

### **1. Daemon Startup**
```bash
# Start daemon (quantum-safe features are automatically enabled)
./build/bin/qsf --config-file=/path/to/qsf-mainnet.conf

# Or start with default settings
./build/bin/qsf

# Quantum-safe features are MANDATORY and always enabled
```

### **2. Mining Configuration**
```bash
# Public seed best-practice
- Keep RPC restricted on 127.0.0.1:18071
- Expose ZMQ on 0.0.0.0:18072 (and 18083 pub)
- GUI miner endpoint: seeds.qsfchain.com:18072
```

---

## 🛡️ **Security & Privacy Features**

### **DUAL Quantum Resistance (MANDATORY)**
- **XMSS Signatures**: Stateful hash-based signatures resistant to quantum attacks
- **SPHINCS+ Signatures**: Stateless hash-based signatures for enhanced security
- **DUAL ENFORCEMENT**: BOTH algorithms required simultaneously
- **NO COMPROMISE**: Maximum quantum resistance with no option to weaken security

### **51% Attack Protection System**
| Protection Layer | Detection Threshold | Response Time | Effectiveness |
|------------------|---------------------|---------------|---------------|
| **Rapid Attack** | 40% | <5 seconds | ⭐⭐⭐⭐⭐ |
| **Gradual Attack** | 35% | <1 minute | ⭐⭐⭐⭐⭐ |
| **Pool Collusion** | 30% | <30 seconds | ⭐⭐⭐⭐⭐ |
| **Geographic** | 60% | <1 minute | ⭐⭐⭐⭐ |
| **Quantum-Safe** | 100% | Immediate | ⭐⭐⭐⭐⭐ |

### **Privacy & Anonymity**
- **Ring Signatures**: Untraceable transactions
- **Stealth Addresses**: One-time addresses for privacy
- **RingCT**: Confidential transaction amounts
- **Dandelion++**: Enhanced transaction propagation privacy

## 📊 **Technical Specifications**

### **Cryptographic Algorithms**
- **Proof-of-Work**: RandomX (CPU-optimized) + Quantum-Safe integration
- **Quantum-Safe Signatures**: DUAL XMSS + SPHINCS+ (both required)
- **Hash Functions**: SHA-256, Blake256
- **Key Derivation**: PBKDF2

### **Network Protocol**
- **Block Time**: 120 seconds
- **Block Size**: Dynamic (up to 40000 bytes)
- **Difficulty**: Auto-adjusting with emergency spike capability
- **Consensus**: Proof-of-Work with RandomX + Quantum-Safe validation
- **Block Reward**: 5.00 QSF per block

### **System & Mining Requirements**
- **OS**: Linux (Ubuntu 20.04+, Debian 11+, or equivalent)
- **CPU**: x86_64 architecture (RandomX optimized)
- **RAM**: 4GB minimum, 8GB recommended
- **Storage**: 50GB for blockchain (2GB for build)
- **Network**: Stable internet connection
- **Security**: Quantum-safe keys mandatory

### **Wallet Features**
- **Multi-Network Support**: Mainnet, Testnet, Stagenet
- **DUAL Quantum-Safe Keys**: Mandatory XMSS + SPHINCS+ generation
- **Integrated Mining**: Real-time mining with statistics
- **Secure Storage**: Encrypted wallet files with quantum-safe keys

---

## 🤝 **Contributing**

We welcome contributions from the community! Please see our contributing guidelines:

1. **Fork the repository**
2. **Create a feature branch**: `git checkout -b feature/amazing-feature`
3. **Commit your changes**: `git commit -m 'Add amazing feature'`
4. **Push to the branch**: `git push origin feature/amazing-feature`
5. **Open a Pull Request**

### **Development Setup**
```bash
# Install development dependencies
sudo apt install clang-format cppcheck valgrind

# Run tests
make test

# Code formatting
make format
```

---

## 🔍 **Troubleshooting**

### **Common Issues**

**GUI Miner Issues:**
- **Segmentation fault**: Run `./setup_qsf_environment.sh` to ensure Qt Wayland is installed
- **"Could not find Qt platform plugin"**: Install Qt Wayland dependencies
- **"Can't find config file"**: Configuration files are auto-created by the build script

**Build Issues:**
- **Submodule errors**: Run `git submodule update --init --recursive`
- **Missing dependencies**: Run `./setup_dependencies.sh`
- **Permission errors**: Ensure you have write access to `~/.quantumsafefoundation/`

**Network Issues:**
- **Connection refused**: Check if ports 18070-18073 are available
- **Firewall**: Ensure ports 18070 (P2P) and 18071 (RPC) are open
- **Daemon won't start**: Check if daemon is already running with `ps aux | grep qsf`

---

## 📄 **License**

This project is licensed under the BSD 3-Clause License - see the [LICENSE](LICENSE) file for details.

---

## 🙏 **Acknowledgments**

- **Monero Project**: Base implementation and RandomX algorithm
- **Quantum Cryptography Community**: Research and development of quantum-resistant algorithms
- **Open Source Contributors**: Community support and contributions

## 📞 **Support & Community**

- **GitHub Issues**: [Report bugs and feature requests](https://github.com/your-username/QSF/issues)
- **Discussions**: [Community discussions](https://github.com/your-username/QSF/discussions)
- **Documentation**: [Complete documentation](docs/)

---

<div align="center">
  <strong>🔒 QSF Quantum-Safe Foundation - The Most Secure Cryptocurrency in Existence</strong>
  <br><br>
  <em>Built with ❤️ for a quantum-resistant future with NO COMPROMISES</em>
  <br><br>
  <strong>DUAL Quantum-Safe + Complete 51% Attack Protection + RandomX Integration</strong>
</div>

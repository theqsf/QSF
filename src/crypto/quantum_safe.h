// Copyright (c) 2024, QuantumSafeFoundation
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of
//    conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include "crypto/hash.h"

namespace crypto
{
  // Forward declarations
  class xmss_private_key;
  class xmss_public_key;
  class xmss_signature;
  class sphincs_private_key;
  class sphincs_public_key;
  class sphincs_signature;

  // Quantum-safe signature algorithm types
  enum class quantum_algorithm : uint8_t
  {
    XMSS = 0,
    SPHINCS_PLUS = 1,
    DUAL = 2
  };

  // XMSS (eXtended Merkle Signature Scheme) implementation
  class xmss_private_key
  {
  public:
    static constexpr size_t KEY_SIZE = 32;
    static constexpr size_t SIGNATURE_SIZE = 1024;
    static constexpr size_t TREE_HEIGHT = 10;

    xmss_private_key();
    ~xmss_private_key();

    // Generate new XMSS key pair
    bool generate();
    
    // Load from bytes
    bool load(const std::vector<uint8_t>& data);
    
    // Save to bytes
    std::vector<uint8_t> save() const;
    
    // Get public key
    xmss_public_key get_public_key() const;
    
    // Sign message
    xmss_signature sign(const crypto::hash& message) const;
    
    // Get remaining signatures
    uint32_t get_remaining_signatures() const;
    
    // Get tree height
    uint32_t get_tree_height() const;

  private:
    std::vector<uint8_t> m_seed;
    std::vector<uint8_t> m_private_key;
    uint32_t m_index;
    uint32_t m_max_signatures;
  };

  class xmss_public_key
  {
  public:
    static constexpr size_t KEY_SIZE = 32;

    xmss_public_key();
    ~xmss_public_key();

    // Load from bytes
    bool load(const std::vector<uint8_t>& data);
    
    // Save to bytes
    std::vector<uint8_t> save() const;
    
    // Verify signature
    bool verify(const crypto::hash& message, const xmss_signature& signature) const;
    
    // Get public key bytes
    std::vector<uint8_t> get_public_key() const;

  private:
    std::vector<uint8_t> m_public_key;
  };

  class xmss_signature
  {
  public:
    static constexpr size_t SIGNATURE_SIZE = 1024;

    xmss_signature();
    ~xmss_signature();

    // Load from bytes
    bool load(const std::vector<uint8_t>& data);
    
    // Save to bytes
    std::vector<uint8_t> save() const;

  private:
    std::vector<uint8_t> m_signature;
    uint32_t m_index;
  };

  // SPHINCS+ implementation
  class sphincs_private_key
  {
  public:
    static constexpr size_t KEY_SIZE = 64;
    static constexpr size_t SIGNATURE_SIZE = 1024;
    static constexpr size_t TREE_LEVEL = 5;

    sphincs_private_key();
    ~sphincs_private_key();

    // Generate new SPHINCS+ key pair
    bool generate();
    
    // Load from bytes
    bool load(const std::vector<uint8_t>& data);
    
    // Save to bytes
    std::vector<uint8_t> save() const;
    
    // Get public key
    sphincs_public_key get_public_key() const;
    
    // Sign message
    sphincs_signature sign(const crypto::hash& message) const;
    
    // Get tree level
    uint32_t get_level() const;

  private:
    std::vector<uint8_t> m_seed;
    std::vector<uint8_t> m_private_key;
  };

  class sphincs_public_key
  {
  public:
    static constexpr size_t KEY_SIZE = 32;

    sphincs_public_key();
    ~sphincs_public_key();

    // Load from bytes
    bool load(const std::vector<uint8_t>& data);
    
    // Save to bytes
    std::vector<uint8_t> save() const;
    
    // Verify signature
    bool verify(const crypto::hash& message, const sphincs_signature& signature) const;
    
    // Get public key bytes
    std::vector<uint8_t> get_public_key() const;

  private:
    std::vector<uint8_t> m_public_key;
  };

  class sphincs_signature
  {
  public:
    static constexpr size_t SIGNATURE_SIZE = 1024;

    sphincs_signature();
    ~sphincs_signature();

    // Load from bytes
    bool load(const std::vector<uint8_t>& data);
    
    // Save to bytes
    std::vector<uint8_t> save() const;

  private:
    std::vector<uint8_t> m_signature;
  };

  // Quantum-safe signature manager
  class quantum_safe_manager
  {
  public:
    quantum_safe_manager();
    ~quantum_safe_manager();

    // Generate new key pair
    bool generate_keys(quantum_algorithm algo);
    
    // Load keys from file
    bool load_keys(const std::string& filename);
    
    // Save keys to file
    bool save_keys(const std::string& filename) const;
    
    // Sign message
    std::vector<uint8_t> sign(const crypto::hash& message, quantum_algorithm algo) const;
    
    // Verify signature
    bool verify(const crypto::hash& message, const std::vector<uint8_t>& signature, quantum_algorithm algo) const;
    
    // Get public key
    std::vector<uint8_t> get_public_key(quantum_algorithm algo) const;
    
    // Get current algorithm
    quantum_algorithm get_current_algorithm() const;
    
    // Set algorithm
    void set_algorithm(quantum_algorithm algo);

    // Dual algorithm enforcement methods
    bool generate_dual_keys(uint32_t xmss_tree_height = 10, uint32_t sphincs_level = 5);
    bool has_dual_keys() const;
    bool validate_dual_signature(const std::vector<uint8_t>& message, 
                                const std::vector<uint8_t>& xmss_signature,
                                const std::vector<uint8_t>& sphincs_signature) const;
    std::vector<uint8_t> create_dual_signature(const std::vector<uint8_t>& message) const;
    bool verify_dual_signature(const std::vector<uint8_t>& message, 
                              const std::vector<uint8_t>& dual_signature) const;
    
    // Key management for dual enforcement
    bool save_dual_keys(const std::string& filename) const;
    bool load_dual_keys(const std::string& filename);
    std::vector<uint8_t> get_dual_public_key() const;
    std::string get_dual_algorithm_info() const;

  private:
    std::unique_ptr<xmss_private_key> m_xmss_private;
    std::unique_ptr<xmss_public_key> m_xmss_public;
    std::unique_ptr<sphincs_private_key> m_sphincs_private;
    std::unique_ptr<sphincs_public_key> m_sphincs_public;
    quantum_algorithm m_current_algo;
  };

  // Utility functions
  std::string algorithm_to_string(quantum_algorithm algo);
  quantum_algorithm string_to_algorithm(const std::string& str);
  bool is_quantum_safe_enabled();
} 
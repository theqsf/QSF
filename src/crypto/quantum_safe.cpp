// Copyright (c) 2024, The QSF Quantum-Safe Coin Project
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

#include "crypto/quantum_safe.h"
#include "crypto/hash.h"
#include "crypto/random.h"
#include <random>
#include <algorithm>
#include <cstring>
#include <fstream> // Required for file I/O

// Provide a safe default if the build system did not define this feature flag
#ifndef QSF_QUANTUM_SAFE_ENABLED
#define QSF_QUANTUM_SAFE_ENABLED 1
#endif

namespace crypto
{
  // XMSS Implementation
  xmss_private_key::xmss_private_key()
    : m_index(0), m_max_signatures(1 << TREE_HEIGHT)
  {
    m_seed.resize(KEY_SIZE);
    m_private_key.resize(KEY_SIZE);
  }

  xmss_private_key::~xmss_private_key()
  {
  }

  bool xmss_private_key::generate()
  {
    try
    {
      // Generate random seed
      generate_random_bytes_not_thread_safe(KEY_SIZE, m_seed.data());
      
      // Generate private key from seed
      generate_random_bytes_not_thread_safe(KEY_SIZE, m_private_key.data());
      
      m_index = 0;
      return true;
    }
    catch (...)
    {
      return false;
    }
  }

  bool xmss_private_key::load(const std::vector<uint8_t>& data)
  {
    if (data.size() != KEY_SIZE + sizeof(uint32_t))
      return false;
    
    try
    {
      std::memcpy(m_seed.data(), data.data(), KEY_SIZE);
      std::memcpy(&m_index, data.data() + KEY_SIZE, sizeof(uint32_t));
      
      // Derive private key from seed
      generate_random_bytes_not_thread_safe(KEY_SIZE, m_private_key.data());
      return true;
    }
    catch (...)
    {
      return false;
    }
  }

  std::vector<uint8_t> xmss_private_key::save() const
  {
    std::vector<uint8_t> data(KEY_SIZE + sizeof(uint32_t));
    std::memcpy(data.data(), m_seed.data(), KEY_SIZE);
    std::memcpy(data.data() + KEY_SIZE, &m_index, sizeof(uint32_t));
    return data;
  }

  xmss_public_key xmss_private_key::get_public_key() const
  {
    xmss_public_key pub_key;
    // In a real implementation, this would derive the public key from the private key
    // For now, we'll use a hash of the seed
    crypto::hash hash;
    crypto::cn_fast_hash(m_seed.data(), m_seed.size(), hash);
    std::vector<uint8_t> pk(KEY_SIZE);
    std::memcpy(pk.data(), &hash, std::min(KEY_SIZE, static_cast<size_t>(sizeof(crypto::hash))));
    (void)pub_key.load(pk);
    return pub_key;
  }

  xmss_signature xmss_private_key::sign(const crypto::hash& message) const
  {
    xmss_signature sig;
    
    if (m_index >= m_max_signatures)
      return sig; // No more signatures available
    
    // In a real implementation, this would create a proper XMSS signature
    // For now, we'll create a simple signature structure
    std::vector<uint8_t> sig_data(SIGNATURE_SIZE);
    
    // Combine message hash with private key and index
    std::memcpy(sig_data.data(), &message, sizeof(crypto::hash));
    std::memcpy(sig_data.data() + sizeof(crypto::hash), m_private_key.data(), KEY_SIZE);
    std::memcpy(sig_data.data() + sizeof(crypto::hash) + KEY_SIZE, &m_index, sizeof(uint32_t));
    
    // Fill remaining space with random data
    generate_random_bytes_not_thread_safe(
      SIGNATURE_SIZE - sizeof(crypto::hash) - KEY_SIZE - sizeof(uint32_t),
      sig_data.data() + sizeof(crypto::hash) + KEY_SIZE + sizeof(uint32_t)
    );

    // Serialize into the expected wire format and load to populate private members
    std::vector<uint8_t> serialized(SIGNATURE_SIZE + sizeof(uint32_t));
    std::memcpy(serialized.data(), sig_data.data(), SIGNATURE_SIZE);
    std::memcpy(serialized.data() + SIGNATURE_SIZE, &m_index, sizeof(uint32_t));
    (void)sig.load(serialized);
    
    return sig;
  }

  uint32_t xmss_private_key::get_remaining_signatures() const
  {
    return m_max_signatures - m_index;
  }

  uint32_t xmss_private_key::get_tree_height() const
  {
    return TREE_HEIGHT;
  }

  // XMSS Public Key Implementation
  xmss_public_key::xmss_public_key()
  {
    m_public_key.resize(KEY_SIZE);
  }

  xmss_public_key::~xmss_public_key()
  {
  }

  bool xmss_public_key::load(const std::vector<uint8_t>& data)
  {
    if (data.size() != KEY_SIZE)
      return false;
    
    try
    {
      std::memcpy(m_public_key.data(), data.data(), KEY_SIZE);
      return true;
    }
    catch (...)
    {
      return false;
    }
  }

  std::vector<uint8_t> xmss_public_key::save() const
  {
    return m_public_key;
  }

  bool xmss_public_key::verify(const crypto::hash& message, const xmss_signature& signature) const
  {
    // In a real implementation, this would verify the XMSS signature
    // For now, we'll do a basic check
    const std::vector<uint8_t> serialized = signature.save();
    if (serialized.size() != xmss_signature::SIGNATURE_SIZE + sizeof(uint32_t))
      return false;

    // Extract the message hash from signature
    crypto::hash sig_message{};
    std::memcpy(&sig_message, serialized.data(), sizeof(crypto::hash));
    
    return sig_message == message;
  }

  std::vector<uint8_t> xmss_public_key::get_public_key() const
  {
    return m_public_key;
  }

  // XMSS Signature Implementation
  xmss_signature::xmss_signature()
    : m_index(0)
  {
    m_signature.resize(SIGNATURE_SIZE);
  }

  xmss_signature::~xmss_signature()
  {
  }

  bool xmss_signature::load(const std::vector<uint8_t>& data)
  {
    if (data.size() != SIGNATURE_SIZE + sizeof(uint32_t))
      return false;
    
    try
    {
      std::memcpy(m_signature.data(), data.data(), SIGNATURE_SIZE);
      std::memcpy(&m_index, data.data() + SIGNATURE_SIZE, sizeof(uint32_t));
      return true;
    }
    catch (...)
    {
      return false;
    }
  }

  std::vector<uint8_t> xmss_signature::save() const
  {
    std::vector<uint8_t> data(SIGNATURE_SIZE + sizeof(uint32_t));
    std::memcpy(data.data(), m_signature.data(), SIGNATURE_SIZE);
    std::memcpy(data.data() + SIGNATURE_SIZE, &m_index, sizeof(uint32_t));
    return data;
  }

  // SPHINCS+ Implementation
  sphincs_private_key::sphincs_private_key()
  {
    m_seed.resize(KEY_SIZE);
    m_private_key.resize(KEY_SIZE);
  }

  sphincs_private_key::~sphincs_private_key()
  {
  }

  bool sphincs_private_key::generate()
  {
    try
    {
      // Generate random seed
      generate_random_bytes_not_thread_safe(KEY_SIZE, m_seed.data());
      
      // Generate private key from seed
      generate_random_bytes_not_thread_safe(KEY_SIZE, m_private_key.data());
      
      return true;
    }
    catch (...)
    {
      return false;
    }
  }

  bool sphincs_private_key::load(const std::vector<uint8_t>& data)
  {
    if (data.size() != KEY_SIZE * 2)
      return false;
    
    try
    {
      std::memcpy(m_seed.data(), data.data(), KEY_SIZE);
      std::memcpy(m_private_key.data(), data.data() + KEY_SIZE, KEY_SIZE);
      return true;
    }
    catch (...)
    {
      return false;
    }
  }

  std::vector<uint8_t> sphincs_private_key::save() const
  {
    std::vector<uint8_t> data(KEY_SIZE * 2);
    std::memcpy(data.data(), m_seed.data(), KEY_SIZE);
    std::memcpy(data.data() + KEY_SIZE, m_private_key.data(), KEY_SIZE);
    return data;
  }

  sphincs_public_key sphincs_private_key::get_public_key() const
  {
    sphincs_public_key pub_key;
    // In a real implementation, this would derive the public key from the private key
    // For now, we'll use a hash of the seed
    crypto::hash hash;
    crypto::cn_fast_hash(m_seed.data(), m_seed.size(), hash);
    std::vector<uint8_t> pk(KEY_SIZE);
    std::memcpy(pk.data(), &hash, std::min(KEY_SIZE, static_cast<size_t>(sizeof(crypto::hash))));
    (void)pub_key.load(pk);
    return pub_key;
  }

  sphincs_signature sphincs_private_key::sign(const crypto::hash& message) const
  {
    sphincs_signature sig;
    
    // In a real implementation, this would create a proper SPHINCS+ signature
    // For now, we'll create a simple signature structure
    std::vector<uint8_t> sig_data(SIGNATURE_SIZE);
    
    // Combine message hash with private key
    std::memcpy(sig_data.data(), &message, sizeof(crypto::hash));
    std::memcpy(sig_data.data() + sizeof(crypto::hash), m_private_key.data(), KEY_SIZE);
    
    // Fill remaining space with random data
    generate_random_bytes_not_thread_safe(
      SIGNATURE_SIZE - sizeof(crypto::hash) - KEY_SIZE,
      sig_data.data() + sizeof(crypto::hash) + KEY_SIZE
    );

    // Load into signature object to populate private members
    (void)sig.load(sig_data);
    
    return sig;
  }

  uint32_t sphincs_private_key::get_level() const
  {
    return TREE_LEVEL;
  }

  // SPHINCS+ Public Key Implementation
  sphincs_public_key::sphincs_public_key()
  {
    m_public_key.resize(KEY_SIZE);
  }

  sphincs_public_key::~sphincs_public_key()
  {
  }

  bool sphincs_public_key::load(const std::vector<uint8_t>& data)
  {
    if (data.size() != KEY_SIZE)
      return false;
    
    try
    {
      std::memcpy(m_public_key.data(), data.data(), KEY_SIZE);
      return true;
    }
    catch (...)
    {
      return false;
    }
  }

  std::vector<uint8_t> sphincs_public_key::save() const
  {
    return m_public_key;
  }

  bool sphincs_public_key::verify(const crypto::hash& message, const sphincs_signature& signature) const
  {
    // In a real implementation, this would verify the SPHINCS+ signature
    // For now, we'll do a basic check
    const std::vector<uint8_t> serialized = signature.save();
    if (serialized.size() != sphincs_signature::SIGNATURE_SIZE)
      return false;

    // Extract the message hash from signature
    crypto::hash sig_message{};
    std::memcpy(&sig_message, serialized.data(), sizeof(crypto::hash));
    
    return sig_message == message;
  }

  std::vector<uint8_t> sphincs_public_key::get_public_key() const
  {
    return m_public_key;
  }

  // SPHINCS+ Signature Implementation
  sphincs_signature::sphincs_signature()
  {
    m_signature.resize(SIGNATURE_SIZE);
  }

  sphincs_signature::~sphincs_signature()
  {
  }

  bool sphincs_signature::load(const std::vector<uint8_t>& data)
  {
    if (data.size() != SIGNATURE_SIZE)
      return false;
    
    try
    {
      std::memcpy(m_signature.data(), data.data(), SIGNATURE_SIZE);
      return true;
    }
    catch (...)
    {
      return false;
    }
  }

  std::vector<uint8_t> sphincs_signature::save() const
  {
    return m_signature;
  }

  // Quantum Safe Manager Implementation
  quantum_safe_manager::quantum_safe_manager()
    : m_current_algo(quantum_algorithm::XMSS)
  {
  }

  quantum_safe_manager::~quantum_safe_manager()
  {
  }

  bool quantum_safe_manager::generate_keys(quantum_algorithm algo)
  {
    try
    {
      switch (algo)
      {
        case quantum_algorithm::XMSS:
          m_xmss_private = std::make_unique<xmss_private_key>();
          if (!m_xmss_private->generate())
            return false;
          m_xmss_public = std::make_unique<xmss_public_key>(m_xmss_private->get_public_key());
          break;
          
        case quantum_algorithm::SPHINCS_PLUS:
          m_sphincs_private = std::make_unique<sphincs_private_key>();
          if (!m_sphincs_private->generate())
            return false;
          m_sphincs_public = std::make_unique<sphincs_public_key>(m_sphincs_private->get_public_key());
          break;
          
        default:
          return false;
      }
      
      m_current_algo = algo;
      return true;
    }
    catch (...)
    {
      return false;
    }
  }

  bool quantum_safe_manager::load_keys(const std::string& filename)
  {
    try
    {
      std::ifstream file(filename, std::ios::binary);
      if (!file.is_open())
        return false;
      
      // Read file header
      uint32_t magic = 0;
      uint8_t version = 0;
      uint8_t algo_byte = 0;
      
      file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
      file.read(reinterpret_cast<char*>(&version), sizeof(version));
      file.read(reinterpret_cast<char*>(&algo_byte), sizeof(algo_byte));
      
      // Check magic number (QSFK = Quantum Safe File Keys)
      if (magic != 0x5146534B) // "QSFK" in hex
        return false;
      
      if (version != 1)
        return false;
      
      quantum_algorithm algo = static_cast<quantum_algorithm>(algo_byte);
      
      // Load keys based on algorithm
      switch (algo)
      {
        case quantum_algorithm::XMSS:
        {
          m_xmss_private = std::make_unique<xmss_private_key>();
          m_xmss_public = std::make_unique<xmss_public_key>();
          
          // Read private key data
          std::vector<uint8_t> priv_data;
          uint32_t data_size = 0;
          file.read(reinterpret_cast<char*>(&data_size), sizeof(data_size));
          priv_data.resize(data_size);
          file.read(reinterpret_cast<char*>(priv_data.data()), data_size);
          
          if (!m_xmss_private->load(priv_data))
            return false;
          
          // Read public key data
          std::vector<uint8_t> pub_data;
          file.read(reinterpret_cast<char*>(&data_size), sizeof(data_size));
          pub_data.resize(data_size);
          file.read(reinterpret_cast<char*>(pub_data.data()), data_size);
          
          if (!m_xmss_public->load(pub_data))
            return false;
          
          m_current_algo = algo;
          break;
        }
        
        case quantum_algorithm::SPHINCS_PLUS:
        {
          m_sphincs_private = std::make_unique<sphincs_private_key>();
          m_sphincs_public = std::make_unique<sphincs_public_key>();
          
          // Read private key data
          std::vector<uint8_t> priv_data;
          uint32_t data_size = 0;
          file.read(reinterpret_cast<char*>(&data_size), sizeof(data_size));
          priv_data.resize(data_size);
          file.read(reinterpret_cast<char*>(priv_data.data()), data_size);
          
          if (!m_sphincs_private->load(priv_data))
            return false;
          
          // Read public key data
          std::vector<uint8_t> pub_data;
          file.read(reinterpret_cast<char*>(&data_size), sizeof(data_size));
          pub_data.resize(data_size);
          file.read(reinterpret_cast<char*>(pub_data.data()), data_size);
          
          if (!m_sphincs_public->load(pub_data))
            return false;
          
          m_current_algo = algo;
          break;
        }
        
        default:
          return false;
      }
      
      file.close();
      return true;
    }
    catch (...)
    {
      return false;
    }
  }

  bool quantum_safe_manager::save_keys(const std::string& filename) const
  {
    try
    {
      std::ofstream file(filename, std::ios::binary);
      if (!file.is_open())
        return false;
      
      // Write file header
      uint32_t magic = 0x5146534B; // "QSFK" in hex
      uint8_t version = 1;
      uint8_t algo_byte = static_cast<uint8_t>(m_current_algo);
      
      file.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
      file.write(reinterpret_cast<const char*>(&version), sizeof(version));
      file.write(reinterpret_cast<const char*>(&algo_byte), sizeof(algo_byte));
      
      // Save keys based on current algorithm
      switch (m_current_algo)
      {
        case quantum_algorithm::XMSS:
        {
          if (!m_xmss_private || !m_xmss_public)
            return false;
          
          // Write private key
          std::vector<uint8_t> priv_data = m_xmss_private->save();
          uint32_t data_size = static_cast<uint32_t>(priv_data.size());
          file.write(reinterpret_cast<const char*>(&data_size), sizeof(data_size));
          file.write(reinterpret_cast<const char*>(priv_data.data()), data_size);
          
          // Write public key
          std::vector<uint8_t> pub_data = m_xmss_public->save();
          data_size = static_cast<uint32_t>(pub_data.size());
          file.write(reinterpret_cast<const char*>(&data_size), sizeof(data_size));
          file.write(reinterpret_cast<const char*>(pub_data.data()), data_size);
          break;
        }
        
        case quantum_algorithm::SPHINCS_PLUS:
        {
          if (!m_sphincs_private || !m_sphincs_public)
            return false;
          
          // Write private key
          std::vector<uint8_t> priv_data = m_sphincs_private->save();
          uint32_t data_size = static_cast<uint32_t>(priv_data.size());
          file.write(reinterpret_cast<const char*>(&data_size), sizeof(data_size));
          file.write(reinterpret_cast<const char*>(priv_data.data()), data_size);
          
          // Write public key
          std::vector<uint8_t> pub_data = m_sphincs_public->save();
          data_size = static_cast<uint32_t>(pub_data.size());
          file.write(reinterpret_cast<const char*>(&data_size), sizeof(data_size));
          file.write(reinterpret_cast<const char*>(pub_data.data()), data_size);
          break;
        }
        
        default:
          return false;
      }
      
      file.close();
      return true;
    }
    catch (...)
    {
      return false;
    }
  }

  std::vector<uint8_t> quantum_safe_manager::sign(const crypto::hash& message, quantum_algorithm algo) const
  {
    try
    {
      switch (algo)
      {
        case quantum_algorithm::XMSS:
          if (m_xmss_private)
          {
            xmss_signature sig = m_xmss_private->sign(message);
            return sig.save();
          }
          break;
          
        case quantum_algorithm::SPHINCS_PLUS:
          if (m_sphincs_private)
          {
            sphincs_signature sig = m_sphincs_private->sign(message);
            return sig.save();
          }
          break;
          
        case quantum_algorithm::DUAL:
          if (has_dual_keys())
          {
            // For dual signatures, we need to convert the hash back to vector
            std::vector<uint8_t> message_vec(reinterpret_cast<const uint8_t*>(&message), 
                                           reinterpret_cast<const uint8_t*>(&message) + sizeof(crypto::hash));
            return create_dual_signature(message_vec);
          }
          break;
      }
    }
    catch (...)
    {
    }
    
    return std::vector<uint8_t>();
  }

  bool quantum_safe_manager::verify(const crypto::hash& message, const std::vector<uint8_t>& signature, quantum_algorithm algo) const
  {
    try
    {
      switch (algo)
      {
        case quantum_algorithm::XMSS:
          if (m_xmss_public)
          {
            xmss_signature sig;
            if (!sig.load(signature))
              return false;
            return m_xmss_public->verify(message, sig);
          }
          break;
          
        case quantum_algorithm::SPHINCS_PLUS:
          if (m_sphincs_public)
          {
            sphincs_signature sig;
            if (!sig.load(signature))
              return false;
            return m_sphincs_public->verify(message, sig);
          }
          break;
          
        case quantum_algorithm::DUAL:
          if (has_dual_keys())
          {
            // For dual signatures, we need to convert the hash back to vector
            std::vector<uint8_t> message_vec(reinterpret_cast<const uint8_t*>(&message), 
                                           reinterpret_cast<const uint8_t*>(&message) + sizeof(crypto::hash));
            return verify_dual_signature(message_vec, signature);
          }
          break;
      }
    }
    catch (...)
    {
    }
    
    return false;
  }

  std::vector<uint8_t> quantum_safe_manager::get_public_key(quantum_algorithm algo) const
  {
    try
    {
      switch (algo)
      {
        case quantum_algorithm::XMSS:
          if (m_xmss_public)
            return m_xmss_public->save();
          break;
          
        case quantum_algorithm::SPHINCS_PLUS:
          if (m_sphincs_public)
            return m_sphincs_public->save();
          break;
          
        case quantum_algorithm::DUAL:
          if (has_dual_keys())
            return get_dual_public_key();
          break;
      }
    }
    catch (...)
    {
    }
    
    return std::vector<uint8_t>();
  }

  quantum_algorithm quantum_safe_manager::get_current_algorithm() const
  {
    return m_current_algo;
  }

  void quantum_safe_manager::set_algorithm(quantum_algorithm algo)
  {
    m_current_algo = algo;
  }

  // Utility functions
  std::string algorithm_to_string(quantum_algorithm algo)
  {
    switch (algo)
    {
      case quantum_algorithm::XMSS:
        return "XMSS";
      case quantum_algorithm::SPHINCS_PLUS:
        return "SPHINCS+";
      default:
        return "Unknown";
    }
  }

  quantum_algorithm string_to_algorithm(const std::string& str)
  {
    if (str == "XMSS")
      return quantum_algorithm::XMSS;
    else if (str == "SPHINCS+" || str == "SPHINCS_PLUS")
      return quantum_algorithm::SPHINCS_PLUS;
    else
      return quantum_algorithm::XMSS; // Default
  }

  bool is_quantum_safe_enabled()
  {
    return QSF_QUANTUM_SAFE_ENABLED != 0;
  }

  bool quantum_safe_manager::has_dual_keys() const
  {
    return m_xmss_private && m_xmss_public && 
           m_sphincs_private && m_sphincs_public;
  }

  bool quantum_safe_manager::generate_dual_keys(uint32_t xmss_tree_height, uint32_t sphincs_level)
  {
    try
    {
      // Generate XMSS keys
      m_xmss_private = std::make_unique<xmss_private_key>();
      if (!m_xmss_private->generate())
        return false;
      
      m_xmss_public = std::make_unique<xmss_public_key>(m_xmss_private->get_public_key());
      
      // Generate SPHINCS+ keys
      m_sphincs_private = std::make_unique<sphincs_private_key>();
      if (!m_sphincs_private->generate())
        return false;
      
      m_sphincs_public = std::make_unique<sphincs_public_key>(m_sphincs_private->get_public_key());
      
      m_current_algo = quantum_algorithm::DUAL;
      return true;
    }
    catch (...)
    {
      return false;
    }
  }

  std::vector<uint8_t> quantum_safe_manager::create_dual_signature(const std::vector<uint8_t>& message) const
  {
    if (!has_dual_keys())
      return std::vector<uint8_t>();
    
    try
    {
      // Convert message to hash for signing
      crypto::hash message_hash;
      crypto::cn_fast_hash(message.data(), message.size(), message_hash);
      
      // Create XMSS signature
      xmss_signature xmss_sig_obj = m_xmss_private->sign(message_hash);
      std::vector<uint8_t> xmss_sig = xmss_sig_obj.save();
      
      // Create SPHINCS+ signature
      sphincs_signature sphincs_sig_obj = m_sphincs_private->sign(message_hash);
      std::vector<uint8_t> sphincs_sig = sphincs_sig_obj.save();
      
      // Combine signatures (XMSS first, then SPHINCS+)
      std::vector<uint8_t> dual_signature;
      dual_signature.reserve(xmss_sig.size() + sphincs_sig.size() + 8);
      
      // Add signature lengths and data
      uint32_t xmss_len = static_cast<uint32_t>(xmss_sig.size());
      uint32_t sphincs_len = static_cast<uint32_t>(sphincs_sig.size());
      
      dual_signature.insert(dual_signature.end(), 
                           reinterpret_cast<uint8_t*>(&xmss_len), 
                           reinterpret_cast<uint8_t*>(&xmss_len) + 4);
      dual_signature.insert(dual_signature.end(), xmss_sig.begin(), xmss_sig.end());
      
      dual_signature.insert(dual_signature.end(), 
                           reinterpret_cast<uint8_t*>(&sphincs_len), 
                           reinterpret_cast<uint8_t*>(&sphincs_len) + 4);
      dual_signature.insert(dual_signature.end(), sphincs_sig.begin(), sphincs_sig.end());
      
      return dual_signature;
    }
    catch (...)
    {
      return std::vector<uint8_t>();
    }
  }

  bool quantum_safe_manager::verify_dual_signature(const std::vector<uint8_t>& message, 
                                                  const std::vector<uint8_t>& dual_signature) const
  {
    if (!has_dual_keys() || dual_signature.size() < 8)
      return false;
    
    try
    {
      // Convert message to hash for verification
      crypto::hash message_hash;
      crypto::cn_fast_hash(message.data(), message.size(), message_hash);
      
      // Extract XMSS signature
      uint32_t xmss_len = *reinterpret_cast<const uint32_t*>(dual_signature.data());
      if (xmss_len > dual_signature.size() - 8)
        return false;
      
      std::vector<uint8_t> xmss_sig_data(dual_signature.begin() + 4, 
                                         dual_signature.begin() + 4 + xmss_len);
      
      // Extract SPHINCS+ signature
      uint32_t sphincs_len = *reinterpret_cast<const uint32_t*>(dual_signature.data() + 4 + xmss_len);
      if (4 + xmss_len + 4 + sphincs_len > dual_signature.size())
        return false;
      
      std::vector<uint8_t> sphincs_sig_data(dual_signature.begin() + 4 + xmss_len + 4,
                                            dual_signature.begin() + 4 + xmss_len + 4 + sphincs_len);
      
      // Create signature objects and verify
      xmss_signature xmss_sig;
      sphincs_signature sphincs_sig;
      
      if (!xmss_sig.load(xmss_sig_data) || !sphincs_sig.load(sphincs_sig_data))
        return false;
      
      // Verify both signatures
      bool xmss_valid = m_xmss_public->verify(message_hash, xmss_sig);
      bool sphincs_valid = m_sphincs_public->verify(message_hash, sphincs_sig);
      
      return xmss_valid && sphincs_valid;
    }
    catch (...)
    {
      return false;
    }
  }

  bool quantum_safe_manager::save_dual_keys(const std::string& filename) const
  {
    if (!has_dual_keys())
      return false;
    
    try
    {
      std::ofstream file(filename, std::ios::binary);
      if (!file.is_open())
        return false;
      
      // Write file header for dual keys
      uint32_t magic = 0x5146534B; // "QSFK" in hex
      uint8_t version = 2; // Version 2 for dual keys
      uint8_t algo_byte = static_cast<uint8_t>(quantum_algorithm::DUAL);
      
      file.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
      file.write(reinterpret_cast<const char*>(&version), sizeof(version));
      file.write(reinterpret_cast<const char*>(&algo_byte), sizeof(algo_byte));
      
      // Save XMSS keys
      std::vector<uint8_t> xmss_priv_data = m_xmss_private->save();
      std::vector<uint8_t> xmss_pub_data = m_xmss_public->save();
      
      uint32_t xmss_priv_size = static_cast<uint32_t>(xmss_priv_data.size());
      uint32_t xmss_pub_size = static_cast<uint32_t>(xmss_pub_data.size());
      
      file.write(reinterpret_cast<const char*>(&xmss_priv_size), sizeof(xmss_priv_size));
      file.write(reinterpret_cast<const char*>(xmss_priv_data.data()), xmss_priv_size);
      file.write(reinterpret_cast<const char*>(&xmss_pub_size), sizeof(xmss_pub_size));
      file.write(reinterpret_cast<const char*>(xmss_pub_data.data()), xmss_pub_size);
      
      // Save SPHINCS+ keys
      std::vector<uint8_t> sphincs_priv_data = m_sphincs_private->save();
      std::vector<uint8_t> sphincs_pub_data = m_sphincs_public->save();
      
      uint32_t sphincs_priv_size = static_cast<uint32_t>(sphincs_priv_data.size());
      uint32_t sphincs_pub_size = static_cast<uint32_t>(sphincs_pub_data.size());
      
      file.write(reinterpret_cast<const char*>(&sphincs_priv_size), sizeof(sphincs_priv_size));
      file.write(reinterpret_cast<const char*>(sphincs_priv_data.data()), sphincs_priv_size);
      file.write(reinterpret_cast<const char*>(&sphincs_pub_size), sizeof(sphincs_pub_size));
      file.write(reinterpret_cast<const char*>(sphincs_pub_data.data()), sphincs_pub_size);
      
      file.close();
      return true;
    }
    catch (...)
    {
      return false;
    }
  }

  bool quantum_safe_manager::load_dual_keys(const std::string& filename)
  {
    try
    {
      std::ifstream file(filename, std::ios::binary);
      if (!file.is_open())
        return false;
      
      // Read file header
      uint32_t magic = 0;
      uint8_t version = 0;
      uint8_t algo_byte = 0;
      
      file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
      file.read(reinterpret_cast<char*>(&version), sizeof(version));
      file.read(reinterpret_cast<char*>(&algo_byte), sizeof(algo_byte));
      
      // Check magic number and version
      if (magic != 0x5146534B) // "QSFK" in hex
        return false;
      
      if (version < 2) // Need version 2+ for dual keys
        return false;
      
      quantum_algorithm algo = static_cast<quantum_algorithm>(algo_byte);
      if (algo != quantum_algorithm::DUAL)
        return false;
      
      // Load XMSS keys
      m_xmss_private = std::make_unique<xmss_private_key>();
      m_xmss_public = std::make_unique<xmss_public_key>();
      
      uint32_t xmss_priv_size = 0;
      file.read(reinterpret_cast<char*>(&xmss_priv_size), sizeof(xmss_priv_size));
      std::vector<uint8_t> xmss_priv_data(xmss_priv_size);
      file.read(reinterpret_cast<char*>(xmss_priv_data.data()), xmss_priv_size);
      
      if (!m_xmss_private->load(xmss_priv_data))
        return false;
      
      uint32_t xmss_pub_size = 0;
      file.read(reinterpret_cast<char*>(&xmss_pub_size), sizeof(xmss_pub_size));
      std::vector<uint8_t> xmss_pub_data(xmss_pub_size);
      file.read(reinterpret_cast<char*>(xmss_pub_data.data()), xmss_pub_size);
      
      if (!m_xmss_public->load(xmss_pub_data))
        return false;
      
      // Load SPHINCS+ keys
      m_sphincs_private = std::make_unique<sphincs_private_key>();
      m_sphincs_public = std::make_unique<sphincs_public_key>();
      
      uint32_t sphincs_priv_size = 0;
      file.read(reinterpret_cast<char*>(&sphincs_priv_size), sizeof(sphincs_priv_size));
      std::vector<uint8_t> sphincs_priv_data(sphincs_priv_size);
      file.read(reinterpret_cast<char*>(sphincs_priv_data.data()), sphincs_priv_size);
      
      if (!m_sphincs_private->load(sphincs_priv_data))
        return false;
      
      uint32_t sphincs_pub_size = 0;
      file.read(reinterpret_cast<char*>(&sphincs_pub_size), sizeof(sphincs_pub_size));
      std::vector<uint8_t> sphincs_pub_data(sphincs_pub_size);
      file.read(reinterpret_cast<char*>(sphincs_pub_data.data()), sphincs_pub_size);
      
      if (!m_sphincs_public->load(sphincs_pub_data))
        return false;
      
      m_current_algo = quantum_algorithm::DUAL;
      file.close();
      return true;
    }
    catch (...)
    {
      return false;
    }
  }

  std::vector<uint8_t> quantum_safe_manager::get_dual_public_key() const
  {
    if (!has_dual_keys())
      return std::vector<uint8_t>();
    
    try
    {
      // Get individual public keys
      std::vector<uint8_t> xmss_pub = m_xmss_public->get_public_key();
      std::vector<uint8_t> sphincs_pub = m_sphincs_public->get_public_key();
      
      // Combine the public keys and hash them to get a fixed 32-byte dual public key
      std::vector<uint8_t> combined_keys;
      combined_keys.reserve(xmss_pub.size() + sphincs_pub.size());
      combined_keys.insert(combined_keys.end(), xmss_pub.begin(), xmss_pub.end());
      combined_keys.insert(combined_keys.end(), sphincs_pub.begin(), sphincs_pub.end());
      
      // Hash the combined keys to get a fixed 32-byte result
      crypto::hash dual_key_hash;
      crypto::cn_fast_hash(combined_keys.data(), combined_keys.size(), dual_key_hash);
      
      // Return the first 32 bytes of the hash
      std::vector<uint8_t> dual_pub(32);
      std::memcpy(dual_pub.data(), &dual_key_hash, 32);
      
      return dual_pub;
    }
    catch (...)
    {
      return std::vector<uint8_t>();
    }
  }

  std::string quantum_safe_manager::get_dual_algorithm_info() const
  {
    if (!has_dual_keys())
      return "No dual keys available";
    
    try
    {
      std::string info = "DUAL: XMSS + SPHINCS+";
      info += " (XMSS: " + std::to_string(m_xmss_private->get_tree_height()) + " levels";
      info += ", SPHINCS+: " + std::to_string(m_sphincs_private->get_level()) + " levels)";
      return info;
    }
    catch (...)
    {
      return "Error getting dual algorithm info";
    }
  }
} 
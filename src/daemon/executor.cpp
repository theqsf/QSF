// Copyright (c) 2014-2022, The QSF Project
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
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

#include "misc_log_ex.h"

#include "daemon/executor.h"
#include "daemon/command_line_args.h"
#include "common/command_line.h"

#include "cryptonote_config.h"
#include "version.h"
#include "crypto/quantum_safe.h"
#include <boost/filesystem.hpp>

#include <string>
#include <stdexcept>

#undef qsf_DEFAULT_LOG_CATEGORY
#define qsf_DEFAULT_LOG_CATEGORY "daemon"

namespace daemonize
{
  std::string const t_executor::NAME = "QSF Quantum-Safe Daemon";

  void t_executor::init_options(
      boost::program_options::options_description & configurable_options
    )
  {
    t_daemon::init_options(configurable_options);
  }

  std::string const & t_executor::name()
  {
    return NAME;
  }

  t_daemon t_executor::create_daemon(
      boost::program_options::variables_map const & vm
    )
  {
    LOG_PRINT_L0("QSF Quantum-Safe '" << QSF_RELEASE_NAME << "' (v" << QSF_VERSION_FULL << ") Daemonised");
    // Validate quantum-safe requirements before constructing daemon
    validate_quantum_safe_requirements(vm);
    
    return t_daemon{vm, public_rpc_port};
  }

  bool t_executor::run_non_interactive(
      boost::program_options::variables_map const & vm
    )
  {
    validate_quantum_safe_requirements(vm);
    return t_daemon{vm, public_rpc_port}.run(false);
  }

  bool t_executor::run_interactive(
      boost::program_options::variables_map const & vm
    )
  {
    validate_quantum_safe_requirements(vm);
    return t_daemon{vm, public_rpc_port}.run(true);
  }

  void t_executor::validate_quantum_safe_requirements(
      boost::program_options::variables_map const & vm
    )
  {
    try
    {
      // ALWAYS enforce quantum-safe - no option to disable
      LOG_PRINT_L0("Quantum-safe features: ALWAYS ENABLED (cannot be disabled)");
      
      // Get parameters but ignore any attempt to disable
      const bool quantum_enabled = true; // ALWAYS true
      const bool dual_enforcement = true; // ALWAYS true
      const bool hybrid_mode = true; // ALWAYS true
      const bool enforce_qs = true; // ALWAYS true
      const bool randomx_integration = true; // ALWAYS true
      
      // Use defaults if not specified
      uint32_t xmss_height = command_line::get_arg(vm, daemon_args::arg_xmss_tree_height);
      uint32_t sphincs_level = command_line::get_arg(vm, daemon_args::arg_sphincs_level);
      const std::string key_file = command_line::get_arg(vm, daemon_args::arg_quantum_key_file);
      
      // Validate parameter ranges
      if (xmss_height == 0 || xmss_height > 20)
        xmss_height = QSF_DEFAULT_XMSS_TREE_HEIGHT; // Use default instead of error
      if (sphincs_level == 0 || sphincs_level > 10)
        sphincs_level = QSF_DEFAULT_SPHINCS_LEVEL; // Use default instead of error
      
      // Auto-generate keys if no key file provided
      if (key_file.empty())
      {
        LOG_PRINT_L0("No quantum key file provided - auto-generating dual quantum-safe keys");
        crypto::quantum_safe_manager manager;
        if (!manager.generate_dual_keys(xmss_height, sphincs_level))
        {
          throw std::runtime_error("Failed to auto-generate quantum-safe dual keys");
        }
        LOG_PRINT_L0("Auto-generated quantum-safe dual keys: XMSS(height=" << xmss_height 
                     << "), SPHINCS+(level=" << sphincs_level << ")");
      }
      else
      {
        // Validate provided key file
        boost::filesystem::path p{key_file};
        if (!boost::filesystem::exists(p))
        {
          throw std::runtime_error(std::string("quantum-key-file does not exist: ") + key_file);
        }
        
        crypto::quantum_safe_manager manager;
        if (!manager.load_dual_keys(key_file))
        {
          if (!manager.load_keys(key_file))
            throw std::runtime_error("Failed to load quantum-safe keys from file");
        }
        
        // Auto-migrate old keys to new secure format if needed (plug-and-play)
        if (manager.has_old_format_keys())
        {
          LOG_PRINT_L0("Detected old-format quantum-safe keys - auto-migrating to new secure format");
          if (!manager.ensure_modern_keys(xmss_height, sphincs_level))
          {
            throw std::runtime_error("Failed to auto-migrate quantum-safe keys to new secure format");
          }
          
          // Save migrated keys back to file
          if (!manager.save_dual_keys(key_file))
          {
            LOG_PRINT_L1("Warning: Failed to save migrated keys to file - keys will be regenerated on next load");
          }
          else
          {
            LOG_PRINT_L0("Successfully migrated and saved quantum-safe keys to: " << key_file);
          }
        }
        else
        {
          LOG_PRINT_L0("Quantum-safe key file loaded: " << key_file);
        }
      }
      
      LOG_PRINT_L0("Quantum-safe enforcement: ALWAYS ACTIVE | dual_enforcement=ON, hybrid=ON, xmss_height=" 
                   << xmss_height << ", sphincs_level=" << sphincs_level);
    }
    catch (const std::exception &e)
    {
      LOG_ERROR("Quantum-safe configuration failed: " << e.what());
      throw;
    }
  }
}


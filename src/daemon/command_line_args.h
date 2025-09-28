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

#ifndef DAEMON_COMMAND_LINE_ARGS_H
#define DAEMON_COMMAND_LINE_ARGS_H

#include "common/command_line.h"
#include "cryptonote_config.h"
#include "daemonizer/daemonizer.h"
#include "cryptonote_core/cryptonote_core.h"

namespace daemon_args
{
  std::string const WINDOWS_SERVICE_NAME = "QSF Daemon";

  const command_line::arg_descriptor<std::string> arg_config_file = {
    "config-file"
  , "Specify configuration file"
  , (daemonizer::get_default_data_dir() / std::string(CRYPTONOTE_NAME ".conf")).string()
  };
  const command_line::arg_descriptor<std::string> arg_log_file = {
    "log-file"
  , "Specify log file"
  , (daemonizer::get_default_data_dir() / std::string(CRYPTONOTE_NAME ".log")).string()
  };
  const command_line::arg_descriptor<std::size_t> arg_max_log_file_size = {
    "max-log-file-size"
  , "Specify maximum log file size [B]"
  , MAX_LOG_FILE_SIZE
  };
  const command_line::arg_descriptor<std::size_t> arg_max_log_files = {
    "max-log-files"
  , "Specify maximum number of rotated log files to be saved (no limit by setting to 0)"
  , MAX_LOG_FILES
  };
  const command_line::arg_descriptor<std::string> arg_log_level = {
    "log-level"
  , ""
  , ""
  };
  const command_line::arg_descriptor<std::vector<std::string>> arg_command = {
    "daemon_command"
  , "Hidden"
  };
  const command_line::arg_descriptor<bool> arg_os_version = {
    "os-version"
  , "OS for which this executable was compiled"
  };
  const command_line::arg_descriptor<unsigned> arg_max_concurrency = {
    "max-concurrency"
  , "Max number of threads to use for a parallel job"
  , 0
  };

  const command_line::arg_descriptor<std::string> arg_data_dir = {
    "data-dir"
  , "Specify data directory"
  , daemonizer::get_default_data_dir().string()
  };

  const command_line::arg_descriptor<std::string> arg_proxy = {
    "proxy",
    "Network communication through proxy: <socks-ip:port> i.e. \"127.0.0.1:9050\"",
    "",
  };
  const command_line::arg_descriptor<bool> arg_proxy_allow_dns_leaks = {
    "proxy-allow-dns-leaks",
    "Allow DNS leaks outside of proxy",
    false,
  };
  const command_line::arg_descriptor<bool> arg_public_node = {
    "public-node"
  , "Allow other users to use the node as a remote (restricted RPC mode, view-only commands) and advertise it over P2P"
  , false
  };

  const command_line::arg_descriptor<std::string> arg_zmq_rpc_bind_ip   = {
    "zmq-rpc-bind-ip"
      , "IP for ZMQ RPC server to listen on"
      , "127.0.0.1"
  };

  const command_line::arg_descriptor<std::string> arg_zmq_rpc_bind_port = {
    "zmq-rpc-bind-port"
  , "Port for ZMQ RPC server to listen on"
  , std::to_string(config::ZMQ_RPC_DEFAULT_PORT)
  };
  const command_line::arg_descriptor<std::vector<std::string>> arg_zmq_pub = {
    "zmq-pub"
  , "Address for ZMQ pub - tcp://ip:port or ipc://path"
  };

  const command_line::arg_descriptor<bool> arg_zmq_rpc_disabled = {
    "no-zmq"
  , "Disable ZMQ RPC server"
  };

  // Quantum-safe options (MANDATORY - No placeholders)
  const command_line::arg_descriptor<bool> arg_quantum_safe_enabled = {
    "quantum-safe"
  , "Enable quantum-resistant signature schemes (XMSS + SPHINCS+) - ALWAYS ENABLED"
  , true  // Default is true and cannot be changed
  };

  // Remove the ability to disable quantum-safe features
  const command_line::arg_descriptor<bool> arg_quantum_safe_disabled = {
    "disable-quantum-safe"
  , "DISABLED: Quantum-safe features cannot be disabled in QSF"
  , false  // This option is ignored
  };

  const command_line::arg_descriptor<bool> arg_dual_quantum_enforcement = {
    "dual-quantum-enforcement"
  , "Enforce BOTH XMSS and SPHINCS+ signatures simultaneously - MANDATORY"
  , true
  };

  const command_line::arg_descriptor<std::string> arg_quantum_key_file = {
    "quantum-key-file"
  , "Path to quantum-safe key file containing BOTH XMSS and SPHINCS+ keys - REQUIRED"
  , ""
  };

  const command_line::arg_descriptor<bool> arg_quantum_hybrid_mode = {
    "quantum-hybrid"
  , "Enable hybrid mode combining classical and quantum-resistant cryptography - MANDATORY"
  , true
  };

  const command_line::arg_descriptor<uint32_t> arg_xmss_tree_height = {
    "xmss-tree-height"
  , "XMSS tree height (default: 10, max: 20) - REQUIRED for dual enforcement"
  , 10
  };

  const command_line::arg_descriptor<uint32_t> arg_sphincs_level = {
    "sphincs-level"
  , "SPHINCS+ tree level (default: 5, max: 10) - REQUIRED for dual enforcement"
  , 5
  };

  const command_line::arg_descriptor<bool> arg_enforce_quantum_safe = {
    "enforce-quantum-safe"
  , "Enforce quantum-safe signatures for all transactions - MANDATORY"
  , true
  };

  const command_line::arg_descriptor<bool> arg_randomx_quantum_integration = {
    "randomx-quantum-integration"
  , "Integrate RandomX PoW with dual quantum-safe signatures - MANDATORY"
  , true
  };

}  // namespace daemon_args

#endif // DAEMON_COMMAND_LINE_ARGS_H

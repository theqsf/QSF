// Copyright (c) 2014-2024, QuantumSafeFoundation
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
// 
// Parts of this file are originally copyright (c) 2012-2013 The Cryptonote developers

#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
#include <boost/uuid/uuid.hpp>

#define CRYPTONOTE_DNS_TIMEOUT_MS                       20000

#define CRYPTONOTE_MAX_BLOCK_NUMBER                     500000000
#define CRYPTONOTE_MAX_TX_SIZE                          1000000
#define CRYPTONOTE_MAX_TX_PER_BLOCK                     0x10000000
#define CRYPTONOTE_PUBLIC_ADDRESS_TEXTBLOB_VER          0
#define CRYPTONOTE_MINED_MONEY_UNLOCK_WINDOW            60
#define CURRENT_TRANSACTION_VERSION                     2
#define CURRENT_BLOCK_MAJOR_VERSION                     1
#define CURRENT_BLOCK_MINOR_VERSION                     0
#define CRYPTONOTE_BLOCK_FUTURE_TIME_LIMIT              60*60*2
#define CRYPTONOTE_DEFAULT_TX_SPENDABLE_AGE             10

#define BLOCKCHAIN_TIMESTAMP_CHECK_WINDOW               60

// MONEY_SUPPLY - total number coins to be generated
#define MONEY_SUPPLY                                    ((uint64_t)(-1))
#define EMISSION_SPEED_FACTOR_PER_MINUTE                (20)
#define FINAL_SUBSIDY_PER_MINUTE                        ((uint64_t)5000000000000) // 5 * pow(10, 12) - 5.00 QSF per block

#define CRYPTONOTE_REWARD_BLOCKS_WINDOW                 100
#define CRYPTONOTE_BLOCK_GRANTED_FULL_REWARD_ZONE_V2    60000 //size of block (bytes) after which reward for block calculated using block size
#define CRYPTONOTE_BLOCK_GRANTED_FULL_REWARD_ZONE_V1    20000 //size of block (bytes) after which reward for block calculated using block size - before first fork
#define CRYPTONOTE_BLOCK_GRANTED_FULL_REWARD_ZONE_V5    300000 //size of block (bytes) after which reward for block calculated using block size - second change, from v5
#define CRYPTONOTE_LONG_TERM_BLOCK_WEIGHT_WINDOW_SIZE   100000 // size in blocks of the long term block weight median window
#define CRYPTONOTE_SHORT_TERM_BLOCK_WEIGHT_SURGE_FACTOR 50
#define CRYPTONOTE_COINBASE_BLOB_RESERVED_SIZE          600
#define CRYPTONOTE_DISPLAY_DECIMAL_POINT                12
// COIN - number of smallest units in one coin
#define COIN                                            ((uint64_t)1000000000000) // pow(10, 12)

#define FEE_PER_KB_OLD                                  ((uint64_t)10000000000) // pow(10, 10)
#define FEE_PER_KB                                      ((uint64_t)2000000000) // 2 * pow(10, 9)
#define FEE_PER_BYTE                                    ((uint64_t)300000)
#define DYNAMIC_FEE_PER_KB_BASE_FEE                     ((uint64_t)2000000000) // 2 * pow(10,9)
#define DYNAMIC_FEE_PER_KB_BASE_BLOCK_REWARD            ((uint64_t)10000000000000) // 10 * pow(10,12)
#define DYNAMIC_FEE_PER_KB_BASE_FEE_V5                  ((uint64_t)2000000000 * (uint64_t)CRYPTONOTE_BLOCK_GRANTED_FULL_REWARD_ZONE_V2 / CRYPTONOTE_BLOCK_GRANTED_FULL_REWARD_ZONE_V5)
#define DYNAMIC_FEE_REFERENCE_TRANSACTION_WEIGHT         ((uint64_t)3000)

#define ORPHANED_BLOCKS_MAX_COUNT                       100


#define DIFFICULTY_TARGET_V2                            120  // seconds
#define DIFFICULTY_TARGET_V1                            60  // seconds - before first fork
#define DIFFICULTY_WINDOW                               720 // blocks
#define DIFFICULTY_LAG                                  15  // !!!
#define DIFFICULTY_CUT                                  60  // timestamps to cut after sorting
#define DIFFICULTY_BLOCKS_COUNT                         DIFFICULTY_WINDOW + DIFFICULTY_LAG


#define CRYPTONOTE_LOCKED_TX_ALLOWED_DELTA_SECONDS_V1   DIFFICULTY_TARGET_V1 * CRYPTONOTE_LOCKED_TX_ALLOWED_DELTA_BLOCKS
#define CRYPTONOTE_LOCKED_TX_ALLOWED_DELTA_SECONDS_V2   DIFFICULTY_TARGET_V2 * CRYPTONOTE_LOCKED_TX_ALLOWED_DELTA_BLOCKS
#define CRYPTONOTE_LOCKED_TX_ALLOWED_DELTA_BLOCKS       1


#define DIFFICULTY_BLOCKS_ESTIMATE_TIMESPAN             DIFFICULTY_TARGET_V1 //just alias; used by tests

// QuantumSafe Foundation PoW Recovery Fork parameters
#define QSF_POW_FORK_HEIGHT_MAINNET                     31000
#define QSF_POW_FORK_HEIGHT_TESTNET                     60800
#define QSF_POW_FORK_HEIGHT_STAGENET                    47000
#define QSF_POW_FORK_DIFFICULTY_RESET                   ((uint64_t)1000000)
#define QSF_POW_TARGET_BLOCK_TIME                       120
#define QSF_POW_MIN_BLOCK_TIME                          60
#define QSF_POW_LWMA_WINDOW_MAINNET                     90
#define QSF_POW_LWMA_WINDOW_TESTNET                     30
#define QSF_POW_LWMA_WINDOW                             QSF_POW_LWMA_WINDOW_MAINNET

// Hard Fork 18 (HF18) - Difficulty Stability Upgrade
#define QSF_HARDFORK_18_HEIGHT_MAINNET                  34000
#define QSF_HARDFORK_18_HEIGHT_TESTNET                  61000
#define QSF_HARDFORK_18_HEIGHT_STAGENET                 45000

// Difficulty Rescue (v3.0.6) - One-time rescue at height 31,671 + ongoing safety valve
#define QSF_DIFFICULTY_RESCUE_HEIGHT_MAINNET            31671
#define QSF_DIFFICULTY_RESCUE_VALUE_MAINNET             ((uint64_t)0)         // 0 means derive from divisor (see below)
#define QSF_DIFFICULTY_RESCUE_DIVISOR_MAINNET           16                    // Divide computed difficulty by 16 at rescue height
#define QSF_DIFFICULTY_SAFETY_VALVE_STUCK_TIME          14400                 // 4 hours in seconds (4 * 60 * 60)
#define QSF_DIFFICULTY_SAFETY_VALVE_MIN_DIFFICULTY      1000000               // Minimum difficulty floor


#define BLOCKS_IDS_SYNCHRONIZING_DEFAULT_COUNT          10000  //by default, blocks ids count in synchronizing
#define BLOCKS_IDS_SYNCHRONIZING_MAX_COUNT              25000  //max blocks ids count in synchronizing
#define BLOCKS_SYNCHRONIZING_DEFAULT_COUNT_PRE_V4       100    //by default, blocks count in blocks downloading
#define BLOCKS_SYNCHRONIZING_DEFAULT_COUNT              20     //by default, blocks count in blocks downloading
#define BLOCKS_SYNCHRONIZING_MAX_COUNT                  2048   //must be a power of 2, greater than 128, equal to SEEDHASH_EPOCH_BLOCKS

#define CRYPTONOTE_MEMPOOL_TX_LIVETIME                    (86400*3) //seconds, three days
#define CRYPTONOTE_MEMPOOL_TX_FROM_ALT_BLOCK_LIVETIME     604800 //seconds, one week


#define CRYPTONOTE_DANDELIONPP_STEMS              2 // number of outgoing stem connections per epoch
#define CRYPTONOTE_DANDELIONPP_FLUFF_PROBABILITY 20 // out of 100
#define CRYPTONOTE_DANDELIONPP_MIN_EPOCH         10 // minutes
#define CRYPTONOTE_DANDELIONPP_EPOCH_RANGE       30 // seconds
#define CRYPTONOTE_DANDELIONPP_FLUSH_AVERAGE      5 // seconds average for poisson distributed fluff flush
#define CRYPTONOTE_DANDELIONPP_EMBARGO_AVERAGE   39 // seconds (see tx_pool.cpp for more info)

// see src/cryptonote_protocol/levin_notify.cpp
#define CRYPTONOTE_NOISE_MIN_EPOCH                      5      // minutes
#define CRYPTONOTE_NOISE_EPOCH_RANGE                    30     // seconds
#define CRYPTONOTE_NOISE_MIN_DELAY                      10     // seconds
#define CRYPTONOTE_NOISE_DELAY_RANGE                    5      // seconds
#define CRYPTONOTE_NOISE_BYTES                          3*1024 // 3 KiB
#define CRYPTONOTE_NOISE_CHANNELS                       2      // Max outgoing connections per zone used for noise/covert sending

// Both below are in seconds. The idea is to delay forwarding from i2p/tor
// to ipv4/6, such that 2+ incoming connections _could_ have sent the tx
#define CRYPTONOTE_FORWARD_DELAY_BASE (CRYPTONOTE_NOISE_MIN_DELAY + CRYPTONOTE_NOISE_DELAY_RANGE)
#define CRYPTONOTE_FORWARD_DELAY_AVERAGE (CRYPTONOTE_FORWARD_DELAY_BASE + (CRYPTONOTE_FORWARD_DELAY_BASE / 2))

#define CRYPTONOTE_MAX_FRAGMENTS                        20 // ~20 * NOISE_BYTES max payload size for covert/noise send

#define COMMAND_RPC_GET_BLOCKS_FAST_MAX_BLOCK_COUNT     1000
#define COMMAND_RPC_GET_BLOCKS_FAST_MAX_TX_COUNT        20000
#define DEFAULT_RPC_MAX_CONNECTIONS_PER_PUBLIC_IP       3
#define DEFAULT_RPC_MAX_CONNECTIONS_PER_PRIVATE_IP      25
#define DEFAULT_RPC_MAX_CONNECTIONS                     100
#define DEFAULT_RPC_SOFT_LIMIT_SIZE                     25 * 1024 * 1024 // 25 MiB
#define MAX_RPC_CONTENT_LENGTH                          1048576 // 1 MB

#define P2P_LOCAL_WHITE_PEERLIST_LIMIT                  1000
#define P2P_LOCAL_GRAY_PEERLIST_LIMIT                   5000

#define P2P_DEFAULT_CONNECTIONS_COUNT                   12
#define P2P_DEFAULT_HANDSHAKE_INTERVAL                  60           //secondes
#define P2P_DEFAULT_PACKET_MAX_SIZE                     50000000     //50000000 bytes maximum packet size
#define P2P_DEFAULT_PEERS_IN_HANDSHAKE                  250
#define P2P_MAX_PEERS_IN_HANDSHAKE                      250
#define P2P_DEFAULT_CONNECTION_TIMEOUT                  5000       //5 seconds
#define P2P_DEFAULT_SOCKS_CONNECT_TIMEOUT               45         // seconds
#define P2P_DEFAULT_PING_CONNECTION_TIMEOUT             2000       //2 seconds
#define P2P_DEFAULT_INVOKE_TIMEOUT                      60*2*1000  //2 minutes
#define P2P_DEFAULT_HANDSHAKE_INVOKE_TIMEOUT            5000       //5 seconds
#define P2P_DEFAULT_WHITELIST_CONNECTIONS_PERCENT       70
#define P2P_DEFAULT_ANCHOR_CONNECTIONS_COUNT            2
#define P2P_DEFAULT_SYNC_SEARCH_CONNECTIONS_COUNT       2
#define P2P_DEFAULT_LIMIT_RATE_UP                       8192       // kB/s
#define P2P_DEFAULT_LIMIT_RATE_DOWN                     32768      // kB/s

#define P2P_FAILED_ADDR_FORGET_SECONDS                  (60*60)     //1 hour
#define P2P_IP_BLOCKTIME                                (60*60*24)  //24 hour
#define P2P_IP_FAILS_BEFORE_BLOCK                       10
#define P2P_IDLE_CONNECTION_KILL_INTERVAL               (5*60) //5 minutes

#define P2P_SUPPORT_FLAG_FLUFFY_BLOCKS                  0x01
#define P2P_SUPPORT_FLAGS                               P2P_SUPPORT_FLAG_FLUFFY_BLOCKS

#define RPC_IP_FAILS_BEFORE_BLOCK                       3

// QSF Coin Configuration
#define CRYPTONOTE_NAME                         "quantumsafefoundation"
#define CRYPTONOTE_TICKER                       "QSF"
#define CRYPTONOTE_POOLDATA_FILENAME           "poolstate"
#define CRYPTONOTE_BLOCKCHAINDATA_FILENAME     "data.mdb"
#define CRYPTONOTE_BLOCKCHAINDATA_LOCK_FILENAME "lock.mdb"
#define P2P_NET_DATA_FILENAME                  "p2pstate.bin"
#define RPC_PAYMENTS_DATA_FILENAME              "rpcpayments.bin"
#define MINER_CONFIG_FILE_NAME                 "miner_conf.json"

// Quantum-Safe Configuration
#define QSF_QUANTUM_SAFE_ENABLED               1
#define QSF_XMSS_HEIGHT                        10
#define QSF_SPHINCS_LEVEL                      5
#define QSF_QUANTUM_SIGNATURE_SIZE             1024
#define QSF_XMSS_SIGNATURE_SIZE                1024
#define QSF_SPHINCS_SIGNATURE_SIZE             1024
#define QSF_QUANTUM_KEY_SIZE                   32

// Quantum-Safe Enforcement Configuration
#define QSF_QUANTUM_SAFE_ENFORCED             1  // MANDATORY quantum-safe features
#define QSF_RANDOMX_QUANTUM_INTEGRATION       1  // MANDATORY RandomX + quantum integration
#define QSF_TRANSACTION_QUANTUM_ENFORCED      1  // MANDATORY quantum-safe transactions
#define QSF_BLOCK_QUANTUM_VALIDATION          1  // MANDATORY quantum-safe block validation

// Force quantum-safe to always be enabled - cannot be disabled
#define QSF_QUANTUM_SAFE_ALWAYS_ENABLED       1  // NEW: Force enable
#define QSF_DISABLE_QUANTUM_SAFE_OPTION       0  // NEW: Disable command-line override

// Quantum-Safe Algorithm Defaults
#define QSF_DEFAULT_QUANTUM_ALGORITHM         "XMSS"
#define QSF_DEFAULT_XMSS_TREE_HEIGHT          10
#define QSF_DEFAULT_SPHINCS_LEVEL             5

// Quantum-Safe Signature Requirements
#define QSF_MIN_XMSS_TREE_HEIGHT              8
#define QSF_MAX_XMSS_TREE_HEIGHT              20
#define QSF_MIN_SPHINCS_LEVEL                3
#define QSF_MAX_SPHINCS_LEVEL                10

// Quantum-Safe Transaction Requirements
#define QSF_QUANTUM_SIGNATURE_SIZE            1024  // bytes
#define QSF_QUANTUM_PUBLIC_KEY_SIZE           32    // bytes (XMSS)
#define QSF_QUANTUM_PRIVATE_KEY_SIZE          32    // bytes (XMSS)

#define THREAD_STACK_SIZE                       5 * 1024 * 1024

#define HF_VERSION_DYNAMIC_FEE                  4
#define HF_VERSION_MIN_MIXIN_4                  6
#define HF_VERSION_MIN_MIXIN_6                  7
#define HF_VERSION_MIN_MIXIN_10                 8
#define HF_VERSION_MIN_MIXIN_15                 15
#define HF_VERSION_ENFORCE_RCT                  6
#define HF_VERSION_PER_BYTE_FEE                 8
#define HF_VERSION_SMALLER_BP                   10
#define HF_VERSION_LONG_TERM_BLOCK_WEIGHT       10
#define HF_VERSION_MIN_2_OUTPUTS                12
#define HF_VERSION_MIN_V2_COINBASE_TX           12
#define HF_VERSION_SAME_MIXIN                   12
#define HF_VERSION_REJECT_SIGS_IN_COINBASE      12
#define HF_VERSION_ENFORCE_MIN_AGE              12
#define HF_VERSION_EFFECTIVE_SHORT_TERM_MEDIAN_IN_PENALTY 12
#define HF_VERSION_EXACT_COINBASE               13
#define HF_VERSION_CLSAG                        13
#define HF_VERSION_DETERMINISTIC_UNLOCK_TIME    13
#define HF_VERSION_BULLETPROOF_PLUS             15
#define HF_VERSION_VIEW_TAGS                    15
#define HF_VERSION_2021_SCALING                 15
#define HF_VERSION_POW_RECOVERY                 17

#define PER_KB_FEE_QUANTIZATION_DECIMALS        8
#define CRYPTONOTE_SCALING_2021_FEE_ROUNDING_PLACES 2

#define HASH_OF_HASHES_STEP                     512

#define DEFAULT_TXPOOL_MAX_WEIGHT               648000000ull // 3 days at 300000, in bytes

#define BULLETPROOF_MAX_OUTPUTS                 16
#define BULLETPROOF_PLUS_MAX_OUTPUTS            16

#define CRYPTONOTE_PRUNING_STRIPE_SIZE          4096 // the smaller, the smoother the increase
#define CRYPTONOTE_PRUNING_LOG_STRIPES          3 // the higher, the more space saved
#define CRYPTONOTE_PRUNING_TIP_BLOCKS           5500 // the smaller, the more space saved

#define RPC_CREDITS_PER_HASH_SCALE ((float)(1<<24))

#define DNS_BLOCKLIST_LIFETIME (86400 * 8)

//The limit is enough for the mandatory transaction content with 16 outputs (547 bytes),
//a custom tag (1 byte) and up to 32 bytes of custom data for each recipient.
// (1+32) + (1+1+16*32) + (1+16*32) = 1060
#define MAX_TX_EXTRA_SIZE                       1060

// New constants are intended to go here
namespace config
{
  uint64_t const DEFAULT_FEE_ATOMIC_QSF_PER_KB = 500; // Just a placeholder!  Change me!
  uint8_t const FEE_CALCULATION_MAX_RETRIES = 10;
  uint64_t const DEFAULT_DUST_THRESHOLD = ((uint64_t)2000000000); // 2 * pow(10, 9)
  uint64_t const BASE_REWARD_CLAMP_THRESHOLD = ((uint64_t)100000000); // pow(10, 8)

  uint64_t const CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX = 15;
  uint64_t const CRYPTONOTE_PUBLIC_INTEGRATED_ADDRESS_BASE58_PREFIX = 16;
  uint64_t const CRYPTONOTE_PUBLIC_SUBADDRESS_BASE58_PREFIX = 43;
  uint16_t const P2P_DEFAULT_PORT = 18070;
  uint16_t const RPC_DEFAULT_PORT = 18071;
  uint16_t const ZMQ_RPC_DEFAULT_PORT = 18072;
  boost::uuids::uuid const NETWORK_ID = { {
      0x12 ,0x30, 0xF1, 0x71 , 0x61, 0x04 , 0x41, 0x61, 0x17, 0x31, 0x00, 0x82, 0x17, 0xA1, 0xA1, 0x11
    } }; // Bender's nightmare
  std::string const GENESIS_TX = "013c01ff0001ffffffffffff03022b789c81430435fc350010755ae870f1e081ef822a2738ae4bc156d57a55f26a210148eed6bf260a573905b9ef65b54dbbe7deee401a307b4c56ab4d81196112f56f";
  uint32_t const GENESIS_NONCE = 70;
  std::string const DAEMON_URL = "http://qsfchain.com:18071"; // QSF_MAINNET - Silicon Valley
  std::vector<std::string> const SEED_NODES = {
    "seeds.qsfchain.com",      // QSF_MAINNET - Silicon Valley
    "seeds.qsfcoin.com",       // qsf4_mainnet - London
    "seeds.qsfcoin.org",       // qsf3_mainnet - Sydney
    "seeds.qsfnetwork.co"      // qsf5_mainnet - Tokyo
  };

  // Hash domain separators
  const char HASH_KEY_BULLETPROOF_EXPONENT[] = "bulletproof";
  const char HASH_KEY_BULLETPROOF_PLUS_EXPONENT[] = "bulletproof_plus";
  const char HASH_KEY_BULLETPROOF_PLUS_TRANSCRIPT[] = "bulletproof_plus_transcript";
  const char HASH_KEY_RINGDB[] = "ringdsb";
  const char HASH_KEY_SUBADDRESS[] = "SubAddr";
  const unsigned char HASH_KEY_ENCRYPTED_PAYMENT_ID = 0x8d;
  const unsigned char HASH_KEY_WALLET = 0x8c;
  const unsigned char HASH_KEY_WALLET_CACHE = 0x8d;
  const unsigned char HASH_KEY_BACKGROUND_CACHE = 0x8e;
  const unsigned char HASH_KEY_BACKGROUND_KEYS_FILE = 0x8f;
  const unsigned char HASH_KEY_RPC_PAYMENT_NONCE = 0x58;
  const unsigned char HASH_KEY_MEMORY = 'k';
  const unsigned char HASH_KEY_MULTISIG[] = {'M', 'u', 'l', 't' , 'i', 's', 'i', 'g', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  const unsigned char HASH_KEY_MULTISIG_KEY_AGGREGATION[] = "Multisig_key_agg";
  const unsigned char HASH_KEY_CLSAG_ROUND_MULTISIG[] = "CLSAG_round_ms_merge_factor";
  const unsigned char HASH_KEY_TXPROOF_V2[] = "TXPROOF_V2";
  const unsigned char HASH_KEY_CLSAG_ROUND[] = "CLSAG_round";
  const unsigned char HASH_KEY_CLSAG_AGG_0[] = "CLSAG_agg_0";
  const unsigned char HASH_KEY_CLSAG_AGG_1[] = "CLSAG_agg_1";
  const char HASH_KEY_MESSAGE_SIGNING[] = "QSFMessageSignature";
  const unsigned char HASH_KEY_MM_SLOT = 'm';
  const constexpr char HASH_KEY_MULTISIG_TX_PRIVKEYS_SEED[] = "multisig_tx_privkeys_seed";
  const constexpr char HASH_KEY_MULTISIG_TX_PRIVKEYS[] = "multisig_tx_privkeys";
  const constexpr char HASH_KEY_TXHASH_AND_MIXRING[] = "txhashandmixring";

  constexpr uint64_t POW_FORK_HEIGHT = QSF_POW_FORK_HEIGHT_MAINNET;
  constexpr uint64_t POW_DIFFICULTY_RESET = QSF_POW_FORK_DIFFICULTY_RESET;
  constexpr uint64_t POW_TARGET_BLOCK_TIME = QSF_POW_TARGET_BLOCK_TIME;
  constexpr uint64_t POW_MIN_BLOCK_TIME = QSF_POW_MIN_BLOCK_TIME;
  constexpr size_t POW_LWMA_WINDOW = QSF_POW_LWMA_WINDOW;
  constexpr uint64_t RANDOMX_TWEAK_HEIGHT = POW_FORK_HEIGHT;
  constexpr uint64_t HARDFORK_18_HEIGHT = QSF_HARDFORK_18_HEIGHT_MAINNET;
  constexpr uint64_t DIFFICULTY_RESCUE_HEIGHT = QSF_DIFFICULTY_RESCUE_HEIGHT_MAINNET;
  constexpr uint64_t DIFFICULTY_RESCUE_VALUE = QSF_DIFFICULTY_RESCUE_VALUE_MAINNET;
  constexpr uint64_t DIFFICULTY_RESCUE_DIVISOR = QSF_DIFFICULTY_RESCUE_DIVISOR_MAINNET;
  constexpr uint64_t DIFFICULTY_SAFETY_VALVE_STUCK_TIME = QSF_DIFFICULTY_SAFETY_VALVE_STUCK_TIME;
  constexpr uint64_t DIFFICULTY_SAFETY_VALVE_MIN_DIFFICULTY = QSF_DIFFICULTY_SAFETY_VALVE_MIN_DIFFICULTY;

  // Multisig
  const uint32_t MULTISIG_MAX_SIGNERS{16};

  namespace testnet
  {
    uint64_t const CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX = 53;
    uint64_t const CRYPTONOTE_PUBLIC_INTEGRATED_ADDRESS_BASE58_PREFIX = 54;
    uint64_t const CRYPTONOTE_PUBLIC_SUBADDRESS_BASE58_PREFIX = 63;
    uint16_t const P2P_DEFAULT_PORT = 28070;
    uint16_t const RPC_DEFAULT_PORT = 28071;
    uint16_t const ZMQ_RPC_DEFAULT_PORT = 28072;
    boost::uuids::uuid const NETWORK_ID = { {
        0x12 ,0x30, 0xF1, 0x71 , 0x61, 0x04 , 0x41, 0x61, 0x17, 0x31, 0x00, 0x84, 0x18, 0xA1, 0xA1, 0x12
      } }; // Bender's daydream
    std::string const GENESIS_TX = "013c01ff0001ffffffffffff0302c0014913971b8a125cd5df6aec536bd668dcef7d7802fff6800ea0a9853168532101f232e98ee2ed026234393395899cecb597cebac5d24f26dd1d67c918d853af60";
    uint32_t const GENESIS_NONCE = 71;
    std::string const DAEMON_URL = "http://qsfnetwork.com:28071"; // seed-test-qsf - Atlanta
    std::vector<std::string> const SEED_NODES = {
      "seeds.qsfnetwork.com"  // seed-test-qsf - Atlanta
    };

    constexpr uint64_t POW_FORK_HEIGHT = QSF_POW_FORK_HEIGHT_TESTNET;
    constexpr uint64_t POW_DIFFICULTY_RESET = QSF_POW_FORK_DIFFICULTY_RESET;
    constexpr uint64_t POW_TARGET_BLOCK_TIME = QSF_POW_TARGET_BLOCK_TIME;
    constexpr uint64_t POW_MIN_BLOCK_TIME = QSF_POW_MIN_BLOCK_TIME;
    constexpr size_t POW_LWMA_WINDOW = QSF_POW_LWMA_WINDOW_TESTNET;
    constexpr uint64_t RANDOMX_TWEAK_HEIGHT = POW_FORK_HEIGHT;
    constexpr uint64_t HARDFORK_18_HEIGHT = QSF_HARDFORK_18_HEIGHT_TESTNET;
    constexpr uint64_t DIFFICULTY_RESCUE_HEIGHT = 0;  // No rescue on testnet
    constexpr uint64_t DIFFICULTY_RESCUE_VALUE = 0;
    constexpr uint64_t DIFFICULTY_RESCUE_DIVISOR = 1;
    constexpr uint64_t DIFFICULTY_SAFETY_VALVE_STUCK_TIME = QSF_DIFFICULTY_SAFETY_VALVE_STUCK_TIME;
    constexpr uint64_t DIFFICULTY_SAFETY_VALVE_MIN_DIFFICULTY = QSF_DIFFICULTY_SAFETY_VALVE_MIN_DIFFICULTY;
  }

  namespace stagenet
  {
    uint64_t const CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX = 24;
    uint64_t const CRYPTONOTE_PUBLIC_INTEGRATED_ADDRESS_BASE58_PREFIX = 25;
    uint64_t const CRYPTONOTE_PUBLIC_SUBADDRESS_BASE58_PREFIX = 36;
    uint16_t const P2P_DEFAULT_PORT = 38070;
    uint16_t const RPC_DEFAULT_PORT = 38071;
    uint16_t const ZMQ_RPC_DEFAULT_PORT = 38072;
    boost::uuids::uuid const NETWORK_ID = { {
        0x12 ,0x30, 0xF1, 0x71 , 0x61, 0x04 , 0x41, 0x61, 0x17, 0x31, 0x00, 0x85, 0x19, 0xA1, 0xA1, 0x13
      } }; // Bender's daydream
    std::string const GENESIS_TX = "013c01ff0001ffffffffffff03028d950dde8dd947536edfd2e5acd032c5f5fac3cff53b1e2624726d47a6ae209f21010cbe52dbc8f302fe0da054b1ac5241bc749dddf143a8a4d429ef65e5795a47a1";
    uint32_t const GENESIS_NONCE = 72;
    std::string const DAEMON_URL = "http://qsfcoin.network:38071"; // qsf_seedstage - Chicago
    std::vector<std::string> const SEED_NODES = {
      "seeds.qsfcoin.network" // qsf_seedstage - Chicago
    };

    constexpr uint64_t POW_FORK_HEIGHT = QSF_POW_FORK_HEIGHT_STAGENET;
    constexpr uint64_t POW_DIFFICULTY_RESET = QSF_POW_FORK_DIFFICULTY_RESET;
    constexpr uint64_t POW_TARGET_BLOCK_TIME = QSF_POW_TARGET_BLOCK_TIME;
    constexpr uint64_t POW_MIN_BLOCK_TIME = QSF_POW_MIN_BLOCK_TIME;
    constexpr size_t POW_LWMA_WINDOW = QSF_POW_LWMA_WINDOW;
    constexpr uint64_t RANDOMX_TWEAK_HEIGHT = POW_FORK_HEIGHT;
    constexpr uint64_t HARDFORK_18_HEIGHT = QSF_HARDFORK_18_HEIGHT_STAGENET;
    constexpr uint64_t DIFFICULTY_RESCUE_HEIGHT = 0;  // No rescue on stagenet
    constexpr uint64_t DIFFICULTY_RESCUE_VALUE = 0;
    constexpr uint64_t DIFFICULTY_RESCUE_DIVISOR = 1;
    constexpr uint64_t DIFFICULTY_SAFETY_VALVE_STUCK_TIME = QSF_DIFFICULTY_SAFETY_VALVE_STUCK_TIME;
    constexpr uint64_t DIFFICULTY_SAFETY_VALVE_MIN_DIFFICULTY = QSF_DIFFICULTY_SAFETY_VALVE_MIN_DIFFICULTY;
  }
}

namespace cryptonote
{
  enum network_type : uint8_t
  {
    MAINNET = 0,
    TESTNET,
    STAGENET,
    FAKECHAIN,
    UNDEFINED = 255
  };
  struct config_t
  {
    uint64_t const CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX;
    uint64_t const CRYPTONOTE_PUBLIC_INTEGRATED_ADDRESS_BASE58_PREFIX;
    uint64_t const CRYPTONOTE_PUBLIC_SUBADDRESS_BASE58_PREFIX;
    uint16_t const P2P_DEFAULT_PORT;
    uint16_t const RPC_DEFAULT_PORT;
    uint16_t const ZMQ_RPC_DEFAULT_PORT;
    boost::uuids::uuid const NETWORK_ID;
    std::string const GENESIS_TX;
    uint32_t const GENESIS_NONCE;
    std::string const DAEMON_URL;
    std::vector<std::string> const SEED_NODES;
  };
  inline const config_t& get_config(network_type nettype)
  {
    static const config_t mainnet = {
      ::config::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX,
      ::config::CRYPTONOTE_PUBLIC_INTEGRATED_ADDRESS_BASE58_PREFIX,
      ::config::CRYPTONOTE_PUBLIC_SUBADDRESS_BASE58_PREFIX,
      ::config::P2P_DEFAULT_PORT,
      ::config::RPC_DEFAULT_PORT,
      ::config::ZMQ_RPC_DEFAULT_PORT,
      ::config::NETWORK_ID,
      ::config::GENESIS_TX,
      ::config::GENESIS_NONCE,
      ::config::DAEMON_URL,
      ::config::SEED_NODES
    };
    static const config_t testnet = {
      ::config::testnet::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX,
      ::config::testnet::CRYPTONOTE_PUBLIC_INTEGRATED_ADDRESS_BASE58_PREFIX,
      ::config::testnet::CRYPTONOTE_PUBLIC_SUBADDRESS_BASE58_PREFIX,
      ::config::testnet::P2P_DEFAULT_PORT,
      ::config::testnet::RPC_DEFAULT_PORT,
      ::config::testnet::ZMQ_RPC_DEFAULT_PORT,
      ::config::testnet::NETWORK_ID,
      ::config::testnet::GENESIS_TX,
      ::config::testnet::GENESIS_NONCE,
      ::config::testnet::DAEMON_URL,
      ::config::testnet::SEED_NODES
    };
    static const config_t stagenet = {
      ::config::stagenet::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX,
      ::config::stagenet::CRYPTONOTE_PUBLIC_INTEGRATED_ADDRESS_BASE58_PREFIX,
      ::config::stagenet::CRYPTONOTE_PUBLIC_SUBADDRESS_BASE58_PREFIX,
      ::config::stagenet::P2P_DEFAULT_PORT,
      ::config::stagenet::RPC_DEFAULT_PORT,
      ::config::stagenet::ZMQ_RPC_DEFAULT_PORT,
      ::config::stagenet::NETWORK_ID,
      ::config::stagenet::GENESIS_TX,
      ::config::stagenet::GENESIS_NONCE,
      ::config::stagenet::DAEMON_URL,
      ::config::stagenet::SEED_NODES
    };
    switch (nettype)
    {
      case MAINNET: return mainnet;
      case TESTNET: return testnet;
      case STAGENET: return stagenet;
      case FAKECHAIN: return mainnet;
      default: throw std::runtime_error("Invalid network type");
    }
  };
}

// QSF Security Enhancements - 51% Attack Prevention
#define QSF_SECURITY_ENHANCED 1
#define QSF_ATTACK_DETECTION_ENABLED 1
#define QSF_HYBRID_CONSENSUS_ENABLED 1
#define QSF_EMERGENCY_RESPONSE_ENABLED 1

// Enhanced RandomX Configuration
#define QSF_RANDOMX_MEMORY_SIZE (2ULL * 1024 * 1024 * 1024) // 2GB memory requirement
#define QSF_RANDOMX_ITERATIONS 2048
#define QSF_RANDOMX_PROGRAM_SIZE 256
#define QSF_RANDOMX_PROGRAM_COUNT 8

// Mining Pool Protection
#define QSF_MAX_POOL_HASHRATE_PERCENT 15  // Maximum 15% hashrate per pool
#define QSF_MIN_POOL_NODES 3              // Minimum nodes per pool
#define QSF_POOL_ROTATION_INTERVAL 3600   // Rotate every hour

// Network Security Limits
#define QSF_MAX_INBOUND_CONNECTIONS 100
#define QSF_MAX_OUTBOUND_CONNECTIONS 50
#define QSF_CONNECTION_RATE_LIMIT 10      // connections per minute

// Multi-Signature Security
#define QSF_MULTISIG_THRESHOLD_AMOUNT 10000  // QSF amount requiring multi-sig
#define QSF_MULTISIG_MIN_SIGNATURES 3
#define QSF_MULTISIG_TIMEOUT 3600  // 1 hour timeout

// Staking Configuration
#define QSF_STAKING_APY 5.0               // 5% annual yield
#define QSF_MIN_STAKE_AMOUNT 1000         // Minimum QSF to stake
#define QSF_STAKE_LOCK_PERIOD 2592000     // 30 days lock period

// Security Deposits
#define QSF_POOL_SECURITY_DEPOSIT 50000   // QSF security deposit for pools
#define QSF_POOL_SLASHING_PERCENT 50      // 50% slashing for attacks

// Network Monitoring and Attack Prevention
#define QSF_ATTACK_DETECTION_THRESHOLD 0.51  // 51% hashrate threshold
#define QSF_WARNING_THRESHOLD 0.45           // 45% warning threshold
#define QSF_DETECTION_WINDOW 300             // 5 minutes detection window
#define QSF_BLACKLIST_DURATION 3600          // 1 hour blacklist duration
#define QSF_MAX_SUSPICIOUS_NODES 100         // Maximum suspicious nodes to track
#define QSF_NETWORK_HEALTH_CHECK_INTERVAL 60 // Network health check interval in seconds

// Enhanced 51% Attack Protection (Qubic-style attack resistance)
#define QSF_QUANTUM_SAFE_51_PROTECTION 1    // Quantum-safe 51% protection
#define QSF_MULTI_LAYER_CONSENSUS 1         // Multi-layer consensus protection
#define QSF_DYNAMIC_DIFFICULTY_ADJUSTMENT 1 // Dynamic difficulty adjustment
#define QSF_POOL_DIVERSITY_ENFORCEMENT 1    // Enforce pool diversity
#define QSF_GEOGRAPHIC_DISTRIBUTION_CHECK 1 // Geographic distribution validation
#define QSF_ISP_DIVERSITY_ENFORCEMENT 1     // ISP diversity enforcement
#define QSF_REAL_TIME_THREAT_ANALYSIS 1     // Real-time threat analysis
#define QSF_AUTOMATIC_RECOVERY_MODE 1       // Automatic recovery mode

// Advanced Attack Detection Thresholds
#define QSF_RAPID_ATTACK_DETECTION 0.40     // 40% rapid detection threshold
#define QSF_GRADUAL_ATTACK_DETECTION 0.35   // 35% gradual attack detection
#define QSF_SYBILL_ATTACK_THRESHOLD 0.25   // 25% sybill attack threshold
#define QSF_POOL_COLLUSION_THRESHOLD 0.30  // 30% pool collusion threshold
#define QSF_ECLIPSE_ATTACK_THRESHOLD 0.60  // 60% eclipse attack threshold

// Response Mechanisms
#define QSF_IMMEDIATE_DIFFICULTY_SPIKE 1    // Immediate difficulty spike on attack
#define QSF_AUTOMATIC_NODE_ISOLATION 1      // Automatically isolate suspicious nodes
#define QSF_NETWORK_SEGMENTATION 1          // Network segmentation on attack
#define QSF_EMERGENCY_FORK_PROTECTION 1     // Emergency fork protection
#define QSF_AUTOMATIC_RECOVERY_SEQUENCE 1   // Automatic recovery sequence

// Recovery and Resilience
#define QSF_ATTACK_RECOVERY_TIME 1800       // 30 minutes recovery time
#define QSF_NETWORK_HEALING_INTERVAL 300    // 5 minutes healing interval
#define QSF_MAX_RECOVERY_ATTEMPTS 5         // Maximum recovery attempts
#define QSF_RECOVERY_SUCCESS_THRESHOLD 0.8  // 80% success threshold for recovery

// Geographic Distribution
#define QSF_MIN_GEOGRAPHIC_DIVERSITY 50      // Minimum countries for network health
#define QSF_MIN_ISP_DIVERSITY 100            // Minimum ISPs for network health
#define QSF_MIN_ACTIVE_NODES 1000            // Minimum active nodes for network health

// Small utility to construct a genesis coinbase tx blob for a given address and timestamp,
// and to search a nonce producing a low-difficulty hash (optional quick scan).

#include "cryptonote_config.h"
#include "cryptonote_basic/cryptonote_basic_impl.h"
#include "cryptonote_core/cryptonote_tx_utils.h"
#include "cryptonote_core/blockchain.h"
#include "crypto/hash.h"
#include "common/command_line.h"
#include <iostream>

namespace po = boost::program_options;

static bool construct_genesis_tx_blob(const cryptonote::account_public_address &addr, std::string &tx_hex)
{
  cryptonote::transaction tx{};
  // height 0, zero fees, minimal sizes
  if (!cryptonote::construct_miner_tx(/*height=*/0, /*median_weight=*/1, /*already_generated_coins=*/0,
                                      /*current_block_weight=*/1, /*fee=*/0, addr, tx, /*extra_nonce=*/cryptonote::blobdata(), /*max_outs=*/1, /*hf=*/CURRENT_BLOCK_MAJOR_VERSION))
    return false;
  cryptonote::blobdata b = cryptonote::tx_to_blob(tx);
  tx_hex = epee::string_tools::buff_to_hex_nodelimer(b);
  return true;
}

int main(int argc, char** argv)
{
  try {
    std::string address_str;
    std::string net_str{"mainnet"};
    uint32_t nonce = 70; // default placeholder
    std::string tx_hex_in;

    po::options_description desc{"Options"};
    desc.add_options()
      ("help", "Show help")
      ("address", po::value<std::string>(&address_str), "Base58 address to receive genesis coinbase")
      ("net", po::value<std::string>(&net_str)->default_value("mainnet"), "Network: mainnet|testnet|stagenet")
      ("nonce", po::value<uint32_t>(&nonce)->default_value(70), "Genesis nonce (simple placeholder)")
      ("tx-hex", po::value<std::string>(&tx_hex_in), "Existing genesis tx hex (if provided, --address is ignored)");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
      std::cout << desc << std::endl;
      return 0;
    }
    po::notify(vm);

    cryptonote::network_type net = cryptonote::MAINNET;
    if (net_str == "testnet") net = cryptonote::TESTNET;
    else if (net_str == "stagenet") net = cryptonote::STAGENET;

    std::string tx_hex;
    if (!tx_hex_in.empty()) {
      tx_hex = tx_hex_in;
    } else {
      if (address_str.empty()) {
        std::cerr << "Either --tx-hex or --address must be provided" << std::endl;
        return 1;
      }
      cryptonote::address_parse_info info{};
      if (!cryptonote::get_account_address_from_str(info, net, address_str)) {
        std::cerr << "Invalid address: " << address_str << std::endl;
        return 1;
      }
      const cryptonote::account_public_address &addr = info.address;
      if (!construct_genesis_tx_blob(addr, tx_hex)) {
        std::cerr << "Failed to construct genesis tx" << std::endl;
        return 1;
      }
    }

    // Assemble a block to show the resulting hash with the provided nonce
    cryptonote::block bl{};
    if (!cryptonote::generate_genesis_block(bl, tx_hex, nonce)) {
      std::cerr << "Failed to generate genesis block" << std::endl;
      return 1;
    }
    const crypto::hash h = cryptonote::get_block_longhash(nullptr, bl, 0);

    std::cout << "GENESIS_TX_HEX=" << tx_hex << std::endl;
    std::cout << "GENESIS_NONCE=" << nonce << std::endl;
    std::cout << "GENESIS_HASH=" << h << std::endl;
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}



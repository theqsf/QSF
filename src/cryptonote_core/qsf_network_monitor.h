#pragma once

#include <map>
#include <vector>
#include <string>
#include <atomic>
#include <chrono>
#include <mutex>

namespace cryptonote {

class QSFNetworkMonitor {
private:
    std::map<std::string, uint64_t> node_hashrates;
    std::map<std::string, std::chrono::steady_clock::time_point> node_last_seen;
    std::vector<std::string> suspicious_nodes;
    std::vector<std::string> blacklisted_nodes;
    
    std::atomic<uint64_t> total_network_hashrate;
    std::atomic<uint64_t> suspicious_hashrate;
    std::atomic<bool> attack_detected;
    
    // Enhanced 51% Attack Protection Variables
    std::map<std::string, std::string> node_geographic_locations;
    std::map<std::string, std::string> node_isp_info;
    std::map<std::string, std::string> node_pool_affiliations;
    std::map<std::string, std::chrono::steady_clock::time_point> attack_detection_times;
    std::vector<std::string> isolated_nodes;
    std::vector<std::string> network_segments;
    
    std::atomic<bool> rapid_attack_detected;
    std::atomic<bool> gradual_attack_detected;
    std::atomic<bool> recovery_mode_active;
    std::atomic<uint32_t> recovery_attempts;
    std::atomic<std::chrono::steady_clock::time_point> last_recovery_attempt;
    
    std::mutex monitor_mutex;
    
    // Attack detection thresholds
    static constexpr double HASHRATE_THRESHOLD = QSF_ATTACK_DETECTION_THRESHOLD;  // 51% threshold
    static constexpr double SUSPICIOUS_THRESHOLD = QSF_WARNING_THRESHOLD; // 45% warning threshold
    static constexpr uint64_t DETECTION_WINDOW = QSF_DETECTION_WINDOW;   // 5 minutes
    static constexpr uint64_t BLACKLIST_DURATION = QSF_BLACKLIST_DURATION; // 1 hour

public:
    QSFNetworkMonitor();
    ~QSFNetworkMonitor();
    
    // Node management
    void add_node(const std::string& node_id, uint64_t hashrate);
    void update_node_hashrate(const std::string& node_id, uint64_t hashrate);
    void remove_node(const std::string& node_id);
    
    // Attack detection
    bool detect_51_percent_attack();
    bool detect_sybil_attack();
    bool detect_eclipse_attack();
    bool detect_pool_centralization();
    
    // Enhanced 51% Attack Protection (Qubic-style attack resistance)
    bool detect_rapid_attack();
    bool detect_gradual_attack();
    bool detect_pool_collusion();
    bool detect_geographic_concentration();
    bool detect_isp_concentration();
    bool detect_quantum_safe_violation();
    
    // Response mechanisms
    void trigger_emergency_response();
    void blacklist_node(const std::string& node_id);
    void whitelist_node(const std::string& node_id);
    void increase_difficulty_emergency();
    
    // Advanced Response Mechanisms
    void trigger_immediate_difficulty_spike();
    void isolate_suspicious_nodes();
    void segment_network();
    void activate_emergency_fork_protection();
    void initiate_automatic_recovery();
    void heal_network_segments();
    
    // Monitoring
    double get_network_decentralization_score();
    double get_attack_resistance_score();
    uint64_t get_total_hashrate() const;
    uint64_t get_suspicious_hashrate() const;
    
    // Enhanced Monitoring
    double get_geographic_diversity_score();
    double get_isp_diversity_score();
    double get_pool_diversity_score();
    double get_quantum_safe_compliance_score();
    bool is_network_healthy();
    bool is_under_attack();
    
    // Reporting
    void generate_security_report();
    std::vector<std::string> get_suspicious_nodes() const;
    std::vector<std::string> get_blacklisted_nodes() const;
    
    // Advanced Reporting
    void generate_threat_analysis_report();
    void generate_recovery_status_report();
    void generate_network_health_report();
    
private:
    void cleanup_expired_blacklist();
    bool is_node_blacklisted(const std::string& node_id) const;
    double calculate_hashrate_percentage(uint64_t hashrate) const;
};

} // namespace cryptonote

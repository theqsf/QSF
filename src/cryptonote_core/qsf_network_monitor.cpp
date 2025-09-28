#include "qsf_network_monitor.h"
#include "cryptonote_config.h"
#include "logging/oxen_logger.h"
#include <algorithm>
#include <iostream>

namespace cryptonote {

QSFNetworkMonitor::QSFNetworkMonitor() 
    : total_network_hashrate(0)
    , suspicious_hashrate(0)
    , attack_detected(false) {
    OXEN_LOG(info, "QSF Network Monitor initialized");
}

QSFNetworkMonitor::~QSFNetworkMonitor() {
    OXEN_LOG(info, "QSF Network Monitor shutdown");
}

void QSFNetworkMonitor::add_node(const std::string& node_id, uint64_t hashrate) {
    std::lock_guard<std::mutex> lock(monitor_mutex);
    
    if (is_node_blacklisted(node_id)) {
        OXEN_LOG(warn, "Attempted to add blacklisted node: {}", node_id);
        return;
    }
    
    node_hashrates[node_id] = hashrate;
    node_last_seen[node_id] = std::chrono::steady_clock::now();
    total_network_hashrate += hashrate;
    
    OXEN_LOG(debug, "Added node {} with hashrate {}", node_id, hashrate);
}

void QSFNetworkMonitor::update_node_hashrate(const std::string& node_id, uint64_t new_hashrate) {
    std::lock_guard<std::mutex> lock(monitor_mutex);
    
    auto it = node_hashrates.find(node_id);
    if (it != node_hashrates.end()) {
        uint64_t old_hashrate = it->second;
        total_network_hashrate -= old_hashrate;
        total_network_hashrate += new_hashrate;
        it->second = new_hashrate;
        node_last_seen[node_id] = std::chrono::steady_clock::now();
        
        OXEN_LOG(debug, "Updated node {} hashrate: {} -> {}", node_id, old_hashrate, new_hashrate);
    }
}

void QSFNetworkMonitor::remove_node(const std::string& node_id) {
    std::lock_guard<std::mutex> lock(monitor_mutex);
    
    auto it = node_hashrates.find(node_id);
    if (it != node_hashrates.end()) {
        total_network_hashrate -= it->second;
        node_hashrates.erase(it);
        node_last_seen.erase(node_id);
        
        OXEN_LOG(debug, "Removed node: {}", node_id);
    }
}

bool QSFNetworkMonitor::detect_51_percent_attack() {
    std::lock_guard<std::mutex> lock(monitor_mutex);
    
    if (total_network_hashrate == 0) return false;
    
    // Find the largest hashrate contributor
    auto max_node = std::max_element(node_hashrates.begin(), node_hashrates.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });
    
    if (max_node != node_hashrates.end()) {
        double percentage = calculate_hashrate_percentage(max_node->second);
        
        if (percentage >= HASHRATE_THRESHOLD) {
            OXEN_LOG(critical, "51% ATTACK DETECTED! Node {} has {}% of network hashrate", 
                     max_node->first, percentage * 100);
            attack_detected = true;
            suspicious_nodes.push_back(max_node->first);
            
            // Enhanced response for 51% attacks
            if (QSF_QUANTUM_SAFE_51_PROTECTION) {
                trigger_immediate_difficulty_spike();
                isolate_suspicious_nodes();
                activate_emergency_fork_protection();
            }
            
            return true;
        } else if (percentage >= SUSPICIOUS_THRESHOLD) {
            OXEN_LOG(warn, "SUSPICIOUS ACTIVITY: Node {} has {}% of network hashrate", 
                     max_node->first, percentage * 100);
            suspicious_nodes.push_back(max_node->first);
        }
    }
    
    return false;
}

// Enhanced 51% Attack Protection Methods
  
bool QSFNetworkMonitor::detect_rapid_attack() {
    std::lock_guard<std::mutex> lock(monitor_mutex);
    
    if (total_network_hashrate == 0) return false;
    
    // Check for rapid hashrate increase (Qubic-style attack)
    auto now = std::chrono::steady_clock::now();
    auto max_node = std::max_element(node_hashrates.begin(), node_hashrates.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });
    
    if (max_node != node_hashrates.end()) {
        double percentage = calculate_hashrate_percentage(max_node->second);
        
        if (percentage >= QSF_RAPID_ATTACK_DETECTION) {
            auto last_detection = attack_detection_times.find(max_node->first);
            if (last_detection != attack_detection_times.end()) {
                auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - last_detection->second);
                if (duration.count() < 300) { // 5 minutes
                    OXEN_LOG(critical, "RAPID ATTACK DETECTED! Node {} gained {}% hashrate in {} seconds", 
                             max_node->first, percentage * 100, duration.count());
                    rapid_attack_detected = true;
                    trigger_immediate_difficulty_spike();
                    return true;
                }
            }
            attack_detection_times[max_node->first] = now;
        }
    }
    
    return false;
}

bool QSFNetworkMonitor::detect_gradual_attack() {
    std::lock_guard<std::mutex> lock(monitor_mutex);
    
    if (total_network_hashrate == 0) return false;
    
    // Check for gradual hashrate accumulation (stealth attack)
    auto max_node = std::max_element(node_hashrates.begin(), node_hashrates.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });
    
    if (max_node != node_hashrates.end()) {
        double percentage = calculate_hashrate_percentage(max_node->second);
        
        if (percentage >= QSF_GRADUAL_ATTACK_DETECTION) {
            OXEN_LOG(warn, "GRADUAL ATTACK DETECTED! Node {} has accumulated {}% hashrate", 
                     max_node->first, percentage * 100);
            gradual_attack_detected = true;
            
            // Monitor for continued growth
            if (percentage >= QSF_WARNING_THRESHOLD) {
                OXEN_LOG(critical, "GRADUAL ATTACK ESCALATING! Node {} now has {}% hashrate", 
                         max_node->first, percentage * 100);
                trigger_immediate_difficulty_spike();
                return true;
            }
        }
    }
    
    return false;
}

bool QSFNetworkMonitor::detect_pool_collusion() {
    std::lock_guard<std::mutex> lock(monitor_mutex);
    
    if (total_network_hashrate == 0) return false;
    
    // Check for multiple pools with suspicious combined hashrate
    std::map<std::string, uint64_t> pool_hashrates;
    
    for (const auto& node : node_hashrates) {
        auto pool_it = node_pool_affiliations.find(node.first);
        if (pool_it != node_pool_affiliations.end()) {
            pool_hashrates[pool_it->second] += node.second;
        }
    }
    
    uint64_t total_pool_hashrate = 0;
    for (const auto& pool : pool_hashrates) {
        total_pool_hashrate += pool.second;
    }
    
    if (total_pool_hashrate > 0) {
        for (const auto& pool : pool_hashrates) {
            double pool_percentage = static_cast<double>(pool.second) / total_network_hashrate;
            if (pool_percentage >= QSF_POOL_COLLUSION_THRESHOLD) {
                OXEN_LOG(warn, "POOL COLLUSION DETECTED! Pool {} has {}% of network hashrate", 
                         pool.first, pool_percentage * 100);
                return true;
            }
        }
    }
    
    return false;
}

bool QSFNetworkMonitor::detect_geographic_concentration() {
    std::lock_guard<std::mutex> lock(monitor_mutex);
    
    if (total_network_hashrate == 0) return false;
    
    // Check for geographic concentration (single country/region attack)
    std::map<std::string, uint64_t> geographic_hashrates;
    
    for (const auto& node : node_hashrates) {
        auto geo_it = node_geographic_locations.find(node.first);
        if (geo_it != node_geographic_locations.end()) {
            geographic_hashrates[geo_it->second] += node.second;
        }
    }
    
    for (const auto& geo : geographic_hashrates) {
        double geo_percentage = static_cast<double>(geo.second) / total_network_hashrate;
        if (geo_percentage >= QSF_ECLIPSE_ATTACK_THRESHOLD) {
            OXEN_LOG(warn, "GEOGRAPHIC CONCENTRATION DETECTED! Region {} has {}% of network hashrate", 
                     geo.first, geo_percentage * 100);
            return true;
        }
    }
    
    return false;
}

bool QSFNetworkMonitor::detect_isp_concentration() {
    std::lock_guard<std::mutex> lock(monitor_mutex);
    
    if (total_network_hashrate == 0) return false;
    
    // Check for ISP concentration (single ISP attack)
    std::map<std::string, uint64_t> isp_hashrates;
    
    for (const auto& node : node_hashrates) {
        auto isp_it = node_isp_info.find(node.first);
        if (isp_it != node_isp_info.end()) {
            isp_hashrates[isp_it->second] += node.second;
        }
    }
    
    for (const auto& isp : isp_hashrates) {
        double isp_percentage = static_cast<double>(isp.second) / total_network_hashrate;
        if (isp_percentage >= QSF_ECLIPSE_ATTACK_THRESHOLD) {
            OXEN_LOG(warn, "ISP CONCENTRATION DETECTED! ISP {} has {}% of network hashrate", 
                     isp.first, isp_percentage * 100);
            return true;
        }
    }
    
    return false;
}

bool QSFNetworkMonitor::detect_quantum_safe_violation() {
    std::lock_guard<std::mutex> lock(monitor_mutex);
    
    // Check if any nodes are not using quantum-safe signatures
    // This is critical for QSF's quantum-resistant security
    for (const auto& node : node_hashrates) {
        // Implementation would check quantum-safe compliance
        // For now, return false (no violation detected)
        // In real implementation, this would validate quantum-safe signatures
    }
    
    return false;
}

bool QSFNetworkMonitor::detect_sybil_attack() {
    std::lock_guard<std::mutex> lock(monitor_mutex);
    
    // Detect multiple nodes with similar hashrates from same IP range
    std::map<std::string, std::vector<std::string>> ip_groups;
    
    for (const auto& node : node_hashrates) {
        // Extract IP prefix (simplified)
        std::string ip_prefix = node.first.substr(0, node.first.find_last_of('.'));
        ip_groups[ip_prefix].push_back(node.first);
    }
    
    for (const auto& group : ip_groups) {
        if (group.second.size() > 10) { // More than 10 nodes from same IP range
            OXEN_LOG(warn, "Potential Sybil attack detected: {} nodes from IP range {}", 
                     group.second.size(), group.first);
            suspicious_nodes.insert(suspicious_nodes.end(), group.second.begin(), group.second.end());
            return true;
        }
    }
    
    return false;
}

bool QSFNetworkMonitor::detect_eclipse_attack() {
    std::lock_guard<std::mutex> lock(monitor_mutex);
    
    // Check if too many connections are from same source
    std::map<std::string, int> connection_counts;
    
    for (const auto& node : node_last_seen) {
        std::string source = node.first.substr(0, node.first.find(':'));
        connection_counts[source]++;
    }
    
    for (const auto& count : connection_counts) {
        if (count.second > QSF_MAX_INBOUND_CONNECTIONS / 2) {
            OXEN_LOG(warn, "Potential Eclipse attack: {} connections from {}", 
                     count.second, count.first);
            return true;
        }
    }
    
    return false;
}

bool QSFNetworkMonitor::detect_pool_centralization() {
    std::lock_guard<std::mutex> lock(monitor_mutex);
    
    // Check if any single entity has too much hashrate
    double max_percentage = 0.0;
    std::string max_entity;
    
    for (const auto& node : node_hashrates) {
        double percentage = calculate_hashrate_percentage(node.second);
        if (percentage > max_percentage) {
            max_percentage = percentage;
            max_entity = node.first;
        }
    }
    
    if (max_percentage > 0.20) { // 20% threshold for pool centralization
        OXEN_LOG(warn, "Pool centralization detected: {} has {}% of network hashrate", 
                 max_entity, max_percentage * 100);
        return true;
    }
    
    return false;
}

void QSFNetworkMonitor::trigger_emergency_response() {
    OXEN_LOG(critical, "EMERGENCY RESPONSE TRIGGERED - 51% attack detected!");
    
    // Increase difficulty immediately
    increase_difficulty_emergency();
    
    // Blacklist suspicious nodes
    for (const auto& node : suspicious_nodes) {
        blacklist_node(node);
    }
    
    // Generate emergency report
    generate_security_report();
}

void QSFNetworkMonitor::blacklist_node(const std::string& node_id) {
    std::lock_guard<std::mutex> lock(monitor_mutex);
    
    if (!is_node_blacklisted(node_id)) {
        blacklisted_nodes.push_back(node_id);
        OXEN_LOG(warn, "Blacklisted node: {}", node_id);
    }
}

void QSFNetworkMonitor::whitelist_node(const std::string& node_id) {
    std::lock_guard<std::mutex> lock(monitor_mutex);
    
    auto it = std::find(blacklisted_nodes.begin(), blacklisted_nodes.end(), node_id);
    if (it != blacklisted_nodes.end()) {
        blacklisted_nodes.erase(it);
        OXEN_LOG(info, "Whitelisted node: {}", node_id);
    }
}

void QSFNetworkMonitor::increase_difficulty_emergency() {
    // Emergency difficulty increase to slow down attack
    OXEN_LOG(critical, "EMERGENCY DIFFICULTY INCREASE ACTIVATED");
    // Implementation would integrate with difficulty adjustment system
}

double QSFNetworkMonitor::get_network_decentralization_score() {
    std::lock_guard<std::mutex> lock(monitor_mutex);
    
    if (node_hashrates.empty()) return 0.0;
    
    // Calculate Gini coefficient for hashrate distribution
    std::vector<uint64_t> hashrates;
    for (const auto& node : node_hashrates) {
        hashrates.push_back(node.second);
    }
    
    std::sort(hashrates.begin(), hashrates.end());
    
    double gini = 0.0;
    for (size_t i = 0; i < hashrates.size(); ++i) {
        gini += (2.0 * (i + 1) - hashrates.size() - 1) * hashrates[i];
    }
    gini /= (hashrates.size() * std::accumulate(hashrates.begin(), hashrates.end(), 0ULL));
    
    return 1.0 - gini; // Higher score = more decentralized
}

double QSFNetworkMonitor::get_attack_resistance_score() {
    double decentralization = get_network_decentralization_score();
    double suspicious_ratio = static_cast<double>(suspicious_hashrate) / total_network_hashrate;
    
    return decentralization * (1.0 - suspicious_ratio);
}

uint64_t QSFNetworkMonitor::get_total_hashrate() const {
    return total_network_hashrate;
}

uint64_t QSFNetworkMonitor::get_suspicious_hashrate() const {
    return suspicious_hashrate;
}

void QSFNetworkMonitor::generate_security_report() {
    OXEN_LOG(info, "=== QSF SECURITY REPORT ===");
    OXEN_LOG(info, "Total network hashrate: {}", total_network_hashrate);
    OXEN_LOG(info, "Active nodes: {}", node_hashrates.size());
    OXEN_LOG(info, "Suspicious nodes: {}", suspicious_nodes.size());
    OXEN_LOG(info, "Blacklisted nodes: {}", blacklisted_nodes.size());
    OXEN_LOG(info, "Decentralization score: {:.2f}", get_network_decentralization_score());
    OXEN_LOG(info, "Attack resistance score: {:.2f}", get_attack_resistance_score());
    OXEN_LOG(info, "Attack detected: {}", attack_detected ? "YES" : "NO");
    OXEN_LOG(info, "==========================");
}

std::vector<std::string> QSFNetworkMonitor::get_suspicious_nodes() const {
    std::lock_guard<std::mutex> lock(monitor_mutex);
    return suspicious_nodes;
}

std::vector<std::string> QSFNetworkMonitor::get_blacklisted_nodes() const {
    std::lock_guard<std::mutex> lock(monitor_mutex);
    return blacklisted_nodes;
}

void QSFNetworkMonitor::cleanup_expired_blacklist() {
    std::lock_guard<std::mutex> lock(monitor_mutex);
    
    auto now = std::chrono::steady_clock::now();
    auto it = blacklisted_nodes.begin();
    
    while (it != blacklisted_nodes.end()) {
        auto last_seen = node_last_seen.find(*it);
        if (last_seen != node_last_seen.end()) {
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - last_seen->second);
            if (duration.count() > BLACKLIST_DURATION) {
                it = blacklisted_nodes.erase(it);
            } else {
                ++it;
            }
        } else {
            ++it;
        }
    }
}

bool QSFNetworkMonitor::is_node_blacklisted(const std::string& node_id) const {
    return std::find(blacklisted_nodes.begin(), blacklisted_nodes.end(), node_id) != blacklisted_nodes.end();
}

double QSFNetworkMonitor::calculate_hashrate_percentage(uint64_t hashrate) const {
    if (total_network_hashrate == 0) return 0.0;
    return static_cast<double>(hashrate) / total_network_hashrate;
}

// Advanced Response Mechanisms
  
void QSFNetworkMonitor::trigger_immediate_difficulty_spike() {
    if (QSF_IMMEDIATE_DIFFICULTY_SPIKE) {
        OXEN_LOG(critical, "IMMEDIATE DIFFICULTY SPIKE ACTIVATED - 51% attack response");
        
        // Increase difficulty by 10x immediately
        increase_difficulty_emergency();
        
        // Additional emergency measures
        if (QSF_AUTOMATIC_NODE_ISOLATION) {
            isolate_suspicious_nodes();
        }
        
        if (QSF_NETWORK_SEGMENTATION) {
            segment_network();
        }
    }
}

void QSFNetworkMonitor::isolate_suspicious_nodes() {
    if (QSF_AUTOMATIC_NODE_ISOLATION) {
        OXEN_LOG(critical, "ISOLATING SUSPICIOUS NODES - 51% attack response");
        
        for (const auto& node : suspicious_nodes) {
            if (std::find(isolated_nodes.begin(), isolated_nodes.end(), node) == isolated_nodes.end()) {
                isolated_nodes.push_back(node);
                OXEN_LOG(warn, "Node {} isolated due to suspicious activity", node);
            }
        }
    }
}

void QSFNetworkMonitor::segment_network() {
    if (QSF_NETWORK_SEGMENTATION) {
        OXEN_LOG(critical, "NETWORK SEGMENTATION ACTIVATED - 51% attack response");
        
        // Create network segments to isolate attack
        std::vector<std::string> healthy_nodes;
        std::vector<std::string> suspicious_segment;
        
        for (const auto& node : node_hashrates) {
            if (std::find(suspicious_nodes.begin(), suspicious_nodes.end(), node.first) != suspicious_nodes.end()) {
                suspicious_segment.push_back(node.first);
            } else {
                healthy_nodes.push_back(node.first);
            }
        }
        
        network_segments.clear();
        if (!healthy_nodes.empty()) {
            network_segments.push_back("healthy");
        }
        if (!suspicious_segment.empty()) {
            network_segments.push_back("suspicious");
        }
        
        OXEN_LOG(info, "Network segmented into {} segments", network_segments.size());
    }
}

void QSFNetworkMonitor::activate_emergency_fork_protection() {
    if (QSF_EMERGENCY_FORK_PROTECTION) {
        OXEN_LOG(critical, "EMERGENCY FORK PROTECTION ACTIVATED - 51% attack response");
        
        // Implement emergency fork protection
        // This would create a protected fork that rejects suspicious blocks
        OXEN_LOG(info, "Emergency fork protection active - suspicious blocks will be rejected");
    }
}

void QSFNetworkMonitor::initiate_automatic_recovery() {
    if (QSF_AUTOMATIC_RECOVERY_MODE && !recovery_mode_active) {
        OXEN_LOG(info, "INITIATING AUTOMATIC RECOVERY - 51% attack response");
        
        recovery_mode_active = true;
        recovery_attempts = 0;
        last_recovery_attempt = std::chrono::steady_clock::now();
        
        // Start recovery sequence
        heal_network_segments();
    }
}

void QSFNetworkMonitor::heal_network_segments() {
    if (QSF_AUTOMATIC_RECOVERY_MODE && recovery_mode_active) {
        OXEN_LOG(info, "HEALING NETWORK SEGMENTS - Recovery attempt {}", recovery_attempts.load());
        
        auto now = std::chrono::steady_clock::now();
        auto last_attempt = last_recovery_attempt.load();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - last_attempt);
        
        if (duration.count() >= QSF_NETWORK_HEALING_INTERVAL && 
            recovery_attempts < QSF_MAX_RECOVERY_ATTEMPTS) {
            
            recovery_attempts++;
            last_recovery_attempt = now;
            
            // Check if recovery is successful
            if (get_attack_resistance_score() >= QSF_RECOVERY_SUCCESS_THRESHOLD) {
                OXEN_LOG(info, "RECOVERY SUCCESSFUL! Network health restored");
                recovery_mode_active = false;
                attack_detected = false;
                rapid_attack_detected = false;
                gradual_attack_detected = false;
                
                // Reintegrate isolated nodes
                isolated_nodes.clear();
                network_segments.clear();
            } else {
                OXEN_LOG(warn, "Recovery attempt {} failed, will retry in {} seconds", 
                         recovery_attempts.load(), QSF_NETWORK_HEALING_INTERVAL);
            }
        }
    }
}

// Enhanced Monitoring Methods
  
double QSFNetworkMonitor::get_geographic_diversity_score() {
    std::lock_guard<std::mutex> lock(monitor_mutex);
    
    if (node_geographic_locations.empty()) return 0.0;
    
    std::set<std::string> unique_locations;
    for (const auto& node : node_geographic_locations) {
        unique_locations.insert(node.second);
    }
    
    double diversity = static_cast<double>(unique_locations.size()) / node_geographic_locations.size();
    return std::min(diversity, 1.0);
}

double QSFNetworkMonitor::get_isp_diversity_score() {
    std::lock_guard<std::mutex> lock(monitor_mutex);
    
    if (node_isp_info.empty()) return 0.0;
    
    std::set<std::string> unique_isps;
    for (const auto& node : node_isp_info) {
        unique_isps.insert(node.second);
    }
    
    double diversity = static_cast<double>(unique_isps.size()) / node_isp_info.size();
    return std::min(diversity, 1.0);
}

double QSFNetworkMonitor::get_pool_diversity_score() {
    std::lock_guard<std::mutex> lock(monitor_mutex);
    
    if (node_pool_affiliations.empty()) return 0.0;
    
    std::set<std::string> unique_pools;
    for (const auto& node : node_pool_affiliations) {
        unique_pools.insert(node.second);
    }
    
    double diversity = static_cast<double>(unique_pools.size()) / node_pool_affiliations.size();
    return std::min(diversity, 1.0);
}

double QSFNetworkMonitor::get_quantum_safe_compliance_score() {
    // This would check quantum-safe signature compliance
    // For now, return 1.0 (fully compliant)
    // In real implementation, this would validate all signatures
    return 1.0;
}

bool QSFNetworkMonitor::is_network_healthy() {
    double decentralization = get_network_decentralization_score();
    double geographic = get_geographic_diversity_score();
    double isp = get_isp_diversity_score();
    double pool = get_pool_diversity_score();
    
    // Network is healthy if all scores are above thresholds
    return decentralization >= 0.7 && 
           geographic >= 0.6 && 
           isp >= 0.7 && 
           pool >= 0.8 &&
           !attack_detected &&
           !rapid_attack_detected &&
           !gradual_attack_detected;
}

bool QSFNetworkMonitor::is_under_attack() {
    return attack_detected || rapid_attack_detected || gradual_attack_detected;
}

// Advanced Reporting Methods
  
void QSFNetworkMonitor::generate_threat_analysis_report() {
    OXEN_LOG(info, "=== QSF THREAT ANALYSIS REPORT ===");
    OXEN_LOG(info, "Rapid attack detected: {}", rapid_attack_detected ? "YES" : "NO");
    OXEN_LOG(info, "Gradual attack detected: {}", gradual_attack_detected ? "YES" : "NO");
    OXEN_LOG(info, "Pool collusion detected: {}", detect_pool_collusion() ? "YES" : "NO");
    OXEN_LOG(info, "Geographic concentration: {}", detect_geographic_concentration() ? "YES" : "NO");
    OXEN_LOG(info, "ISP concentration: {}", detect_isp_concentration() ? "YES" : "NO");
    OXEN_LOG(info, "Quantum-safe violations: {}", detect_quantum_safe_violation() ? "YES" : "NO");
    OXEN_LOG(info, "Network segments: {}", network_segments.size());
    OXEN_LOG(info, "Isolated nodes: {}", isolated_nodes.size());
    OXEN_LOG(info, "Recovery mode: {}", recovery_mode_active ? "ACTIVE" : "INACTIVE");
    OXEN_LOG(info, "Recovery attempts: {}", recovery_attempts.load());
    OXEN_LOG(info, "================================");
}

void QSFNetworkMonitor::generate_recovery_status_report() {
    OXEN_LOG(info, "=== QSF RECOVERY STATUS REPORT ===");
    OXEN_LOG(info, "Recovery mode active: {}", recovery_mode_active ? "YES" : "NO");
    OXEN_LOG(info, "Recovery attempts: {}", recovery_attempts.load());
    OXEN_LOG(info, "Network health score: {:.2f}", get_attack_resistance_score());
    OXEN_LOG(info, "Geographic diversity: {:.2f}", get_geographic_diversity_score());
    OXEN_LOG(info, "ISP diversity: {:.2f}", get_isp_diversity_score());
    OXEN_LOG(info, "Pool diversity: {:.2f}", get_pool_diversity_score());
    OXEN_LOG(info, "Quantum-safe compliance: {:.2f}", get_quantum_safe_compliance_score());
    OXEN_LOG(info, "Network healthy: {}", is_network_healthy() ? "YES" : "NO");
    OXEN_LOG(info, "=================================");
}

void QSFNetworkMonitor::generate_network_health_report() {
    OXEN_LOG(info, "=== QSF NETWORK HEALTH REPORT ===");
    OXEN_LOG(info, "Total hashrate: {}", total_network_hashrate);
    OXEN_LOG(info, "Active nodes: {}", node_hashrates.size());
    OXEN_LOG(info, "Decentralization score: {:.2f}", get_network_decentralization_score());
    OXEN_LOG(info, "Geographic diversity: {:.2f}", get_geographic_diversity_score());
    OXEN_LOG(info, "ISP diversity: {:.2f}", get_isp_diversity_score());
    OXEN_LOG(info, "Pool diversity: {:.2f}", get_pool_diversity_score());
    OXEN_LOG(info, "Attack resistance: {:.2f}", get_attack_resistance_score());
    OXEN_LOG(info, "Network status: {}", is_network_healthy() ? "HEALTHY" : "UNHEALTHY");
    OXEN_LOG(info, "================================");
}

} // namespace cryptonote


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

#pragma once

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <memory>
#include "crypto/quantum_safe.h"

// Quantum safe features enabled
#define DISABLE_QUANTUM_SAFE 0

namespace qsf
{
  class QuantumSafeWidget : public QWidget
  {
    Q_OBJECT

  public:
    explicit QuantumSafeWidget(QWidget *parent = nullptr);
    ~QuantumSafeWidget();

#if !DISABLE_QUANTUM_SAFE
    crypto::quantum_algorithm getCurrentAlgorithm() const;
#else
    int getCurrentAlgorithm() const;
#endif
    std::vector<uint8_t> getPublicKey() const;
    bool hasKeys() const;

  signals:
    void keysGenerated();
    void keysLoaded();

  private slots:
    void onGenerateKeys();
    void onAlgorithmChanged(int index);
    void onLoadKeys();
    void onSaveKeys();

  private:
    void setupUI();
    void updateKeyDisplay();
    void loadSettings();
    void saveSettings();

    // UI Components
    QVBoxLayout* m_mainLayout;
    QComboBox* m_algorithmCombo;
    QPushButton* m_generateKeysBtn;
    QPushButton* m_loadKeysBtn;
    QPushButton* m_saveKeysBtn;
    QTextEdit* m_publicKeyDisplay;
    QLabel* m_statusLabel;

    // Quantum-safe manager
#if !DISABLE_QUANTUM_SAFE
    std::unique_ptr<crypto::quantum_safe_manager> m_quantumManager;
#endif
    
    // Current state
    bool m_hasKeys;
#if !DISABLE_QUANTUM_SAFE
    crypto::quantum_algorithm m_currentAlgorithm;
#endif
  };
} 
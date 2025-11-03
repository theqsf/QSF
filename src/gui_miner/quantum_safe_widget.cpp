// Copyright (c) 2024, QuantumSafeFoundation
//
// All rights reserved.
//
// See license header in headers.

#include "quantum_safe_widget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>

// Quantum safe features enabled
#define DISABLE_QUANTUM_SAFE 0

namespace qsf
{
  QuantumSafeWidget::QuantumSafeWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(new QVBoxLayout(this))
    , m_algorithmCombo(new QComboBox(this))
    , m_generateKeysBtn(new QPushButton("Generate Quantum-Safe Keys", this))
    , m_loadKeysBtn(new QPushButton("Load Keys", this))
    , m_saveKeysBtn(new QPushButton("Save Keys", this))
    , m_publicKeyDisplay(new QTextEdit(this))
    , m_statusLabel(new QLabel(this))
#if !DISABLE_QUANTUM_SAFE
    , m_quantumManager(new crypto::quantum_safe_manager())
#endif
    , m_hasKeys(false)
#if !DISABLE_QUANTUM_SAFE
    , m_currentAlgorithm(crypto::quantum_algorithm::XMSS)
#endif
  {
    setupUI();
    loadSettings();
  }

  QuantumSafeWidget::~QuantumSafeWidget()
  {
    saveSettings();
  }

  void QuantumSafeWidget::setupUI()
  {
    // Algorithm chooser
    QHBoxLayout *algoLayout = new QHBoxLayout();
    algoLayout->addWidget(new QLabel("Signature Algorithm:"));
    m_algorithmCombo->addItem("XMSS");
    m_algorithmCombo->addItem("SPHINCS+");
    algoLayout->addWidget(m_algorithmCombo);

    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(m_generateKeysBtn);
    btnLayout->addWidget(m_loadKeysBtn);
    btnLayout->addWidget(m_saveKeysBtn);

    // Public key display
    m_publicKeyDisplay->setReadOnly(true);
    m_publicKeyDisplay->setPlaceholderText("Public key will appear here...");

    // Status
    m_statusLabel->setText("No keys generated");

    m_mainLayout->addLayout(algoLayout);
    m_mainLayout->addLayout(btnLayout);
    m_mainLayout->addWidget(m_publicKeyDisplay);
    m_mainLayout->addWidget(m_statusLabel);

    connect(m_generateKeysBtn, &QPushButton::clicked, this, &QuantumSafeWidget::onGenerateKeys);
    connect(m_algorithmCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &QuantumSafeWidget::onAlgorithmChanged);
    connect(m_loadKeysBtn, &QPushButton::clicked, this, &QuantumSafeWidget::onLoadKeys);
    connect(m_saveKeysBtn, &QPushButton::clicked, this, &QuantumSafeWidget::onSaveKeys);
  }

#if !DISABLE_QUANTUM_SAFE
  crypto::quantum_algorithm QuantumSafeWidget::getCurrentAlgorithm() const
  {
    return m_currentAlgorithm;
  }

  std::vector<uint8_t> QuantumSafeWidget::getPublicKey() const
  {
    if (!m_hasKeys)
      return {};
    return m_quantumManager->get_public_key(m_currentAlgorithm);
  }

  bool QuantumSafeWidget::hasKeys() const
  {
    return m_hasKeys;
  }
#else
  int QuantumSafeWidget::getCurrentAlgorithm() const
  {
    return 0; // XMSS
  }

  std::vector<uint8_t> QuantumSafeWidget::getPublicKey() const
  {
    return {};
  }

  bool QuantumSafeWidget::hasKeys() const
  {
    return false;
  }
#endif

  void QuantumSafeWidget::onGenerateKeys()
  {
#if !DISABLE_QUANTUM_SAFE
    m_currentAlgorithm = (m_algorithmCombo->currentIndex() == 0)
      ? crypto::quantum_algorithm::XMSS
      : crypto::quantum_algorithm::SPHINCS_PLUS;

    if (!m_quantumManager->generate_keys(m_currentAlgorithm))
    {
      QMessageBox::warning(this, "Quantum-Safe Keys", "Failed to generate keys.");
      return;
    }
#else
    QMessageBox::information(this, "Quantum-Safe Keys", "Quantum-safe features are temporarily disabled for basic GUI miner.");
    return;
#endif

    m_hasKeys = true;
    updateKeyDisplay();
    m_statusLabel->setText("Keys generated successfully");
    emit keysGenerated();
  }

  void QuantumSafeWidget::onAlgorithmChanged(int index)
  {
    Q_UNUSED(index)
#if !DISABLE_QUANTUM_SAFE
    m_currentAlgorithm = (m_algorithmCombo->currentIndex() == 0)
      ? crypto::quantum_algorithm::XMSS
      : crypto::quantum_algorithm::SPHINCS_PLUS;
#endif
    updateKeyDisplay();
  }

  void QuantumSafeWidget::onLoadKeys()
  {
    // Placeholder: would show file dialog and load keys via manager
    QMessageBox::information(this, "Load Keys", "Key load UI not implemented in this prototype.");
  }

  void QuantumSafeWidget::onSaveKeys()
  {
    // Placeholder: would show file dialog and save keys via manager
    QMessageBox::information(this, "Save Keys", "Key save UI not implemented in this prototype.");
  }

  void QuantumSafeWidget::updateKeyDisplay()
  {
    if (!m_hasKeys)
    {
      m_publicKeyDisplay->clear();
      return;
    }

#if !DISABLE_QUANTUM_SAFE
    auto pk = m_quantumManager->get_public_key(m_currentAlgorithm);
    QString hex;
    hex.reserve(static_cast<int>(pk.size() * 2));
    const char *digits = "0123456789abcdef";
    for (uint8_t b : pk)
    {
      hex.append(digits[b >> 4]);
      hex.append(digits[b & 0x0f]);
    }
    m_publicKeyDisplay->setPlainText(hex);
#else
    m_publicKeyDisplay->setPlainText("Quantum-safe features disabled");
#endif
  }

  void QuantumSafeWidget::loadSettings()
  {
    QSettings st("QSFCoin", "QuantumSafeMiner");
    int algo = st.value("qs_algo", 0).toInt();
    m_algorithmCombo->setCurrentIndex(algo);
  }

  void QuantumSafeWidget::saveSettings()
  {
    QSettings st("QSFCoin", "QuantumSafeMiner");
    st.setValue("qs_algo", m_algorithmCombo->currentIndex());
  }
}

#include "quantum_safe_widget.moc" 
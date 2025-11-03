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

#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QTextStream>
#include <QFile>
#include <QDateTime>
#include <QIcon>
#include <QCoreApplication>
#include <QDebug>
#include "main_window.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  
  // Set application properties
  app.setApplicationName("QSF Quantum-Safe Miner");
  app.setApplicationVersion("2.0");
  app.setOrganizationName("QSF Coin Project");
  app.setOrganizationDomain("qsfcoin.com");
  
  // Set application icon (platform-specific)
  // Try multiple paths for the application icon
  QStringList iconPaths = {
    ":/icons/qsf_icon.ico",
    ":/icons/qsf_icon.png",
    QCoreApplication::applicationDirPath() + "/qsf_icon.ico",
    QCoreApplication::applicationDirPath() + "/qsf_icon.png"
  };
  
  bool iconSet = false;
  for (const QString& path : iconPaths) {
    QIcon icon(path);
    if (!icon.isNull()) {
      app.setWindowIcon(icon);
      iconSet = true;
      qDebug() << "Application icon loaded from:" << path;
      break;
    }
  }
  
  if (!iconSet) {
    qDebug() << "Failed to load application icon from all paths";
  }
  
  // Set application style
  app.setStyle(QStyleFactory::create("Fusion"));
  
  // Create application directory if it doesn't exist
  QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir().mkpath(appDataPath);
  
  // Setup logging
  QString logPath = appDataPath + "/miner.log";
  QFile logFile(logPath);
  if (logFile.open(QIODevice::WriteOnly | QIODevice::Append))
  {
    QTextStream logStream(&logFile);
    logStream << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") 
              << " - QSF Quantum-Safe Miner started\n";
    logFile.close();
  }
  
  try
  {
    // Create and show main window
    qsf::MainWindow mainWindow;
    mainWindow.show();
    
    // Start event loop
    return app.exec();
  }
  catch (const std::exception& e)
  {
    QMessageBox::critical(nullptr, "Fatal Error", 
                         QString("Failed to start QSF Quantum-Safe Miner: %1").arg(e.what()));
    return 1;
  }
  catch (...)
  {
    QMessageBox::critical(nullptr, "Fatal Error", 
                         "Failed to start QSF Quantum-Safe Miner: Unknown error");
    return 1;
  }
} 
// Copyright (c) 2011-2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/forms/ui_openuridialog.h>
#include <qt/openuridialog.h>

#include <qt/guiutil.h>
#include <qt/walletmodel.h>

#include <QUrl>

OpenURIDialog::OpenURIDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::OpenURIDialog) {
  ui->setupUi(this);
#if QT_VERSION >= 0x040700
  ui->uriEdit->setPlaceholderText("lightningcashr:");
#endif
}

OpenURIDialog::~OpenURIDialog() { delete ui; }

QString OpenURIDialog::getURI() { return ui->uriEdit->text(); }

void OpenURIDialog::accept() {
  SendCoinsRecipient rcp;
  if (GUIUtil::parseBitcoinURI(getURI(), &rcp)) {
    QDialog::accept();
  } else {
    ui->uriEdit->setValid(false);
  }
}

void OpenURIDialog::on_selectFileButton_clicked() {
  QString filename = GUIUtil::getOpenFileName(
      this, tr("Select payment request file to open"), "", "", nullptr);
  if (filename.isEmpty())
    return;
  QUrl fileUri = QUrl::fromLocalFile(filename);
  ui->uriEdit->setText("lightningcashr:?r=" +
                       QUrl::toPercentEncoding(fileUri.toString()));
}

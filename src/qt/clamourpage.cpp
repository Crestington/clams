#include "clamourpage.h"
#include "ui_clamourpage.h"
#include "openssl/sha.h"
#include "conspeech.h"
#include "main.h"
#include "util.h"
#include "walletmodel.h"

#include <QDebug>
#include <QMessageBox>

ClamourPage::ClamourPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClamourPage),
    model(0)
{
    ui->setupUi(this);
    ui->createPetitionButton->setEnabled(false);
    ui->setVoteCheckBox->setEnabled(false);

    ui->searchClamourTable->setColumnCount(1);
    ui->searchClamourTable->setRowCount(4);
    QStringList tableHeaders = (QStringList() << tr("Height") << tr("TxID") << tr("Petition Hash") << tr("URL"));
    ui->searchClamourTable->setVerticalHeaderLabels(tableHeaders);

    QStringList horizontalHeaders = (QStringList() << tr("Petition"));
    ui->searchClamourTable->setHorizontalHeaderLabels(horizontalHeaders);
    ui->searchClamourTable->horizontalHeader()->setStretchLastSection(true);

    clearSearchTable();
}

ClamourPage::~ClamourPage()
{
    delete ui;
}

void ClamourPage::clearSearchTable()
{
    ui->searchClamourTable->clearContents();
    ui->searchClamourTable->setColumnCount(1);
    ui->searchClamourTable->setRowCount(4);
    for (int i = 0; i < ui->searchClamourTable->rowCount(); i++) {
        for (int j = 0; j < ui->searchClamourTable->columnCount(); j++) {
            QTableWidgetItem *item = new QTableWidgetItem("");
            item->setFlags(item->flags() ^ Qt::ItemIsEditable);
            ui->searchClamourTable->setItem(i, j, item);
        }
    }
}

// Calculate notary ID when text changes.
void ClamourPage::on_createPetitionEdit_textChanged()
{
    std::string petitionText(ui->createPetitionEdit->toPlainText().toStdString());
    if (petitionText.length() == 0)
    {
        ui->petitionIDEdit->clear();
        ui->createPetitionButton->setEnabled(false);
        ui->setVoteCheckBox->setEnabled(false);
        return;
    }
    ui->createPetitionButton->setEnabled(true);
    ui->setVoteCheckBox->setEnabled(true);
    std::string petitionHash(StrToSHA256(petitionText));
    ui->petitionIDEdit->setText(QString::fromStdString(petitionHash));
}

// Create a tx that creates a petitition
void ClamourPage::on_createPetitionButton_clicked()
{
    std::string petitionHash(ui->petitionIDEdit->text().toStdString());

    WalletModel::UnlockContext ctx(model->requestUnlock());
    if (!ctx.isValid()) {
        return;
    }

    model->sendClamourTx(petitionHash);

    if (ui->setVoteCheckBox->isChecked())
    {
        strDefaultStakeSpeech = "clamour " + petitionHash.substr(0, 8);
        clamourConSpeech.push_back(strDefaultStakeSpeech);
        qDebug() << "saving clamour petitions";
        if ( !SaveClamourConSpeech() )
            qDebug() << "Clamour CONspeech petitions FAILED to save!";
        loadVotes();
    }
}

void ClamourPage::on_setVotesButton_clicked()
{
    saveVotes();
}

void ClamourPage::loadVotes()
{
    QStringList list;
    for (std::vector<std::string>::iterator it = clamourConSpeech.begin(); it != clamourConSpeech.end(); ++it)
    {
        list.append(QString::fromStdString(*it).mid(8));
    }
    ui->votesEdit->setPlainText(list.join("\n"));
}

void ClamourPage::saveVotes()
{
    QStringList list = ui->votesEdit->toPlainText().replace("\n", ",").replace(" ", ",").split(',', QString::SkipEmptyParts);
    std::vector<std::string> newSpeeches;
    clamourConSpeech.clear();

    if (list.length() > 0)
    {
        newSpeeches.push_back("clamour");
        foreach ( const QString &strLine, list )
            if ( !strLine.isEmpty() && strLine.length() >= 8 && IsHex(strLine.toStdString()) )
            {
                // Create new string if necessary
                if (newSpeeches.back().length() > MAX_TX_COMMENT_LEN - 9)
                {
                    newSpeeches.push_back("clamour");
                }
                std::string &newSpeech = newSpeeches.back();
                newSpeech = newSpeech + " " + strLine.trimmed().left(8).toStdString();
            }


        for (std::vector<std::string>::iterator it = newSpeeches.begin(); it != newSpeeches.end(); ++it)
        {
            clamourConSpeech.push_back(*it);
        }
    }

    // save new speech
    qDebug() << "saving clamour petitions";
    if ( !SaveClamourConSpeech() )
        qDebug() << "Clamour CONspeech petitions FAILED to save!";

    loadVotes();
}

void ClamourPage::showClamourTxResult(std::string txID, std::string txError)
{
    if (txError == "") {
        std::string txSentMsg = "Clamour petition created successfully: " + txID;
        QMessageBox::information(this, tr("Create Clamour Petition"),
            tr(txSentMsg.c_str()),
            QMessageBox::Ok, QMessageBox::Ok);
        ui->createPetitionButton->setEnabled(false);
        ui->setVoteCheckBox->setEnabled(false);
    } else {
        QMessageBox::warning(this, tr("Create Clamour Petition"),
            tr(txError.c_str()),
            QMessageBox::Ok, QMessageBox::Ok);
    }
}

void ClamourPage::setClamourSearchResults(CClamour *pResult)
{
    if (!pResult)
    {
        LogPrintf("No clamour results.\n");
        QMessageBox::warning(this, tr("Clamour Search"),
            tr("No clamour petition found."),
            QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    QTableWidgetItem *heightItem = new QTableWidgetItem(QString::number(pResult->nHeight));
    heightItem->setFlags(heightItem->flags() ^ Qt::ItemIsEditable);
    QTableWidgetItem *txidItem = new QTableWidgetItem(QString::fromStdString(pResult->txid.GetHex()));
    txidItem->setFlags(txidItem->flags() ^ Qt::ItemIsEditable);
    QTableWidgetItem *hashItem = new QTableWidgetItem(QString::fromStdString(pResult->strHash));
    hashItem->setFlags(hashItem->flags() ^ Qt::ItemIsEditable);
    QTableWidgetItem *urlItem = new QTableWidgetItem(QString::fromStdString(pResult->strURL));
    urlItem->setFlags(urlItem->flags() ^ Qt::ItemIsEditable);
    ui->searchClamourTable->setItem(0, 0, heightItem);
    ui->searchClamourTable->setItem(1, 0, txidItem);
    ui->searchClamourTable->setItem(2, 0, hashItem);
    ui->searchClamourTable->setItem(3, 0, urlItem);
}

void ClamourPage::setModel(WalletModel *model)
{
    this->model = model;
    connect(this->model, SIGNAL(clamourTxSent(std::string, std::string)), this, SLOT(showClamourTxResult(std::string, std::string)));
    connect(this->model, SIGNAL(clamourSearchComplete(CClamour*)), this, SLOT(setClamourSearchResults(CClamour*)));
    loadVotes();
}

void ClamourPage::on_searchClamourButton_clicked()
{
    std::string pid(ui->searchClamourEdit->text().toStdString());
    if (!(IsHex(pid) && pid.length() == 8)) {
        ui->searchClamourEdit->setValid(false);
        return;
    }
    clearSearchTable();
    model->searchClamours(pid);
}


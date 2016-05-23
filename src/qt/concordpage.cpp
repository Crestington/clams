#include "concordpage.h"
#include "ui_concordpage.h"
#include "openssl/sha.h"
#include "conspeech.h"
#include "main.h"
#include "util.h"
#include "walletmodel.h"

#include <QDebug>
#include <QMessageBox>

ConcordPage::ConcordPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConcordPage),
    model(0)
{
    ui->setupUi(this);
    ui->createPetitionButton->setEnabled(false);
    ui->setVoteCheckBox->setEnabled(false);

    ui->searchConcordTable->setColumnCount(1);
    ui->searchConcordTable->setRowCount(4);
    QStringList tableHeaders = (QStringList() << tr("Height") << tr("TxID") << tr("Petition Hash") << tr("URL"));
    ui->searchConcordTable->setVerticalHeaderLabels(tableHeaders);

    QStringList horizontalHeaders = (QStringList() << tr("Petition"));
    ui->searchConcordTable->setHorizontalHeaderLabels(horizontalHeaders);
    ui->searchConcordTable->horizontalHeader()->setStretchLastSection(true);

    clearSearchTable();
}

ConcordPage::~ConcordPage()
{
    delete ui;
}

void ConcordPage::clearSearchTable()
{
    ui->searchConcordTable->clearContents();
    ui->searchConcordTable->setColumnCount(1);
    ui->searchConcordTable->setRowCount(4);
    for (int i = 0; i < ui->searchConcordTable->rowCount(); i++) {
        for (int j = 0; j < ui->searchConcordTable->columnCount(); j++) {
            QTableWidgetItem *item = new QTableWidgetItem("");
            item->setFlags(item->flags() ^ Qt::ItemIsEditable);
            ui->searchConcordTable->setItem(i, j, item);
        }
    }
}

// Calculate notary ID when text changes.
void ConcordPage::on_createPetitionEdit_textChanged()
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
void ConcordPage::on_createPetitionButton_clicked()
{
    std::string petitionHash(ui->petitionIDEdit->text().toStdString());

    WalletModel::UnlockContext ctx(model->requestUnlock());
    if (!ctx.isValid()) {
        return;
    }

    model->sendConcordTx(petitionHash);

    if (ui->setVoteCheckBox->isChecked())
    {
        strDefaultStakeSpeech = "concord " + petitionHash.substr(0, 8);
        concordConSpeech.push_back(strDefaultStakeSpeech);
        qDebug() << "saving concord petitions";
        if ( !SaveConcordConSpeech() )
            qDebug() << "Concord CONspeech petitions FAILED to save!";
        loadVotes();
    }
}

void ConcordPage::on_setVotesButton_clicked()
{
    saveVotes();
}

void ConcordPage::loadVotes()
{
    QStringList list;
    for (std::vector<std::string>::iterator it = concordConSpeech.begin(); it != concordConSpeech.end(); ++it)
    {
        list.append(QString::fromStdString(*it).mid(8));
    }
    ui->votesEdit->setPlainText(list.join("\n"));
}

void ConcordPage::saveVotes()
{
    QStringList list = ui->votesEdit->toPlainText().replace("\n", ",").replace(" ", ",").split(',', QString::SkipEmptyParts);
    std::vector<std::string> newSpeeches;
    concordConSpeech.clear();

    if (list.length() > 0)
    {
        newSpeeches.push_back("concord");
        foreach ( const QString &strLine, list )
            if ( !strLine.isEmpty() && strLine.length() >= 8 && IsHex(strLine.toStdString()) )
            {
                // Create new string if necessary
                if (newSpeeches.back().length() > MAX_TX_COMMENT_LEN - 9)
                {
                    newSpeeches.push_back("concord");
                }
                std::string &newSpeech = newSpeeches.back();
                newSpeech = newSpeech + " " + strLine.trimmed().left(8).toStdString();
            }


        for (std::vector<std::string>::iterator it = newSpeeches.begin(); it != newSpeeches.end(); ++it)
        {
            concordConSpeech.push_back(*it);
        }
    }

    // save new speech
    qDebug() << "saving concord petitions";
    if ( !SaveConcordConSpeech() )
        qDebug() << "Concord CONspeech petitions FAILED to save!";

    loadVotes();
}

void ConcordPage::showConcordTxResult(std::string txID, std::string txError)
{
    if (txError == "") {
        std::string txSentMsg = "Concord petition created successfully: " + txID;
        QMessageBox::information(this, tr("Create Concord Petition"),
            tr(txSentMsg.c_str()),
            QMessageBox::Ok, QMessageBox::Ok);
        ui->createPetitionButton->setEnabled(false);
        ui->setVoteCheckBox->setEnabled(false);
    } else {
        QMessageBox::warning(this, tr("Create Concord Petition"),
            tr(txError.c_str()),
            QMessageBox::Ok, QMessageBox::Ok);
    }
}

void ConcordPage::setConcordSearchResults(CConcord *pResult)
{
    if (!pResult)
    {
        LogPrintf("No concord results.\n");
        QMessageBox::warning(this, tr("Concord Search"),
            tr("No concord petition found."),
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
    ui->searchConcordTable->setItem(0, 0, heightItem);
    ui->searchConcordTable->setItem(1, 0, txidItem);
    ui->searchConcordTable->setItem(2, 0, hashItem);
    ui->searchConcordTable->setItem(3, 0, urlItem);
}

void ConcordPage::setModel(WalletModel *model)
{
    this->model = model;
    connect(this->model, SIGNAL(concordTxSent(std::string, std::string)), this, SLOT(showConcordTxResult(std::string, std::string)));
    connect(this->model, SIGNAL(concordSearchComplete(CConcord*)), this, SLOT(setConcordSearchResults(CConcord*)));
    loadVotes();
}

void ConcordPage::on_searchConcordButton_clicked()
{
    std::string pid(ui->searchConcordEdit->text().toStdString());
    if (!(IsHex(pid) && pid.length() == 8)) {
        ui->searchConcordEdit->setValid(false);
        return;
    }
    clearSearchTable();
    model->searchConcords(pid);
}


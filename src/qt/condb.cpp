#include "condb.h"
#include "ui_condb.h"
#include "walletmodel.h"
#include "concordpage.h"
#include "notarypage.h"

ConDB::ConDB(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConDB),
    model(0),
    concordPage(0),
    notaryPage(0)
{
    ui->setupUi(this);

    notaryPage = new NotaryPage();
    ui->stackedWidget->addWidget(this->notaryPage);
    ui->pageList->addItem("Notary");

    concordPage = new ConcordPage();
    ui->stackedWidget->addWidget(this->concordPage);
    ui->pageList->addItem("CONcord");

    connect(ui->pageList, SIGNAL(currentRowChanged(int)), ui->stackedWidget, SLOT(setCurrentIndex(int)));
}

ConDB::~ConDB()
{
    delete ui;
}

void ConDB::setModel(WalletModel *model)
{
    this->model = model;
    this->concordPage->setModel(model);
    this->notaryPage->setModel(model);
    ui->pageList->setCurrentRow(0);
}


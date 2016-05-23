#ifndef CONDB_H
#define CONDB_H

#include <QWidget>

namespace Ui {
class ConDB;
}
class WalletModel;
class ConcordPage;
class NotaryPage;

class ConDB : public QWidget
{
    Q_OBJECT

public:
    explicit ConDB(QWidget *parent = 0);
    ~ConDB();

    void setModel(WalletModel *model);

private:
    Ui::ConDB *ui;
    WalletModel *model;
    ConcordPage *concordPage;
    NotaryPage *notaryPage;
};

#endif // CONDB_H

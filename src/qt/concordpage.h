#ifndef CONCORDPAGE_H
#define CONCORDPAGE_H

#include <QWidget>

namespace Ui {
class ConcordPage;
}
class WalletModel;
class CConcord;

class ConcordPage : public QWidget
{
    Q_OBJECT

public:
    explicit ConcordPage(QWidget *parent = 0);
    ~ConcordPage();

    void setModel(WalletModel *model);

public slots:
    void showConcordTxResult(std::string txID, std::string txError);
    void setConcordSearchResults(CConcord *pResult);

private slots:
    void on_createPetitionEdit_textChanged();

    void on_createPetitionButton_clicked();

    void on_setVotesButton_clicked();

    void on_searchConcordButton_clicked();

private:
    Ui::ConcordPage *ui;
    WalletModel *model;

    void loadVotes();
    void saveVotes();
    void clearSearchTable();
};

#endif // CONCORDPAGE_H

#ifndef ADDPAGEDIALOG_H
#define ADDPAGEDIALOG_H

#include <QDialog>

namespace Ui {
class AddPageDialog;
}

class QSqlDatabase;
struct addpagedialog_p;

class AddPageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddPageDialog(const QSqlDatabase& db, QWidget *parent = 0);
    ~AddPageDialog();

public slots:
    virtual void accept();

private:
    Ui::AddPageDialog *ui;
    addpagedialog_p *self;
    void loadTags();

private slots:
    void loadPageTags();
    void addTagToPage();
    void removeTagFromPage();
    int insertPage();
    int insertTag(const QString&);
};

#endif // ADDPAGEDIALOG_H

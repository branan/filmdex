#ifndef FILMDEX_H
#define FILMDEX_H

#include <QDialog>

namespace Ui {
class FilmDex;
}

struct FilmDex_p;

class FilmDex : public QDialog
{
    Q_OBJECT

public:
    explicit FilmDex(QWidget *parent = 0);
    ~FilmDex();

public slots:
    void findForTag(QString tag);
    void addPage(void);

private:
    Ui::FilmDex *ui;
    FilmDex_p *self;

    void updateTags();
};

#endif // FILMDEX_H

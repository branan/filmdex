#include "filmdex.h"
#include "ui_filmdex.h"
#include "addpagedialog.h"

#include <QtSql>
#include <QStandardPaths>
#include <QMessageBox>
#include <QDir>

struct FilmDex_p {
    QSqlDatabase db;
    QMessageBox mb;
};

FilmDex::FilmDex(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FilmDex)
{
    self = new FilmDex_p;
    ui->setupUi(this);
    connect(ui->TagSelector, SIGNAL(currentIndexChanged(QString)), this, SLOT(findForTag(QString)));
    connect(ui->AddItem, SIGNAL(clicked()), this, SLOT(addPage()));

    self->mb.setIcon(QMessageBox::Critical);
    self->mb.setStandardButtons(QMessageBox::Ok);

    QString db_dir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + QDir::separator();
    QDir dir;
    dir.mkpath(db_dir);
    self->db = QSqlDatabase::addDatabase("QSQLITE", "catalog");
    self->db.setDatabaseName(db_dir + "filmdex.sqlite");
    if(!self->db.open()) {
        self->mb.setText(self->db.lastError().text() + self->db.databaseName());
        self->mb.exec();
    }

    QSqlQuery result = self->db.exec("CREATE TABLE IF NOT EXISTS keywords (id INTEGER PRIMARY KEY AUTOINCREMENT, word TEXT);");
    if(result.lastError().type() != QSqlError::NoError) {
        self->mb.setText(result.lastError().text());
        self->mb.exec();
    }

    result = self->db.exec("CREATE TABLE IF NOT EXISTS pages (id INTEGER PRIMARY KEY AUTOINCREMENT, year INTEGER, month TEXT, page INTEGER);");
    if(result.lastError().type() != QSqlError::NoError) {
        self->mb.setText(result.lastError().text());
        self->mb.exec();
    }

    result = self->db.exec("CREATE TABLE IF NOT EXISTS page_tags (tag INTEGER, page INTEGER);");
    if(result.lastError().type() != QSqlError::NoError) {
        self->mb.setText(result.lastError().text());
        self->mb.exec();
    }

    updateTags();
}

FilmDex::~FilmDex()
{
    delete ui;
}

void FilmDex::findForTag(QString tag) {
    int tag_id = 0;
    QStringList locations;
    QSqlQuery query = self->db.exec(QString("SELECT id FROM keywords WHERE word = \"%1\";").arg(tag));
    if(query.lastError().type() != QSqlError::NoError) {
        self->mb.setText(query.lastError().text());
        self->mb.exec();
    } else if(query.next()) {
        tag_id = query.value(0).toInt();
    }

    if(!tag_id)
        return;

    query = self->db.exec(QString("SELECT page FROM page_tags WHERE tag = %1;").arg(tag_id));
    if(query.lastError().type() != QSqlError::NoError) {
        self->mb.setText(query.lastError().text());
        self->mb.exec();
    } else {
        while(query.next()) {
            int page_id = query.value(0).toInt();
            QSqlQuery entry = self->db.exec(QString("SELECT year, month, page FROM pages WHERE id = %1;").arg(page_id));
            if (entry.lastError().type() != QSqlError::NoError) {
                self->mb.setText(entry.lastError().text());
                self->mb.exec();
            } else if(entry.next()) {
                int year, page;
                QString month;
                year = entry.value(0).toInt();
                month = entry.value(1).toString();
                page = entry.value(2).toInt();
                QString line = QString("%1 %2, page %3").arg(month).arg(year).arg(page);
                locations << line;
            }
        }
    }
    ui->PageList->clear();
    ui->PageList->addItems(locations);
}

void FilmDex::updateTags() {
    QStringList tags;
    QString current_tag = ui->TagSelector->currentText();
    QSqlQuery result = self->db.exec("SELECT word FROM keywords");
    if(result.lastError().type() != QSqlError::NoError) {
        self->mb.setText(result.lastError().text());
        self->mb.exec();
    } else {
        while(result.next()) {
            tags << result.value(0).toString();
        }
    }
    tags.sort(Qt::CaseInsensitive);
    ui->TagSelector->clear();
    ui->TagSelector->addItems(tags);
    ui->TagSelector->setCurrentText(current_tag);
}

void FilmDex::addPage() {
    AddPageDialog dlg(self->db, this);
    dlg.exec();
    updateTags();
}

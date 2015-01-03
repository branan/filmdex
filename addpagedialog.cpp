#include "addpagedialog.h"
#include "ui_addpagedialog.h"

#include <QtSql>
#include <QMessageBox>

struct addpagedialog_p {
    QSqlDatabase db;
    QMessageBox mb;
};

AddPageDialog::AddPageDialog(const QSqlDatabase& db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddPageDialog)
{
    ui->setupUi(this);
    ui->monthDropdown->addItem("January");
    ui->monthDropdown->addItem("February");
    ui->monthDropdown->addItem("March");
    ui->monthDropdown->addItem("April");
    ui->monthDropdown->addItem("May");
    ui->monthDropdown->addItem("June");
    ui->monthDropdown->addItem("July");
    ui->monthDropdown->addItem("Auguest");
    ui->monthDropdown->addItem("September");
    ui->monthDropdown->addItem("October");
    ui->monthDropdown->addItem("November");
    ui->monthDropdown->addItem("December");

    self = new addpagedialog_p;
    self->db = db;
    self->mb.setIcon(QMessageBox::Critical);
    self->mb.setStandardButtons(QMessageBox::Ok);

    connect(ui->loadTagsButton, SIGNAL(clicked()), this, SLOT(loadPageTags()));
    connect(ui->addTagButton, SIGNAL(clicked()), this, SLOT(addTagToPage()));
    connect(ui->delTagButton, SIGNAL(clicked()), this, SLOT(removeTagFromPage()));

    loadTags();
}

AddPageDialog::~AddPageDialog()
{
    delete ui;
}

void AddPageDialog::accept()
{
    int page_id = insertPage();
    if(!page_id) return;

    QSqlQuery query = self->db.exec(QString("DELETE FROM page_tags WHERE page = %1").arg(page_id));
    if(query.lastError().type() != QSqlError::NoError) {
        self->mb.setText(query.lastError().text());
        self->mb.exec();
        return;
    }

    for(int i = 0; i < ui->tagsList->count(); i++) {
        QString tag = ui->tagsList->item(i)->text();
        int tag_id = insertTag(tag);
        if(!tag_id) continue;

        query = self->db.exec(QString("INSERT INTO page_tags (page, tag) VALUES (%1, %2)").arg(page_id).arg(tag_id));
        if(query.lastError().type() != QSqlError::NoError) {
            self->mb.setText(query.lastError().text());
            self->mb.exec();
            return;
        }
    }
    ui->tagsList->count();
    QDialog::accept();
}

void AddPageDialog::loadPageTags() {
    int page_id = 0;
    QStringList tags;

    QString month  = ui->monthDropdown->currentText();
    int year = ui->yearSpinner->value();
    int page = ui->pageSpinner->value();
    QSqlQuery query = self->db.exec(QString("SELECT id FROM pages WHERE year = %1 AND month = \"%2\" AND page = %3").arg(year).arg(month).arg(page));
    if(query.lastError().type() != QSqlError::NoError) {
        self->mb.setText(query.lastError().text());
        self->mb.exec();
    } else if (query.next()){
        page_id = query.value(0).toInt();
    }
    if(!page_id)
        return;

    query = self->db.exec(QString("SELECT tag FROM page_tags WHERE page = %1").arg(page_id));
    if(query.lastError().type() != QSqlError::NoError) {
        self->mb.setText(query.lastError().text());
        self->mb.exec();
    } else {
        while(query.next()) {
            int tag_id = query.value(0).toInt();
            QSqlQuery tag_query = self->db.exec(QString("SELECT word from keywords WHERE id = %1").arg(tag_id));
            if(tag_query.lastError().type() != QSqlError::NoError) {
                self->mb.setText(tag_query.lastError().text());
                self->mb.exec();
            } else if (tag_query.next()){
                tags << tag_query.value(0).toString();
            }
        }
    }
    ui->tagsList->clear();
    ui->tagsList->addItems(tags);
}

void AddPageDialog::loadTags() {
    QStringList tags;
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
    ui->tagsDropdown->clear();
    ui->tagsDropdown->addItems(tags);
}

void AddPageDialog::addTagToPage() {
    ui->tagsList->addItem(ui->tagsDropdown->currentText());
}

void AddPageDialog::removeTagFromPage() {
    delete ui->tagsList->currentItem();
}

int AddPageDialog::insertPage() {
    int page_id = 0;

    QString month  = ui->monthDropdown->currentText();
    int year = ui->yearSpinner->value();
    int page = ui->pageSpinner->value();
    QSqlQuery query = self->db.exec(QString("SELECT id FROM pages WHERE year = %1 AND month = \"%2\" AND page = %3").arg(year).arg(month).arg(page));
    if(query.lastError().type() != QSqlError::NoError) {
        self->mb.setText(query.lastError().text());
        self->mb.exec();
    } else if (query.next()){
        page_id = query.value(0).toInt();
    } else {
        query = self->db.exec(QString("INSERT INTO pages (year, month, page) VALUES (%1, \"%2\", %3);").arg(year).arg(month).arg(page));
        if(query.lastError().type() != QSqlError::NoError) {
            self->mb.setText(query.lastError().text());
            self->mb.exec();
        } else {
            query = self->db.exec("SELECT last_insert_rowid();");
            if(query.lastError().type() != QSqlError::NoError) {
                self->mb.setText(query.lastError().text());
                self->mb.exec();
            } else if (query.next()) {
                page_id = query.value(0).toInt();
            }
        }
    }

    return page_id;
}

int AddPageDialog::insertTag(const QString& tag) {
    int tag_id = 0;

    QSqlQuery query = self->db.exec(QString("SELECT id FROM keywords WHERE word = \"%1\";").arg(tag));
    if(query.lastError().type() != QSqlError::NoError) {
        self->mb.setText(query.lastError().text());
        self->mb.exec();
    } else if (query.next()){
        tag_id = query.value(0).toInt();
    } else {
        query = self->db.exec(QString("INSERT INTO keywords (word) VALUES (\"%1\");").arg(tag));
        if(query.lastError().type() != QSqlError::NoError) {
            self->mb.setText(query.lastError().text());
            self->mb.exec();
        } else {
            query = self->db.exec("SELECT last_insert_rowid();");
            if(query.lastError().type() != QSqlError::NoError) {
                self->mb.setText(query.lastError().text());
                self->mb.exec();
            } else if (query.next()) {
                tag_id = query.value(0).toInt();
            }
        }
    }

    return tag_id;
}

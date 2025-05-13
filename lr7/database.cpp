#include "database.h"
#include <QDebug>
#include <QStandardPaths>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

Database::Database(QObject *parent) : QObject(parent)
{
}

Database::~Database()
{
    closeDatabase();
}

bool Database::connectToDatabase(const QString &path)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(path);

    if (!m_db.open()) {
        qDebug() << "Error: connection with database failed:" << m_db.lastError().text();
        return false;
    }

    // Отключаем внешние ключи для диагностики
    QSqlQuery query;
    query.exec("PRAGMA foreign_keys = OFF;");
    qDebug() << "Foreign keys disabled for debugging";

    // Создаем таблицы, если они не существуют
    query.exec("CREATE TABLE IF NOT EXISTS books ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "title TEXT, "
               "author TEXT, "
               "year INTEGER, "
               "genre TEXT, "
               "available BOOLEAN)");
    query.exec("CREATE TABLE IF NOT EXISTS readers ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "name TEXT, "
               "contact TEXT)");
    query.exec("CREATE TABLE IF NOT EXISTS loans ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "book_id INTEGER, "
               "reader_id INTEGER, "
               "issue_date TEXT)");
    qDebug() << "Database: connection ok, tables created";
    return true;
}

void Database::closeDatabase()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool Database::addBook(const QString &title, const QString &author, int year, const QString &genre, bool available)
{
    if (!m_db.isOpen()) {
        qDebug() << "Add book error: database is not open";
        return false;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO books (title, author, year, genre, available) "
                  "VALUES (:title, :author, :year, :genre, :available)");
    query.bindValue(":title", title);
    query.bindValue(":author", author);
    query.bindValue(":year", year);
    query.bindValue(":genre", genre);
    query.bindValue(":available", available);

    if (!query.exec()) {
        qDebug() << "Add book error:" << query.lastError().text();
        return false;
    }
    qDebug() << "Book added successfully";
    return true;
}

bool Database::updateBook(int id, const QString &title, const QString &author, int year, const QString &genre, bool available)
{
    if (!m_db.isOpen()) {
        qDebug() << "Update book error: database is not open";
        return false;
    }

    QSqlQuery query;
    query.prepare("UPDATE books SET title = :title, author = :author, year = :year, "
                  "genre = :genre, available = :available WHERE id = :id");
    query.bindValue(":title", title);
    query.bindValue(":author", author);
    query.bindValue(":year", year);
    query.bindValue(":genre", genre);
    query.bindValue(":available", available);
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Update book error for ID" << id << ":" << query.lastError().text();
        return false;
    }
    qDebug() << "Book updated successfully for ID:" << id;
    return true;
}

bool Database::deleteBook(int id)
{
    if (!m_db.isOpen()) {
        qDebug() << "Delete book error: database is not open";
        return false;
    }

    qDebug() << "Starting delete book with ID:" << id;

    // Проверяем существование книги
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM books WHERE id = :id");
    checkQuery.bindValue(":id", id);
    if (!checkQuery.exec() || !checkQuery.next()) {
        qDebug() << "Delete book error: book with ID" << id << "does not exist";
        return false;
    }

    m_db.transaction();

    // Удаляем связанные записи в loans
    QSqlQuery query;
    query.prepare("DELETE FROM loans WHERE book_id = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        qDebug() << "Delete loans error for book ID" << id << ":" << query.lastError().text();
        m_db.rollback();
        return false;
    }
    qDebug() << "Loans deleted for book ID:" << id;

    // Удаляем книгу
    query.prepare("DELETE FROM books WHERE id = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        qDebug() << "Delete book error for ID" << id << ":" << query.lastError().text();
        m_db.rollback();
        return false;
    }
    qDebug() << "Book deleted successfully with ID:" << id;

    if (!m_db.commit()) {
        qDebug() << "Commit failed for book deletion:" << m_db.lastError().text();
        m_db.rollback();
        return false;
    }
    qDebug() << "Transaction committed for book ID:" << id;
    return true;
}

QVariantList Database::getAllBooks()
{
    QVariantList books;
    if (!m_db.isOpen()) {
        qDebug() << "Get all books error: database is not open";
        return books;
    }

    QSqlQuery query("SELECT b.id, b.title, b.author, b.year, b.genre, b.available, r.name AS reader_name "
                    "FROM books b "
                    "LEFT JOIN loans l ON b.id = l.book_id "
                    "LEFT JOIN readers r ON l.reader_id = r.id "
                    "ORDER BY b.title");
    if (!query.exec()) {
        qDebug() << "Get all books error:" << query.lastError().text();
        return books;
    }

    while (query.next()) {
        QVariantMap book;
        book["id"] = query.value("id");
        book["title"] = query.value("title");
        book["author"] = query.value("author");
        book["year"] = query.value("year");
        book["genre"] = query.value("genre");
        book["available"] = query.value("available");
        book["reader_name"] = query.value("reader_name").toString();
        books.append(book);
    }
    qDebug() << "Retrieved" << books.size() << "books";
    return books;
}

QVariantList Database::searchBooks(const QString &searchTerm)
{
    QVariantList books;
    if (!m_db.isOpen()) {
        qDebug() << "Search books error: database is not open";
        return books;
    }

    QSqlQuery query;
    query.prepare("SELECT b.id, b.title, b.author, b.year, b.genre, b.available, r.name AS reader_name "
                  "FROM books b "
                  "LEFT JOIN loans l ON b.id = l.book_id "
                  "LEFT JOIN readers r ON l.reader_id = r.id "
                  "WHERE b.title LIKE :search OR b.author LIKE :search OR b.genre LIKE :search "
                  "ORDER BY b.title");
    query.bindValue(":search", "%" + searchTerm + "%");

    if (!query.exec()) {
        qDebug() << "Search error:" << query.lastError().text();
        return books;
    }

    while (query.next()) {
        QVariantMap book;
        book["id"] = query.value("id");
        book["title"] = query.value("title");
        book["author"] = query.value("author");
        book["year"] = query.value("year");
        book["genre"] = query.value("genre");
        book["available"] = query.value("available");
        book["reader_name"] = query.value("reader_name").toString();
        books.append(book);
    }
    qDebug() << "Found" << books.size() << "books for search term:" << searchTerm;
    return books;
}

QVariantList Database::searchBooksAdvanced(const QString &searchTerm, int minYear, int maxYear, bool onlyAvailable, const QString &author, const QString &genre)
{
    QVariantList books;
    if (!m_db.isOpen()) {
        qDebug() << "Advanced search error: database is not open";
        return books;
    }

    QString queryStr = "SELECT b.id, b.title, b.author, b.year, b.genre, b.available, r.name AS reader_name "
                      "FROM books b "
                      "LEFT JOIN loans l ON b.id = l.book_id "
                      "LEFT JOIN readers r ON l.reader_id = r.id "
                      "WHERE 1=1";
    if (!searchTerm.isEmpty()) {
        queryStr += " AND (b.title LIKE :search OR b.author LIKE :search OR b.genre LIKE :search)";
    }
    if (minYear > 0) {
        queryStr += " AND b.year >= :minYear";
    }
    if (maxYear > 0) {
        queryStr += " AND b.year <= :maxYear";
    }
    if (onlyAvailable) {
        queryStr += " AND b.available = 1";
    }
    if (!author.isEmpty()) {
        queryStr += " AND b.author LIKE :author";
    }
    if (!genre.isEmpty()) {
        queryStr += " AND b.genre LIKE :genre";
    }
    queryStr += " ORDER BY b.title";

    QSqlQuery query;
    query.prepare(queryStr);
    if (!searchTerm.isEmpty()) {
        query.bindValue(":search", "%" + searchTerm + "%");
    }
    if (minYear > 0) {
        query.bindValue(":minYear", minYear);
    }
    if (maxYear > 0) {
        query.bindValue(":maxYear", maxYear);
    }
    if (!author.isEmpty()) {
        query.bindValue(":author", "%" + author + "%");
    }
    if (!genre.isEmpty()) {
        query.bindValue(":genre", "%" + genre + "%");
    }

    if (!query.exec()) {
        qDebug() << "Advanced search error:" << query.lastError().text();
        return books;
    }

    while (query.next()) {
        QVariantMap book;
        book["id"] = query.value("id");
        book["title"] = query.value("title");
        book["author"] = query.value("author");
        book["year"] = query.value("year");
        book["genre"] = query.value("genre");
        book["available"] = query.value("available");
        book["reader_name"] = query.value("reader_name").toString();
        books.append(book);
    }
    qDebug() << "Advanced search returned" << books.size() << "books";
    return books;
}

bool Database::exportToCSV(const QString &filePath)
{
    if (!m_db.isOpen()) {
        qDebug() << "Export error: database is not open";
        return false;
    }

    QSqlQuery query("SELECT b.id, b.title, b.author, b.year, b.genre, b.available, r.name AS reader_name "
                    "FROM books b "
                    "LEFT JOIN loans l ON b.id = l.book_id "
                    "LEFT JOIN readers r ON l.reader_id = r.id "
                    "ORDER BY b.title");
    if (!query.exec()) {
        qDebug() << "Export error:" << query.lastError().text();
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Cannot open file for writing:" << filePath;
        return false;
    }

    QTextStream out(&file);
    out << "ID,Title,Author,Year,Genre,Available,Reader\n";

    while (query.next()) {
        out << query.value("id").toString() << ","
            << "\"" << query.value("title").toString() << "\","
            << "\"" << query.value("author").toString() << "\","
            << query.value("year").toString() << ","
            << "\"" << query.value("genre").toString() << "\","
            << (query.value("available").toBool() ? "Yes" : "No") << ","
            << "\"" << query.value("reader_name").toString() << "\"\n";
    }

    file.close();
    qDebug() << "Exported to CSV:" << filePath;
    return true;
}

bool Database::addReader(const QString &name, const QString &contact)
{
    if (!m_db.isOpen()) {
        qDebug() << "Add reader error: database is not open";
        return false;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO readers (name, contact) VALUES (:name, :contact)");
    query.bindValue(":name", name);
    query.bindValue(":contact", contact);

    if (!query.exec()) {
        qDebug() << "Add reader error:" << query.lastError().text();
        return false;
    }
    qDebug() << "Reader added successfully";
    return true;
}

bool Database::updateReader(int id, const QString &name, const QString &contact)
{
    if (!m_db.isOpen()) {
        qDebug() << "Update reader error: database is not open";
        return false;
    }

    QSqlQuery query;
    query.prepare("UPDATE readers SET name = :name, contact = :contact WHERE id = :id");
    query.bindValue(":name", name);
    query.bindValue(":contact", contact);
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Update reader error for ID" << id << ":" << query.lastError().text();
        return false;
    }
    qDebug() << "Reader updated successfully for ID:" << id;
    return true;
}

bool Database::deleteReader(int id)
{
    if (!m_db.isOpen()) {
        qDebug() << "Delete reader error: database is not open";
        return false;
    }

    qDebug() << "Starting delete reader with ID:" << id;

    // Проверяем существование читателя
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM readers WHERE id = :id");
    checkQuery.bindValue(":id", id);
    if (!checkQuery.exec() || !checkQuery.next()) {
        qDebug() << "Delete reader error: reader with ID" << id << "does not exist";
        return false;
    }

    m_db.transaction();

    // Удаляем связанные записи в loans
    QSqlQuery query;
    query.prepare("DELETE FROM loans WHERE reader_id = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        qDebug() << "Delete loans error for reader ID" << id << ":" << query.lastError().text();
        m_db.rollback();
        return false;
    }
    qDebug() << "Loans deleted for reader ID:" << id;

    // Удаляем читателя
    query.prepare("DELETE FROM readers WHERE id = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        qDebug() << "Delete reader error for ID" << id << ":" << query.lastError().text();
        m_db.rollback();
        return false;
    }
    qDebug() << "Reader deleted successfully with ID:" << id;

    if (!m_db.commit()) {
        qDebug() << "Commit failed for reader deletion:" << m_db.lastError().text();
        m_db.rollback();
        return false;
    }
    qDebug() << "Transaction committed for reader ID:" << id;
    return true;
}

QVariantList Database::getAllReaders()
{
    QVariantList readers;
    if (!m_db.isOpen()) {
        qDebug() << "Get all readers error: database is not open";
        return readers;
    }

    QSqlQuery query("SELECT * FROM readers ORDER BY name");
    if (!query.exec()) {
        qDebug() << "Get all readers error:" << query.lastError().text();
        return readers;
    }

    while (query.next()) {
        QVariantMap reader;
        reader["id"] = query.value("id");
        reader["name"] = query.value("name");
        reader["contact"] = query.value("contact");
        readers.append(reader);
    }
    qDebug() << "Retrieved" << readers.size() << "readers";
    return readers;
}

bool Database::issueBook(int bookId, int readerId)
{
    if (!m_db.isOpen()) {
        qDebug() << "Issue book error: database is not open";
        return false;
    }

    // Проверяем, доступна ли книга
    QSqlQuery query;
    query.prepare("SELECT available FROM books WHERE id = :id");
    query.bindValue(":id", bookId);
    if (!query.exec() || !query.next()) {
        qDebug() << "Check book availability error for book ID" << bookId << ":" << query.lastError().text();
        return false;
    }
    if (!query.value("available").toBool()) {
        qDebug() << "Book with ID" << bookId << "is not available";
        return false;
    }

    // Добавляем запись в loans
    query.prepare("INSERT INTO loans (book_id, reader_id, issue_date) "
                  "VALUES (:book_id, :reader_id, :issue_date)");
    query.bindValue(":book_id", bookId);
    query.bindValue(":reader_id", readerId);
    query.bindValue(":issue_date", QDateTime::currentDateTime().toString(Qt::ISODate));
    if (!query.exec()) {
        qDebug() << "Issue book error:" << query.lastError().text();
        return false;
    }

    // Обновляем статус книги
    query.prepare("UPDATE books SET available = 0 WHERE id = :id");
    query.bindValue(":id", bookId);
    if (!query.exec()) {
        qDebug() << "Update book availability error:" << query.lastError().text();
        return false;
    }

    qDebug() << "Book issued successfully: book ID" << bookId << "to reader ID" << readerId;
    return true;
}

bool Database::returnBook(int bookId)
{
    if (!m_db.isOpen()) {
        qDebug() << "Return book error: database is not open";
        return false;
    }

    // Удаляем запись из loans
    QSqlQuery query;
    query.prepare("DELETE FROM loans WHERE book_id = :id");
    query.bindValue(":id", bookId);
    if (!query.exec()) {
        qDebug() << "Return book error (delete loan):" << query.lastError().text();
        return false;
    }

    // Обновляем статус книги
    query.prepare("UPDATE books SET available = 1 WHERE id = :id");
    query.bindValue(":id", bookId);
    if (!query.exec()) {
        qDebug() << "Update book availability error:" << query.lastError().text();
        return false;
    }

    qDebug() << "Book returned successfully: book ID" << bookId;
    return true;
}

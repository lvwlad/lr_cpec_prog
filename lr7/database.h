#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariantList>

class Database : public QObject
{
    Q_OBJECT
public:
    explicit Database(QObject *parent = nullptr);
    ~Database();

    bool connectToDatabase(const QString &path);
    void closeDatabase();

    // Методы для книг
    Q_INVOKABLE bool addBook(const QString &title, const QString &author, int year, const QString &genre, bool available);
    Q_INVOKABLE bool updateBook(int id, const QString &title, const QString &author, int year, const QString &genre, bool available);
    Q_INVOKABLE bool deleteBook(int id);
    Q_INVOKABLE QVariantList getAllBooks();
    Q_INVOKABLE QVariantList searchBooks(const QString &searchTerm);
    Q_INVOKABLE QVariantList searchBooksAdvanced(const QString &searchTerm, int minYear, int maxYear, bool onlyAvailable, const QString &author, const QString &genre);
    Q_INVOKABLE bool exportToCSV(const QString &filePath);

    // Методы для читателей
    Q_INVOKABLE bool addReader(const QString &name, const QString &contact);
    Q_INVOKABLE bool updateReader(int id, const QString &name, const QString &contact);
    Q_INVOKABLE bool deleteReader(int id);
    Q_INVOKABLE QVariantList getAllReaders();

    // Методы для выдачи/возврата книг
    Q_INVOKABLE bool issueBook(int bookId, int readerId);
    Q_INVOKABLE bool returnBook(int bookId);

private:
    QSqlDatabase m_db;
};

#endif // DATABASE_H

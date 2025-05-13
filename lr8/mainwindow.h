#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QDirIterator>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// Класс задачи — выполняется в фон. потоке
class SearchTask : public QObject {
    Q_OBJECT
public:
    SearchTask(QString root, QString filter);
public slots:
    void start();  // слот, запускающий обход
    void stop();   // слот для прерывания задачи
signals:
    void fileFound(const QString &filePath);
    void currentDir(const QString &dirPath);
    void finished();
    void error(const QString &message);  // Сигнал о ошибке
private:
    QString m_root, m_filter;
    bool m_interrupted = false;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_selectDirButton_clicked();
    void on_searchButton_clicked();
    void addFile(const QString &filePath);
    void updateStatus(const QString &dirPath);
    void onSearchFinished();
    void onSearchError(const QString &message);

private:
    Ui::MainWindow *ui;
    QThread *workerThread = nullptr;
    SearchTask *task = nullptr;

protected:
    void closeEvent(QCloseEvent *event) override;
};
#endif // MAINWINDOW_H

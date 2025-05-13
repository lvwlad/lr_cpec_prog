#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QThread>
#include <QDebug>

// ================= SearchTask =================
SearchTask::SearchTask(QString root, QString filter)
    : m_root(std::move(root)), m_filter(std::move(filter)) {}

void SearchTask::start() {
    try {
        QDirIterator it(m_root, QStringList{m_filter}, QDir::Files,
                       QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
        while (it.hasNext()) {
            if (m_interrupted) {
                emit finished();
                return;
            }
            QString file = it.next();
            emit currentDir(QFileInfo(file).path());
            emit fileFound(file);
            QThread::msleep(10); // Для обработки событий
        }
        emit finished();
    } catch (...) {
        emit error("Критическая ошибка при обходе директории");
    }
}

void SearchTask::stop() {
    m_interrupted = true;
}

// ================= MainWindow =================
void MainWindow::cleanupThread() {
    if (workerThread) {
        QMetaObject::invokeMethod(task, "stop", Qt::BlockingQueuedConnection);
        workerThread->quit();
        if (!workerThread->wait(1000)) {
            qWarning() << "Принудительное завершение потока";
            workerThread->terminate();
            workerThread->wait();
        }
        task->deleteLater();
        workerThread->deleteLater();
        task = nullptr;
        workerThread = nullptr;
    }
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->statusLabel->setText("Ожидание...");
}

MainWindow::~MainWindow() {
    cleanupThread();
    delete ui;
}

void MainWindow::on_selectDirButton_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, "Выберите директорию");
    if (!dir.isEmpty()) {
        ui->dirEdit->setText(dir);
    }
}

void MainWindow::on_searchButton_clicked() {
    QString filter = ui->filterEdit->text().trimmed();
    QString dir = ui->dirEdit->text().trimmed();

    if (filter.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите фильтр файлов");
        return;
    }
    if (dir.isEmpty() || !QDir(dir).exists()) {
        QMessageBox::warning(this, "Ошибка", "Директория не существует или недоступна");
        return;
    }

    ui->searchButton->setEnabled(false);
    ui->fileList->clear();
    ui->statusLabel->setText("Запуск поиска...");

    cleanupThread(); // Гарантированная очистка предыдущих задач

    workerThread = new QThread;
    task = new SearchTask(dir, filter);
    task->moveToThread(workerThread);

    // Подключение сигналов
    connect(workerThread, &QThread::started, task, &SearchTask::start);
    connect(task, &SearchTask::fileFound, this, &MainWindow::addFile);
    connect(task, &SearchTask::currentDir, this, &MainWindow::updateStatus);
    connect(task, &SearchTask::finished, this, &MainWindow::onSearchFinished);
    connect(task, &SearchTask::error, this, &MainWindow::onSearchError);
    connect(workerThread, &QThread::finished, workerThread, &QObject::deleteLater);
    connect(workerThread, &QThread::finished, task, &QObject::deleteLater);

    workerThread->start();
}

void MainWindow::addFile(const QString &filePath) {
    ui->fileList->addItem(filePath);
}

void MainWindow::updateStatus(const QString &dirPath) {
    ui->statusLabel->setText("Сканируется: " + dirPath);
}

void MainWindow::onSearchFinished() {
    ui->statusLabel->setText("Поиск завершён.");
    ui->searchButton->setEnabled(true);
}

void MainWindow::onSearchError(const QString &message) {
    ui->statusLabel->setText("Ошибка: " + message);
    QMessageBox::critical(this, "Ошибка", message);
    ui->searchButton->setEnabled(true);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    cleanupThread();
    QMainWindow::closeEvent(event);
}

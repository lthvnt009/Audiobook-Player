#include "LibraryWidget.h"
#include "BookModel.h"
#include "ChapterModel.h"
#include "SettingsDialog.h"
#include "ButtonDelegate.h"
#include "DatabaseManager.h" // Thêm header

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTableView>
#include <QSplitter>
#include <QHeaderView>
#include <QFileDialog>
#include <QDir>
#include <QDebug>
#include <QItemSelectionModel>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QSizePolicy>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/dict.h>
}

LibraryWidget::LibraryWidget(QWidget *parent) : QWidget(parent)
{
    m_settings = new QSettings(this);
    m_settingsDialog = new SettingsDialog(this);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);
    
    QWidget *toolbarWidget = new QWidget(this);
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbarWidget);
    toolbarLayout->setContentsMargins(0, 0, 0, 0);
    searchLineEdit = new QLineEdit(this);
    searchLineEdit->setPlaceholderText(tr("🔍 Tìm kiếm..."));
    scanLibraryButton = new QPushButton(tr("Quét lại Thư viện 🔄"), this);
    settingsButton = new QPushButton(tr("Cài đặt ⚙️"), this);
    
    toolbarLayout->addWidget(searchLineEdit, 1);
    toolbarLayout->addWidget(scanLibraryButton);
    toolbarLayout->addWidget(settingsButton);

    mainSplitter = new QSplitter(Qt::Vertical, this);
    booksTableView = new QTableView(this);
    chaptersTableView = new QTableView(this);

    booksTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    chaptersTableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    booksTableView->setSortingEnabled(true);
    booksTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    chaptersTableView->setSortingEnabled(true);

    scanLibraryButton->setToolTip(tr("Quét lại thư mục thư viện hiện tại để tìm sách mới."));
    settingsButton->setToolTip(tr("Mở cửa sổ Cài đặt."));
    searchLineEdit->setToolTip(tr("Nhập tên sách hoặc tác giả để tìm kiếm."));

    mainSplitter->addWidget(booksTableView);
    mainSplitter->addWidget(chaptersTableView);
    
    mainLayout->addWidget(toolbarWidget);
    mainLayout->addWidget(mainSplitter, 1);

    m_bookModel = new BookModel(this);
    m_chapterModel = new ChapterModel(this);
    booksTableView->setModel(m_bookModel);
    chaptersTableView->setModel(m_chapterModel);
    
    m_buttonDelegate = new ButtonDelegate(this);
    chaptersTableView->setItemDelegateForColumn(4, m_buttonDelegate);

    QString styleSheet = R"(
        QTableView::item:selected {
            background-color: #3399ff;
            color: white;
        }
    )";
    booksTableView->setStyleSheet(styleSheet);
    chaptersTableView->setStyleSheet(styleSheet);

    connect(scanLibraryButton, &QPushButton::clicked, this, &LibraryWidget::onRescanLibraryClicked);
    connect(settingsButton, &QPushButton::clicked, this, &LibraryWidget::onSettingsClicked);
    
    connect(booksTableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &LibraryWidget::onBookSelectionChanged);
    connect(booksTableView, &QTableView::doubleClicked, this, &LibraryWidget::onTableViewDoubleClicked);
    connect(chaptersTableView, &QTableView::doubleClicked, this, &LibraryWidget::onTableViewDoubleClicked);
    connect(searchLineEdit, &QLineEdit::textChanged, this, &LibraryWidget::onSearchQueryChanged);
    connect(booksTableView, &QTableView::customContextMenuRequested, this, &LibraryWidget::showBookContextMenu);
    
    connect(m_buttonDelegate, &ButtonDelegate::resetClicked, this, &LibraryWidget::onChapterResetClicked);
    connect(m_buttonDelegate, &ButtonDelegate::doneClicked, this, &LibraryWidget::onChapterDoneClicked);

}

void LibraryWidget::forceUpdateChapterView()
{
    onBookSelectionChanged(QItemSelection(), QItemSelection());
}

void LibraryWidget::onProgressUpdated(const QString &bookPath, int chapterIndex, qint64 position)
{
    // Cập nhật dữ liệu trong bộ nhớ (m_allBooks)
    for (auto& book : m_allBooks) {
        if (book.path == bookPath) {
            if (chapterIndex >= 0 && chapterIndex < book.chapters.size()) {
                book.chapters[chapterIndex].listenedDuration = position;
                int chapterId = book.chapters[chapterIndex].id;
                
                // Cập nhật vào model để giao diện thay đổi ngay lập tức
                m_bookModel->updateBook(book);
                m_chapterModel->updateChapter(chapterIndex);
                
                // Lưu vào database
                DatabaseManager::instance().updateChapterProgress(chapterId, position);
                return;
            }
        }
    }
}

void LibraryWidget::saveLayoutState()
{
    m_settings->setValue("LibraryView/splitterState", mainSplitter->saveState());
    m_settings->setValue("LibraryView/booksHeaderState", booksTableView->horizontalHeader()->saveState());
    m_settings->setValue("LibraryView/chaptersHeaderState", chaptersTableView->horizontalHeader()->saveState());
}

void LibraryWidget::restoreLayoutState()
{
    if (m_settings->contains("LibraryView/splitterState")) {
        mainSplitter->restoreState(m_settings->value("LibraryView/splitterState").toByteArray());
    } else {
        mainSplitter->setSizes({300, 300});
    }

    if (m_settings->contains("LibraryView/booksHeaderState")) {
        booksTableView->horizontalHeader()->restoreState(m_settings->value("LibraryView/booksHeaderState").toByteArray());
    } else {
        booksTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
        booksTableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
        booksTableView->horizontalHeader()->setStretchLastSection(true);
    }

    if (m_settings->contains("LibraryView/chaptersHeaderState")) {
        chaptersTableView->horizontalHeader()->restoreState(m_settings->value("LibraryView/chaptersHeaderState").toByteArray());
    } else {
        chaptersTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
        chaptersTableView->horizontalHeader()->setStretchLastSection(true);
    }
}

void LibraryWidget::onSettingsClicked()
{
    m_settingsDialog->setLibraryPath(m_libraryPath);
    
    connect(m_settingsDialog, &SettingsDialog::changeLibraryPathRequested, this, &LibraryWidget::changeLibraryPath);
    connect(m_settingsDialog, &SettingsDialog::settingsChanged, this, &LibraryWidget::settingsChanged);
    
    m_settingsDialog->exec();
    
    disconnect(m_settingsDialog, &SettingsDialog::changeLibraryPathRequested, this, &LibraryWidget::changeLibraryPath);
    disconnect(m_settingsDialog, &SettingsDialog::settingsChanged, this, &LibraryWidget::settingsChanged);
}

void LibraryWidget::changeLibraryPath()
{
    QString dirPath = QFileDialog::getExistingDirectory(this, tr("Chọn Thư mục Thư viện Mới"), m_libraryPath);
    if (!dirPath.isEmpty() && dirPath != m_libraryPath) {
        m_libraryPath = dirPath;
        emit libraryPathSelected(dirPath); // Lưu đường dẫn mới vào QSettings
        scanDirectory(dirPath);
    }
}

void LibraryWidget::onRescanLibraryClicked()
{
    if (!m_libraryPath.isEmpty()) {
        scanDirectory(m_libraryPath);
    } else {
        changeLibraryPath();
    }
}

void LibraryWidget::scanPathOnStartup(const QString &path)
{
    m_libraryPath = path;
    if (!path.isEmpty()) {
        scanDirectory(path);
    }
}

ChapterInfo LibraryWidget::getChapterInfo(const QString& bookPath, int chapterIndex) const
{
    for(const auto& book : m_allBooks) {
        if (book.path == bookPath) {
            if (chapterIndex >= 0 && chapterIndex < book.chapters.size()) {
                return book.chapters[chapterIndex];
            }
        }
    }
    return ChapterInfo(); // Trả về chapter rỗng nếu không tìm thấy
}

BookInfo LibraryWidget::getBookByPath(const QString &path) const
{
    for (const auto& book : m_allBooks) {
        if (book.path == path) {
            return book;
        }
    }
    return BookInfo();
}

void LibraryWidget::showBookContextMenu(const QPoint &pos)
{
    QModelIndex index = booksTableView->indexAt(pos);
    if (!index.isValid()) {
        return;
    }

    QMenu contextMenu(this);
    QAction *removeFromLibraryAction = contextMenu.addAction(tr("Xóa khỏi thư viện"));
    QAction *deletePermanentlyAction = contextMenu.addAction(tr("Xóa vĩnh viễn"));

    connect(removeFromLibraryAction, &QAction::triggered, this, &LibraryWidget::onRemoveFromLibrary);
    connect(deletePermanentlyAction, &QAction::triggered, this, &LibraryWidget::onDeletePermanently);

    contextMenu.exec(booksTableView->viewport()->mapToGlobal(pos));
}

void LibraryWidget::onRemoveFromLibrary()
{
    QModelIndexList selectedRows = booksTableView->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) return;

    int row = selectedRows.first().row();
    const BookInfo& selectedBook = m_bookModel->getBookAt(row);

    // Xóa khỏi database
    DatabaseManager::instance().removeBook(selectedBook.id);

    // Xóa khỏi bộ nhớ
    m_allBooks.removeIf([&](const BookInfo& book) {
        return book.path == selectedBook.path;
    });

    onSearchQueryChanged(searchLineEdit->text());
}

void LibraryWidget::onDeletePermanently()
{
    QModelIndexList selectedRows = booksTableView->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) return;

    int row = selectedRows.first().row();
    const BookInfo& selectedBook = m_bookModel->getBookAt(row);

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Xác nhận Xóa"), 
                                  tr("Bạn có chắc chắn muốn xóa vĩnh viễn cuốn sách '%1' và tất cả các file của nó không?\nHành động này không thể hoàn tác.").arg(selectedBook.title),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QDir bookDir(selectedBook.path);
        if (bookDir.removeRecursively()) {
            onRemoveFromLibrary(); // Gọi hàm này để xóa cả trong DB
        } else {
            QMessageBox::critical(this, tr("Lỗi"), tr("Không thể xóa thư mục sách."));
        }
    }
}

void LibraryWidget::onSearchQueryChanged(const QString &text)
{
    if (text.isEmpty()) {
        m_bookModel->setBooks(m_allBooks);
        return;
    }

    QList<BookInfo> filteredBooks;
    for (const BookInfo &book : m_allBooks) {
        if (book.title.contains(text, Qt::CaseInsensitive) || book.author.contains(text, Qt::CaseInsensitive)) {
            filteredBooks.append(book);
        }
    }
    m_bookModel->setBooks(filteredBooks);
}

void LibraryWidget::selectBookByPath(const QString &bookPath)
{
    booksTableView->selectionModel()->clear();
    for (int i = 0; i < m_bookModel->rowCount(); ++i) {
        if (m_bookModel->getBookAt(i).path == bookPath) {
            booksTableView->selectRow(i);
            booksTableView->scrollTo(m_bookModel->index(i, 0));
            return;
        }
    }
}

void LibraryWidget::onTableViewDoubleClicked(const QModelIndex &index)
{
    QObject *senderObj = sender();
    if (senderObj == booksTableView) {
        int bookRow = index.row();
        const BookInfo& selectedBook = m_bookModel->getBookAt(bookRow);
        
        int chapterToPlay = 0; 
        for (int i = 0; i < selectedBook.chapters.size(); ++i) {
            const auto& chapter = selectedBook.chapters[i];
            if (chapter.listenedDuration < chapter.duration * 1000) {
                chapterToPlay = i;
                break;
            }
        }
        emit playRequest(selectedBook, chapterToPlay);
        
    } else if (senderObj == chaptersTableView) {
        if (index.column() == 4) return;

        QModelIndexList selectedBookRows = booksTableView->selectionModel()->selectedRows();
        if (selectedBookRows.isEmpty()) return;

        int bookRow = selectedBookRows.first().row();
        const BookInfo& selectedBook = m_bookModel->getBookAt(bookRow);
        int chapterRow = index.row();

        emit playRequest(selectedBook, chapterRow);
    }
}

void LibraryWidget::updateAndSaveChapterProgress(int chapterId, qint64 newPosition)
{
    // Cập nhật database
    DatabaseManager::instance().updateChapterProgress(chapterId, newPosition);

    // Cập nhật dữ liệu trong bộ nhớ để giao diện phản hồi ngay
    for (auto& book : m_allBooks) {
        for (auto& chapter : book.chapters) {
            if (chapter.id == chapterId) {
                chapter.listenedDuration = newPosition;
                m_bookModel->updateBook(book); // Cập nhật lại thông tin tổng của sách
                
                // Kiểm tra xem sách có đang được chọn không để cập nhật bảng chapter
                QModelIndexList selectedRows = booksTableView->selectionModel()->selectedRows();
                if (!selectedRows.isEmpty() && m_bookModel->getBookAt(selectedRows.first().row()).id == book.id) {
                    m_chapterModel->setChapters(book.chapters);
                }
                return;
            }
        }
    }
}


void LibraryWidget::onChapterResetClicked(const QModelIndex &index)
{
    const ChapterInfo& chapter = m_chapterModel->getChapterAt(index.row());
    updateAndSaveChapterProgress(chapter.id, 0);
}

void LibraryWidget::onChapterDoneClicked(const QModelIndex &index)
{
    const ChapterInfo& chapter = m_chapterModel->getChapterAt(index.row());
    updateAndSaveChapterProgress(chapter.id, chapter.duration * 1000);
}


void LibraryWidget::scanDirectory(const QString &path)
{
    QList<BookInfo> booksFromDisk;
    QDir rootDir(path);
    QStringList bookDirs = rootDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    QStringList audioFilters;
    audioFilters << "*.mp3" << "*.m4a" << "*.m4b" << "*.ogg" << "*.wma" << "*.flac" << "*.opus"
                 << "*.wav" << "*.aac" << "*.aiff" << "*.ape";
                 
    QStringList imageFilters;
    imageFilters << "cover.jpg" << "cover.png" << "folder.jpg" << "folder.png";

    for (const QString &bookDirName : bookDirs) {
        BookInfo currentBook;
        currentBook.title = bookDirName;
        currentBook.path = rootDir.filePath(bookDirName);
        
        QDir bookDir(currentBook.path);
        QStringList audioFiles = bookDir.entryList(audioFilters, QDir::Files, QDir::Name);
        if (audioFiles.isEmpty()) continue;

        QStringList foundImages = bookDir.entryList(imageFilters, QDir::Files);
        if (!foundImages.isEmpty()) {
            currentBook.coverImagePath = bookDir.filePath(foundImages.first());
        }

        // Thêm/Cập nhật sách vào DB và lấy ID
        DatabaseManager::instance().addOrUpdateBook(currentBook);

        bool authorFound = !currentBook.author.isEmpty();
        for (const QString &fileName : audioFiles) {
            ChapterInfo chapter;
            chapter.title = fileName;
            chapter.filePath = bookDir.filePath(fileName);
            QFileInfo fileInfo(chapter.filePath);
            chapter.format = fileInfo.suffix().toUpper();
            chapter.size = fileInfo.size();
            chapter.duration = getAudioDuration(chapter.filePath);
            
            if (!authorFound) {
                QString author = getMetadata(chapter.filePath, "artist");
                if (author.isEmpty()) author = getMetadata(chapter.filePath, "album_artist");
                if(!author.isEmpty()) {
                    currentBook.author = author;
                    DatabaseManager::instance().addOrUpdateBook(currentBook); // Cập nhật lại sách với tác giả mới
                    authorFound = true;
                }
            }
            // Thêm/Cập nhật chương vào DB và lấy tiến độ đã lưu
            DatabaseManager::instance().addOrUpdateChapter(currentBook.id, chapter);
            currentBook.chapters.append(chapter);
        }
        booksFromDisk.append(currentBook);
    }
    
    // Tải lại toàn bộ dữ liệu từ DB để đảm bảo tính nhất quán
    m_allBooks = DatabaseManager::instance().getAllBooks();

    onSearchQueryChanged(searchLineEdit->text());
    m_chapterModel->clear();
}

void LibraryWidget::onBookSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QModelIndexList selectedRows = booksTableView->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        m_chapterModel->clear();
        return;
    }

    int selectedRow = selectedRows.first().row();
    const BookInfo& selectedBook = m_bookModel->getBookAt(selectedRow);
    m_chapterModel->setChapters(selectedBook.chapters);
}

QString LibraryWidget::getMetadata(const QString &filePath, const char* key)
{
    AVFormatContext *formatContext = nullptr;
    QString result = "";
    std::string path_std = filePath.toStdString();

    if (avformat_open_input(&formatContext, path_std.c_str(), nullptr, nullptr) != 0) {
        return "";
    }

    AVDictionaryEntry *tag = nullptr;
    tag = av_dict_get(formatContext->metadata, key, nullptr, AV_DICT_IGNORE_SUFFIX);
    if (tag) {
        result = QString::fromUtf8(tag->value);
    }

    avformat_close_input(&formatContext);
    return result;
}


qint64 LibraryWidget::getAudioDuration(const QString &filePath)
{
    AVFormatContext *formatContext = nullptr;
    qint64 duration = 0;
    std::string path_std = filePath.toStdString();
    if (avformat_open_input(&formatContext, path_std.c_str(), nullptr, nullptr) != 0) {
        return 0;
    }
    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        avformat_close_input(&formatContext);
        return 0;
    }
    if (formatContext->duration != AV_NOPTS_VALUE) {
        duration = formatContext->duration;
        duration = duration / AV_TIME_BASE;
    }
    avformat_close_input(&formatContext);
    return duration;
}

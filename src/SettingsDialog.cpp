#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QSettings>
#include <QLabel>
#include <QMessageBox>
#include <QAbstractSpinBox>

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent)
{
    m_settings = new QSettings(this);
    setWindowTitle(tr("Cài đặt"));
    setModal(true);
    setMinimumWidth(400);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QGroupBox *pathGroup = new QGroupBox(tr("Thư viện"));
    pathGroup->setToolTip(tr("Nơi lưu trữ các cuốn sách của bạn."));
    QVBoxLayout *pathLayout = new QVBoxLayout();
    m_pathLabel = new QLabel(tr("Đường dẫn hiện tại: Chưa có"));
    m_pathLabel->setWordWrap(true);
    QPushButton *changePathButton = new QPushButton(tr("Thay đổi"));
    changePathButton->setToolTip(tr("Chọn một thư mục gốc mới cho thư viện."));
    pathLayout->addWidget(m_pathLabel);
    pathLayout->addWidget(changePathButton, 0, Qt::AlignLeft);
    pathGroup->setLayout(pathLayout);
    connect(changePathButton, &QPushButton::clicked, this, &SettingsDialog::onChangePathClicked);

    // --- CÁC TÍNH NĂNG ĐANG TẠM VÔ HIỆU HÓA ---
    // QGroupBox *themeGroup = new QGroupBox(tr("Chủ đề"));
    // QGroupBox *langGroup = new QGroupBox(tr("Ngôn ngữ"));
    // --- KẾT THÚC ---

    QGroupBox *seekGroup = new QGroupBox(tr("Tùy chỉnh thời gian tua"));
    QHBoxLayout *seekLayout = new QHBoxLayout();
    m_seekTimeSpinBox = new QSpinBox();
    m_seekTimeSpinBox->setRange(1, 60);
    m_seekTimeSpinBox->setSuffix(tr(" giây"));
    
    m_seekTimeSpinBox->setStyleSheet(
        "QSpinBox::up-button {"
        "    subcontrol-origin: border;"
        "    subcontrol-position: top right;"
        "    width: 16px;"
        "}"
        "QSpinBox::down-button {"
        "    subcontrol-origin: border;"
        "    subcontrol-position: bottom right;"
        "    width: 16px;"
        "}"
    );

    seekLayout->addWidget(new QLabel(tr("Thời gian tua:")));
    seekLayout->addWidget(m_seekTimeSpinBox);
    seekLayout->addStretch();
    seekGroup->setLayout(seekLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Hủy"));
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::onSave);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(pathGroup);
    // mainLayout->addWidget(themeGroup); // Vô hiệu hóa
    // mainLayout->addWidget(langGroup);  // Vô hiệu hóa
    mainLayout->addWidget(seekGroup);
    mainLayout->addStretch();
    mainLayout->addWidget(buttonBox);

    loadSettings();
}

void SettingsDialog::setLibraryPath(const QString &path)
{
    if (path.isEmpty()) {
        m_pathLabel->setText(tr("Đường dẫn hiện tại: Chưa có"));
    } else {
        m_pathLabel->setText(tr("Đường dẫn hiện tại: %1").arg(path));
    }
}

void SettingsDialog::loadSettings()
{
    // Các cài đặt theme và ngôn ngữ đang bị vô hiệu hóa
    // QString theme = m_settings->value("Settings/theme", "light").toString();
    // m_languageCombo->setCurrentIndex(m_settings->value("Settings/language", 0).toInt());
    m_seekTimeSpinBox->setValue(m_settings->value("Settings/seekSeconds", 15).toInt());
}

void SettingsDialog::onSave()
{
    // Các cài đặt theme và ngôn ngữ đang bị vô hiệu hóa
    // m_settings->setValue("Settings/theme", m_darkThemeRadio->isChecked() ? "dark" : "light");
    // m_settings->setValue("Settings/language", m_languageCombo->currentIndex());
    m_settings->setValue("Settings/seekSeconds", m_seekTimeSpinBox->value());

    QMessageBox::information(this, tr("Thông báo"), tr("Cài đặt đã được lưu.\nMột số thay đổi (như Chủ đề hoặc Ngôn ngữ) cần khởi động lại chương trình để áp dụng."));
    
    emit settingsChanged();
    accept();
}

void SettingsDialog::onChangePathClicked()
{
    emit changeLibraryPathRequested();
    // Cập nhật lại label sau khi người dùng có thể đã chọn đường dẫn mới
    setLibraryPath(m_settings->value("Library/path").toString());
}

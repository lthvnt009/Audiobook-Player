#pragma once

#include <QDialog>

class QSettings;
class QSpinBox;
class QRadioButton;
class QComboBox;
class QLabel;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    void setLibraryPath(const QString &path);

signals:
    void changeLibraryPathRequested();
    void settingsChanged();

private slots:
    void onSave();
    void onChangePathClicked();

private:
    void loadSettings();

    QSettings *m_settings;

    // UI Components
    QLabel *m_pathLabel;
    // QRadioButton *m_lightThemeRadio; // Vô hiệu hóa
    // QRadioButton *m_darkThemeRadio;  // Vô hiệu hóa
    // QComboBox *m_languageCombo;      // Vô hiệu hóa
    QSpinBox *m_seekTimeSpinBox;
};

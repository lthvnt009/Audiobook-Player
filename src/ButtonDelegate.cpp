#include "ButtonDelegate.h"
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QApplication>
#include <QPainter>
#include <QDebug>

// Widget con chứa 2 nút bấm và label.
// Widget này được tạo ra cho mỗi hàng trong cột "Tiến độ".
class ProgressWidget : public QWidget
{
    // Lớp này không định nghĩa signals/slots mới nên không cần Q_OBJECT.
public:
    QLabel *progressLabel;
    QPushButton *resetButton;
    QPushButton *doneButton;

    explicit ProgressWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setContentsMargins(5, 0, 5, 0);
        layout->setSpacing(5);

        progressLabel = new QLabel();
        resetButton = new QPushButton(tr("Đặt lại"));
        doneButton = new QPushButton(tr("Xong"));

        resetButton->setFixedSize(45, 22);
        doneButton->setFixedSize(45, 22);

        resetButton->setToolTip(tr("Đặt lại tiến độ chương này về 0%"));
        doneButton->setToolTip(tr("Đánh dấu đã nghe xong chương này (100%)"));

        layout->addWidget(progressLabel);
        layout->addStretch();
        layout->addWidget(resetButton);
        layout->addWidget(doneButton);

        // Đặt nền trong suốt để màu highlight của hàng có thể hiển thị qua.
        setAutoFillBackground(false);
    }
};

ButtonDelegate::ButtonDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

// 1. Tạo ra widget thật sự (ProgressWidget) để đặt vào ô
QWidget *ButtonDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    ProgressWidget *editor = new ProgressWidget(parent);

    ButtonDelegate *nonConstThis = const_cast<ButtonDelegate*>(this);
    connect(editor->resetButton, &QPushButton::clicked, nonConstThis, [nonConstThis, index](){
        emit nonConstThis->resetClicked(index);
    });
    connect(editor->doneButton, &QPushButton::clicked, nonConstThis, [nonConstThis, index](){
        emit nonConstThis->doneClicked(index);
    });

    return editor;
}

// 2. Cập nhật dữ liệu từ model vào widget
void ButtonDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    ProgressWidget *widget = static_cast<ProgressWidget*>(editor);
    QString progressText = index.model()->data(index, Qt::DisplayRole).toString();
    widget->progressLabel->setText(progressText);
}

// 3. Hàm này không cần thiết vì widget không chỉnh sửa model
void ButtonDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    // Không làm gì cả
}

// 4. Định vị widget bên trong ô
void ButtonDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

// 5. Hàm paint đã được tối ưu để sửa lỗi hiển thị.
// Nhiệm vụ của nó chỉ là vẽ nền khi hàng được chọn.
// Nó KHÔNG vẽ bất kỳ văn bản nào. Việc này để tránh lỗi chữ ("50%")
// bị vẽ đè lên các nút bấm (widget) mà createEditor tạo ra.
// Widget editor (ProgressWidget) sẽ chịu hoàn toàn trách nhiệm vẽ nội dung.
void ButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // Lấy các tùy chọn style mặc định cho item
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    // Nếu trạng thái của item là "Selected" (được chọn)
    if (opt.state & QStyle::State_Selected) {
        // Vẽ một hình chữ nhật với màu highlight của theme hiện tại.
        // Đây là lý do tại sao hàng được bôi xanh khi bạn nhấp vào.
        painter->fillRect(opt.rect, opt.palette.highlight());
    }
    // Chúng ta không cần vẽ nền mặc định hoặc text ở đây, vì editor widget sẽ che nó đi.
}

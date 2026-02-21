#pragma once
#include <QStyledItemDelegate>

// ─────────────────────────────────────────────────────────────
// ExtensionDelegate
//   Renders each extension row in the list view with:
//   • Icon placeholder (coloured square)
//   • Name (bold) + version
//   • Author
//   • Short description
//   • Star rating + install count
//   • "Install" / "Installed ✓" badge on the right
// ─────────────────────────────────────────────────────────────
class ExtensionDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ExtensionDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

    // Returns true if the Install button was hit
    bool editorEvent(QEvent *event,
                     QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

signals:
    void installRequested(const QModelIndex &index);

private:
    QRect installButtonRect(const QStyleOptionViewItem &option) const;
    void  drawStars(QPainter *p, const QRectF &rect, double rating) const;
};

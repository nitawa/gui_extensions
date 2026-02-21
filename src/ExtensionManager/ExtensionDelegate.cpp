#include "ExtensionDelegate.h"
#include "ExtensionModel.h"
#include <QPainter>
#include <QApplication>
#include <QMouseEvent>
#include <QAbstractItemView>

static const int ROW_HEIGHT    = 90;
static const int ICON_SIZE     = 50;
static const int PADDING       = 10;
static const int BTN_W         = 90;
static const int BTN_H         = 26;

ExtensionDelegate::ExtensionDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

// ── Geometry helpers ─────────────────────────────────────────

QRect ExtensionDelegate::installButtonRect(const QStyleOptionViewItem &option) const
{
    return QRect(option.rect.right() - BTN_W - PADDING,
                 option.rect.top() + (option.rect.height() - BTN_H) / 2,
                 BTN_W, BTN_H);
}

// ── Paint ─────────────────────────────────────────────────────

void ExtensionDelegate::paint(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    painter->save();

    // Background
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, QColor("#094771"));
    } else if (option.state & QStyle::State_MouseOver) {
        painter->fillRect(option.rect, QColor("#2a2d2e"));
    } else {
        painter->fillRect(option.rect, QColor("#252526"));
    }

    // Separator line
    painter->setPen(QColor("#3c3c3c"));
    painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());

    const int x0 = option.rect.left() + PADDING;
    const int y0 = option.rect.top()  + PADDING;

    // ── Icon placeholder ─────────────────────────────────────
    QRect iconRect(x0, y0, ICON_SIZE, ICON_SIZE);
    // Derive a colour from the extension id for variety
    QString id = index.data(ExtensionModel::IdRole).toString();
    uint hash = qHash(id);
    QColor iconColor = QColor::fromHsv(hash % 360, 180, 200);
    painter->fillRect(iconRect, iconColor);
    painter->setPen(Qt::white);
    QFont iconFont = painter->font();
    iconFont.setPointSize(18);
    iconFont.setBold(true);
    painter->setFont(iconFont);
    painter->drawText(iconRect, Qt::AlignCenter,
                      index.data(Qt::DisplayRole).toString().left(1).toUpper());

    // ── Text area ────────────────────────────────────────────
    const int textX = x0 + ICON_SIZE + PADDING;
    const int textW = option.rect.right() - BTN_W - PADDING * 2 - textX;

    // Name + version
    QString name    = index.data(Qt::DisplayRole).toString();
    QString version = index.data(ExtensionModel::VersionRole).toString();
    QString author  = index.data(ExtensionModel::AuthorRole).toString();
    QString desc    = index.data(ExtensionModel::DescriptionRole).toString();
    double  rating  = index.data(ExtensionModel::RatingRole).toDouble();
    int     installs= index.data(ExtensionModel::InstallsRole).toInt();
    bool    installed = index.data(ExtensionModel::InstalledRole).toBool();

    QFont boldFont = painter->font();
    boldFont.setPointSize(10);
    boldFont.setBold(true);
    painter->setFont(boldFont);
    painter->setPen(QColor("#cccccc"));
    QRect nameRect(textX, y0, textW - 60, 20);
    painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter,
                      QFontMetrics(boldFont).elidedText(name, Qt::ElideRight, nameRect.width()));

    // Version badge
    QFont smallFont = boldFont;
    smallFont.setPointSize(8);
    smallFont.setBold(false);
    painter->setFont(smallFont);
    painter->setPen(QColor("#888888"));
    painter->drawText(QRect(nameRect.right(), y0, 60, 20),
                      Qt::AlignLeft | Qt::AlignVCenter, "v" + version);

    // Author
    painter->setPen(QColor("#75beff"));
    painter->drawText(QRect(textX, y0 + 20, textW, 16),
                      Qt::AlignLeft | Qt::AlignVCenter, author);

    // Description
    painter->setPen(QColor("#9d9d9d"));
    QFontMetrics fm(smallFont);
    QString elidedDesc = fm.elidedText(desc, Qt::ElideRight, textW);
    painter->drawText(QRect(textX, y0 + 36, textW, 16),
                      Qt::AlignLeft | Qt::AlignVCenter, elidedDesc);

    // Stars + install count
    QRectF starsRect(textX, y0 + 55, 80, 14);
    drawStars(painter, starsRect, rating);
    painter->setPen(QColor("#888888"));
    QString installStr = installs >= 1000
        ? QString("%1K").arg(installs / 1000)
        : QString::number(installs);
    painter->drawText(QRect(textX + 86, y0 + 54, 100, 16),
                      Qt::AlignLeft | Qt::AlignVCenter,
                      installStr + " installs");

    // ── Install button ───────────────────────────────────────
    QRect btnRect = installButtonRect(option);
    if (installed) {
        painter->setPen(QColor("#4ec9b0"));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(btnRect, 3, 3);
        painter->setPen(QColor("#4ec9b0"));
        QFont f = smallFont; f.setBold(true); painter->setFont(f);
        painter->drawText(btnRect, Qt::AlignCenter, "✓ Installed");
    } else {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor("#0e639c"));
        painter->drawRoundedRect(btnRect, 3, 3);
        painter->setPen(Qt::white);
        QFont f = smallFont; f.setBold(true); painter->setFont(f);
        painter->drawText(btnRect, Qt::AlignCenter, "Install");
    }

    painter->restore();
}

// ── Star drawing ─────────────────────────────────────────────

void ExtensionDelegate::drawStars(QPainter *p, const QRectF &rect, double rating) const
{
    const int   nStars  = 5;
    const qreal starW   = rect.width() / nStars;
    const qreal starH   = rect.height();

    for (int i = 0; i < nStars; ++i) {
        QRectF sr(rect.left() + i * starW, rect.top(), starW - 1, starH);
        double fill = qBound(0.0, rating - i, 1.0);

        // Empty star
        p->setPen(QColor("#666666"));
        p->setBrush(Qt::NoBrush);
        // draw a simple ★ character
        QFont f; f.setPointSizeF(starH * 0.85);
        p->setFont(f);
        p->setPen(fill > 0.5 ? QColor("#f0a500") : QColor("#555555"));
        p->drawText(sr, Qt::AlignCenter, "★");
    }
}

// ── Size hint ────────────────────────────────────────────────

QSize ExtensionDelegate::sizeHint(const QStyleOptionViewItem &,
                                  const QModelIndex &) const
{
    return QSize(300, ROW_HEIGHT);
}

// ── Mouse event (Install button click) ───────────────────────

bool ExtensionDelegate::editorEvent(QEvent *event,
                                    QAbstractItemModel *,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        QRect btnRect   = installButtonRect(option);
        if (btnRect.contains(me->pos())) {
            bool installed = index.data(ExtensionModel::InstalledRole).toBool();
            if (!installed)
                emit installRequested(index);
            return true;
        }
    }
    return false;
}

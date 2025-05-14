#ifndef TILE_ICON_MANAGER_H
#define TILE_ICON_MANAGER_H

#include <QMap>
#include <QPushButton>
#include <QIcon>
#include <QString>
#include <QSize>

enum class TileType {
    Air,
    Wall,
    DarkWall,
    Coin,
    Player,
    Spikes,
    Enemy,
    Exit
};

class TileIconManager
{
public:
    TileIconManager() {}

    void registerButton(TileType type, const QString& path, QPushButton* button) {
        iconPaths[type] = path;
        buttons[type] = button;
    }

    void scaleIcons(const QSize& size) {
        for (auto it = buttons.begin(); it != buttons.end(); ++it) {
            TileType type = it.key();
            QPushButton* button = it.value();
            QString path = iconPaths.value(type);
            QIcon icon = getScaledIcon(path, size);
            button->setIcon(icon);
            button->setIconSize(size);
        }
    }

    QIcon getScaledIcon(const QString& path, const QSize& size) {
        if (path.isEmpty()) return QIcon();
        QPixmap pixmap(path);
        if (pixmap.isNull()) return QIcon();
        return QIcon(pixmap.scaled(size, Qt::KeepAspectRatio, Qt::FastTransformation));
    }

    QString getPath(TileType type) const {
        return iconPaths.value(type, "");
    }

    void updateButtonStyles(TileType selectedTile)
    {
        for (auto button : buttons) {
            button->setStyleSheet("");
        }

        QPushButton* activeButton = buttons[selectedTile];
        if (activeButton) {
            activeButton->setStyleSheet("background-color: yellow;");
        }
    }

private:
    QMap<TileType, QString> iconPaths;
    QMap<TileType, QPushButton*> buttons;
};

#endif // TILE_ICON_MANAGER_H

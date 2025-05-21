#ifndef TILEICONMANAGER_H
#define TILEICONMANAGER_H

#include <QMap>
#include <QPushButton>
#include <QIcon>
#include <QString>

enum class TileType {
    Air,
    Wall,
    DarkWall,
    Coin,
    Spikes,
    Enemy,
    PlayerLeft,
    PlayerRight,
    PlayerUp,
    PlayerDown,
    Platform,
    Spring
};

class TileIconManager
{
public:
    TileIconManager() = default;
    ~TileIconManager() = default;

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

    static QIcon getScaledIcon(const QString& path, const QSize& size) {
        if (path.isEmpty()) return {};
        QPixmap pixmap(path);
        if (pixmap.isNull()) return {};
        return {pixmap.scaled(size, Qt::KeepAspectRatio, Qt::FastTransformation)};
    }

    void updateButtonStyles(const TileType selectedTile) {
        for (const auto button : buttons) button->setStyleSheet("");
        if (QPushButton* activeButton = buttons[selectedTile]) activeButton->setStyleSheet("background-color: yellow;");
    }

private:
    QMap<TileType, QString> iconPaths;
    QMap<TileType, QPushButton*> buttons;
};

#endif // TILEICONMANAGER_H
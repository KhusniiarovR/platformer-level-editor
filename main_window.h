#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QtWidgets>
#include "tile_icon_manager.h"
#include "direction_input_widget.h"


class MainWindow : public QMainWindow
{
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    int next_level[4] = {0, 0, 0, 0};

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void selectTile(char tile);
    void onTileClicked(int row, int col);
    void undoTilePlacement();

    void clearLevel();
    void resizeLevel(int newWidth, int newHeight);
    void newLevel();
    void deleteLevel();
    void importFromFile();
    void exportToFile();

    struct TileAction {
        int row;
        int col;
        QIcon previousIcon;
        char previousData;
        bool isEmpty;
    };

    QStack<TileAction> undoStack;
    TileType selectedTile;
    QPushButton* createButton(const QIcon &icon, TileType  tileType, QToolBar* layout);

    QTableWidget *level;
    QToolBar *buttonLayout;
    TileIconManager tileIconManager;

    QWidget* createActionButtons();

    void openResizeDialog();

    bool isDrawing = false;

    QListWidget* levelListWidget;
    void loadLevelListFromFile(const QString& path);
    void parseLevel(const QString& levelString);
    void saveLevel();

    DirectionInputWidget *dirWidget;
};

#endif // MAIN_WINDOW_H

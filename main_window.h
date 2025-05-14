#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QtWidgets>
#include "tile_icon_manager.h"


class MainWindow : public QMainWindow
{
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

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

    QTableWidget *level;
    QToolBar *buttonLayout;

    TileIconManager tileIconManager;
    QWidget* createActionButtons();
    void openResizeDialog();

    bool isDrawing = false;
    int lastRow = -1;
    int lastCol = -1;

    QPushButton* createButton(const QIcon &icon, TileType  tileType, QToolBar* layout);
};

#endif // MAIN_WINDOW_H

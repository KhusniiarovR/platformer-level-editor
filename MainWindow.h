#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QtWidgets>
#include "TileIconManager.h"
#include "DirectionInputWidget.h"

class MainWindow : public QMainWindow
{
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;
    int next_level[4] = {0, 0, 0, 0};

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void onTileClicked(int row, int col);

    void saveLevel();
    void newLevel();
    void deleteLevel();
    void importFromFile();
    void exportToFile();
    void helpDialog();
    void clearLevel();
    void resizeLevel(int newWidth, int newHeight);
    void undoTilePlacement();

    QWidget* createActionButtons();
    void resizeDialog();
    void parseLevel(const QString& levelString);
    void loadLevelListFromFile(const QString& path) const;

    struct TileAction {
        int row;
        int col;
        QIcon previousIcon;
        QVariant previousData;
        bool isEmpty;
    };

    QStack<TileAction> undoStack;
    TileType selectedTile;
    bool isDrawing = false;

    QTableWidget *level;
    QToolBar *buttonLayout;
    TileIconManager tileIconManager;
    QListWidget* levelListWidget;
    DirectionInputWidget *dirWidget;
};

#endif // MAIN_WINDOW_H

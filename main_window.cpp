#include "main_window.h"
#include "utilities.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), selectedTile(TileType::Wall)
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    setWindowTitle("Level Editor");
    setFocusPolicy(Qt::StrongFocus);
    setFocus();

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    level = new QTableWidget(20, 20);

    level->verticalHeader()->setDefaultSectionSize(32);
    level->horizontalHeader()->setDefaultSectionSize(32);
    level->verticalHeader()->setMinimumSectionSize(32);
    level->horizontalHeader()->setMinimumSectionSize(32);
    level->verticalHeader()->setMaximumSectionSize(300);
    level->horizontalHeader()->setMaximumSectionSize(300);
    level->verticalHeader()->hide();
    level->horizontalHeader()->hide();
    level->setEditTriggers(QAbstractItemView::NoEditTriggers);
    level->setSelectionMode(QAbstractItemView::NoSelection);
    level->setSelectionBehavior(QAbstractItemView::SelectItems);
    level->setDragDropMode(QAbstractItemView::NoDragDrop);
    level->setFocusPolicy(Qt::StrongFocus);
    level->viewport()->installEventFilter(this);

    connect(level, &QTableWidget::cellClicked, this, &MainWindow::onTileClicked);

    mainLayout->addWidget(level);

    buttonLayout = new QToolBar();

    auto addTileButton = [&](TileType type, const QString& iconPath) {
        QPushButton* button = new QPushButton();
        button->setToolTip(QString::number((int)type));
        buttonLayout->addWidget(button);
        connect(button, &QPushButton::clicked, this, [this, type]() {
            this->selectedTile = type;
            tileIconManager.updateButtonStyles(selectedTile);
        });

        tileIconManager.registerButton(type, iconPath, button);
    };

    addTileButton(TileType::Air,      "data/sprites/air.png");
    addTileButton(TileType::Wall,     "data/sprites/wall.png");
    addTileButton(TileType::DarkWall, "data/sprites/wall_dark.png");
    addTileButton(TileType::Coin,     "data/sprites/coin.png");
    addTileButton(TileType::Player,   "data/sprites/player.png");
    addTileButton(TileType::Spikes,   "data/sprites/spikes.png");
    addTileButton(TileType::Enemy,    "data/sprites/enemy.png");
    addTileButton(TileType::Exit,     "data/sprites/exit.png");

    tileIconManager.updateButtonStyles(selectedTile);

    mainLayout->addWidget(buttonLayout);

    QDockWidget* dockWidget = new QDockWidget("Actions", this);
    dockWidget->setAllowedAreas(Qt::RightDockWidgetArea);
    dockWidget->setWidget(createActionButtons());

    dockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
    addDockWidget(Qt::RightDockWidgetArea, dockWidget);

    centralWidget->show();

    this->showMaximized();
}

void MainWindow::parseLevel(const QString& encryptedData) {
    int rows, cols;
    std::vector<char> data;

    if (!decrypt(encryptedData, rows, cols, data)) {
        QMessageBox::warning(this, "Error", "Can't decode level");
        return;
    }

    level->setRowCount(rows);
    level->setColumnCount(cols);
    resizeLevel(cols, rows);

    QSize iconSize = level->iconSize() * 0.95;

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            QChar ch = data[i * cols + j];
            QTableWidgetItem* item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignCenter);

            QIcon icon;
            switch (ch.toLatin1()) {
                case ' ':   icon = tileIconManager.getScaledIcon("data/sprites/air.png", iconSize); break;
                case '#':   icon = tileIconManager.getScaledIcon("data/sprites/wall.png", iconSize); break;
                case '=':   icon = tileIconManager.getScaledIcon("data/sprites/wall_dark.png", iconSize); break;
                case '*':   icon = tileIconManager.getScaledIcon("data/sprites/coin.png", iconSize); break;
                case '@':   icon = tileIconManager.getScaledIcon("data/sprites/player.png", iconSize); break;
                case '^':   icon = tileIconManager.getScaledIcon("data/sprites/spikes.png", iconSize); break;
                case '&':   icon = tileIconManager.getScaledIcon("data/sprites/enemy.png", iconSize); break;
                case 'E':   icon = tileIconManager.getScaledIcon("data/sprites/exit.png", iconSize); break;
                default:    icon = QIcon();
            }

            item->setIcon(icon);
            level->setItem(i, j, item);
        }
    }
}


QWidget* MainWindow::createActionButtons() {
    QWidget* container = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(container);

    levelListWidget = new QListWidget;
    layout->addWidget(new QLabel("Уровни:"));
    layout->addWidget(levelListWidget, 1);

    connect(levelListWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        QString levelData = item->data(Qt::UserRole).toString();
        parseLevel(levelData);
    });

    QWidget* saveExportPanel = new QWidget;
    QVBoxLayout* topButtonLayout = new QVBoxLayout(saveExportPanel);


    QPushButton* saveButton = new QPushButton("Save level");
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveLevel);
    topButtonLayout->addWidget(saveButton);

    QPushButton* exportButton = new QPushButton("Export all files");
    connect(exportButton, &QPushButton::clicked, this, &MainWindow::exportToFile);
    topButtonLayout->addWidget(exportButton);

    QPushButton* newButton = new QPushButton("New level");
    connect(newButton, &QPushButton::clicked, this, &MainWindow::newLevel);
    topButtonLayout->addWidget(newButton);

    QPushButton* deleteButton = new QPushButton("Delete Level");
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteLevel);
    topButtonLayout->addWidget(deleteButton);

    layout->addWidget(saveExportPanel);

    layout->addStretch();

    layout->addItem(new QSpacerItem(20, 30, QSizePolicy::Minimum, QSizePolicy::Fixed));

    QWidget* bottomPanel = new QWidget;
    QVBoxLayout* bottomLayout = new QVBoxLayout(bottomPanel);

    QPushButton* clearButton = new QPushButton("Clear level");
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::clearLevel);
    bottomLayout->addWidget(clearButton);

    QPushButton* resizeButton = new QPushButton("Resize level");
    connect(resizeButton, &QPushButton::clicked, this, &MainWindow::openResizeDialog);
    bottomLayout->addWidget(resizeButton);

    QPushButton* undoButton = new QPushButton("Undo");
    connect(undoButton, &QPushButton::clicked, this, &MainWindow::undoTilePlacement);
    bottomLayout->addWidget(undoButton);

    layout->addWidget(bottomPanel);

    QDir dir;
    if (!dir.exists("data/saves")) {
        dir.mkpath("data/saves");
    }

    loadLevelListFromFile("data/saves/levels.rll");

    return container;
}

void MainWindow::loadLevelListFromFile(const QString& filePath) {

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot create level file:" << filePath;
    }

    QTextStream in(&file);
    QString currentLevelName;
    QStringList currentLevelData;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.startsWith("; Level")) {
            if (!currentLevelName.isEmpty()) {
                QListWidgetItem* item = new QListWidgetItem(currentLevelName);
                item->setData(Qt::UserRole, currentLevelData.join("|"));
                levelListWidget->addItem(item);
                currentLevelData.clear();
            }
            currentLevelName = line.mid(2).trimmed();
        } else if (!line.isEmpty()) {
            currentLevelData << line;
        }
    }

    if (!currentLevelName.isEmpty()) {
        QListWidgetItem* item = new QListWidgetItem(currentLevelName);
        item->setData(Qt::UserRole, currentLevelData.join("|"));
        levelListWidget->addItem(item);
    }
}


MainWindow::~MainWindow() { }

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_Z) {
        undoTilePlacement();
        event->accept();
        return;
    }

    if (event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_C) {
        clearLevel();
        event->accept();
        return;
    }

    if (event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_E) {
        exportToFile();
        event->accept();
        return;
    }


    if (event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_S) {
        saveLevel();
        event->accept();
        return;
    }

    if (event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_R) {
        openResizeDialog();
        event->accept();
        return;
    }

    if (event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_N) {
        newLevel();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_Delete) {
        deleteLevel();
        event->accept();
        return;
    }

    QMainWindow::keyPressEvent(event);
}


bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == level->viewport()) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                isDrawing = true;
            }
        } else if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (isDrawing) {
                int row = level->rowAt(mouseEvent->pos().y());
                int col = level->columnAt(mouseEvent->pos().x());
                if (row != -1 && col != -1) {
                    onTileClicked(row, col);
                }
            }
        } else if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                isDrawing = false;
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::openResizeDialog()
{
    QDialog resizeDialog(this);
    resizeDialog.setWindowTitle("Resize Level");

    QVBoxLayout* layout = new QVBoxLayout(&resizeDialog);
    QFormLayout* formLayout = new QFormLayout();
    QLineEdit* widthEdit = new QLineEdit(QString::number(level->columnCount()));
    QLineEdit* heightEdit = new QLineEdit(QString::number(level->rowCount()));

    formLayout->addRow("Width:", widthEdit);
    formLayout->addRow("Height:", heightEdit);

    QPushButton* applyButton = new QPushButton("Apply");
    connect(applyButton, &QPushButton::clicked, [this, widthEdit, heightEdit, &resizeDialog]() {
        bool widthValid, heightWalid;
        int newWidth = widthEdit->text().toInt(&widthValid);
        int newHeight = heightEdit->text().toInt(&heightWalid);

        if (widthValid && heightWalid && newWidth > 0 && newHeight > 0) {
            resizeLevel(newWidth, newHeight);
            resizeDialog.accept();
        } else {
            QMessageBox::warning(this, "Invalid Input", "Please enter valid positive integers for width and height.");
        }
    });

    layout->addLayout(formLayout);
    layout->addWidget(applyButton);

    resizeDialog.exec();
}


void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    int rows = level->rowCount();
    int columns = level->columnCount();

    int cellSize = std::max(std::min(level->viewport()->width() / columns, level->viewport()->height() / rows), 25);

    for (int row = 0; row < rows; ++row)
        level->setRowHeight(row, cellSize);

    for (int col = 0; col < columns; ++col)
        level->setColumnWidth(col, cellSize);

    QSize iconSize(cellSize, cellSize);
    level->setIconSize(iconSize);
    buttonLayout->setIconSize(iconSize);

    tileIconManager.scaleIcons(iconSize);
}


void MainWindow::selectTile(char tile) {
    switch (tile) {
        case '*': selectedTile = TileType::Coin; break;
        case '&': selectedTile = TileType::Enemy; break;
        case 'E': selectedTile = TileType::Exit; break;
        case '@': selectedTile = TileType::Player; break;
        case '^': selectedTile = TileType::Spikes; break;
        case '#': selectedTile = TileType::Wall; break;
        case '=': selectedTile = TileType::DarkWall; break;
        default: selectedTile = TileType::Air; break;
    }
}

void MainWindow::onTileClicked(int row, int col)
{
    QTableWidgetItem* item = level->item(row, col);
    if (item == nullptr) {
        item = new QTableWidgetItem();
        level->setItem(row, col, item);
    }

    char currentChar = item->data(Qt::UserRole).toChar().toLatin1();
    char targetChar = '-';

    switch (selectedTile) {
        case TileType::Air:     targetChar = ' '; break;
        case TileType::Wall:    targetChar = '#'; break;
        case TileType::DarkWall:targetChar = '='; break;
        case TileType::Coin:    targetChar = '*'; break;
        case TileType::Player:  targetChar = '@'; break;
        case TileType::Spikes:  targetChar = '^'; break;
        case TileType::Enemy:   targetChar = '&'; break;
        case TileType::Exit:    targetChar = 'E'; break;
    }

    if (currentChar == targetChar) {
        return;
    }

    item->setText(QString(1, targetChar));
    item->setForeground(Qt::transparent);

    TileAction action;
    action.row = row;
    action.col = col;
    action.previousIcon = item->icon();
    action.previousData = item->text().toStdString()[0];
    action.isEmpty = item->icon().isNull();


    QSize iconSize = level->iconSize() * 0.95;
    QIcon icon;
    switch (selectedTile) {
        case TileType::Air:     icon = tileIconManager.getScaledIcon("data/sprites/air.png", iconSize); break;
        case TileType::Wall:    icon = tileIconManager.getScaledIcon("data/sprites/wall.png", iconSize); break;
        case TileType::DarkWall:icon = tileIconManager.getScaledIcon("data/sprites/wall_dark.png", iconSize); break;
        case TileType::Coin:    icon = tileIconManager.getScaledIcon("data/sprites/coin.png", iconSize); break;
        case TileType::Player:  icon = tileIconManager.getScaledIcon("data/sprites/player.png", iconSize); break;
        case TileType::Spikes:  icon = tileIconManager.getScaledIcon("data/sprites/spikes.png", iconSize); break;
        case TileType::Enemy:   icon = tileIconManager.getScaledIcon("data/sprites/enemy.png", iconSize); break;
        case TileType::Exit:    icon = tileIconManager.getScaledIcon("data/sprites/exit.png", iconSize); break;
    }

    item->setIcon(icon);
    item->setData(Qt::UserRole, targetChar);

    undoStack.push(action);
}

void MainWindow::undoTilePlacement()
{
    if (undoStack.isEmpty()) return;

    TileAction action = undoStack.pop();

    QTableWidgetItem* item = level->item(action.row, action.col);
    if (item == nullptr) {
        item = new QTableWidgetItem();
        level->setItem(action.row, action.col, item);
    }

    if (action.isEmpty) {
        item->setIcon(QIcon());
        item->setData(Qt::UserRole, '-');
        item->setData(Qt::DisplayRole, QString('-'));
    } else {
        item->setIcon(action.previousIcon);
        item->setData(Qt::UserRole, action.previousData);
        item->setData(Qt::DisplayRole, QString(action.previousData));
    }
}

QPushButton* MainWindow::createButton(const QIcon &icon, TileType  tileType, QToolBar* layout)
{
    QPushButton *button = new QPushButton();
    button->setFixedSize(32, 32);
    button->setIcon(icon);
    connect(button, &QPushButton::clicked, this, [this, tileType]() {selectTile(static_cast<char>(tileType));});
    layout->addWidget(button);
    return button;
}

void MainWindow::clearLevel() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Clear Level", "Are you sure you want to clear the level?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        int rows = level->rowCount();
        int columns = level->columnCount();
        QSize iconSize = level->iconSize() * 0.95;
        QIcon airIcon = tileIconManager.getScaledIcon("data/sprites/air.png", iconSize);

        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < columns; ++col) {
                QTableWidgetItem* item = level->item(row, col);
                if (!item) {
                    item = new QTableWidgetItem();
                    level->setItem(row, col, item);
                }
                item->setIcon(airIcon);
                item->setData(Qt::UserRole, '-');
                item->setText("-");
                item->setForeground(Qt::transparent);
            }
        }
    }
}


void MainWindow::resizeLevel(int newWidth, int newHeight)
{
    level->setRowCount(newHeight);
    level->setColumnCount(newWidth);

    int cellSize = std::min(level->viewport()->width() / newWidth, level->viewport()->height() / newHeight);
    for (int row = 0; row < newHeight; ++row)
        level->setRowHeight(row, cellSize);

    for (int col = 0; col < newWidth; ++col)
        level->setColumnWidth(col, cellSize);

    QSize newSize(newWidth * cellSize, newHeight * cellSize);
    QResizeEvent event(newSize, size());
    resizeEvent(&event);
}

void MainWindow::saveLevel() {
    int rows = level->rowCount();
    int cols = level->columnCount();
    std::vector<char> data(rows * cols, '-');

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            QTableWidgetItem* item = level->item(i, j);
            if (item) {
                data[i * cols + j] = item->data(Qt::UserRole).toChar().toLatin1();
            }
        }
    }

    QString encryptedData;
    encrypt(rows, cols, data, encryptedData);

    QListWidgetItem* currentItem = levelListWidget->currentItem();

    if (currentItem) {
        currentItem->setData(Qt::UserRole, encryptedData);

        QFile file("data/saves/levels.rll");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);

            for (int i = 0; i < levelListWidget->count(); ++i) {
                QListWidgetItem* item = levelListWidget->item(i);
                QString levelName = item->text();
                QString levelData = item->data(Qt::UserRole).toString();

                out << "; " << levelName << "\n";
                out << levelData << "\n";
            }

            file.close();
        }
    } else {
        int nextIndex = levelListWidget->count() + 1;
        QString levelName = QString("Level %1").arg(nextIndex);
        QListWidgetItem* newItem = new QListWidgetItem(levelName);
        newItem->setData(Qt::UserRole, encryptedData);
        levelListWidget->addItem(newItem);
        levelListWidget->setCurrentItem(newItem);

        QFile file("data/saves/levels.rll");
        if (file.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&file);
            out << "\n; " << levelName << "\n";
            out << encryptedData << "\n";
            file.close();
        }
    }

}

void MainWindow::newLevel() {
    int rows = level->rowCount();
    int cols = level->columnCount();

    int maxNumber = 0;
    for (int i = 0; i < levelListWidget->count(); ++i) {
        QListWidgetItem* item = levelListWidget->item(i);
        QString text = item->text();
        QRegularExpression re("Level (\\d+)");
        QRegularExpressionMatch match = re.match(text);
        if (match.hasMatch()) {
            int num = match.captured(1).toInt();
            maxNumber = std::max(maxNumber, num);
        }
    }

    int newLevelNumber = maxNumber + 1;
    QString newLevelName = QString("Level %1").arg(newLevelNumber);

    std::vector<char> emptyData(rows * cols, '-');
    QString encrypted;
    encrypt(rows, cols, emptyData, encrypted);

    QListWidgetItem* newItem = new QListWidgetItem(newLevelName);
    newItem->setData(Qt::UserRole, encrypted);
    levelListWidget->addItem(newItem);

    levelListWidget->setCurrentItem(newItem);
    parseLevel(encrypted);
}

void MainWindow::deleteLevel() {
    QListWidgetItem* selectedItem = levelListWidget->currentItem();
    if (!selectedItem) {
        QMessageBox::information(this, "Info", "No level selected to delete.");
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Delete Level", "Are you sure you want to delete this level?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No) {
        return;
    }

    int row = levelListWidget->row(selectedItem);
    QString levelName = selectedItem->text();

    delete levelListWidget->takeItem(row);

    QFile file("data/saves/levels.rll");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QMessageBox::warning(this, "Error", "Unable to update file.");
        return;
    }

    QTextStream out(&file);
    for (int i = 0; i < levelListWidget->count(); ++i) {
        QListWidgetItem* item = levelListWidget->item(i);
        out << "\n; Level " << (i + 1) << "\n";
        out << item->data(Qt::UserRole).toString() << "\n";
        item->setText(QString("Level %1").arg(i + 1));
    }

    file.close();
}



void MainWindow::exportToFile() {
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Export Level",
        "data/saves",
        "RLL Files (*.rll);;All Files (*)"
    );

    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Error", "Unable to open file for saving.");
        return;
    }

    QTextStream out(&file);

    int rows = level->rowCount();
    int columns = level->columnCount();
    std::vector<char> data(rows * columns, '-');

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            QTableWidgetItem *item = level->item(i, j);
            if (item) {
                data[i * columns + j] = item->text().toStdString()[0];
            }
        }
    }
    QString encryptedData;
    encrypt(rows, columns, data, encryptedData);

    out << encryptedData;


    file.close();
}

// todo separate parts in different files
// todo add sounds
// todo player spawn logic here and in proj1
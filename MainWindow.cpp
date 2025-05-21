#include "MainWindow.h"
#include "utilities.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), selectedTile(TileType::Wall)
{
    auto *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    setWindowTitle("Level Editor");
    setFocusPolicy(Qt::StrongFocus);

    auto *mainLayout = new QVBoxLayout(centralWidget);

    level = new QTableWidget(20, 20);
    level->verticalHeader()->setDefaultSectionSize(32);
    level->horizontalHeader()->setDefaultSectionSize(32);
    level->verticalHeader()->setMinimumSectionSize(32);
    level->horizontalHeader()->setMinimumSectionSize(32);
    level->verticalHeader()->setMaximumSectionSize(300);
    level->horizontalHeader()->setMaximumSectionSize(300);
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
        auto* button = new QPushButton();
        button->setToolTip(QString::number(static_cast<int>(type)));
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
    addTileButton(TileType::Spikes,   "data/sprites/spikes.png");
    addTileButton(TileType::Enemy,    "data/sprites/enemy.png");
    addTileButton(TileType::PlayerLeft,"data/sprites/player_left.png");
    addTileButton(TileType::PlayerRight,"data/sprites/player_right.png");
    addTileButton(TileType::PlayerUp, "data/sprites/player_up.png");
    addTileButton(TileType::PlayerDown,"data/sprites/player_down.png");
    addTileButton(TileType::Platform,"data/sprites/platform.png");
    addTileButton(TileType::Spring,     "data/sprites/spring.png");
    tileIconManager.updateButtonStyles(selectedTile);
    mainLayout->addWidget(buttonLayout);

    dirWidget = new DirectionInputWidget(this);
    mainLayout->addWidget(dirWidget);

    auto* dockWidget = new QDockWidget("Actions", this);
    dockWidget->setAllowedAreas(Qt::RightDockWidgetArea);
    dockWidget->setWidget(createActionButtons());

    dockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
    addDockWidget(Qt::RightDockWidgetArea, dockWidget);

    centralWidget->show();
    this->showMaximized();
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_S) {
        saveLevel();
        event->accept();
        return;}
    if (event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_N) {
        newLevel();
        event->accept();
        return;}
    if (event->key() == Qt::Key_Delete) {
        deleteLevel();
        event->accept();
        return;}
    if (event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_I) {
        importFromFile();
        event->accept();
        return;}
    if (event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_E) {
        exportToFile();
        event->accept();
        return;}
    if (event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_H) {
        helpDialog();
        event->accept();
        return;}
    if (event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_C) {
        clearLevel();
        event->accept();
        return;}
    if (event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_R) {
        resizeDialog();
        event->accept();
        return;}
    if (event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_Z) {
        undoTilePlacement();
        event->accept();
        return;}
    QMainWindow::keyPressEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    int rows = level->rowCount();
    int columns = level->columnCount();
    int cellSize = std::max(std::min(level->viewport()->width() / columns, level->viewport()->height() / rows), 25);
    for (int row = 0; row < rows; ++row) level->setRowHeight(row, cellSize);
    for (int col = 0; col < columns; ++col) level->setColumnWidth(col, cellSize);

    QSize iconSize(cellSize, cellSize);
    level->setIconSize(iconSize);
    buttonLayout->setIconSize(iconSize);
    tileIconManager.scaleIcons(iconSize);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == level->viewport()) {
        if (event->type() == QEvent::MouseButtonPress) {
            auto *mouseEvent = dynamic_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) isDrawing = true;
        }
        else if (event->type() == QEvent::MouseMove) {
            auto *mouseEvent = dynamic_cast<QMouseEvent*>(event);
            if (isDrawing) {
                int row = level->rowAt(mouseEvent->pos().y());
                int col = level->columnAt(mouseEvent->pos().x());
                if (row != -1 && col != -1) onTileClicked(row, col);
            }
        }
        else if (event->type() == QEvent::MouseButtonRelease) {
            auto *mouseEvent = dynamic_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) isDrawing = false;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::onTileClicked(int row, int col) {
    QTableWidgetItem* item = level->item(row, col);
    if (item == nullptr) {
        item = new QTableWidgetItem();
        level->setItem(row, col, item);
    }

    char currentChar = item->data(Qt::UserRole).toChar().toLatin1();
    char targetChar = '-';
    switch (selectedTile) {
        case TileType::Air:        targetChar = '-'; break;
        case TileType::Wall:       targetChar = '#'; break;
        case TileType::DarkWall:   targetChar = '='; break;
        case TileType::Coin:       targetChar = '*'; break;
        case TileType::Spikes:     targetChar = '^'; break;
        case TileType::Enemy:      targetChar = '&'; break;
        case TileType::PlayerLeft: targetChar = 'L'; break;
        case TileType::PlayerRight:targetChar = 'R'; break;
        case TileType::PlayerUp:   targetChar = 'U'; break;
        case TileType::PlayerDown: targetChar = 'D'; break;
        case TileType::Platform:   targetChar = 'P'; break;
        case TileType::Spring:     targetChar = 'S'; break;
    }
    if (currentChar == targetChar) return;

    item->setText(QString(1, targetChar));
    item->setForeground(Qt::transparent);
    TileAction action;
    action.row = row;
    action.col = col;
    action.previousIcon = item->icon();
    action.previousData = item->data(Qt::UserRole);
    action.isEmpty = item->icon().isNull();
    QSize iconSize = level->iconSize() * 0.95;
    QIcon icon;
    switch (selectedTile) {
        case TileType::Air:        icon = TileIconManager::getScaledIcon("data/sprites/air.png", iconSize); break;
        case TileType::Wall:       icon = TileIconManager::getScaledIcon("data/sprites/wall.png", iconSize); break;
        case TileType::DarkWall:   icon = TileIconManager::getScaledIcon("data/sprites/wall_dark.png", iconSize); break;
        case TileType::Coin:       icon = TileIconManager::getScaledIcon("data/sprites/coin.png", iconSize); break;
        case TileType::Spikes:     icon = TileIconManager::getScaledIcon("data/sprites/spikes.png", iconSize); break;
        case TileType::Enemy:      icon = TileIconManager::getScaledIcon("data/sprites/enemy.png", iconSize); break;
        case TileType::PlayerLeft: icon = TileIconManager::getScaledIcon("data/sprites/player_left.png", iconSize); break;
        case TileType::PlayerRight:icon = TileIconManager::getScaledIcon("data/sprites/player_right.png", iconSize); break;
        case TileType::PlayerUp:   icon = TileIconManager::getScaledIcon("data/sprites/player_up.png", iconSize); break;
        case TileType::PlayerDown: icon = TileIconManager::getScaledIcon("data/sprites/player_down.png", iconSize); break;
        case TileType::Platform:   icon = TileIconManager::getScaledIcon("data/sprites/platform.png", iconSize); break;
        case TileType::Spring:     icon = TileIconManager::getScaledIcon("data/sprites/spring.png", iconSize); break;
    }
    item->setIcon(icon);
    item->setData(Qt::UserRole, targetChar);
    undoStack.push(action);
}

void MainWindow::saveLevel() {
    int rows = level->rowCount();
    int cols = level->columnCount();
    std::vector<char> data(rows * cols, '-');
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            QTableWidgetItem* item = level->item(i, j);
            if (item) {
                QVariant var = item->data(Qt::UserRole);
                char symbol = '-';
                if (var.isValid() && !var.isNull()) {
                    QChar ch = var.toChar();
                    if (!ch.isNull()) symbol = ch.toLatin1();
                }
                data[i * cols + j] = symbol;
            }
        }
    }
    QString encryptedData;
    dirWidget->getValues(next_level);
    encrypt(rows, cols, data, next_level, encryptedData);

    if (QListWidgetItem* currentItem = levelListWidget->currentItem()) {
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
    }
    else {
        int nextIndex = levelListWidget->count() + 1;
        QString levelName = QString("Level %1").arg(nextIndex);
        auto* newItem = new QListWidgetItem(levelName);
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
    undoStack.clear();
}

void MainWindow::newLevel() {
    int rows = level->rowCount();
    int cols = level->columnCount();
    int maxNumber = 0;
    for (int i = 0; i < levelListWidget->count(); ++i) {
        QListWidgetItem* item = levelListWidget->item(i);
        QString text = item->text();
        QRegularExpression re("Level (\\d+)");
        if (QRegularExpressionMatch match = re.match(text); match.hasMatch()) {
            int num = match.captured(1).toInt();
            maxNumber = std::max(maxNumber, num);
        }
    }
    int newLevelNumber = maxNumber + 1;
    QString newLevelName = QString("Level %1").arg(newLevelNumber);
    std::vector<char> emptyData(rows * cols, '-');
    QString encrypted;
    dirWidget->getValues(next_level);
    encrypt(rows, cols, emptyData, next_level, encrypted);

    auto* newItem = new QListWidgetItem(newLevelName);
    newItem->setData(Qt::UserRole, encrypted);
    levelListWidget->addItem(newItem);
    levelListWidget->setCurrentItem(newItem);
    for (int & i : next_level) i = 0;
    parseLevel(encrypted);
}

void MainWindow::deleteLevel() {
    QListWidgetItem* selectedItem = levelListWidget->currentItem();
    if (!selectedItem) {
        QMessageBox::information(this, "Info", "No level selected to delete.");
        return;
    }
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Delete Level", "Are you sure you want to delete this level?",
                                                              QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No) return;
    int row = levelListWidget->row(selectedItem);
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

void MainWindow::importFromFile() {
    QString sourcePath = QFileDialog::getOpenFileName(
        this,
        "Select File to Import",
        QDir::homePath(),
        "RLL Files (*.rll);;All Files (*)"
    );
    if (sourcePath.isEmpty() || !QFile::exists(sourcePath)) {
        QMessageBox::warning(this, "Error", "Selected file does not exist.");
        return;
    }
    QFileInfo fileInfo(sourcePath);
    QString fileName = fileInfo.fileName();
    QString destinationDir = QDir::currentPath() + "/data/saves";
    QString destinationPath = QDir(destinationDir).filePath(fileName);

    if (QFile::exists(destinationPath)) {
        auto reply = QMessageBox::question(
            this, "Overwrite?", "File already exists in data/saves/. Overwrite?",
            QMessageBox::Yes | QMessageBox::No
        );
        if (reply != QMessageBox::Yes) return;
        QFile::remove(destinationPath);
    }
    if (!QFile::copy(sourcePath, destinationPath)) QMessageBox::critical(this, "Error", "Failed to import the file.");
    else QMessageBox::information(this, "Success", "File imported successfully to:\n" + destinationPath);
    loadLevelListFromFile("data/saves/levels.rll");
}

void MainWindow::exportToFile() {
    QString sourceDir = QDir::currentPath() + "/data/saves";
    QString sourcePath = QFileDialog::getOpenFileName(
        this, "Select File to Export from data/saves",
        sourceDir, "RLL Files (*.rll);;All Files (*)"
    );
    if (sourcePath.isEmpty() || !QFile::exists(sourcePath)) {
        QMessageBox::warning(this, "Error", "Selected file does not exist.");
        return;
    }
    QString targetDir = QFileDialog::getExistingDirectory(
        this,
        "Select Target Directory",
        QDir::homePath()
    );
    if (targetDir.isEmpty()) return;
    QFileInfo fileInfo(sourcePath);
    QString fileName = fileInfo.fileName();
    QString destinationPath = QDir(targetDir).filePath(fileName);
    if (QFile::exists(destinationPath)) {
        auto reply = QMessageBox::question(
            this, "Overwrite?", "File already exists in the target directory. Overwrite?",
            QMessageBox::Yes | QMessageBox::No
        );
        if (reply != QMessageBox::Yes) return;
        QFile::remove(destinationPath);
    }
    if (!QFile::copy(sourcePath, destinationPath)) QMessageBox::critical(this, "Error", "Failed to export the file.");
    else QMessageBox::information(this, "Success", "File exported successfully to:\n" + destinationPath);
}

void MainWindow::helpDialog() {
    auto *dialog = new QDialog(this);
    dialog->setWindowTitle("Level Editor Guide");
    auto *browser = new QTextBrowser(dialog);

    QFile file("Editor.md");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Cannot open help file.");
        return;
    }
    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    browser->setHtml(content);
    auto *layout = new QVBoxLayout(dialog);
    layout->addWidget(browser);
    dialog->setLayout(layout);
    dialog->resize(1200,800);
    dialog->exec();
}

void MainWindow::clearLevel() {
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Clear Level", "Are you sure you want to clear the level?",
                                                              QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        int rows = level->rowCount();
        int columns = level->columnCount();
        QSize iconSize = level->iconSize() * 0.95;
        QIcon airIcon = TileIconManager::getScaledIcon("data/sprites/air.png", iconSize);
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

void MainWindow::resizeLevel(int newWidth, int newHeight) {
    level->setRowCount(newHeight);
    level->setColumnCount(newWidth);
    int cellSize = std::min(level->viewport()->width() / newWidth, level->viewport()->height() / newHeight);
    for (int row = 0; row < newHeight; ++row) level->setRowHeight(row, cellSize);
    for (int col = 0; col < newWidth; ++col) level->setColumnWidth(col, cellSize);
    QSize newSize(newWidth * cellSize, newHeight * cellSize);
    QResizeEvent event(newSize, size());
    resizeEvent(&event);
}

void MainWindow::undoTilePlacement() {
    if (undoStack.isEmpty()) return;
    TileAction action = undoStack.pop();

    QTableWidgetItem* item = level->item(action.row, action.col);
    if (item == nullptr) {
        item = new QTableWidgetItem();
        level->setItem(action.row, action.col, item);
    }

    if (action.isEmpty) {
        item->setIcon(QIcon());
        item->setData(Qt::UserRole, QString('-'));
    }
    else {
        item->setIcon(action.previousIcon);
        item->setData(Qt::UserRole, action.previousData);
    }
}

void MainWindow::resizeDialog() {
    QDialog resizeDialog(this);
    resizeDialog.setWindowTitle("Resize Level");
    auto* layout = new QVBoxLayout(&resizeDialog);
    auto* formLayout = new QFormLayout();
    auto* widthEdit = new QLineEdit(QString::number(level->columnCount()));
    auto* heightEdit = new QLineEdit(QString::number(level->rowCount()));
    formLayout->addRow("Width:", widthEdit);
    formLayout->addRow("Height:", heightEdit);
    auto* applyButton = new QPushButton("Apply");
    connect(applyButton, &QPushButton::clicked, [this, widthEdit, heightEdit, &resizeDialog]() {
        bool widthValid, heightWalid;
        int newWidth = widthEdit->text().toInt(&widthValid);
        int newHeight = heightEdit->text().toInt(&heightWalid);
        if (widthValid && heightWalid && newWidth > 0 && newHeight > 0) {
            resizeLevel(newWidth, newHeight);
            resizeDialog.accept();
        }
        else QMessageBox::warning(this, "Invalid Input", "Please enter valid positive integers for width and height.");
    });
    layout->addLayout(formLayout);
    layout->addWidget(applyButton);
    resizeDialog.exec();
}

void MainWindow::parseLevel(const QString& encryptedData) {
    int rows, cols;
    std::vector<char> data;
    dirWidget->getValues(next_level);
    if (!decrypt(encryptedData, rows, cols, next_level, data)) {
        QMessageBox::warning(this, "Error", "Can't decode level");
        return;
    }
    dirWidget->setNextLevel(next_level);
    level->setRowCount(rows);
    level->setColumnCount(cols);
    resizeLevel(cols, rows);

    const QSize iconSize = level->iconSize() * 0.95;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            QChar ch = data[i * cols + j];
            auto* item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignCenter);
            QIcon icon;
            switch (ch.toLatin1()) {
                case '-':   icon = TileIconManager::getScaledIcon("data/sprites/air.png", iconSize); break;
                case '#':   icon = TileIconManager::getScaledIcon("data/sprites/wall.png", iconSize); break;
                case '=':   icon = TileIconManager::getScaledIcon("data/sprites/wall_dark.png", iconSize); break;
                case '*':   icon = TileIconManager::getScaledIcon("data/sprites/coin.png", iconSize); break;
                case '^':   icon = TileIconManager::getScaledIcon("data/sprites/spikes.png", iconSize); break;
                case '&':   icon = TileIconManager::getScaledIcon("data/sprites/enemy.png", iconSize); break;
                case 'E':   icon = TileIconManager::getScaledIcon("data/sprites/exit.png", iconSize); break;
                case 'L':   icon = TileIconManager::getScaledIcon("data/sprites/player_left.png", iconSize); break;
                case 'R':   icon = TileIconManager::getScaledIcon("data/sprites/player_right.png", iconSize); break;
                case 'U':   icon = TileIconManager::getScaledIcon("data/sprites/player_up.png", iconSize); break;
                case 'D':   icon = TileIconManager::getScaledIcon("data/sprites/player_down.png", iconSize); break;
                case 'P':   icon = TileIconManager::getScaledIcon("data/sprites/platform.png", iconSize); break;
                case 'S':   icon = TileIconManager::getScaledIcon("data/sprites/spring.png", iconSize); break;
                default:    icon = QIcon();
            }
            item->setData(Qt::UserRole, ch);
            item->setIcon(icon);
            level->setItem(i, j, item);
        }
    }
}

QWidget* MainWindow::createActionButtons() {
    auto* container = new QWidget;
    auto* layout = new QVBoxLayout(container);
    levelListWidget = new QListWidget;
    layout->addWidget(new QLabel("Levels:"));
    layout->addWidget(levelListWidget, 1);
    connect(levelListWidget, &QListWidget::itemClicked, this, [this](const QListWidgetItem* item) {
        QString levelData = item->data(Qt::UserRole).toString();
        parseLevel(levelData);
    });

    auto* topPanel = new QWidget;
    auto* topButtonLayout = new QVBoxLayout(topPanel);
    auto* saveButton   = new QPushButton("Save level");connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveLevel);topButtonLayout->addWidget(saveButton);
    auto* newButton    = new QPushButton("New level");connect(newButton, &QPushButton::clicked, this, &MainWindow::newLevel);topButtonLayout->addWidget(newButton);
    auto* deleteButton = new QPushButton("Delete Level");connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteLevel);topButtonLayout->addWidget(deleteButton);
    auto* importButton = new QPushButton("Import");connect(importButton, &QPushButton::clicked, this, &MainWindow::importFromFile);topButtonLayout->addWidget(importButton);
    auto* exportButton = new QPushButton("Export");connect(exportButton, &QPushButton::clicked, this, &MainWindow::exportToFile);topButtonLayout->addWidget(exportButton);
    auto* helpButton   = new QPushButton("Help");connect(helpButton, &QPushButton::clicked, this, &MainWindow::helpDialog);topButtonLayout->addWidget(helpButton);
    layout->addWidget(topPanel);
    layout->addStretch();
    layout->addItem(new QSpacerItem(20, 30, QSizePolicy::Minimum, QSizePolicy::Fixed));

    auto* bottomPanel = new QWidget;
    auto* bottomLayout = new QVBoxLayout(bottomPanel);
    auto* clearButton = new QPushButton("Clear level");connect(clearButton, &QPushButton::clicked, this, &MainWindow::clearLevel);bottomLayout->addWidget(clearButton);
    auto* resizeButton = new QPushButton("Resize level");connect(resizeButton, &QPushButton::clicked, this, &MainWindow::resizeDialog);bottomLayout->addWidget(resizeButton);
    auto* undoButton = new QPushButton("Undo");connect(undoButton, &QPushButton::clicked, this, &MainWindow::undoTilePlacement);bottomLayout->addWidget(undoButton);
    layout->addWidget(bottomPanel);

    if (const QDir dir; !dir.exists("data/saves")) dir.mkpath("data/saves");
    loadLevelListFromFile("data/saves/levels.rll");
    return container;
}

void MainWindow::loadLevelListFromFile(const QString& path) const {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) qWarning() << "Cannot create level file:" << path;
    QTextStream in(&file);
    QString currentLevelName;
    QStringList currentLevelData;
    while (!in.atEnd()) {
        if (QString line = in.readLine().trimmed(); line.startsWith("; Level")) {
            if (!currentLevelName.isEmpty()) {
                auto* item = new QListWidgetItem(currentLevelName);
                item->setData(Qt::UserRole, currentLevelData.join("|"));
                levelListWidget->addItem(item);
                currentLevelData.clear();
            }
            currentLevelName = line.mid(2).trimmed();
        }
        else if (!line.isEmpty()) currentLevelData << line;
    }
    if (!currentLevelName.isEmpty()) {
        auto* item = new QListWidgetItem(currentLevelName);
        item->setData(Qt::UserRole, currentLevelData.join("|"));
        levelListWidget->addItem(item);
    }
}


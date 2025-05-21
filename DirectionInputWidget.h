#ifndef DIRECTIONINPUTWIDGET_H
#define DIRECTIONINPUTWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QFormLayout>
#include <QIntValidator>

class DirectionInputWidget : public QWidget
{
public:
    explicit DirectionInputWidget(QWidget *parent)
    : QWidget(parent) {
        leftEdit = new QLineEdit(this);
        rightEdit = new QLineEdit(this);
        upEdit = new QLineEdit(this);
        downEdit = new QLineEdit(this);

        for (QLineEdit *edit : {leftEdit, rightEdit, upEdit, downEdit}) edit->setValidator(new QIntValidator(-2, 9999, this));

        auto *layout = new QFormLayout;
        layout->addRow("Left:", leftEdit);
        layout->addRow("Right:", rightEdit);
        layout->addRow("Up:", upEdit);
        layout->addRow("Down:", downEdit);
        setLayout(layout);
    }

    void getValues(int (&next_level)[4]) const {
        next_level[0] = leftEdit->text().toInt();
        next_level[1] = rightEdit->text().toInt();
        next_level[2] = upEdit->text().toInt();
        next_level[3] = downEdit->text().toInt();
    }

    void setNextLevel(const int arr[4]) const {
        leftEdit->setText(QString::number(arr[0]));
        rightEdit->setText(QString::number(arr[1]));
        upEdit->setText(QString::number(arr[2]));
        downEdit->setText(QString::number(arr[3]));
    }

private:
    QLineEdit *leftEdit;
    QLineEdit *rightEdit;
    QLineEdit *upEdit;
    QLineEdit *downEdit;
};

#endif // DIRECTIONINPUTWIDGET_H

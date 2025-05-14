#pragma once
#include <QString>
#include <iostream>
#include <sstream>

void encrypt(int rows, int columns, const std::vector<char>& data, QString &output) {
    std::ostringstream result;

    for (int i = 0; i < rows; i++) {
        char current_char = data[i * columns];
        int char_strike = 1;

        for (int j = 1; j < columns; j++) {
            char next_char = data[i * columns + j];
            if (next_char == current_char) {
                char_strike++;
            }
            else {
                if (char_strike > 1) {
                    result << char_strike;
                }
                result << current_char;
                current_char = next_char;
                char_strike = 1;
            }
        }

        if (char_strike > 1) {
            result << char_strike;
        }
        result << current_char;

        if (i < rows - 1) {
            result << '|';
        }
    }

    output = QString::fromStdString(result.str());
}

bool decrypt(const QString& encoded, int& rows, int& cols, std::vector<char>& data) {
    std::string nextLine = encoded.toStdString();

    data.clear();
    rows = 0;
    cols = 0;
    int columns_number = 0;
    size_t i = 0;

    while (i < nextLine.length()) {
        if (isdigit(nextLine[i])) {
            int num = 0;
            while (i < nextLine.length() && isdigit(nextLine[i])) {
                num = num * 10 + (nextLine[i] - '0');
                ++i;
            }
            if (i < nextLine.length()) {
                char symbol = nextLine[i++];
                data.insert(data.end(), num, symbol);
                columns_number += num;
                continue;
            } else {
                return false;
            }
        } else if (nextLine[i] == '|') {
            if (cols == 0)
                cols = columns_number;
            else if (cols != columns_number)
                return false;

            columns_number = 0;
            rows++;
            ++i;
            continue;
        } else {
            data.push_back(nextLine[i]);
            columns_number++;
            ++i;
        }
    }

    if (columns_number > 0) {
        if (cols == 0)
            cols = columns_number;
        else if (cols != columns_number)
            return false;

        rows++;
    }

    return true;
}

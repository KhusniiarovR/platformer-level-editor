#pragma once
#include <QString>
#include <iostream>
#include <sstream>

void encrypt(int rows, int columns, const std::vector<char>& data, int next_level[4], QString &output) {
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

    result << "::";

    result << next_level[0] << " "
           << next_level[1] << " "
           << next_level[2] << " "
           << next_level[3];

    output = QString::fromStdString(result.str());
}

bool decrypt(const QString& encoded, int& rows, int& cols, int next_level[4], std::vector<char>& data) {
    std::string levelData = encoded.toStdString();

    size_t sep_pos = levelData.find("::");
    std::string body;
    std::string level_str;
    if (sep_pos == std::string::npos) {
        body = levelData;
        level_str = "";
        for (int i = 0; i < 4; ++i)
            next_level[i] = 0;
    } else {
        body = levelData.substr(0, sep_pos);
        level_str = levelData.substr(sep_pos + 2);
    }

    data.clear();
    rows = 0;
    cols = 0;
    int columns_number = 0;

    size_t i = 0;
    while (i < body.length()) {
        if (isdigit(body[i])) {
            int num = 0;
            while (i < body.length() && isdigit(body[i])) {
                num = num * 10 + (body[i] - '0');
                ++i;
            }
            if (i < body.length()) {
                char symbol = body[i++];
                data.insert(data.end(), num, symbol);
                columns_number += num;
                continue;
            } else {
                return false;
            }
        } else if (body[i] == '|') {
            if (cols == 0)
                cols = columns_number;
            else if (cols != columns_number)
                return false;
            columns_number = 0;
            rows++;
            ++i;
        } else {
            data.push_back(body[i]);
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

    std::istringstream level_stream(level_str);
    for (int j = 0; j < 4; ++j) {
        if (!(level_stream >> next_level[j])) {
            next_level[j] = 0;
        }
    }

    return true;
}

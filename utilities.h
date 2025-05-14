#include <QString>

#include <iostream>
#include <sstream>

void encrypt(int rows, int columns, const std::vector<char>& data, QString &output)
{
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

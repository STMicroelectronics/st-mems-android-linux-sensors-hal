/*
 * Copyright (C) 2018 The Android Open Source Project
 * Copyright (C) 2021 STMicroelectronics
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <array>
#include <string>

template <ssize_t rows, ssize_t columns, typename T>
class Matrix {
public:
    Matrix(void) = default;

    Matrix(std::array<std::array<T, columns>, rows> v) : data {v} {};
    Matrix(std::array<std::array<T, columns>, rows> &&v) : data { std::move(v) } {};

    const std::array<T, columns>& operator[](int row) const {
        return data[row];
    }

    std::array<T, columns>& operator[](int row) {
        return data[row];
    }

    int numRows(void) const {
        return rows;
    }

    int numColumns(void) const {
        return columns;
    }

    Matrix<rows, columns, T> & operator*(const Matrix<columns, rows, T>& rhl) {
        Matrix<rows, columns, T> result;

        for (auto row = 0; row < rows; ++row) {
            for (auto col = 0; col < columns; ++col) {
                result[row][col] = 0;

                for (auto i = 0; i < columns; ++i) {
                    result[row][col] += data[row][i] * rhl[i][col];
                }
            }
        }

        *this = std::move(result);

        return *this;
    }

    std::array<T, columns> operator*(const std::array<T, columns>& rhl) {
        std::array<T, columns> result;

        for (auto row = 0; row < rows; ++row) {
            result[row] = 0;

            for (auto i = 0; i < columns; ++i) {
                result[row] += data[row][i] * rhl[i];
            }
        }

        return result;
    }

    operator std::string() {
        std::string text;

        for (auto i = 0; i < numRows(); ++i) {
            for (auto j = 0; j < numColumns(); ++j) {
                text.append(std::to_string(data[i][j]));
                if (j < numColumns() - 1) {
                    text.append(",");
                }
            }
            if (i < numRows() - 1) {
                text.append(";");
            }
        }

        return text;
    }

    operator std::array<T, columns * rows>() {
        std::array<T, columns * rows> output;

        for (auto i = 0; i < numRows(); ++i) {
            for (auto j = 0; j < numColumns(); ++j) {
                output[3 * i + j] = data[i][j];
            }
        }

        return output;
    }

private:
    std::array<std::array<T, columns>, rows> data;
};

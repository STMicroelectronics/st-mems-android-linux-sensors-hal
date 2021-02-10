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

private:
    std::array<std::array<T, columns>, rows> data;
};

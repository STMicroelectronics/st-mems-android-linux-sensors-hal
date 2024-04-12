/*
 * Copyright (C) 2021 STMicroelectronics - All Rights Reserved
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#pragma once

#include <cmath>

#include "Matrix.h"

static constexpr float GRAVITY_EARTH = 9.80665f;

struct Conversion {
    static constexpr float mdegC_to_degC(float mdegC) {
        return mdegC / 1000.0f;
    }

    static constexpr float ns_to_ms(float ns) {
        return ns / 1000000.0f;
    }

    static constexpr float ms2_to_millig(float ms2) {
        return (ms2 * 1000.0f) / GRAVITY_EARTH;
    }

    static constexpr float ms2_to_g(float ms2) {
        return ms2 / GRAVITY_EARTH;
    }

    static constexpr float g_to_ms2(float g) {
        return g * GRAVITY_EARTH;
    }

    static constexpr float millig_to_ms2(float millig) {
        return (millig * GRAVITY_EARTH) / 1000.0f;
    }

    static constexpr float rads_to_degs(float rads) {
        return (rads * 180.0f) / M_PI;
    }

    static constexpr float degs_to_rads(float degs) {
        return (degs * M_PI) / 180.0f;
    }

    static constexpr int uTesla_to_milliG(float uT) {
        return uT * 10.0f;
    }

    static constexpr float milliG_to_uTesla(int mG) {
        return mG / 10.0f;
    }

    static constexpr float G_to_uTesla(float G) {
        return G * 100.0f;
    }

    static constexpr float uTesla_to_G(float uT) {
        return uT / 100.0f;
    }
};

struct MatrixF {
    template <ssize_t rows, ssize_t columns>
    static bool isZerosMatrix(const Matrix<rows, columns, float>& m) {
        for (auto i = 0; i < m.numRows(); ++i) {
            for (auto j = 0; j < m.numColumns(); ++j) {
                if (fabs(m[i][j]) > 1e-9) return false;
            }
        }
        return true;
    }

    template <ssize_t rows, ssize_t columns>
    static bool isIdentityMatrix(const Matrix<rows, columns, float>& m) {
        for (auto i = 0; i < m.numRows(); ++i) {
            for (auto j = 0; j < m.numColumns(); ++j) {
                if (i == j) {
                    if ((fabs(m[i][j]) - 1.0f) > 1e-9) return false;
                } else {
                    if (fabs(m[i][j]) > 1e-9) return false;
                }
            }
        }
        return true;
    }
};

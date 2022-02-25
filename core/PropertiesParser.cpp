/*
 * Copyright (C) 2018 The Android Open Source Project
 * Copyright (C) 2022 STMicroelectronics
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

#include <regex>

#include "PropertiesParser.h"

namespace stm {
namespace core {

PropertiesParser::PropertiesParser(const std::string& text, int numElements)
    : valid(false)
{
    const std::regex digitsRegex("[+-]?([0-9]*[.])?[0-9]+\\,*");

    auto digitsBegin = std::sregex_iterator(text.begin(),
                                            text.end(),
                                            digitsRegex);

    auto digitsEnd = std::sregex_iterator();

    for (std::sregex_iterator it = digitsBegin; it != digitsEnd; ++it) {
        std::smatch match = *it;
        std::size_t pos = match.str().find(',');

        if (pos != std::string::npos) {
            data.push_back(std::stof(match.str().substr(0, pos)));
        } else {
            data.push_back(std::stof(match.str()));
        }
    }

    if (data.size() == (uint)numElements) {
        valid = true;
    }
}

PropertiesParser PropertiesParser::makeRotationMatrixParser(std::string text)
{
    PropertiesParser parser(text, 9);

    return parser;
}

PropertiesParser PropertiesParser::makeSensorPlacementParser(std::string text)
{
    PropertiesParser parser(text, 3);

    return parser;
}

bool PropertiesParser::isValid() const
{
    return valid;
}

const std::vector<float>& PropertiesParser::getData() const
{
    return data;
}

} // namespace core
} // namespace stm

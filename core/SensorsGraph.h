/*
 * Copyright (C) 2018 The Android Open Source Project
 * Copyright (C) 2020 STMicroelectronics
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

#include <vector>
#include <string>

template<class T>
class Graph {
public:
    Graph() : ids(0) {};

    int addNode(T *data);
    void removeNode(int id);

    int addEdge(int nodeIdT, int nodeIdH);
    void removeEdge(int nodeIdT, int nodeIdH);

    std::string to_string(void) const;

    class Node {
    public:
        std::vector<std::pair<char, int>> edges;
        bool valid;
        T *payload;
        int id;

        Node(int id, T *data) : valid(true), payload(data), id(id) {};

        Node(Node &&n) = default;

        Node& operator=(Node &&n) {
            if (&n == this) {
                return *this;
            }

            payload = n.payload;
            id = n.id;
            edges = n.edges;

            return *this;
        };

        void invalidate() { valid = false; };
    };

    class iterator : public std::iterator<std::output_iterator_tag, Node> {
    public:
        explicit iterator(Graph<T> &graph, int index): ptr(graph), index(index) {}
        Node& operator*() const { return ptr.adjList[index]; };
        iterator& operator++() { index++; return *this; };
        bool operator!=(const iterator &rhs) const { return index != rhs.index; };

    private:
        Graph<T> &ptr;
        int index;
    };

    iterator begin() { return iterator(*this, 0); }
    iterator end() { return iterator(*this, adjList.size()); }

private:
    int ids;

    std::vector<Node> adjList;

    int getAdjListIndex(int nodeId) const;

    bool edgeExist(Node &nodeIdT, Node &nodeIdH) const;
};

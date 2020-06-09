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

#include <sstream>
#include <algorithm>

#include "SensorsGraph.h"
#include "SensorBase.h"

template<class T>
int Graph<T>::addNode(T *data)
{
    int id = ids++;

    adjList.push_back(Node(id, data));

    return id;
}

template<class T>
void Graph<T>::removeNode(int id)
{
    int nodeIndex = getAdjListIndex(id);

    if (nodeIndex < 0) {
        return;
    }

    for (auto edge : adjList[nodeIndex].edges) {
        int nodeH, nodeT;

        if (edge.first == 'a') {
            nodeH = id;
            nodeT = edge.second;
        } else {
            nodeH = edge.second;
            nodeT = id;
        }

        removeEdge(nodeT, nodeH);
    }

    adjList.erase(std::remove_if(adjList.begin(), adjList.end(),
                                 [id] (const Node &node) {
                                     return node.id == id; }),
                  adjList.end());
}

template<class T>
int Graph<T>::addEdge(int nodeIdT, int nodeIdH)
{
    int nodeTIndex = getAdjListIndex(nodeIdT);
    int nodeHIndex = getAdjListIndex(nodeIdH);

    if ((nodeTIndex < 0) || (nodeHIndex < 0) ||
        edgeExist(adjList[nodeTIndex], adjList[nodeHIndex])) {
        return -EINVAL;
    }

    auto edgeL = std::pair<char, int>('l', nodeIdH);
    auto edgeA = std::pair<char, int>('a', nodeIdT);

    adjList[nodeTIndex].edges.push_back(edgeL);
    adjList[nodeHIndex].edges.push_back(edgeA);

    return 0;
}

template<class T>
void Graph<T>::removeEdge(int nodeIdT, int nodeIdH)
{
    int nodeTIndex = getAdjListIndex(nodeIdT);
    int nodeHIndex = getAdjListIndex(nodeIdH);

    if ((nodeTIndex < 0) || (nodeHIndex < 0)) {
        return;
    }

    auto edgeL = std::pair<char, int>('l', nodeIdH);
    auto edgeA = std::pair<char, int>('a', nodeIdT);

    adjList[nodeTIndex].edges.erase(std::remove(adjList[nodeTIndex].edges.begin(),
                                                adjList[nodeTIndex].edges.end(),
                                                edgeL),
                                    adjList[nodeTIndex].edges.end());

    adjList[nodeHIndex].edges.erase(std::remove(adjList[nodeHIndex].edges.begin(),
                                                adjList[nodeHIndex].edges.end(),
                                                edgeA),
                                    adjList[nodeHIndex].edges.end());
}

template<class T>
std::string Graph<T>::to_string(void) const
{
    std::ostringstream oss;

    for (auto &node : adjList) {
        oss << node.id << ": " << node.payload->GetName();
        for (auto &edge : node.edges) {
            oss << " " << edge.first << "," << edge.second;
        }
        oss << "\n";
    }

    return oss.str();
}

template<class T>
int Graph<T>::getAdjListIndex(int nodeId) const
{
    int index = -1;

    for (auto &node : adjList) {
        index++;
        if (node.id == nodeId) {
            return index;
        }
    }

    return -EINVAL;
}

template<class T>
bool Graph<T>::edgeExist(Node &nodeIdT, Node &nodeIdH) const
{
    auto edgeL = std::pair<char, int>('l', nodeIdH.id);
    auto edgeA = std::pair<char, int>('a', nodeIdT.id);

    for (auto &edge : nodeIdT.edges) {
        if (edge == edgeL) {
            return true;
        }
        if (edge == edgeA) {
            return true;
        }
    }

    return false;
}

template class Graph<stm::core::SensorBase>;

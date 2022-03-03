/*
 * Copyright (C) 2018 The Android Open Source Project
 * Copyright (C) 2020-2022 STMicroelectronics
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

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <queue>

namespace stm {
namespace core {

template<typename T>
class Graph {
public:
    Graph() = default;
    ~Graph() = default;

    Graph(const Graph& rhl) = delete;
    Graph(Graph&& rhl) = delete;
    Graph& operator=(const Graph& rhl) = delete;
    Graph& operator=(Graph&& rhl) = delete;

    bool addNode(int id, std::shared_ptr<T> data) {
        return adjList.emplace(id, Node(id, data)).second;
    }

    void removeNode(int id) {
        auto itr = adjList.find(id);
        if (itr == adjList.end()) {
            return;
        }

        for (auto& leavingNodeId : itr->second.leavingEdges) {
            auto itrLeavingNode = adjList.find(leavingNodeId);
            if (itrLeavingNode != adjList.end()) {
                itrLeavingNode->second.arrivingEdges.erase(id);
            }
        }
        for (auto& arrivingNodeId : itr->second.arrivingEdges) {
            auto itrArrivingNode = adjList.find(arrivingNodeId);
            if (itrArrivingNode != adjList.end()) {
                itrArrivingNode->second.leavingEdges.erase(id);
            }
        }

        adjList.erase(itr);
    }

    void removeNodeAnd(int id) {
        auto itr = adjList.find(id);
        if (itr == adjList.end()) {
            return;
        }

        std::unordered_set<int> nodesVisited;
        std::queue<int> nodesToVisit;
        nodesToVisit.push(id);

        while (!nodesToVisit.empty()) {
            auto nodeId = nodesToVisit.front();
            nodesToVisit.pop();
            nodesVisited.insert(nodeId);

            auto itrNodeToVisit = adjList.find(nodeId);
            for (auto& arrivingNodeId : itrNodeToVisit->second.arrivingEdges) {
                bool visited = nodesVisited.count(arrivingNodeId);
                if (!visited) {
                    nodesToVisit.push(arrivingNodeId);
                }
            }
        }

        for (auto& nodeIdToRemove : nodesVisited) {
            removeNode(nodeIdToRemove);
        }
    }

    bool addEdge(int idTailNode, int idHeadNode) {
        if (idTailNode == idHeadNode) {
            return false;
        }

        auto itrTailNode = adjList.find(idTailNode);
        auto itrHeadNode = adjList.find(idHeadNode);
        if (itrTailNode != adjList.end() && itrHeadNode != adjList.end()) {
            itrTailNode->second.leavingEdges.insert(idHeadNode);
            itrHeadNode->second.arrivingEdges.insert(idTailNode);
            return true;
        }

        return false;
    }

    void removeEdge(int idTailNode, int idHeadNode) {
        auto itrTailNode = adjList.find(idTailNode);
        auto itrHeadNode = adjList.find(idHeadNode);

        if (itrTailNode != adjList.end() && itrHeadNode != adjList.end()) {
            itrTailNode->second.leavingEdges.erase(idHeadNode);
            itrHeadNode->second.arrivingEdges.erase(idTailNode);
        }
    }

    bool empty() const {
        return adjList.empty();
    }

    bool edgeExist(int idTailNode, int idHeadNode) const {
        auto itr = adjList.find(idTailNode);
        if (itr == adjList.end()) {
            return false;
        }

        for (auto& leavingNodeId : itr->second.leavingEdges) {
            if (leavingNodeId == idHeadNode) {
                return true;
            }
        }

        return false;
    }

    std::vector<int> getArrivingNodesId(int id) {
        std::vector<int> arrivingNodesId;

        auto itr = adjList.find(id);
        if (itr == adjList.end()) {
            return arrivingNodesId;
        }

        for (auto& id : itr->second.arrivingEdges) {
            arrivingNodesId.push_back(id);
        }

        return arrivingNodesId;
    }

    std::vector<int> getTopologicalSortReverse() {
        std::vector<int> nodesIdSorted;
        std::queue<int> results;

        std::unordered_map<int, bool> nodesVisited;
        for (auto& node : adjList) {
            nodesVisited.insert({ node.first, false });
        }
        for (auto node : nodesVisited) {
            if (!node.second) {
                topologicalSortHelper(results, nodesVisited, node.first);
            }
        }
        while (!results.empty()) {
            nodesIdSorted.push_back(results.front());
            results.pop();
        }

        return nodesIdSorted;
    }

    const std::shared_ptr<T> operator[](int id) {
        auto itr = adjList.find(id);
        if (itr == adjList.end()) {
            return { };
        }

        return itr->second.payload;
    }

    operator std::string() const {
        std::string msg;

        for (auto& node : adjList) {
            msg += "node " + std::to_string(node.first) + " -- " + std::to_string(node.second.id) + ": ";
            msg += "leaving nodes=";
            for (auto& leavingNode : node.second.leavingEdges) {
                msg += " " + std::to_string(leavingNode);
            }
            msg += "; arriving nodes=";
            for (auto& arrivingNode : node.second.arrivingEdges) {
                msg += " " + std::to_string(arrivingNode);
            }
            msg += "\n";
        }

        return msg;
    }

    class Node {
    public:
        std::unordered_set<int> arrivingEdges;
        std::unordered_set<int> leavingEdges;
        std::shared_ptr<T> payload;
        int id;

        Node(int id, std::shared_ptr<T> data)
            : payload(data),
              id(id) { };

        Node(Node&& n)
            : arrivingEdges(std::move(n.arrivingEdges)),
              leavingEdges(std::move(n.leavingEdges)),
              payload(n.payload),
              id(n.id) {
            n.payload = nullptr;
        }

        Node& operator=(const Node& n) = delete;

        Node& operator=(Node&& n) {
            if (&n == this) {
                return *this;
            }

            payload = n.payload;
            n.payload = nullptr;

            id = n.id;
            leavingEdges = std::move(n.leavingEdges);
            arrivingEdges = std::move(n.arrivingEdges);

            return *this;
        };
    };

    auto begin() { return adjList.begin(); }

    auto end() { return adjList.end(); }

    auto cbegin() const { return adjList.cbegin(); }

    auto cend() const { return adjList.cend(); }

private:
    std::unordered_map<int, Node> adjList;

    void topologicalSortHelper(std::queue<int>& results,
                               std::unordered_map<int, bool>& nodesVisited,
                               int nodeId) {
        nodesVisited[nodeId] = true;

        auto itr = adjList.find(nodeId);
        for (auto& leavingNodeId : itr->second.leavingEdges) {
            if (!nodesVisited[leavingNodeId]) {
                topologicalSortHelper(results, nodesVisited, leavingNodeId);
            }
        }

        results.push(nodeId);
    }
};

} // namespace core
} // namespace stm
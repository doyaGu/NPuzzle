#include <iostream>

#include "Board.h"
#include "GraphSearch.h"

using board::Board;
using search::Node;
using search::Result;

template<std::uint8_t N>
inline Result boardBFS(const Board<N> &start, const Board<N> &target)
{
    return search::bfs<Board<N>>(start, target);
}

template<std::uint8_t N>
inline Result boardDFS(const Board<N> &start, const Board<N> &target, std::size_t max_depth)
{
    return search::dfs<Board<N>>(start, target, max_depth);
}

template<std::uint8_t N>
inline Result boardBestFS(const Board<N> &start, const Board<N> &target)
{
    return search::bestFS<Board<N>>(start, target, [target](const Node<Board<N>> &node) {
        return node.getDepth() + target.compatibilityCalculate(node.get());
    });
}

template<std::uint8_t N>
inline Result boardAStar(const Board<N> &start, const Board<N> &target)
{
    return search::aStar<Board<N>>(start, target,
                                   [](const Node<Board<N>> &node) {
                                       return node.getDepth();
                                   },
                                   [target](const Node<Board<N>> &node) {
                                       return target.similarityCalculate(node.get());
                                   });
}

int main() {
    //    Board<3> dfs_sample = {2, 8, 3, 1, 6, 4, 7, 0, 5};
    //    Board<3> bfs_sample = {2, 8, 3, 1, 0, 4, 7, 6, 5};
    //    Board<3> target = {1, 2, 3, 0, 8, 4, 7, 6, 5};

    Board<4> start = {1, 2, 3, 4, 5, 10, 6, 8, 0, 9, 7, 12, 13, 14, 11, 15};
    Board<4> target = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0};

    std::cout << "Please input the start board:" << std::endl;
    std::cin >> start;
    std::cout << "Please input the target board:" << std::endl;
    std::cin >> target;

    std::cout << "The search method implemented: " << std::endl;
    std::cout << "1. Breadth First Search" << std::endl;
    std::cout << "2. Depth First Search" << std::endl;
    std::cout << "3. Best First Search" << std::endl;
    std::cout << "4. A* Search" << std::endl;
    std::cout << "Please select the search method [1-4]: ";

    int option;
    std::cin >> option;
    Result result;
    switch (option) {
        case 1:
            result = boardBFS(start, target);
            break;
        case 2:
        {
            std::size_t depth;
            std::cout << "Please input the max depth: ";
            std::cin >> depth;
            result = boardDFS(start, target, depth);
        }
            break;
        case 3:
            result = boardBestFS(start, target);
            break;
        case 4:
            result = boardAStar(start, target);
            break;
        default:
            std::cout << "Error: Unsupported option!" << std::endl;
            break;
    }

    std::cout << "Total Steps: " << result.steps() << std::endl;
    if (result.success())
        std::cout << "Success.";
    else
        std::cout << "Failed.";

    return 0;
}

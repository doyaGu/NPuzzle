#ifndef NPUZZLE_GRAPHSEARCH_H
#define NPUZZLE_GRAPHSEARCH_H

#include <memory>
#include <vector>
#include <queue>
#include <stack>
#include <forward_list>

#include "Node.h"

namespace search {
    namespace impl {
        template<typename E>
        using Filter = std::function<bool(const NodePtr <E> &)>;

        template<typename E>
        inline bool isNotSameWithAncestors(const NodePtr <E> &node) {
            for (auto parent = node->getParent(); parent != nullptr; parent = parent->getParent())
                if (*node == *parent)
                    return false;
            return true;
        }

        template<typename E>
        std::vector<NodePtr<E>> expand(const NodePtr <E> &node, Filter<E> filter) {
            auto children = node->expand();
            std::vector<NodePtr<E>> result;
            result.reserve(4);
            for (auto &child : children) {
                child->setParent(node);
                child->setDepth(node->getDepth() + 1);
                if (filter(child)) {
                    result.push_back(std::move(child));
                }
            }
            return result;
        }

        template<typename E>
        inline bool check(const NodePtr <E> &node, const NodePtr <E> &target) {
            return *node == *target;
        }

        template<typename E>
        inline void log(std::int64_t step, const NodePtr <E> &node) {
            std::cout << "step " << step << std::endl;
            std::cout << "depth " << node->getDepth() << std::endl;
            std::cout << node->get() << std::endl;
        }
    }

    class Result {
    public:
        enum results {
            SUCCESS = 0,
            FAILED = 1
        };

        Result() : result_(FAILED), steps_(0) {}

        Result(results result, std::int64_t steps) : result_(result), steps_(steps) {}

        bool success() const {
            return result_ == SUCCESS;
        }

        std::int64_t steps() const {
            return steps_;
        }

    private:
        std::uint64_t result_: 1;
        std::uint64_t steps_: 63;
    };

    template<typename E>
    using Evaluator = std::function<int(const Node <E> &)>;

    template<typename E>
    Result bfs(const E &start, const E &target) {
        if (start == target) {
            return {Result::SUCCESS, 0};
        }

        auto ps = std::make_shared<Node<E>>(start);
        auto pt = std::make_shared<Node<E>>(target);
        std::queue<NodePtr<E>> open;
        std::vector<NodePtr<E>> closed;
        std::int64_t steps = 0;

        ++steps;
        impl::log(steps, ps);
        if (impl::check(ps, pt)) {
            return {Result::SUCCESS, steps};
        }

        open.push(ps);
        while (!open.empty()) {
            closed.push_back(open.front());
            open.pop();

            auto pn = closed.back();
            auto children = impl::expand(pn, impl::isNotSameWithAncestors<E>);
            for (auto &child : children) {
                ++steps;
                impl::log(steps, child);
                if (impl::check(child, pt)) {
                    return {Result::SUCCESS, steps};
                }
                open.push(child);
            }

        }

        return {Result::FAILED, steps};
    }

    template<typename E>
    Result dfs(const E &start, const E &target, std::size_t max_depth) {
        if (start == target) {
            return {Result::SUCCESS, 0};
        }

        auto ps = std::make_shared<Node<E>>(start);
        auto pt = std::make_shared<Node<E>>(target);
        std::stack<NodePtr<E>> open;
        std::vector<NodePtr<E>> closed;
        std::int64_t steps = 0;

        open.push(ps);
        while (!open.empty()) {
            closed.push_back(open.top());
            open.pop();

            auto pn = closed.back();

            ++steps;
            impl::log(steps, pn);
            if (impl::check(pn, pt)) {
                return {Result::SUCCESS, steps};
            }

            if (pn->getDepth() < max_depth) {
                auto children = impl::expand(pn, impl::isNotSameWithAncestors<E>);
                for (auto iter = children.rbegin(); iter != children.rend(); ++iter) {
                    if ((*iter)->getDepth() != max_depth) {
                        open.push(*iter);
                    } else {
                        impl::log(steps, *iter);
                        if (impl::check(*iter, pt)) {
                            return {Result::SUCCESS, steps};
                        }
                    }
                }
            }
        }

        return {Result::FAILED, steps};
    }

    template<typename E>
    Result bestFS(const E &start, const E &target, Evaluator<E> evaluator) {
        if (start == target) {
            return {Result::SUCCESS, 0};
        }

        auto ps = std::make_shared<Node<E>>(start);
        auto pt = std::make_shared<Node<E>>(target);
        std::forward_list<NodePtr<E>> open;
        std::forward_list<NodePtr<E>> closed;
        std::int64_t steps = 0;

        open.push_front(ps);
        while (!open.empty()) {
            open.sort([](const NodePtr<E> &lhs, const NodePtr<E> &rhs) {
                return lhs->getCost() < rhs->getCost();
            });
            closed.push_front(open.front());
            open.pop_front();

            auto pn = closed.front();
            auto depth = pn->getDepth();

            ++steps;
            impl::log(steps, pn);

            if (impl::check(pn, pt)) {
                return {Result::SUCCESS, steps};;
            }

            auto children = impl::expand(pn, impl::isNotSameWithAncestors<E>);
            for (auto &child : children) {
                auto cost = evaluator(*child);
                auto iter = find(open.begin(), open.end(), child);
                if (iter != open.end() && cost < (*iter)->getCost()) {
                    (*iter)->setParent(pn);
                    (*iter)->setCost(cost);
                } else if ((iter = find(closed.begin(), closed.end(), child)) != closed.end() &&
                           cost < (*iter)->getCost()) {
                    (*iter)->setParent(pn);
                    (*iter)->setCost(cost);
                    open.push_front(*iter);
                    closed.remove(*iter);
                } else {
                    child->setCost(cost);
                    open.push_front(child);
                }
            }
        }
        return {Result::FAILED, steps};
    }

    template<typename E>
    Result aStar(const E &start, const E &target, Evaluator<E> g, Evaluator<E> h) {
        if (start == target) {
            return {Result::SUCCESS, 0};
        }

        auto ps = std::make_shared<Node<E>>(start);
        auto pt = std::make_shared<Node<E>>(target);
        std::forward_list<NodePtr<E>> open;
        std::forward_list<NodePtr<E>> closed;
        std::int64_t steps = 0;

        open.push_front(ps);
        while (!open.empty()) {
            open.sort([](const NodePtr<E> &lhs, const NodePtr<E> &rhs) {
                return lhs->getCost() < rhs->getCost();
            });
            closed.push_front(open.front());
            open.pop_front();

            auto pbn = closed.front();

            ++steps;
            impl::log(steps, pbn);

            if (impl::check(pbn, pt)) {
                return {Result::SUCCESS, steps};
            }

            auto children = impl::expand(pbn, impl::isNotSameWithAncestors<E>);
            for (auto it = children.begin(); it != children.end(); ++it) {
                auto &child = *it;
                auto hv = h(*child);
                auto iter = find(open.begin(), open.end(), child);
                if (iter != open.end() && g(*child) < g(*(*iter))) {
                    auto &old = *iter;
                    old->setParent(pbn);
                    old->setCost(g(*child) + hv);
                } else if ((iter = find(closed.begin(), closed.end(), child)) != closed.end() &&
                        g(*child) < g(*(*iter))) {
                    auto &old = *iter;
                    old->setParent(pbn);
                    old->setCost(g(*child) + hv);
                    open.push_front(old);
                    closed.remove(old);
                } else {
                    child->setCost(g(*child) + hv);
                    open.push_front(child);
                }
            }
        }

        return {Result::FAILED, steps};
    }
}
#endif //NPUZZLE_GRAPHSEARCH_H

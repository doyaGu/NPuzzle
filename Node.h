#ifndef NPUZZLE_NODE_H
#define NPUZZLE_NODE_H

#include <algorithm>
#include <memory>
#include <functional>
#include <vector>

namespace search {
    template<typename E>
    class Node;

    template<typename E>
    using NodePtr = std::shared_ptr<Node<E>>;

    template<typename E>
    class Node {
    public:
        explicit Node(const E &elem) : elem_(std::make_unique<E>(elem)){}

        explicit Node(std::unique_ptr<E> &&elem) : elem_(std::move(elem)) {}

        Node(const Node &rhs) : elem_(std::make_unique<E>(rhs.elem)), parent_(rhs.parent_) {}

        Node(Node &&rhs) noexcept: elem_(std::move(rhs.elem)), parent_(rhs.parent_) {}

        Node &operator=(const Node &rhs) {
            if (this != &rhs) {
                *elem_ = *rhs.elem_;
                parent_ = rhs.parent_;
            }
            return *this;
        }

        Node &operator=(Node &&rhs) noexcept {
            if (this != &rhs) {
                elem_ = std::move(rhs.elem_);
                parent_ = rhs.parent_;
            }
            return *this;
        };

        ~Node() = default;

        bool operator==(const Node &rhs) const noexcept {
            return *elem_ == *rhs.elem_;
        }

        bool operator!=(const Node &rhs) const noexcept {
            return *elem_ != *rhs.elem_;
        }

        void setParent(const NodePtr<E> &parent) {
            parent_ = parent;
        }

        NodePtr<E> getParent() {
            auto parent = parent_.lock();
            if (!parent) {
                parent_.reset();
            }
            return parent;
        }

        void setDepth(int depth) {
            depth_ = depth;
        }

        int getDepth() const {
            return depth_;
        }

        void setCost(int cost) {
            cost_ = cost;
        }

        int getCost() const {
            return cost_;
        }

        E &get() const {
            return *elem_;
        }

        std::vector<NodePtr<E>> expand();

    private:
        std::unique_ptr<E> elem_;
        std::weak_ptr<Node> parent_{};
        int depth_ = 0;
        int cost_ = 0;
    };

    template<typename E>
    std::vector<NodePtr<E>> Node<E>::expand() {
        auto elements = this->elem_->expand();
        std::vector<NodePtr<E>> children;
        children.reserve(4);
        for (auto &elem : elements) {
            auto node = std::make_shared<Node>(std::move(elem));
            children.push_back(std::move(node));
        }
        return children;
    }

    template<typename InputIt, typename E>
    inline InputIt find(InputIt first, InputIt last, const NodePtr<E> &target) {
        return std::find_if(first, last, [target](const NodePtr<E> &node) {
            return *node == *target;
        });
    }
}
#endif //NPUZZLE_NODE_H

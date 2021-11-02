#ifndef NPUZZLE_BOARD_H
#define NPUZZLE_BOARD_H

#include <cstdint>
#include <array>
#include <iostream>
#include <memory>
#include <cstdlib>
#include <vector>

namespace board {
    enum class Move : std::uint8_t {
        LEFT, UP, RIGHT, DOWN, IDLE
    };

    Move &operator++(Move &d) {
        switch (d) {
            case Move::LEFT:
                return d = Move::UP;
            case Move::UP:
                return d = Move::RIGHT;
            case Move::RIGHT:
                return d = Move::DOWN;
            case Move::DOWN:
                return d = Move::IDLE;
            case Move::IDLE:
                return d = Move::LEFT;
        }
        return d;
    }

    template<std::uint8_t N>
    class Board;

    template<std::uint8_t N>
    using BoardPtr = std::unique_ptr<Board<N>>;

    template<std::uint8_t N>
    class Board {
    public:
        using Piece = std::uint8_t;

        static constexpr int SIZE = N * N;

        Board(std::initializer_list<Piece> il);

        Board(const Board &rhs) = default;

        Board(Board &&rhs) noexcept = default;

        Board &operator=(const Board &rhs) = default;

        Board &operator=(Board &&rhs) noexcept = default;

        ~Board() = default;

        bool operator==(const Board &rhs) const {
            return hashcode_ == rhs.hashcode_ &&
                   std::equal(std::cbegin(grid_), std::cend(grid_), std::cbegin(rhs.grid_), std::cend(rhs.grid_));
        }

        bool operator!=(const Board &rhs) const {
            return !(rhs == *this);
        }

        Piece &operator[](std::size_t n) {
            return grid_[n];
        }

        const Piece &operator[](std::size_t n) const {
            return grid_[n];
        }

        bool moveBlank(Move direction) noexcept;

        std::vector<BoardPtr<N>> expand();

        int compatibilityCalculate(const Board &board) const;

        int similarityCalculate(const Board &board) const;

    private:
        int locate(Piece piece) const noexcept {
            int i;
            for (i = 0; grid_[i] != piece && i < SIZE; ++i)
                continue;
            return i;
        }

        // BKDR hash
        static std::size_t hash(const Board &board) noexcept;

        std::array<Piece, SIZE> grid_;
        int blank_index_;
        std::size_t hashcode_;
    };

    template<std::uint8_t N>
    std::istream &operator>>(std::istream &is, Board<N> &board) {
        for (int i = 0; i < Board<N>::SIZE; ++i)
            is >> board[i];
        return is;
    }

    template<std::uint8_t N>
    std::ostream &operator<<(std::ostream &os, const Board<N> &board) {
        for (int i = 0; i < Board<N>::SIZE;) {
            os << std::to_string(board[i]) << '\t';
            if ((++i) % N == 0)
                os << '\n';
        }
        return os;
    }

    template<std::uint8_t N>
    Board<N>::Board(std::initializer_list<Piece> il) {
        std::copy(il.begin(), il.end(), std::begin(grid_));
        blank_index_ = locate(0);
        hashcode_ = hash(*this);
    }

    template<std::uint8_t N>
    bool Board<N>::moveBlank(Move direction) noexcept {
        using std::swap;
        int next = blank_index_;
        switch (direction) {
            case Move::LEFT:
                if ((blank_index_ % N) != 0) {
                    next = blank_index_ - 1;
                    swap(grid_[blank_index_], grid_[next]);
                }
                break;
            case Move::UP:
                if (blank_index_ >= N) {
                    next = blank_index_ - N;
                    swap(grid_[blank_index_], grid_[next]);
                }
                break;
            case Move::RIGHT:
                if ((blank_index_ % N) != N - 1) {
                    next = blank_index_ + 1;
                    swap(grid_[blank_index_], grid_[next]);
                }
                break;
            case Move::DOWN:
                if (blank_index_ < SIZE - N) {
                    next = blank_index_ + N;
                    swap(grid_[blank_index_], grid_[next]);
                }
                break;
            default:
                break;
        }

        if (blank_index_ != next) {
            blank_index_ = next;
            hashcode_ = hash(*this);
            return true;
        }
        return false;
    }

    template<std::uint8_t N>
    std::vector<BoardPtr<N>> Board<N>::expand() {
        std::vector<BoardPtr<N>> children;
        children.reserve(4);
        for (auto next = Move::LEFT; next != Move::IDLE; ++next) {
            auto child = std::make_unique<Board<N>>(*this);
            if (child->moveBlank(next)) {
                children.push_back(std::move(child));
            }
        }
        return children;
    }

    template<std::uint8_t N>
    int Board<N>::compatibilityCalculate(const Board &board) const {
        int c = 0;
        for (auto i = 0; i < SIZE; ++i) {
            if (grid_[i] != board.grid_[i]) {
                ++c;
            }
        }
        return c;
    }

    template<std::uint8_t N>
    int Board<N>::similarityCalculate(const Board &board) const {
        // Manhattan distance
        int c = 0;
        for (auto i = 0; i < SIZE; ++i) {
            auto j = board.locate(grid_[i]);
            c += std::abs(static_cast<int>(i % N - j % N)) + std::abs(static_cast<int>(i / N - j / N));
        }
        return c;
    }

    template<std::uint8_t N>
    std::size_t Board<N>::hash(const Board &board) noexcept {
        constexpr auto SEED = 131;      // the magic number, 31, 131, 1313, 13131, etc..
        constexpr auto LENGTH = 0x7FFFFFFF;  // hash 表长度
        std::size_t hash{0};
        for (const auto i : board.grid_)
            hash = hash * SEED + i;
        return hash % LENGTH;
    }
}

#endif //NPUZZLE_BOARD_H

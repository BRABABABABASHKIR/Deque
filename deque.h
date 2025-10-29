#pragma once

#include <initializer_list>
#include <algorithm>
#include <memory>

const size_t kSzBlock = 512 / sizeof(int);

class Deque {
public:
    class Block {
    public:
        Block() {
            block_ = nullptr;
            left_ = 0;
            right_ = 0;
        }
        ~Block() = default;

        void MakeBlock() {
            block_ = std::make_unique<int[]>(kSzBlock);
        }

        void SetLeft(size_t new_left) {
            left_ = new_left;
        }
        void SetRight(size_t new_right) {
            right_ = new_right;
        }
        size_t GetLeft() {
            return left_;
        }
        size_t GetRight() {
            return right_;
        }

        void PushBack(int num) {
            if (block_ == nullptr) {
                MakeBlock();
            }
            if (left_ == right_) {
                left_ = 0;
                right_ = 0;
            }
            block_[right_] = num;
            ++right_;
        }
        void PushFront(int num) {
            if (block_ == nullptr) {
                MakeBlock();
            }
            if (left_ == right_) {
                left_ = kSzBlock;
                right_ = kSzBlock;
            }
            --left_;
            block_[left_] = num;
        }

        int GetNum(size_t ind) {
            return block_[left_ + ind];
        }
        int* GetNumPtr(size_t ind) {
            return &block_[left_ + ind];
        }

        void SetNum(size_t ind, int num) {
            block_[left_ + ind] = num;
        }

    private:
        std::unique_ptr<int[]> block_ = nullptr;
        size_t left_ = 0, right_ = 0;
    };

    Deque() = default;
    Deque(const Deque& rhs) {
        size_t sz = rhs.Size();
        *this = Deque(sz);
        size_t ind_block = 0, ind_elem = 0;
        for (size_t i = begin_; i < begin_ + rhs.size_blocks_; ++i) {
            size_t cur_i = i % rhs.capacity_blocks_;
            for (size_t j = rhs.blocks_[cur_i]->GetLeft(); j < rhs.blocks_[cur_i]->GetRight();
                 j++) {
                int num = rhs.blocks_[cur_i]->GetNum(j);
                blocks_[ind_block]->SetNum(ind_elem++, num);
                if (ind_elem == kSzBlock) {
                    ind_elem = 0;
                    ++ind_block;
                }
            }
        }
    }
    Deque(Deque&& rhs) {
        this->Swap(rhs);
    }
    explicit Deque(size_t size) {
        if (size == 0) {
            *this = Deque();
            return;
        }
        begin_ = 0;
        size_deque_ = size;
        size_blocks_ = (size + kSzBlock - 1) / kSzBlock;
        capacity_blocks_ = size_blocks_;
        blocks_ = std::make_unique<Block*[]>(capacity_blocks_);
        for (size_t i = 0; i < capacity_blocks_; ++i) {
            blocks_[i] = new Block();
        }
        for (size_t i = 0; i < size_blocks_; i++) {
            blocks_[i]->MakeBlock();
            blocks_[i]->SetRight(kSzBlock);
        }
        size_t last_right = size % kSzBlock;
        if (last_right == 0) {
            last_right = kSzBlock;
        }
        blocks_[size_blocks_ - 1]->SetRight(last_right);
    }

    Deque(std::initializer_list<int> list) {
        size_t sz = list.size();
        *this = Deque(sz);
        size_t ind_block = 0, ind_elem = 0;
        for (int num : list) {
            blocks_[ind_block]->SetNum(ind_elem++, num);
            if (ind_elem == kSzBlock) {
                ind_elem = 0;
                ++ind_block;
            }
        }
    }

    Deque& operator=(Deque rhs) {
        Swap(rhs);
        return *this;
    }

    void Swap(Deque& rhs) {
        std::swap(blocks_, rhs.blocks_);
        std::swap(begin_, rhs.begin_);
        std::swap(size_blocks_, rhs.size_blocks_);
        std::swap(capacity_blocks_, rhs.capacity_blocks_);
        std::swap(size_deque_, rhs.size_deque_);
    }

    void Relocate() {
        size_t new_capacity = capacity_blocks_ * 2;
        std::unique_ptr<Block*[]> new_blocks = std::make_unique<Block*[]>(capacity_blocks_ * 2);
        for (size_t i = 0; i < size_blocks_; ++i) {
            size_t cur_i = (begin_ + i) % capacity_blocks_;
            new_blocks[i] = blocks_[cur_i];
        }

        blocks_ = std::move(new_blocks);
        begin_ = 0;
        capacity_blocks_ = new_capacity;
    }

    bool TryPushBack(int value) {
        size_t cur_block = (begin_ + size_blocks_ - 1) % capacity_blocks_;
        size_t cur_right = blocks_[cur_block]->GetRight();

        if (cur_right != kSzBlock) {
            blocks_[cur_block]->PushBack(value);
            return true;
        }

        if (size_blocks_ < capacity_blocks_) {
            size_blocks_++;
            cur_block = (begin_ + size_blocks_ - 1) % capacity_blocks_;
            if (blocks_[cur_block] == nullptr) {
                blocks_[cur_block] = new Block();
                blocks_[cur_block]->MakeBlock();
            }
            blocks_[cur_block]->PushBack(value);
            return true;
        }

        return false;
    }

    void PushBack(int value) {
        if (blocks_ == nullptr) {
            *this = Deque{value};
            return;
        }

        ++size_deque_;

        if (size_blocks_ == 0) {
            if (blocks_[begin_] == nullptr) {
                blocks_[begin_] = new Block();
                blocks_[begin_]->MakeBlock();
            }
            blocks_[begin_]->PushBack(value);
            ++size_blocks_;
            return;
        }

        if (TryPushBack(value)) {
            return;
        }

        Relocate();

        TryPushBack(value);
    }

    void PopBack() {
        size_t cur_block = (begin_ + size_blocks_ - 1) % capacity_blocks_;
        size_t cur_left = blocks_[cur_block]->GetLeft();
        size_t cur_right = blocks_[cur_block]->GetRight();

        --cur_right;
        blocks_[cur_block]->SetRight(cur_right);
        --size_deque_;

        if (cur_left == cur_right) {
            --size_blocks_;
            delete blocks_[cur_block];
            blocks_[cur_block] = nullptr;
        }
    }

    bool TryPushFront(int value) {
        size_t cur_block = begin_;
        size_t cur_left = blocks_[cur_block]->GetLeft();

        if (cur_left != 0) {
            blocks_[cur_block]->PushFront(value);
            return true;
        }

        if (size_blocks_ < capacity_blocks_) {
            size_blocks_++;
            if (begin_ == 0) {
                begin_ = capacity_blocks_ - 1;
            } else {
                --begin_;
            }
            cur_block = begin_;
            if (blocks_[cur_block] == nullptr) {
                blocks_[cur_block] = new Block();
                blocks_[cur_block]->MakeBlock();
            }
            blocks_[cur_block]->PushFront(value);
            return true;
        }

        return false;
    }

    void PushFront(int value) {
        if (blocks_ == nullptr) {
            *this = Deque{value};
            return;
        }

        ++size_deque_;

        if (size_blocks_ == 0) {
            if (blocks_[begin_] == nullptr) {
                blocks_[begin_] = new Block();
                blocks_[begin_]->MakeBlock();
            }
            blocks_[begin_]->PushFront(value);
            ++size_blocks_;
            return;
        }

        if (TryPushFront(value)) {
            return;
        }

        Relocate();

        TryPushFront(value);
    }

    void PopFront() {
        size_t cur_block = begin_;
        size_t cur_left = blocks_[cur_block]->GetLeft();
        size_t cur_right = blocks_[cur_block]->GetRight();

        ++cur_left;
        blocks_[cur_block]->SetLeft(cur_left);
        --size_deque_;

        if (cur_left == cur_right) {
            --size_blocks_;
            delete blocks_[cur_block];
            blocks_[cur_block] = nullptr;
            begin_ = (begin_ + 1) % capacity_blocks_;
        }
    }

    int& operator[](size_t ind) {
        size_t sz_begin = blocks_[begin_]->GetRight() - blocks_[begin_]->GetLeft();
        if (sz_begin >= ind + 1) {
            return *blocks_[begin_]->GetNumPtr(ind);
        }

        ind -= sz_begin;
        size_t ind_block = (begin_ + ind / kSzBlock + 1) % capacity_blocks_;
        ind %= kSzBlock;
        return *blocks_[ind_block]->GetNumPtr(ind);
    }

    int operator[](size_t ind) const {
        size_t sz_begin = blocks_[begin_]->GetRight() - blocks_[begin_]->GetLeft();
        if (sz_begin >= ind + 1) {
            return blocks_[begin_]->GetNum(ind);
        }

        ind -= sz_begin;
        size_t ind_block = (begin_ + ind / kSzBlock + 1) % capacity_blocks_;
        ind %= kSzBlock;
        return blocks_[ind_block]->GetNum(ind);
    }

    size_t Size() const {
        return size_deque_;
    }

    void Clear() {
        while (size_deque_) {
            this->PopBack();
        }
    }

    ~Deque() {
        if (blocks_ != nullptr) {
            for (size_t i = 0; i < capacity_blocks_; ++i) {
                if (blocks_[i] != nullptr) {
                    delete blocks_[i];
                    blocks_[i] = nullptr;
                }
            }
        }
        blocks_ = nullptr;
        begin_ = 0;
        size_blocks_ = 0;
        capacity_blocks_ = 0;
        size_deque_ = 0;
    }

private:
    std::unique_ptr<Block*[]> blocks_ = nullptr;
    size_t begin_ = 0;
    size_t size_blocks_ = 0;
    size_t capacity_blocks_ = 0;
    size_t size_deque_ = 0;
};

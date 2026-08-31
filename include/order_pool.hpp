#pragma once

#include <cstddef>
#include <memory>
#include <stdexcept>

#include "order_node.hpp"


class OrderPool {
public:
    explicit OrderPool(
        std::size_t capacity
    )
        : capacity_(capacity),
          storage_(
              std::make_unique<OrderNode[]>(
                  capacity
              )
          ),
          freeHead_(nullptr)
    {

        if (capacity_ == 0) {
            return;
        }


        for (
            std::size_t i = 0;
            i < capacity_;
            ++i
        ) {

            storage_[i].active = false;

            storage_[i].previous = nullptr;
            storage_[i].next = nullptr;


            if (i + 1 < capacity_) {

                storage_[i].nextFree =
                    &storage_[i + 1];
            }

            else {

                storage_[i].nextFree =
                    nullptr;
            }
        }


        freeHead_ =
            &storage_[0];
    }


    OrderNode* acquire(
        const Order& order
    ) {

        if (freeHead_ == nullptr) {

            throw std::runtime_error(
                "OrderPool exhausted"
            );
        }


        OrderNode* node =
            freeHead_;


        freeHead_ =
            freeHead_->nextFree;


        node->order =
            order;

        node->previous =
            nullptr;

        node->next =
            nullptr;

        node->nextFree =
            nullptr;

        node->active =
            true;


        return node;
    }


    void release(
        OrderNode* node
    ) {

        if (node == nullptr) {
            return;
        }


        node->active =
            false;

        node->previous =
            nullptr;

        node->next =
            nullptr;


        node->nextFree =
            freeHead_;


        freeHead_ =
            node;
    }


    std::size_t capacity() const {
        return capacity_;
    }


private:
    std::size_t capacity_;

    std::unique_ptr<OrderNode[]>
        storage_;

    OrderNode* freeHead_;
};
//
// Created by taras on 08.05.19.
//

#include <iostream>
#include "shared.h"

shared::storage::storage(size_t new_size) {
    _data = new uint32_t[new_size];
}

shared::storage::~storage() {
    delete[] _data;
    _data = nullptr;
}

shared::shared() {
    ptr_storage = nullptr;
    ref_count = nullptr;
}


shared::shared(size_t new_size) {
    ptr_storage = new storage(new_size);
    ref_count = new size_t;
    *ref_count = 1;
}


shared& shared::operator=(shared const& other) {
    if (this->ptr_storage == other.ptr_storage)
        return *this;
    if (ref_count != nullptr) {
        decrease_ref_cnt();
    }
    ptr_storage = other.ptr_storage;
    ref_count = other.ref_count;
    increase_ref_cnt();

    return *this;
}


shared::~shared() {
    decrease_ref_cnt();
}


void shared::decrease_ref_cnt() {
    if (ref_count != nullptr && --(*ref_count) == 0) {
        delete ref_count;
        delete ptr_storage;
        ref_count = nullptr;
        ptr_storage = nullptr;
    }
}


void shared::increase_ref_cnt() {
    ++(*ref_count);
}

uint32_t* shared::get() {
    return ptr_storage->_data;
}

bool shared::unique() {
    return *ref_count == 1;
}



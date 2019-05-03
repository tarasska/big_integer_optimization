//
// Created by taras on 03.05.19.
//

#include <cstring>
#include "my_vector.h"

my_vector::my_vector() : _size(0), _is_small(true) {
    _data = small_data;
    _capacity = _small_default_capacity;
}

//void my_vector::allocate_new_big_data(size_t alloc_size) {
//    big_data = std::make_shared<elem_type>(alloc_size);
//}

void my_vector::make_big(size_t new_size, elem_type default_value) {
    big_data = std::make_shared<elem_type>(new_size);
    _data = big_data.get();
    memset(_data, default_value, new_size * sizeof(elem_type));
}

void my_vector::make_copy() {
    std::shared_ptr<elem_type> new_big_data = std::make_shared<elem_type>(_capacity);
    elem_type* new_data = new_big_data.get();
    memcpy(new_data, _data, _capacity * sizeof(elem_type));
    _data = new_data;
    big_data = new_big_data;
}

void my_vector::expansion() {
    std::shared_ptr<elem_type> new_big_data = std::make_shared<elem_type>(_capacity << 1);
    elem_type* new_data = new_big_data.get();
    memcpy(new_data, _data, _size * sizeof(elem_type));
    memset(&new_data[_size], 0, (_capacity - _size) * sizeof(elem_type));
    _data = new_data;
    big_data = new_big_data;
    _capacity <<= 1;
}

my_vector::my_vector(size_t capacity) : _size(capacity), _capacity(capacity) {
    if (capacity <= _small_default_capacity) {
        _is_small = true;
        _data = small_data;
    } else {
        _is_small = false;
        make_big(capacity, 0);
    }
}

my_vector::my_vector(size_t capacity, elem_type default_value) : _size(capacity), _capacity(capacity) {
    if (capacity <= _small_default_capacity) {
        _is_small = true;
        _data = small_data;
    } else {
        _is_small = false;
        make_big(capacity, default_value);
    }
}
my_vector::my_vector(my_vector const& other) noexcept : _size(other._size), _capacity(other._capacity),
                                               _is_small(other._is_small) {
    if (other._is_small) {
        memcpy(_data, other._data, _small_default_capacity * sizeof(elem_type));
        _data = small_data;
    } else {
        big_data = other.big_data;
        _data = big_data.get();
    }
}

my_vector::~my_vector() {
    _data = nullptr;
}

my_vector& my_vector::operator=(my_vector const& other) noexcept {
    if (other._is_small) {
        memcpy(_data, other._data, _small_default_capacity * sizeof(elem_type));
        _data = small_data;
    } else {
        _data = nullptr;
        big_data = other.big_data;
        _data = big_data.get();
    }
    _size = other._size;
    _capacity = other._capacity;
    _is_small = other._is_small;

    return *this;
}

my_vector::elem_type& my_vector::operator[](size_t i) {
    if (!_is_small && !big_data.unique()) {
        make_copy();
    }
    return _data[i];
}

my_vector::elem_type const& my_vector::operator[](size_t i) const {
    return _data[i];
}

my_vector::elem_type const& my_vector::back() const {
    return _data[_size - 1];
}

void my_vector::pop_back() {
    if (!_is_small && !big_data.unique()) {
        make_copy();
    }
    _data[--_size] = 0;
}

void my_vector::push_back(my_vector::elem_type elem) {
    if (_is_small) {
        if (_size >= _capacity) {
            make_big(_capacity <<= 1, 0);
            ++_size;
            _is_small = false;
        }
    } else {
        if (!big_data.unique()) {
            make_copy();
        }
        if (_size + 1 > _capacity / 2) {
            expansion();
        }
    }
    _data[_size++] = elem;
}

my_vector::elem_type* my_vector::get_unique_ptr() {
    if (!_is_small && !big_data.unique()) {
        make_copy();
    }
    return _data;
}

size_t my_vector::size() const {
    return _size;
}

bool my_vector::empty() const {
    return _size == 0;
}





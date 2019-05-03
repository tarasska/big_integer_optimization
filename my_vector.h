//
// Created by taras on 03.05.19.
//

#ifndef BIG_INTEGER_OPTIMIZATION__MY_VECTOR_H_
#define BIG_INTEGER_OPTIMIZATION__MY_VECTOR_H_

#include <cstddef>
#include <iosfwd>
#include <cstdint>
#include <memory>

struct my_vector {
  public:
    typedef uint32_t elem_type;

    my_vector();
    my_vector(size_t new_size);
    my_vector(size_t new_size, elem_type default_value);
    my_vector(my_vector const& other) noexcept;
    ~my_vector();

    my_vector& operator=(my_vector const& other) noexcept;
    elem_type& operator[](size_t i);
    elem_type const& operator[](size_t i) const;

    elem_type const& back() const;
    void pop_back();
    void push_back(elem_type elem);
    size_t size() const;
    elem_type* get_unique_ptr();
    bool empty() const;

  private:
    static const size_t _small_default_capacity = 5;
    size_t _size;
    size_t _capacity;
    bool _is_small;
    elem_type* _data;
    void make_copy();
    void make_big(size_t new_size, elem_type default_value);
    void expansion();

    union {
        std::shared_ptr<elem_type> big_data;
        elem_type small_data[_small_default_capacity]{};
    };
};

#endif //BIG_INTEGER_OPTIMIZATION__MY_VECTOR_H_

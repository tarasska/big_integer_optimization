//
// Created by taras on 08.05.19.
//

#ifndef BIG_INTEGER_OPTIMIZATION__SHARED_H_
#define BIG_INTEGER_OPTIMIZATION__SHARED_H_


#include <cstdint>

struct shared {
  public:
    shared();
    explicit shared(size_t size);
    ~shared();

    shared& operator=(shared const& other);
    size_t ref_cnt() const;
    uint32_t* get();
    bool unique();

  private:
    struct storage {
        explicit storage(size_t size);
        ~storage();

        uint32_t* _data;
    };

    void decrease_ref_cnt();
    void increase_ref_cnt();

    size_t* ref_count;
    storage* ptr_storage;
};

#endif //BIG_INTEGER_OPTIMIZATION__SHARED_H_

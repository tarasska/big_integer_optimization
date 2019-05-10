//
// Created by taras on 13.04.19.
//
#include <iostream>
#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <climits>
#include <cstdlib>
#include <stdint-gcc.h>
#include <string.h>
#include <assert.h>
#include <algorithm>
#include "big_integer.h"

//constructors
big_integer::big_integer() {
    sign = 0;
    digits.push_back(0);
}

big_integer::big_integer(big_integer const& other) : sign(other.sign), digits(other.digits) {}

big_integer::big_integer(int x) {
    if (x == 0) {
        sign = 0;
    } else {
        sign = x > 0 ? 1 : -1;
    }
    if (x == INT_MIN) {
        digits.push_back(2147483648u);
    } else {
        digits.push_back(abs(x));
    }
}

big_integer::big_integer(std::string const& str) : big_integer() {
    size_t i = (str[0] == '-') ? 1 : 0;
    while (i + 9 <= str.size()) {
        *this = *this * 1000000000 + std::stoi(str.substr(i, 9));
        i += 9;
    }
    if (i < str.size()) {
        int mul = 1;
        for (size_t j = i; j < str.size(); ++j) {
            mul *= 10;
        }
        *this = *this * mul + std::stoi(str.substr(i, str.size() - i));
    }
    sign = (str[0] == '-' ? -1 : 1);
    shrink_to_fit();
}

big_integer::big_integer(my_vector const& src, int flag) {
    digits = src;
    sign = flag;
}

//____________________________________________________________________________________________________________
//helpful function
void big_integer::shrink_to_fit() {
    while (digits.back() == 0 && digits.size() > 1) {
        digits.pop_back();
    }
    if (digits.size() == 1 && digits[0] == 0) {
        sign = 0;
    }
}

int big_integer::sub_prefix(big_integer const& b, size_t first_ind) {
    digit_type carry = 0;
    digit_type* ptr_digits = this->digits.get_unique_ptr();
    for (size_t i = 0; i < b.digits.size(); ++i) {
        if (ptr_digits[first_ind + i]  >= uint64_t(carry) + b.digits[i]) {
            ptr_digits[first_ind + i] = static_cast<uint32_t>(ptr_digits[first_ind + i] - carry - b.digits[i]);
            carry = 0;
        } else {
            ptr_digits[first_ind + i] += base - carry - uint64_t(b.digits[i]);
            carry = 1;
        }
    }
    if (carry != 0 && b.digits.size() < digits.size()) {
        if (ptr_digits[b.digits.size() + first_ind] != 0) {
            --ptr_digits[b.digits.size() + first_ind];
            carry = 0;
        } else {
            ptr_digits[b.digits.size() + first_ind] = base - 1;
        }
    }
    digits.set_size(b.digits.size() + first_ind + 1);
    return carry;
}

void big_integer::add_prefix(big_integer const& b, size_t first_ind) {
    uint64_t carry = 0;
    digit_type* ptr_digits = this->digits.get_unique_ptr();
    for (size_t i = 0; i < b.digits.size(); ++i) {
        uint64_t new_digit = carry + ptr_digits[first_ind + i] + b.digits[i];
        ptr_digits[first_ind + i] = digit_type(new_digit % base);
        carry = new_digit >= base ? 1 : 0;
    }
    ptr_digits[first_ind + b.digits.size()] += carry;
    digits.set_size(b.digits.size() + first_ind + 1);
}

big_integer big_integer::mul_by_short_with_shift(big_integer const& a, digit_type d, size_t shift) {
    if (d == 0)
        return big_integer();
    my_vector buf(a.digits.size() + shift + 1);
    digit_type carry = 0;
    digit_type* ptr_buf = buf.get_unique_ptr();
    for (size_t i = 0; i < a.digits.size(); ++i) {
        uint64_t mul = uint64_t(a.digits[i]) * d + carry;
        ptr_buf[i + shift] = digit_type(mul % base);
        carry = digit_type(mul / base);
    }
    buf.set_size(a.digits.size() + shift + 1);
    if (carry != 0) {
        ptr_buf[a.digits.size() + shift] = carry;
    }
    big_integer res(buf, 1);
    res.shrink_to_fit();
    return res;
}

std::pair<big_integer, big_integer::digit_type> big_integer::div_by_short(big_integer const& a, digit_type b) {
    if (b == 0)
        throw std::runtime_error("Division by zero");
    digit_type r = 0;
    my_vector result(a.digits.size());
    digit_type* ptr_result = result.get_unique_ptr();
    for (ptrdiff_t j = a.digits.size() - 1; j >= 0; --j) {
        uint64_t tmp = base * r + a.digits[j];
        ptr_result[j] = digit_type(tmp / b);
        r = digit_type(tmp % b);
    }
    result.set_size(a.digits.size());
    big_integer quotient(result, a.sign);
    quotient.shrink_to_fit();
    return {quotient, r};
}

int big_integer::modulo_compare_to(big_integer const& a, big_integer const& b) {
    if (a.digits.size() > b.digits.size()) {
        return 1;
    }
    if (a.digits.size() < b.digits.size()) {
        return -1;
    }
    for (ptrdiff_t i = a.digits.size() - 1; i >= 0; --i) {
        if (a.digits[i] > b.digits[i]) {
            return 1;
        } else if (b.digits[i] > a.digits[i]) {
            return -1;
        }
    }
    return 0;
}

big_integer big_integer::make_binary_operation(big_integer const& a,
                                               big_integer const& b,
                                               std::function<digit_type(digit_type, digit_type)> const& f,
                                               std::function<int(int, int)> const& get_sign) {
    big_integer big_bin, small_bin;
    if (a.digits.size() >= b.digits.size()) {
        big_bin = a.get_twos_complement_code();
        small_bin = b.get_twos_complement_code();
    } else {
        big_bin = b.get_twos_complement_code();
        small_bin = a.get_twos_complement_code();
    }
    my_vector buf(big_bin.digits.size());
    digit_type* ptr_buf = buf.get_unique_ptr();
    for (size_t i = 0; i < small_bin.digits.size(); ++i) {
        ptr_buf[i] = f(small_bin.digits[i], big_bin.digits[i]);
    }
    digit_type lead_block = small_bin.sign >= 0 ? 0 : max_digit;
    for (size_t i = small_bin.digits.size(); i < big_bin.digits.size(); ++i) {
        ptr_buf[i] = f(lead_block, big_bin.digits[i]);
    }
    buf.set_size(big_bin.digits.size());
    big_integer result(buf, get_sign(a.sign, b.sign));
    result.shrink_to_fit();
    return result.get_sign_code();
}

big_integer big_integer::get_reverse_code_with_add() const {
    my_vector result(digits.size());
    digit_type* ptr_result = result.get_unique_ptr();
    for (size_t i = 0; i < result.size(); ++i) {
        ptr_result[i] = ~digits[i];
    }
    result.set_size(digits.size());
    return big_integer(result, 1) + 1;
}

big_integer big_integer::get_twos_complement_code() const {
    if (sign >= 0) {
        return *this;
    }
    return get_reverse_code_with_add();
}

big_integer big_integer::get_sign_code() const {
    if (sign >= 0) {
        return *this;
    }
    big_integer result(get_reverse_code_with_add());
    result.sign = sign;
    return result;
}

//____________________________________________________________________________________________________________
//print format

std::string to_string(big_integer const& a) {
    big_integer buf(a);
    std::string result;
    do {
        std::pair<big_integer, big_integer::digit_type> p = big_integer::div_by_short(buf, 10);
        result.push_back(static_cast<char> (p.second + '0'));
        buf = p.first;
    } while (buf.digits.size() > 1 || buf.digits[0] != 0);
    if (a.sign == -1) {
        result.push_back('-');
    }
    std::reverse(result.begin(), result.end());
    return result;
}

std::ostream& operator<<(std::ostream& s, big_integer const& a) {
    s << to_string(a);
    return s;
}

//____________________________________________________________________________________________________________
//logical block
bool operator==(big_integer const& a, big_integer const& b) {
    if (a.sign != b.sign || a.digits.size() != b.digits.size()) return false;
    for (size_t i_a = 0, i_b = 0;
         i_a != a.digits.size() && i_b != b.digits.size(); ++i_a, ++i_b) {
        if (a.digits[i_a] != b.digits[i_b]) return false;
    }
    return true;
}

bool operator!=(big_integer const& a, big_integer const& b) {
    return !(a == b);
}

bool operator<(big_integer const& a, big_integer const& b) {
    if (a.sign == b.sign && a.digits.size() == a.digits.size()) {
        for (ptrdiff_t it = a.digits.size() - 1; it >= 0; --it) {
            if (a.digits[it] != b.digits[it]) return a.digits[it] < b.digits[it] && a.sign >= 0;
        }
    } else {
        if (a.sign == b.sign)
            return a.digits.size() < b.digits.size();
        else if (a.sign == 0)
            return b.sign > 0;
        else if (b.sign == 0)
            return a.sign < 0;
        else return a.sign < 0 && b.sign > 0;
    }
    return false;
}

bool operator>(big_integer const& a, big_integer const& b) {
    if (a.sign == b.sign && a.digits.size() == a.digits.size()) {
        for (ptrdiff_t it = a.digits.size() - 1; it >= 0; --it) {
            if (a.digits[it] != b.digits[it]) return a.digits[it] > b.digits[it] && a.sign >= 0;
        }
    } else {
        if (a.sign == b.sign)
            return a.digits.size() > b.digits.size();
        else if (a.sign == 0)
            return b.sign < 0;
        else if (b.sign == 0)
            return a.sign > 0;
        else return !(a.sign < 0 && b.sign > 0);
    }
    return false;
}

bool operator<=(big_integer const& a, big_integer const& b) {
    return !(a > b);
}

bool operator>=(big_integer const& a, big_integer const& b) {
    return !(a < b);
}

big_integer& big_integer::operator&=(big_integer const& rhs) {
    *this = make_binary_operation(*this,
                                  rhs,
                                  [](digit_type x, digit_type y) { return x & y; },
                                  [](int sa, int sb) { return ((sa == -1) && (sb == -1)) ? -1 : 1; });
    return *this;
}

big_integer& big_integer::operator|=(big_integer const& rhs) {
    *this = make_binary_operation(*this,
                                  rhs,
                                  [](digit_type x, digit_type y) { return x | y; },
                                  [](int sa, int sb) { return ((sa == -1) || (sb == -1)) ? -1 : 1; });
    return *this;
}

big_integer& big_integer::operator^=(big_integer const& rhs) {
    *this = make_binary_operation(*this,
                                  rhs,
                                  [](digit_type x, digit_type y) { return x ^ y; },
                                  [](int sa, int sb) { return ((sa == -1) ^ (sb == -1)) ? -1 : 1; });
    return *this;
}

big_integer big_integer::operator~() const {
    big_integer result(get_twos_complement_code());
    digit_type* ptr_result = result.digits.get_unique_ptr();
    for (size_t i = 0; i < result.digits.size(); ++i) {
        ptr_result[i] = ~ptr_result[i];
    }
    result.digits.set_size(result.digits.size());
    result.sign = -sign;
    result = result.get_sign_code();
    result.shrink_to_fit();
    return result;
}

big_integer operator&(big_integer a, big_integer const& b) {
    return a &= b;
}

big_integer operator|(big_integer a, big_integer const& b) {
    return a |= b;
}
big_integer operator^(big_integer a, big_integer const& b) {
    return a ^= b;
}

big_integer& big_integer::operator<<=(int rhs) {
    if (rhs == 0)
        return *this;

    my_vector result;
    static int bits_size = sizeof(digit_type) * 8;
    while (rhs >= bits_size) {
        result.push_back(0);
        rhs -= bits_size;
    }
    digit_type carry = 0;
    for (size_t i = 0; i < this->digits.size(); ++i) {
        result.push_back((this->digits[i] << rhs) + carry);
        carry = this->digits[i] >> (bits_size - rhs);
    }
    result.push_back(carry);
    *this = big_integer(result, sign);
    this->shrink_to_fit();
    return *this;
}

big_integer& big_integer::operator>>=(int rhs) {
    if (rhs == 0)
        return *this;

    static int bit_size = sizeof(digit_type) * 8;
    size_t pop_cnt = rhs / bit_size;
    rhs %= bit_size;
    if (pop_cnt >= this->digits.size()) {
        if (sign >= 0) {
            *this = big_integer();
            return *this;
        } else {
            *this = big_integer(my_vector(this->digits.size(), base - 1), -1);
            return *this;
        }
    }
    big_integer a_bin = get_twos_complement_code();
    if (sign >= 0) {
        a_bin.digits.push_back(0);
    } else {
        a_bin.digits.push_back(base - 1);
    }
    my_vector result(a_bin.digits.size() - 1);
    digit_type* ptr_result = result.get_unique_ptr();
    size_t it = 0;
    for (size_t i = pop_cnt; i < a_bin.digits.size() - 1; ++i) {
        digit_type carry = a_bin.digits[i + 1] << (bit_size - rhs);
        ptr_result[it++] = (a_bin.digits[i] >> rhs) + carry;
    }
    for (size_t i = 0; i < pop_cnt; i++) {
        ptr_result[it++] = a_bin.digits.back();
    }
    result.set_size(it);
    *this = big_integer(result, sign).get_sign_code();
    this->shrink_to_fit();
    return *this;
}

big_integer operator<<(big_integer a, int b) {
    return a <<= b;
}

big_integer operator>>(big_integer a, int b) {
    return a >>= b;
}
//__________________________________________________________________________________________________________
//arithmetic block

big_integer big_integer::operator+() const {
    return *this;
}

big_integer big_integer::operator-() const {
    big_integer result(*this);
    result.sign *= -1;
    return result;
}

big_integer& big_integer::operator=(big_integer const& other) = default;

big_integer& big_integer::operator+=(big_integer const& rhs) {
    big_integer b(rhs);
    if (sign < 0 && rhs.sign > 0) {
        *this = b -= (-*this);
        return *this;
    }
    if (sign > 0 && rhs.sign < 0) {
        *this = *this -= (-rhs);
        return *this;
    }

    size_t small_size = std::min(digits.size(), rhs.digits.size());
    size_t big_size = std::max(digits.size(), rhs.digits.size());
    my_vector buf(big_size);
    digit_type* ptr_buf = buf.get_unique_ptr();
    digit_type carry = 0;
    my_vector const &this_digits = digits;
    for (size_t i = 0; i < small_size; ++i) {
        uint64_t sum = uint64_t(this_digits[i]) + rhs.digits[i] + carry;
        ptr_buf[i] = digit_type(sum % base);
        carry = sum >= base ? 1 : 0;
    }
    big_integer const& c = (big_size == digits.size()) ? *this : rhs;
    for (size_t i = small_size; i < big_size; ++i) {
        uint64_t sum = uint64_t(c.digits[i]) + carry;
        ptr_buf[i] = digit_type(sum % base);
        carry = sum >= base ? 1 : 0;
    }
    buf.set_size(big_size);
    if (carry != 0) {
        buf.push_back(carry);
    }
   *this = big_integer(buf, sign == 0 ? rhs.sign : sign);
    this->shrink_to_fit();
    return *this;
}

big_integer& big_integer::operator-=(big_integer const& rhs) {
    if (sign < 0 && rhs.sign > 0) {
        *this = -((-*this) += rhs);
        return *this;
    }
    if (sign > 0 && rhs.sign < 0) {
        *this = *this + (-rhs);
        return *this;
    }

    big_integer const* big, * small;

    int res_sign = 0;
    if (modulo_compare_to(*this, rhs) >= 0) {
        res_sign = sign;
        small = &rhs;
        big = this;
    } else {
        res_sign = -sign;
        small = this;
        big = &rhs;
    }
    my_vector buf(big->digits.size());
    digit_type* ptr_buf = buf.get_unique_ptr();
    digit_type carry = 0;
    for (size_t i = 0; i < small->digits.size(); ++i) {
        if (carry != 0 && big->digits[i] == 0) {
            ptr_buf[i] = max_digit - small->digits[i];
        } else {
            digit_type big_digit = big->digits[i] - carry;
            if (big_digit < small->digits[i]) {
                carry = 1;
                ptr_buf[i] = digit_type((base - small->digits[i]) + big_digit);
            } else {
                carry = 0;
                ptr_buf[i] = big_digit - small->digits[i];
            }
        }
    }
    for (size_t i = small->digits.size(); i < big->digits.size(); ++i) {
        if (carry != 0 && big->digits[i] == 0) {
            ptr_buf[i] = max_digit;
        } else {
            digit_type big_digit = big->digits[i] - carry;
            carry = 0;
            ptr_buf[i] = big_digit;
        }
    }
    buf.set_size(big->digits.size());
    *this = big_integer(buf, res_sign);
    this->shrink_to_fit();
    return *this;
}

big_integer& big_integer::operator*=(big_integer const& rhs) {
    big_integer const& small = digits.size() < rhs.digits.size() ? *this : rhs;
    big_integer const& big = digits.size() < rhs.digits.size() ? rhs : *this;
    big_integer ans;
    for (size_t i = 0; i < small.digits.size(); i++) {
        big_integer buf = mul_by_short_with_shift(big, small.digits[i], i);
        ans += buf;
    }
    ans.sign = sign * rhs.sign;
    ans.shrink_to_fit();
    *this = ans;
    return *this;
}

std::pair<big_integer, big_integer> big_integer::division(big_integer const& a, big_integer const& b) {
    if (b.sign == 0)
        throw std::runtime_error("Division by zero");
    if (modulo_compare_to(a, b) == -1)
        return {big_integer(), big_integer(a)};
    if (b.digits.size() == 1) {
        std::pair<big_integer, big_integer> p = div_by_short(a, b.digits.back());
        p.first.sign = b.sign * a.sign;
        p.second.sign = a.sign;
        p.first.shrink_to_fit();
        p.second.shrink_to_fit();
        return p;
    }

    auto d = digit_type(base / uint64_t(b.digits.back() + 1));
    big_integer dividend(mul_by_short_with_shift(a, d, 0));
    big_integer const divisor(mul_by_short_with_shift(b, d, 0));
    size_t n = b.digits.size();
    size_t m = a.digits.size() - n;
    my_vector quotient(m + 1);
    digit_type* ptr_quotient = quotient.get_unique_ptr();
    if (dividend.digits.size() != n + m + 1) {
        dividend.digits.push_back(0);
    }
    const digit_type last_digits = divisor.digits.back();
    big_integer const& const_dividend = dividend;
    for (size_t j = m + 1; j-- > 0;) {
        uint64_t tmp = const_dividend.digits[j + n] * base + const_dividend.digits[j + n - 1];
        uint64_t q = tmp / last_digits;
        uint64_t r = tmp % last_digits;
        while (q == base || (r < base && q * divisor.digits[n - 2] > base * r + const_dividend.digits[j + n - 2])) {
            --q;
            r += last_digits;
        }
        if (dividend.sub_prefix(mul_by_short_with_shift(divisor, q, 0), j)) {
            --q;
            dividend.add_prefix(divisor, j);
        }
        ptr_quotient[j] = digit_type(q);
    }
    quotient.set_size(m + 1);
    big_integer res_q(quotient, a.sign * b.sign);
    res_q.shrink_to_fit();
    my_vector buf(n);
    digit_type* ptr_buf = buf.get_unique_ptr();
    for (size_t i = 0; i < n; ++i) {
        ptr_buf[i] = dividend.digits[i];
    }
    buf.set_size(n);
    big_integer res_mod
        (div_by_short(big_integer(buf, a.sign), d).first);
    res_mod.sign = a.sign;
    res_mod.shrink_to_fit();
    return {res_q, res_mod};
}

big_integer& big_integer::operator/=(big_integer const& rhs) {
    *this = division(*this, rhs).first;
    return *this;
}

big_integer& big_integer::operator%=(big_integer const& rhs) {
    *this = division(*this, rhs).second;
    return *this;
}

big_integer operator+(big_integer a, big_integer const& b) {
    return a += b;
}

big_integer operator-(big_integer a, big_integer const& b) {
    return a -= b;
}

big_integer operator*(big_integer a, big_integer const& b) {
    return a *= b;
}

big_integer operator/(big_integer a, big_integer const& b) {
    return a /= b;
}

big_integer operator%(big_integer a, big_integer const& b) {
    return a %= b;
}

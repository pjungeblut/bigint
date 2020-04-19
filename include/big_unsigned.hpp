#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace ntlib {

/**
 * Represents a natural number whose size is only limited by the computers
 * memory.
 */
class big_unsigned {
private:
  /**
   * Integral type used for each digit.
   */
  using digit_type = uint64_t;

  /**
   * Type big enough to contain the sum/product of any two digits.
   */
  using double_digit_type = __uint128_t;

  /**
   * The base in which the numbers are stored is a power of two for optimal
   * memory usage.
   */
  constexpr static uint8_t LOG_BASE = sizeof(digit_type) * 8;
  // Cast is important to avoid overflows.
  constexpr static double_digit_type BASE =
      static_cast<double_digit_type>(1) << LOG_BASE;

public:
  /**
   * Default constructor.
   */
  big_unsigned() {}

  /**
   * Constructor to build big_unsigned from a simple integer.
   *
   * @param n The value to initialize with.
   */
  big_unsigned(digit_type n) {
    if (n > 0) digits.assign(1, n);
  }

  /**
   * Constructor to build big_unsigned from a string representation of the
   * value. Can be given in any base b.
   *
   * @param n The value to initialize with.
   * @param base The base. 2 <= base <= 16. Default is base = 10.
   */
  big_unsigned(const std::string &n, uint8_t base = 10) {
    if (n == "0") return;

    // Create big_unsigned with value 0.
    // Correct value will added in later.
    digits.assign(1, 0);

    // Read digit by digit.
    for (char digit : n) {
      digit_multiply(*this, base);
      digit_add(*this, char_to_value(digit, base));
    }
  }

  /**
   * Copy constructor.
   *
   * @param other Another big_unsigned.
   */
  big_unsigned(const big_unsigned &other) {
    digits = other.digits;
  }

  /**
   * Copy assignment operator.
   *
   * @param other Another big_unsigned.
   * @return Reference to the current big_unsigned containing the same value as
   *         other.
   */
  big_unsigned& operator=(const big_unsigned &other) {
    digits = other.digits;
    return *this;
  };

  /**
   * Move constructor.
   *
   * @param other Another big_unsigned.
   */
  big_unsigned(big_unsigned &&other) {
    digits = std::move(other.digits);
  }

  /**
   * Move assignment operator.
   *
   * @param other Another big_unsigned.
   * @return Reference to the current big_unsigned containing the value of
   *         other.
   */
  big_unsigned& operator=(big_unsigned &&other) {
    digits = std::move(other.digits);
    return *this;
  }

  /**
   * Destructor.
   */
  ~big_unsigned() = default;

  /**
   * Convert value into a string.
   *
   * @param base The base to print in. 2 <= base <= 16.
   * @return The value as a string in the given base.
   */
  std::string to_string(uint8_t base = 10) const {
    // Special case: value = 0.
    if (digits.size() == 0) return "0";

    // Recursively find the digits in the requested base.
    // Must work on a copy, since the value must be repeatedly divided by base.
    std::string s;
    big_unsigned value = *this;

    // Optimization for base 10:
    if (base == 10) {
      const digit_type big_base = 1'000'000'000'000'000'000uLL;
      while (value != 0) {
        digit_type remainder;
        digit_divide_with_remainder(value, big_base, value, remainder);
        std::string s_remainder = std::to_string(remainder);
        for (auto c = s_remainder.rbegin(); c != s_remainder.rend(); ++c) s.push_back(*c);
        for (std::size_t i = s_remainder.size(); i < 18; ++i) s.push_back('0');
      }
      while (s.back() == '0') s.pop_back();
    } else {
      while (value != 0) {
        digit_type remainder;
        digit_divide_with_remainder(value, base, value, remainder);
        s.push_back(value_to_char(remainder, base));
      }
    }
    std::reverse(s.begin(), s.end());
    return s;
  }

  /**
   * Hidden friend for stream output.
   *
   * @param os The out stream.
   * @param bu The big_unsigned to print.
   * @return The out stream.
   */
  friend std::ostream& operator<<(std::ostream &os, const big_unsigned &bu) {
    os << bu.to_string();
    return os;
  }

  /**
   * Less than operator.
   *
   * TODO: Replace this by the three way comparison (spaceship) operator once
   *       it gets implemented for g++.
   *       As soon as this is done, remove the six classic comparison operators.
   *
   * @param a The left hand side to compare.
   * @param b The right hand side to compare.
   * @return True, if and only if a < b.
   */
  friend bool operator<(const big_unsigned &a, const big_unsigned &b) {
    // If they have different length, we can just compare their lengths.
    std::size_t da = a.digits.size();
    std::size_t db = b.digits.size();
    if (da != db) return da < db;

    // They have the same length. Find the highest position where their digits
    // are different.
    for (auto ait = a.digits.rbegin(), bit = b.digits.rbegin();
        ait != a.digits.rend(); ++ait, ++bit) {
      if (*ait != *bit) return *ait < *bit;
    }
    // They must be equal.
    return false;
  }

  /**
   * Greater than operator.
   *
   * @param a The left hand side to compare.
   * @param b The right hand side to compare.
   * @return True, if and only if a > b.
   */
  friend bool operator>(const big_unsigned &a, const big_unsigned &b) {
    return b < a;
  }

  /**
   * Equal operator.
   *
   * @param a The left hand side to compare.
   * @param b The right hand side to compare.
   * @return True, if and only if a == b.
   */
  friend bool operator==(const big_unsigned &a, const big_unsigned &b) {
    return a.digits == b.digits;
  }

  /**
   * Unequal operator.
   *
   * @param a The left hand side to compare.
   * @param b The right hand side to compare.
   * @return True, if and only if a != b.
   */
  friend bool operator!=(const big_unsigned &a, const big_unsigned &b) {
    return a.digits != b.digits;
  }

  /**
   * Less than or equal operator.
   *
   * @param a The left hand side to compare.
   * @param b The right hand side to compare.
   * @return True, if and only if a <= b.
   */
  friend bool operator<=(const big_unsigned &a, const big_unsigned &b) {
    // If they have different length, we can just compare their lengths.
    std::size_t da = a.digits.size();
    std::size_t db = b.digits.size();
    if (da != db) return da < db;

    // They have the same length. Find the highest position where their digits
    // are different.
    for (auto ait = a.digits.rbegin(), bit = b.digits.rbegin();
        ait != a.digits.rend(); ++ait, ++bit) {
      if (*ait != *bit) return *ait < *bit;
    }
    // They must be equal.
    return true;
  }

  /**
   * Greater than or equal operator.
   *
   * @param a The left hand side to compare.
   * @param b The right hand side to compare.
   * @return True, if and only if a >= b.
   */
  friend bool operator>=(const big_unsigned &a, const big_unsigned &b) {
    return b <= a;
  }

  /**
   * Addition for big_unsigned.
   *
   * @param a The first summand.
   * @param b The second summand.
   * @return The sum of a and b.
   */
  friend big_unsigned operator+(const big_unsigned &a, const big_unsigned &b) {
    big_unsigned sum;
    add(a, b, sum);
    return sum;
  }

  /**
   * Addition for a single digit.
   *
   * @param a The first summand.
   * @param b The second summand. A single digit.
   * @return The sum a + b.
   */
  friend big_unsigned operator+(const big_unsigned &a, digit_type b) {
    big_unsigned sum = a;
    digit_add(sum, b);
    return sum;
  }

  /**
   * Adds another big_unsigned to the current one.
   *
   * @param other The other summand.
   * @return Reference to the current big_unsigned containing the sum of its
   *         previous value and other.
   */
  big_unsigned& operator+=(const big_unsigned &other) {
    add(*this, other, *this);
    return *this;
  }

  /**
   * Adds a single digit into the current one.
   *
   * @param other The other summand.
   * @return  Reference to the current big_unsigned containing the sum of its
   *          previous value and other.
   */
   big_unsigned& operator+=(digit_type other) {
     digit_add(*this, other);
     return *this;
   }

  /**
   * Pre-increment.
   *
   * @return Reference to the current (incremented) object.
   */
  big_unsigned& operator++() {
    digit_add(*this, 1);
    return *this;
  }

  /**
   * Post-increment.
   *
   * @return The old value of the object.
   */
  big_unsigned operator++(int) {
    big_unsigned copy = *this;
    digit_add(*this, 1);
    return copy;
  }

  /**
   * Subtraction for big_unsigned.
   * It must be a >= b.
   *
   * @param a The minuend.
   * @param b The subtrahend.
   * @return The difference a - b.
   */
  friend big_unsigned operator-(const big_unsigned &a, const big_unsigned &b) {
    big_unsigned difference;
    subtract(a, b, difference);
    return difference;
  }

  /**
   * Subtraction for a single digit.
   *
   * @param a The minuend.
   * @param b The subtrahend. It must be a >= b.
   * @return The difference a - b.
   */
  friend big_unsigned operator-(const big_unsigned &a, digit_type b) {
    big_unsigned difference = a;
    digit_subtract(difference, b);
    return difference;
  }

  /**
   * Subtracts another big_unsigned from the current one.
   *
   * @param other The subtrahend. Must be at least as big as current value.
   * @return Reference to the current big_unsigned containing the diffrence of
   *         its previous value and other.
   */
  big_unsigned& operator-=(const big_unsigned &other) {
    subtract(*this, other, *this);
    return *this;
  }

  /**
   * Subtracts a single digit from the current big_unsigned.
   *
   * @param other The digit to subtract.
   * @return Reference to the current big_unsigned containing the difference of
   *         its previous value and other.
   */
  big_unsigned& operator-=(digit_type other) {
    digit_subtract(*this, other);
    return *this;
  }

  /**
   * Pre-decrement.
   *
   * @return Reference to the current (decremented) object.
   */
  big_unsigned& operator--() {
    digit_subtract(*this, 1);
    return *this;
  }

  /**
   * Post-decrement.
   *
   * @return The old value of the object.
   */
  big_unsigned operator--(int) {
    big_unsigned copy = *this;
    digit_subtract(*this, 1);
    return copy;
  }

  /**
   * Multiplication for big_unsigned.
   *
   * @param a The first factor.
   * @param b The second factor.
   * @return The product a * b.
   */
  friend big_unsigned operator*(const big_unsigned &a, const big_unsigned &b) {
    big_unsigned product;
    mulitply(a, b, product);
    return product;
  }

  /**
   * Multiplication for a single digit.
   *
   * @param a The first factor.
   * @param b The second factor. A single digit.
   * @return The product a * b.
   */
  friend big_unsigned operator*(const big_unsigned &a, digit_type b) {
    big_unsigned product = a;
    digit_multiply(product, b);
    return product;
  }

  /**
   * Multiplies another big_unsigned to the current one.
   *
   * @param other The other factor.
   * @return Reference to the current big_unsigned containing the product of its
   *         previous value and other.
   */
  big_unsigned& operator*=(const big_unsigned &other) {
    // TODO: This creates an intermediate big_unsigned. Find a way to do the
    //       multiplication in-place.
    big_unsigned product;
    mulitply(*this, other, product);
    digits = std::move(product.digits);
    return *this;
  }

  /**
   * Multiplies a single digit to the current big_unsigned.
   *
   * @param other The other factor. A single digit.
   * @return Reference to the current big_unsigned containing the product of its
   *         previous value and other.
   */
  big_unsigned& operator*=(digit_type other) {
    digit_multiply(*this, other);
    return *this;
  }

  /**
   * Divides a by b.
   *
   * @param a The dividend.
   * @param b The divisor.
   * @return The quotient of a div b.
   */
  friend big_unsigned operator/(const big_unsigned &a, const big_unsigned &b) {
    big_unsigned quotient, remainder;
    divide_with_remainder(a, b, quotient, remainder);
    return quotient;
  }

  /**
   * Division for a single digit.
   *
   * @param a The dividend.
   * @param b The divisor. A single digit.
   * @return The quotient of a div b.
   */
  friend big_unsigned operator/(const big_unsigned &a, digit_type b) {
    big_unsigned quotient = a;
    digit_type remainder;
    digit_divide_with_remainder(quotient, b, quotient, remainder);
    return quotient;
  }

  /**
   * Divides another big_unsigned from the current one.
   *
   * @param other The divisor.
   * @return Reference to the current big_unsigned containing the quotient of
   *         its previous value and other.
   */
  big_unsigned& operator/=(const big_unsigned &other) {
    big_unsigned quotient, remainder;
    divide_with_remainder(*this, other, quotient, remainder);
    digits = std::move(quotient.digits);
    return *this;
  }

  /**
   * Divides a single digit from the current big_unsigned.
   *
   * @param other The digit to divide through.
   * @return Reference to the current big_unsigned containing the quotient of
   *         its previous value and other.
   */
  big_unsigned& operator/=(digit_type other) {
    digit_type remainder;
    digit_divide_with_remainder(*this, other, *this, remainder);
    return *this;
  }

  /**
   * Computes the remainder of a and b.
   *
   * @param a The dividend.
   * @param b The divisor.
   * @return The remainder a mod b.
   */
  friend big_unsigned operator%(const big_unsigned &a, const big_unsigned &b) {
    big_unsigned quotient, remainder;
    divide_with_remainder(a, b, quotient, remainder);
    return remainder;
  }

  /**
   * Modulo for a single digit.
   *
   * @param a The dividend.
   * @param b The divisor.
   * @return The remainder a mod b.
   */
  friend big_unsigned operator%(const big_unsigned &a, digit_type b) {
    big_unsigned quotient = a;
    digit_type remainder;
    digit_divide_with_remainder(quotient, b, quotient, remainder);
    return big_unsigned(remainder);
  }

  /**
   * Computes the remainder of another big_unsigned and the current one.
   *
   * @param other The divisor.
   * @return Reference to the current big_unsigned containing the remainder of
   *         its previous value and other.
   */
  big_unsigned& operator%=(const big_unsigned &other) {
    big_unsigned quotient, remainder;
    divide_with_remainder(*this, other, quotient, remainder);
    digits = std::move(remainder.digits);
    return *this;
  }

  /**
   * Computes the remainder when dividing through a single digit.
   *
   * @param other The divisor.
   * @return Reference to the current big_unsigned containing the remainder of
   *         its previous value and other.
   */
  big_unsigned& operator%=(digit_type other) {
    digit_type remainder;
    digit_divide_with_remainder(*this, other, *this, remainder);
    digits.assign(1, remainder);
    return *this;
  }

  /**
   * Divides a by b and outputs the quotient and the remainder.
   *
   * TODO: Uses binary search to find the quotient. This is far from optimal.
   *       Investigate for a better algorithm.
   *
   * @param a The dividend.
   * @param b The divisor.
   * @param quotient The quotient a div b.
   * @param remainder The remainder a mod b.
   */
  static void divide_with_remainder(const big_unsigned &a, const big_unsigned &b,
      big_unsigned &quotient, big_unsigned &remainder) {
    // Base case, b == 0.
    if (b == 0) {
      throw std::invalid_argument("Division by zero.");
    }

    // Base case, equality.
    if (a == b) {
      quotient = big_unsigned(1);
      remainder = big_unsigned(0);
      return;
    }

    // Base case, a < b.
    if (a < b) {
      quotient = big_unsigned(0);
      remainder = a;
      return;
    }

    // Binary search for the quotient.
    quotient = big_unsigned(0);
    remainder = a - quotient * b;
    // big_unsigned two(2);
    while (remainder >= b) {
      big_unsigned increase(1);
      while ((quotient + increase * 2u) * b <= a) increase *= 2;
      quotient += increase;
      remainder = a - quotient * b;
    }
  }

  /**
   * Divides the big_unsigned a by a single digit b.
   * Parameters a and quotient may be the same.
   *
   * @param a The dividend.
   * @param b The divisor. A single digit.
   * @param quotient The quotient of the division.
   * @param remainder The remainder of the division.
   */
   static void digit_divide_with_remainder(const big_unsigned &a, digit_type b,
      big_unsigned &quotient, digit_type &remainder) {
    // Base case: b == 0.
    if (b == 0) throw std::invalid_argument("Division by zero.");

    // Base case: a == 0.
    if (a == 0) {
      quotient = 0;
      remainder = 0;
      return;
    }

    std::size_t da = a.digits.size();

    // Is the first digit of a at least b?
    // If so, the quotient has the same length as a.
    // Otherwise it is exactly one digit shorter.
    std::size_t dc = da;
    if (a.digits.back() < b) --dc;

    // Resize quotient to its exact size.
    quotient.digits.resize(da);

    // Divide by school method.
    remainder = 0;
    for (std::size_t i = 0; i < da; ++i) {
      std::size_t ia = da - i - 1;

      // Cast is important to avoid overflows.
      double_digit_type double_remainder =
          static_cast<double_digit_type>(remainder) * BASE + a.digits[ia];
      quotient.digits[ia] = double_remainder / b;
      remainder = double_remainder % b;
    }

    // If dc < da, then the result is shifted.
    // Bring it back into place.
    if (dc < da) quotient.digits.pop_back();
  }

  /**
   * Bitwise negation.
   * Negates every digit, including the most significant ones (with its leading
   * zeros).
   *
   * @return A big_unsigned with bitwise negated digits.
   */
  friend big_unsigned operator~(const big_unsigned &a) {
    big_unsigned negation = a;
    for (digit_type &digit : negation.digits) digit = ~digit;
    return negation;
  }

  /**
   * Bitwise and operator.
   *
   * @param a The first operand.
   * @param b The second operand.
   * @return big_unsigned that contains the bitwise and of a and b.
   */
  friend big_unsigned operator&(const big_unsigned &a, const big_unsigned &b) {
    big_unsigned bit_and;
    bitwise_and(a, b, bit_and);
    return bit_and;
  }

  /**
   * Bitwise and for a single digit.
   *
   * @param a The first operand.
   * @param b The second operand. A single digit.
   * @return big_unsigned that contains the bitwise and of a and b.
   */
  friend big_unsigned operator&(const big_unsigned &a, digit_type b) {
    big_unsigned bit_and = a;
    digit_bitwise_and(bit_and, b);
    return bit_and;
  }

  /**
   * Bitwise and with another big_unsigned.
   *
   * @param other The other big_unsigned.
   * @return Reference to the current big_unsigned after bitwise and with
   *         another big_unsigned.
   */
  big_unsigned& operator&=(const big_unsigned &other) {
    bitwise_and(*this, other, *this);
    return *this;
  }

  /**
   * Bitwise and with just another digit.
   *
   * @param other The other digit to bitwise and with.
   * @return Reference to the current big_unsigned after bitwise and with
   *         another digit.
   */
  big_unsigned& operator&=(digit_type other) {
    digit_bitwise_and(*this, other);
    return *this;
  }

  /**
   * Bitwise or operator.
   *
   * @param a The first operand.
   * @param b The second operand.
   * @return big_unsigned that contains the bitwise or of a and b.
   */
  friend big_unsigned operator|(const big_unsigned &a, const big_unsigned &b) {
    big_unsigned bit_or;
    bitwise_or(a, b, bit_or);
    return bit_or;
  }

  /**
   * Bitwise or for a single digit.
   *
   * @param a The first operand.
   * @param b The second operand. A single digit.
   * @return big_unsigned that contains the bitwise or of a and b.
   */
  friend big_unsigned operator|(const big_unsigned &a, digit_type b) {
    big_unsigned bit_or = a;
    digit_bitwise_or(bit_or, b);
    return bit_or;
  }

  /**
   * Bitwise or with another big_unsigned.
   *
   * @param other The other big_unsigned.
   * @return Reference to the current big_unsigned after bitwise or with
   *         another big_unsigned.
   */
  big_unsigned& operator|=(const big_unsigned &other) {
    bitwise_or(*this, other, *this);
    return *this;
  }

  /**
   * Bitwise or with just another digit.
   *
   * @param other The other digit to bitwise or with.
   * @return Reference to the current big_unsigned after bitwise or with
   *         another digit.
   */
  big_unsigned& operator|=(digit_type other) {
    digit_bitwise_or(*this, other);
    return *this;
  }

  /**
   * Bitwise xor operator.
   *
   * @param a The first operand.
   * @param b The second operand.
   * @return big_unsigned that contains the bitwise xor of a and b.
   */
  friend big_unsigned operator^(const big_unsigned &a, const big_unsigned &b) {
    big_unsigned bit_xor;
    bitwise_xor(a, b, bit_xor);
    return bit_xor;
  }

  /**
   * Bitwise xor for a single digit.
   *
   * @param a The first operand.
   * @param b The second operand. A single digit.
   * @return big_unsigned that contains the bitwise xor of a and b.
   */
  friend big_unsigned operator^(const big_unsigned &a, digit_type b) {
    big_unsigned bit_xor = a;
    digit_bitwise_xor(bit_xor, b);
    return bit_xor;
  }

  /**
   * Bitwise xor with another big_unsigned.
   *
   * @param other The other big_unsigned.
   * @return Reference to the current big_unsigned after bitwise xor with
   *         another big_unsigned.
   */
  big_unsigned& operator^=(const big_unsigned &other) {
    bitwise_xor(*this, other, *this);
    return *this;
  }

  /**
   * Bitwise xor with just another digit.
   *
   * @param other The other digit to bitwise xor with.
   * @return Reference to the current big_unsigned after bitwise xor with
   *         another digit.
   */
  big_unsigned& operator^=(digit_type other) {
    digit_bitwise_xor(*this, other);
    return *this;
  }

  /**
   * Left shift operator.
   *
   * @param a The big_unsigned to be shifted.
   * @param b The amount of binary positions to shift.
   * @return A big_unsigned with a's value left shifted by b binary positions.
   */
  friend big_unsigned operator<<(const big_unsigned &a, digit_type b) {
    big_unsigned shifted;
    left_shift(a, b, shifted);
    return shifted;
  }

  /**
   * Left shift assignment operator.
   *
   * @param b The amount of binary positions to shift.
   * @return A reference to the current big_unsigned after being shifted by b
   *         binary positions.
   */
  big_unsigned& operator<<=(digit_type b) {
    left_shift(*this, b, *this);
    return *this;
  }

  /**
   * Right shift operator.
   *
   * @param a The big_unsigned to be shifted.
   * @param b The amount of binary positions to shift.
   * @return A big_unsigned with a's value left shifted by b binary positions.
   */
  friend big_unsigned operator>>(const big_unsigned &a, digit_type b) {
    big_unsigned shifted;
    right_shift(a, b, shifted);
    return shifted;
  }

  /**
   * Right shift assignment operator.
   *
   * @param b The amount of binary positions to shift.
   * @return A reference to the current big_unsigned after being shifted by b
   *         binary positions.
   */
  big_unsigned& operator>>=(digit_type b) {
    right_shift(*this, b, *this);
    return *this;
  }

  /**
   * Cast to digit_type.
   * Only returns the last digit.
   *
   * @return The value of the least significant digit.
   */
  explicit operator digit_type() const {
    return digits.size() ? digits[0] : 0;
  }

  /**
   * Cast to bool.
   */
  explicit operator bool() const {
    return !digits.empty();
  }

private:
  /**
   * An unbounded array containing the digits.
   * Elements are stroed in reversed order, so element i corresponds to the
   * value digits[i] * BASE^i.
   */
  std::vector<digit_type> digits;

  /**
   * Converts a character into its numerical value.
   *
   * @param c The character.
   * @param base The base.
   * @return The numerical value of the character.
   */
  static digit_type char_to_value(char c, uint8_t base) {
    digit_type value = 0;
    if ('0' <= c && c <= '9') value = c - '0';
    else if ('A' <= c && c <= 'F') value = c - 'A' + 10;
    else throw std::invalid_argument("Digits can only be written by 0..9A..F.");

    if (value >= base) {
      throw std::invalid_argument("Invalid digit in given base.");
    }
    return value;
  }

  static char value_to_char(digit_type value, uint8_t base) {
    if (value >= base) {
      throw std::invalid_argument("Value is too big for the given base.");
    }

    if (value <= 9) return value + '0';
    return value - 10 + 'A';
  }

  /**
   * Removes leading zeros.
   */
  void remove_leading_zeros() {
    while (digits.size() && digits.back() == 0) digits.pop_back();
  }

  /**
   * Adds digit b to big_unsigned a.
   *
   * @param a The big_unsigned to add into.
   * @param b The digit to add.
   */
  static void digit_add(big_unsigned &a, digit_type b) {
    std::size_t da = a.digits.size();

    // Base case: a == 0 and b == 0
    if (da == 0 && b == 0) return;

    // Base case: a == 0
    if (da == 0) {
      a.digits.push_back(b);
      return;
    }

    // Add to each digit.
    digit_type carry = b;
    for (std::size_t i = 0; i < da; ++i) {
      // Cast is important to avoid overflows.
      double_digit_type digit_sum =
          static_cast<double_digit_type>(a.digits[i]) + carry;
      // Implicit modulo by BASE.
      a.digits[i] = digit_sum;
      carry = digit_sum / BASE;
    }

    // If the carry is not zero, a has another digit.
    if (carry != 0) a.digits.push_back(carry);
  }

  /**
   * Adds two big_unsigneds into a third.
   * Parameters a and c may be the same, for a += b.
   *
   * @param a The first summand.
   * @param b The second summand.
   * @param c The sum a + b.
   */
  static void add(const big_unsigned &a, const big_unsigned &b,
      big_unsigned &c) {
    std::size_t da = a.digits.size();
    std::size_t db = b.digits.size();

    // Reserve enough space for the sum and zero initialize the minimum amount
    // of needed digits.
    c.digits.reserve(std::max(da, db) + 1);
    c.digits.resize(std::max(da, db));

    // Common positions of a and b.
    digit_type carry = 0;
    std::size_t i = 0;
    for (; i < std::min(da, db); ++i) {
      // Cast is important to avoid overflows.
      double_digit_type digit_sum =
          static_cast<double_digit_type>(a.digits[i]) + b.digits[i] + carry;
      // Implicit modulo by BASE.
      c.digits[i] = digit_sum;
      carry = digit_sum / BASE;
    }

    // Take the longer summand and add remaining digits.
    auto *longer = da > db ? &a : &b;
    for (; i < std::max(da, db); ++i) {
      // Cast is important to avoid overflows.
      double_digit_type digit_sum =
          static_cast<double_digit_type>(longer->digits[i]) + carry;
      // Implicit modulo by BASE.
      c.digits[i] = digit_sum;
      carry = digit_sum / BASE;
    }

    // If the carry is non-zero we need another digit.
    if (carry != 0) c.digits.push_back(carry);
  }

  /**
   * Subtracts a single digit b from a into a.
   * It must be b <= a.
   *
   * @param a The big_unsigned to subtract from.
   * @param b The digit to subtract.
   */
  static void digit_subtract(big_unsigned &a, digit_type b) {
    std::size_t da = a.digits.size();

    // Go through a's digits.
    digit_type carry = b;
    for (std::size_t i = 0; i < da; ++i) {
      // Cast is important to avoid overflows.
      double_digit_type lower = carry;
      double_digit_type upper = a.digits[i];
      if (lower > upper) {
        lower += BASE;
        carry = 1;
      } else carry = 0;
      a.digits[i] = upper - lower;
    }

    // If the carry is not zero, then b was bigger than a.
    if (carry != 0) {
      throw std::invalid_argument(
          "Digit subtraction: Subtrahend bigger than minuend.");
    }

    // a might have leading zeros now.
    a.remove_leading_zeros();
  }

  /**
   * Subtracts two big_unsigneds into a third.
   * Parameters a and c may be the same, for a -= b.
   * It must be a >= b.
   *
   * @param a The minuend.
   * @param b The subtrahend.
   * @param c The difference a - b.
   */
  static void subtract(const big_unsigned &a, const big_unsigned &b,
      big_unsigned &c) {
    std::size_t da = a.digits.size();
    std::size_t db = b.digits.size();

    // Check that da >= db (which must be true, if a >= b).
    if (da < db) {
      throw std::invalid_argument(
          "Subtraction: Subtrahend is bigger than minuend.");
    }

    // Initialize result and resize to the maximum number of digits.
    c.digits.resize(da);

    // Since a >= b it must be da >= db.
    // First go through b's digits.
    digit_type carry = 0;
    std::size_t i = 0;
    for (; i < db; ++i) {
      digit_type lower = b.digits[i] + carry;
      digit_type upper = a.digits[i];
      if (lower > upper) {
        upper += BASE;
        carry = 1;
      } else carry = 0;
      c.digits[i] = upper - lower;
    }

    // Now go through the remaining digits of a.
    for (; i < da; ++i) {
      if (carry > a.digits[i]) {
        c.digits[i] = BASE - 1;
        carry = 1;
      } else {
        c.digits[i] = a.digits[i] - carry;
        carry = 0;
      }
    }

    // If the carry is not zero, b was bigger than a.
    if (carry != 0) {
      throw std::invalid_argument(
          "Subtraction: Subtrahend is bigger than minuend.");
    }

    // Leading digits of the difference might be 0. Remove those.
    c.remove_leading_zeros();
  }

  /**
   * In place mulitplication of a by digit b.
   *
   * @param a The first factor with an arbitrary number of digits.
   * @param b The second factor, a single digit.
   */
  static void digit_multiply(big_unsigned &a, const digit_type b) {
    // Base case: b == 0.
    if (b == 0) {
      a.digits.clear();
      return;
    }

    std::size_t da = a.digits.size();

    // Go through the digits of a.
    digit_type product_carry = 0;
    for (std::size_t i = 0; i < da; ++i) {
      // Cast is important to avoid overflows.
      double_digit_type digit_product =
          static_cast<double_digit_type>(a.digits[i]) * b + product_carry;
      product_carry = digit_product / BASE;
      // Implicit modulo by BASE.
      a.digits[i] = digit_product;
    }

    // If the product_carry is non-zero, it is the most significant digit of the
    // product. Add it into the result array.
    if (product_carry != 0) a.digits.push_back(product_carry);
  }

  /**
   * Performs a digit multiplication.
   *
   * Simultaneously computes each digit of the product and adds it into the
   * partial result.
   *
   * @param a The first factor with an arbitrary number of digits.
   * @param b The second factor, a single digit.
   * @param c The big_unsigned the result is added into.
   * @param d The offset used when writing the result into c. This allows to not
   *          just add the result but mulitply it with a factor BASE^i
   *          beforehand, thus aligning the partial sums.
   */
  static void digit_multiply_offset(const big_unsigned &a, const digit_type b,
      big_unsigned &c, std::size_t d = 0) {
    std::size_t da = a.digits.size();

    // Go through the digits of a.
    digit_type product_carry = 0;
    digit_type sum_carry = 0;
    std::size_t i = 0;
    for (; i < da; ++i) {
      // Cast is important to avoid overflows.
      double_digit_type digit_product =
          static_cast<double_digit_type>(a.digits[i]) * b + product_carry;
      product_carry = digit_product / BASE;
      // Cast is important to avoid overflows.
      double_digit_type digit_sum =
          static_cast<double_digit_type>(c.digits[d + i]) +
          digit_product % BASE +
          sum_carry;
      // Implicit modulo by BASE.
      c.digits[d + i] = digit_sum;
      sum_carry = digit_sum / BASE;
    }

    // If the product_carry is non-zero, it is the most significant digit of the
    // product. Add it into the result array.
    // Further there might still be a sum_carry.
    if (product_carry != 0) {
      // Cast is important to avoid overflows.
      double_digit_type digit_sum =
          static_cast<double_digit_type>(c.digits[d + i]) +
          product_carry +
          sum_carry;
      // Implicit modulo by BASE.
      c.digits[d + i] = digit_sum;
      sum_carry = digit_sum / BASE;
    }

    // Now product_carry is guaranteed to be zero, but we might still have a
    // carry from the addition.
    while (sum_carry != 0) {
      ++i;
      // Cast is important to avoid overflows.
      double_digit_type digit_sum =
          static_cast<double_digit_type>(c.digits[d + i]) + sum_carry;
      // Implicit modulo by base.
      c.digits[d + i] = digit_sum;
      sum_carry = digit_sum / BASE;
    }
  }

  /**
   * Multiplies two big_unsigneds into a third.
   * Parameters a and b must be different to c, as c's content gets erased.
   *
   * @param a The first factor.
   * @param b The second factor.
   * @param c The product a * b.
   */
  static void mulitply(const big_unsigned &a, const big_unsigned &b,
      big_unsigned &c) {
    std::size_t da = a.digits.size();
    std::size_t db = b.digits.size();

    // Reserve enough space for the result and zero initialize.
    c.digits.assign(da + db, 0);

    // Go through the digits of the shorter number and perform digit
    // multiplications adding the shifted results.
    if (da <= db) {
      for (std::size_t i = 0; i < da; ++i) {
        digit_multiply_offset(b, a.digits[i], c, i);
      }
    } else {
      for (std::size_t i = 0; i < db; ++i) {
        digit_multiply_offset(a, b.digits[i], c, i);
      }
    }

    // Result might not need all reserved digits.
    c.remove_leading_zeros();
  }

  /**
   * Bitwise and for a big_unsigned and a single digit.
   *
   * @param a The first operand. Result is stored here.
   * @param b The second operand. A single digit.
   */
  static void digit_bitwise_and(big_unsigned &a, digit_type b) {
    // Base case: a == 0.
    if (a.digits.empty()) return;

    // Compute last digit. All other are zero and can be deleted.
    a.digits[0] &= b;
    a.digits.resize(1);

    // Even the last digit might be zero. If so, it can be deleted.
    a.remove_leading_zeros();
  }

  /**
   * Bitwise and for two big_unsigneds.
   * Parameters a and c may be the same.
   *
   * @param a The first operand.
   * @param b The second operand.
   * @param c The bitwise and of a and b.
   */
  static void bitwise_and(const big_unsigned &a, const big_unsigned &b,
      big_unsigned &c) {
    std::size_t da = a.digits.size();
    std::size_t db = b.digits.size();

    // Reserve enough space for c and zero initialize.
    c.digits.assign(std::max(da, db), 0);

    for (std::size_t i = 0; i < std::min(a, b); ++i) {
      c.digits[i] = a.digits[i] & b.digits[i];
    }
    // There is nood to go through the remaining digits of the longer number,
    // since a bitwise and with 0 is always 0.

    // Bitwise and can lead to leading zeros that need to be deleted.
    c.remove_leading_zeros();
  }

  /**
   * Bitwise or for a big_unsigned and a single digit.
   *
   * @param a The first operand. Result is stored here.
   * @param b The second operand. A single digit.
   */
  static void digit_bitwise_or(big_unsigned &a, digit_type b) {
    // Base case: a == 0.
    if (a.digits.empty()) {
      a.digits.push_back(b);
      return;
    }

    // Compute last digit. All others stay as they are.
    // Cannot become zero, if it wasn't zero before.
    a.digits[0] |= b;
  }

  /**
   * Bitwise or for two big_unsigneds.
   * Parameters a and c may be the same.
   *
   * @param a The first operand.
   * @param b The second operand.
   * @param c The bitwise or of a and b.
   */
  static void bitwise_or(const big_unsigned &a, const big_unsigned &b,
      big_unsigned &c) {
    std::size_t da = a.digits.size();
    std::size_t db = b.digits.size();

    // Reserve enough space for c and zero initialize.
    c.digits.assign(std::max(da, db), 0);

    std::size_t i = 0;
    for (; i < std::min(a, b); ++i) c.digits[i] = a.digits[i] | b.digits[i];
    // All remaining positions can be copied from the longer of the two
    // operands.
    auto *longer = da > db ? &a : &b;
    for (; i < std::max(da, db); ++i) c.digits[i] = longer->digits[i];

    // Bitwise or cannot lead to leading zeros.
  }

  /**
   * Bitwise xor for a big_unsigned and a single digit.
   *
   * @param a The first operand. Result is stored here.
   * @param b The second operand. A single digit.
   */
  static void digit_bitwise_xor(big_unsigned &a, digit_type b) {
    // Base case: a == 0.
    if (a.digits.empty()) {
      a.digits.push_back(b);
      return;
    }

    // Compute last digit. All others stay as they are.
    a.digits[0] ^= b;

    // Bitwise xor can lead to leading zeros that need to be removed.
    a.remove_leading_zeros();
  }

  /**
   * Bitwise xor for two big_unsigneds.
   * Parameters a and c may be the same.
   *
   * @param a The first operand.
   * @param b The second operand.
   * @param c The bitwise xor of a and b.
   */
  static void bitwise_xor(const big_unsigned &a, const big_unsigned &b,
      big_unsigned &c) {
    std::size_t da = a.digits.size();
    std::size_t db = b.digits.size();

    // Reserve enough space for c and zero initialize.
    c.digits.assign(std::max(da, db), 0);

    std::size_t i = 0;
    for (; i < std::min(a, b); ++i) c.digits[i] = a.digits[i] ^ b.digits[i];
    // All remaining positions can be copied from the longer of the two
    // operands.
    auto *longer = da > db ? &a : &b;
    for (; i < std::max(a, b); ++i) c.digits[i] = longer->digits[i];

    // Bitwise xor can lead to leading zeros that need to be deleted.
    c.remove_leading_zeros();
  }

  /**
   * Left shifts a big_unsigned.
   * Parameters a and c may be the same.
   *
   * @param a The big_unsigned to shift.
   * @param b The amount of binary digits to shift.
   * @parmm c The result of c = a << b.
   */
  static void left_shift(const big_unsigned &a, digit_type b, big_unsigned &c) {
    const std::size_t da = a.digits.size();
    const std::size_t positions = b / LOG_BASE;
    const std::size_t remainder = b % LOG_BASE;

    if (remainder == 0) {
      c.digits.resize(da + positions);
      for (std::size_t i = 0; i < da; ++i) {
        c.digits[c.digits.size() - i - 1] = a.digits[da - i - 1];
      }
      for (std::size_t i = 0; i < positions; ++i) c.digits[i] = 0;
    } else {
      // Reserve enough space for the result.
      c.digits.resize(da + positions + 1);

      digit_type previous = 0;
      for (std::size_t i = 0; i < da; ++i) {
        const std::size_t ia = da - i - 1;
        const std::size_t ic = c.digits.size() - i - 1;

        // Cast is important to avoid overflows.
        const double_digit_type shift =
            static_cast<double_digit_type>(a.digits[ia]) << remainder;
        c.digits[ic] = (shift >> LOG_BASE) + previous;
        // Implicit modulo by BASE.
        previous = shift;
      }
      c.digits[positions] = previous;

      // Clear least significant bits.
      for (std::size_t i = 0; i < positions; ++i) c.digits[i] = 0;

      // We might have just shifted zeros into the most significant digit. If so,
      // they need to be removed.
      c.remove_leading_zeros();
    }
  }

  /**
   * Right shifts a big_unsigned.
   * Parameters a and c may be the same.
   *
   * @param a The big_unsigned to shift.
   * @param b The amount of binary digits to shift.
   * @parmm c The result of c = a >> b.
   */
  static void right_shift(const big_unsigned &a, digit_type b, big_unsigned &c) {
    const std::size_t da = a.digits.size();
    const std::size_t positions = b / LOG_BASE;
    const std::size_t remainder = b % LOG_BASE;

    c.digits.resize(da);

    if (remainder == 0) {
      for (std::size_t i = positions; i < da; ++i) {
        c.digits[i - positions] = a.digits[i];
      }
      for (std::size_t i = 0; i < positions; ++i) c.digits.pop_back();
    } else {
      for (std::size_t i = positions; i < da; ++i) {
        std::size_t ic = i - positions;

        if (ic > 0) {
          digit_type to_previous =
              a.digits[i] & ((static_cast<digit_type>(1) << remainder) - 1);
          to_previous <<= LOG_BASE - remainder;
          c.digits[ic - 1] += to_previous;
        }

        c.digits[ic] = a.digits[i] >> remainder;
      }

      // Zero out higher digits.
      for (std::size_t i = da - positions; i < da; ++i) c.digits[i] = 0;

      // Most significant positions can now be zero. If so, they need to be
      // deleted.
      c.remove_leading_zeros();
    }
  }
};

}

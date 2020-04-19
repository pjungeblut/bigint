#pragma once

#include <cstdint>

#include "big_unsigned.hpp"

namespace ntlib {

/**
 * Represents a whole number whose size is only limited by the available memory.
 */
class big_integer {
private:
  /**
   * The magnitude of the big_integer.
   */
  big_unsigned magnitude;

  /**
   * The sign.
   */
  enum struct sign_type {
    MINUS = -1,
    PLUS = 1
  };
  sign_type sign;

public:
  /**
   * Default constructor.
   * Initializes with 0.
   */
  big_integer() : magnitude(0), sign(sign_type::PLUS) {}

  /**
   * Constructor to build big_integer from a simple long long.
   *
   * @param n The value to initialize with.
   */
  big_integer(int64_t n) {
    if (n >= 0) {
      magnitude = big_unsigned(n);
      sign = sign_type::PLUS;
    } else {
      magnitude = big_unsigned(-n);
      sign = sign_type::MINUS;
    }
  }

  /**
   * Constructor to build big_integer from a string representation in a given
   * base.
   *
   * @param n The value to initialize with.
   * @param base The base. 2 <= base <= 16. Default is base 10.
   */
  big_integer(const std::string &n, uint8_t base = 10) {
    if (n.size() && n[0] == '-') {
      magnitude = big_unsigned(n.substr(1), base);
      sign = sign_type::MINUS;
    } else {
      magnitude = big_unsigned(n, base);
      sign = sign_type::PLUS;
    }
  }

  /**
   * Copy constructor.
   *
   * @param other Another big_integer.
   */
  big_integer(const big_integer &other) {
    sign = other.sign;
    magnitude = other.magnitude;
  }

  /**
   * Copy assignment operator.
   *
   * @param other Another big_integer.
   * @return Reference to the current big_integer containing the same value as
   *         other.
   */
  big_integer& operator=(const big_integer &other) {
    sign = other.sign;
    magnitude = other.magnitude;
    return *this;
  };

  /**
   * Move constructor.
   *
   * @param other Another big_integer.
   */
  big_integer(big_integer &&other) {
    sign = std::move(other.sign);
    magnitude = std::move(other.magnitude);
  }

  /**
   * Move assignment operator.
   *
   * @param other Another big_integer.
   * @return Reference to the current big_integer containing the value of
   *         other.
   */
  big_integer& operator=(big_integer &&other) {
    sign = std::move(other.sign);
    magnitude = std::move(other.magnitude);
    return *this;
  }

  /**
   * Destructor.
   */
  ~big_integer() = default;

  /**
   * Convert value into a string.
   *
   * @param base The base to print in. 2 <= base <= 16.
   * @return The value as a string in the given base.
   */
  std::string to_string(uint8_t base = 10) const {
    if (sign == sign_type::MINUS) return "-" + magnitude.to_string(base);
    return magnitude.to_string(base);
  }

  /**
   * Hidden friend for stream output.
   *
   * @param os The out stream.
   * @param bu The big_integer to print.
   * @return The out stream.
   */
  friend std::ostream& operator<<(std::ostream &os, const big_integer &bi) {
    os << bi.to_string();
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
  friend bool operator<(const big_integer &a, const big_integer &b){
    // If they have different sign, we can compare their sign.
    if (a.sign != b.sign) return a.sign < b.sign;

    // Otherwise check their absolute values.
    if (a.sign == sign_type::PLUS) return a.magnitude < b.magnitude;
    return a.magnitude > b.magnitude;
  }

  /**
   * Greater than operator.
   *
   * @param a The left hand side to compare.
   * @param b The right hand side to compare.
   * @return True, if and only if a > b.
   */
  friend bool operator>(const big_integer &a, const big_integer &b) {
    return b < a;
  }

  /**
   * Equal operator.
   *
   * @param a The left hand side to compare.
   * @param b The right hand side to compare.
   * @return True, if and only if a == b.
   */
  friend bool operator==(const big_integer &a, const big_integer &b) {
    return a.sign == b.sign && a.magnitude == b.magnitude;
  }

  /**
   * Unequal operator.
   *
   * @param a The left hand side to compare.
   * @param b The right hand side to compare.
   * @return True, if and only if a != b.
   */
  friend bool operator!=(const big_integer &a, const big_integer &b) {
    return !(a == b);
  }

  /**
   * Less than or equal operator.
   *
   * @param a The left hand side to compare.
   * @param b The right hand side to compare.
   * @return True, if and only if a <= b.
   */
  friend bool operator<=(const big_integer &a, const big_integer &b) {
    // If they have different sign, we can compare their sign.
    if (a.sign != b.sign) return a.sign < b.sign;

    // Otherwise check their absolute values.
    if (a.sign == sign_type::PLUS) return a.magnitude <= b.magnitude;
    return a.magnitude >= b.magnitude;
  }

  /**
   * Greater than or equal operator.
   *
   * @param a The left hand side to compare.
   * @param b The right hand side to compare.
   * @return True, if and only if a >= b.
   */
  friend bool operator>=(const big_integer &a, const big_integer &b) {
    return b <= a;
  }

  /**
   * Unary minus to negate value.
   *
   * @return The big integer with the same value but opposite sign.
   */
  friend big_integer operator-(const big_integer &other) {
    big_integer negation;
    negation.sign = other.sign == sign_type::PLUS
        ? sign_type::MINUS : sign_type::PLUS;
    negation.magnitude = other.magnitude;
    return negation;
  }

  /**
   * Addition for big_integer.
   *
   * @param a The first summand.
   * @param b The second summand.
   * @return The sum of a and b.
   */
  template<typename T>
  friend big_integer operator+(const big_integer &a, const T &b) {
    big_integer sum;
    add(a, b, sum);
    return sum;
  }

  /**
   * Adds another big_integer to the current one.
   *
   * @param other The other summand.
   * @return Reference to the current big_integer containing the sum of its
   *         previous value and other.
   */
  template<typename T>
  big_integer& operator+=(const T &other) {
    add(*this, other, *this);
    return *this;
  }

  /**
   * Pre-increment.
   *
   * @return Reference to the current (incremented) object.
   */
  big_integer& operator++() {
    *this += 1;
    return *this;
  }

  /**
   * Post-increment.
   *
   * @return The old value of the object.
   */
  big_integer operator++(int) {
    big_integer copy = *this;
    *this += 1;
    return copy;
  }

  /**
   * Subtraction for big_integer.
   *
   * @param a The minuend.
   * @param b The subtrahend.
   * @return The difference a - b.
   */
  template<typename T>
  friend big_integer operator-(const big_integer &a, const T &b) {
    big_integer difference;
    subtract(a, b, difference);
    return difference;
  }

  /**
   * Subtracts another big_integer from the current one.
   *
   * @param other The subtrahend. Must be at least as big as current value.
   * @return Reference to the current big_integer containing the diffrence of
   *         its previous value and other.
   */
  template<typename T>
  big_integer& operator-=(const T &other) {
    subtract(*this, other, *this);
    return *this;
  }

  /**
   * Pre-decrement.
   *
   * @return Reference to the current (decremented) object.
   */
  big_integer& operator--() {
    *this -= 1;
    return *this;
  }

  /**
   * Post-decrement.
   *
   * @return The old value of the object.
   */
  big_integer operator--(int) {
    big_integer copy = *this;
    *this -= 1;
    return copy;
  }

  /**
   * Multiplication for big_integer.
   *
   * @param a The first factor.
   * @param b The second factor.
   * @return The product a * b.
   */
  template<typename T>
  friend big_integer operator*(const big_integer &a, const T &b) {
    big_integer product;
    mulitply(a, b, product);
    return product;
  }

  /**
   * Multiplies another big_integer to the current one.
   *
   * @param other The other factor.
   * @return Reference to the current big_integer containing the product of its
   *         previous value and other.
   */
  template<typename T>
  big_integer& operator*=(const T &other) {
    // TODO: This creates an intermediate big_integer. Find a way to do the
    //       multiplication in-place.
    big_integer product;
    mulitply(*this, other, product);
    sign = std::move(product.sign);
    magnitude = std::move(product.magnitude);
    return *this;
  }

  /**
   * Divides a by b.
   *
   * @param a The dividend.
   * @param b The divisor.
   * @return The quotient of a div b.
   */
  template<typename T>
  friend big_integer operator/(const big_integer &a, const T &b) {
    big_integer quotient;
    divide(a, b, quotient);
    return quotient;
  }

  /**
   * Divides another big_integer from the current one.
   *
   * @param other The divisor.
   * @return Reference to the current big_integer containing the quotient of
   *         its previous value and other.
   */
  template<typename T>
  big_integer& operator/=(const T &other) {
    big_integer quotient;
    divide(*this, other, quotient);
    sign = std::move(quotient.sign);
    magnitude = std::move(quotient.magnitude);
    return *this;
  }

  /**
   * Computes modulo of a by b.
   *
   * @param a The dividend.
   * @param b The divisor.
   * @return The modulus a mod b.
   */
  template<typename T>
  friend big_integer operator%(const big_integer &a, const T &b) {
    big_integer remainder;
    modulus(a, b, remainder);
    return remainder;
  }

  /**
   * Computes the modulus of another big_integer and the current one.
   *
   * @param other The divisor.
   * @return Reference to the current big_integer containing the modulus of
   *         its previous value and other.
   */
  template<typename T>
  big_integer& operator%=(const T &other) {
    big_integer remainder;
    modulus(*this, other, remainder);
    sign = get_sign(remainder);
    magnitude = get_magnitude(remainder);
    return *this;
  }

  /**
   * Bitwise negation.
   * Negates every digit, including the most significant ones (with its leading
   * zeros).
   *
   * @return A big_integer with bitwise negated digits.
   */
  friend big_integer operator~(const big_integer &a) {
    big_integer negation = a;
    negation.magnitude = ~negation.magnitude;
    return negation;
  }

  /**
   * Bitwise and operator.
   *
   * @param a The first operand.
   * @param b The second operand.
   * @return big_integer that contains the bitwise and of a and b.
   */
  template<typename T>
  friend big_integer operator&(const big_integer &a, const T &b) {
    big_integer bit_and;
    bitwise_and(a, b, bit_and);
    return bit_and;
  }

  /**
   * Bitwise or operator.
   *
   * @param a The first operand.
   * @param b The second operand.
   * @return big_integer that contains the bitwise or of a and b.
   */
  template<typename T>
  friend big_integer operator|(const big_integer &a, const T &b) {
    big_integer bit_or;
    bitwise_or(a, b, bit_or);
    return bit_or;
  }

  /**
   * Bitwise xor operator.
   *
   * @param a The first operand.
   * @param b The second operand.
   * @return big_integer that contains the bitwise or xof a and b.
   */
  template<typename T>
  friend big_integer operator^(const big_integer &a, const T &b) {
    big_integer bit_xor;
    bitwise_xor(a, b, bit_xor);
    return bit_xor;
  }

  /**
   * Bitwise and with another big_integer.
   *
   * @param other The other big_integers.
   * @return Reference to the current big_integer after bitwise and with
   *         another big_integer.
   */
  template<typename T>
  big_integer& operator&=(const T &other) {
    bitwise_and(*this, other, *this);
    return *this;
  }

  /**
   * Bitwise or with another big_integer.
   *
   * @param other The other big_integers.
   * @return Reference to the current big_integer after bitwise or with
   *         another big_integer.
   */
  template<typename T>
  big_integer& operator|=(const T &other) {
    bitwise_or(*this, other, *this);
    return *this;
  }

  /**
   * Bitwise xor with another big_integer.
   *
   * @param other The other big_integers.
   * @return Reference to the current big_integer after bitwise xor with
   *         another big_integer.
   */
  template<typename T>
  big_integer& operator^=(const T &other) {
    bitwise_xor(*this, other, *this);
    return *this;
  }

  /**
   * Left shift operator.
   *
   * @param a The big_integer to be shifted.
   * @param b The amount of binary positions to shift.
   * @return A big_integer with a's value left shifted by b binary positions.
   */
  friend big_integer operator<<(const big_integer &a, uint64_t b) {
    big_integer shifted;
    big_integer::left_shift(a, b, shifted);
    return shifted;
  }

  /**
   * Right shift operator.
   *
   * @param a The big_integer to be shifted.
   * @param b The amount of binary positions to shift.
   * @return A big_integer with a's value left shifted by b binary positions.
   */
  friend big_integer operator>>(const big_integer &a, uint64_t b) {
    big_integer shifted;
    big_integer::right_shift(a, b, shifted);
    return shifted;
  }

  /**
   * Left shift assignment operator.
   *
   * @param b The amount of binary positions to shift.
   * @return A reference to the current big_integer after being shifted by b
   *         binary positions.
   */
  big_integer& operator<<=(uint64_t b) {
    left_shift(*this, b, *this);
    return *this;
  }

  /**
   * Right shift assignment operator.
   *
   * @param b The amount of binary positions to shift.
   * @return A reference to the current big_integer after being shifted by b
   *         binary positions.
   */
  big_integer& operator>>=(uint64_t b) {
    right_shift(*this, b, *this);
    return *this;
  }

  /**
   * Cast to int64_t.
   * Only returns the last digit.
   *
   * @return The value of the least significant digit.
   */
  explicit operator std::size_t() const {
    return (std::size_t)magnitude;
  }

  /**
   * Cast to bool.
   */
  explicit operator bool() const {
    return (bool)magnitude;
  }

private:
  /**
   * Returns the sign of a big_integer.
   *
   * @param n The number.
   * @return The sign of n.
   */
  static sign_type get_sign(const big_integer &n) {
    return n.sign;
  }

  /**
   * Returns the sign of a number.
   *
   * @param n The number.
   * @return The sign of n.
   */
  static sign_type get_sign(int64_t n) {
    return n >= 0 ? big_integer::sign_type::PLUS : big_integer::sign_type::MINUS;
  }

  /**
   * Returns the magnitude of a big_integer.
   *
   * @param n The number.
   * @return Its magnitude.
   */
  static big_unsigned get_magnitude(const big_integer &n) {
    return n.magnitude;
  }

  /**
   * Returns the magnitude of a number.
   *
   * @param n The number.
   * @return Its magnitude, abs(n).
   */
  static uint64_t get_magnitude(uint64_t n) {
    return abs(n);
  }

  /**
   * Adds two big_integers into a third.
   * Parameters a and c may be the same, for a += b.
   *
   * @param a The first summand.
   * @param b The second summand.
   * @param c The sum a + b.
   */
  template<typename T>
  static void add(const big_integer &a, const T &b, big_integer &c) {
    sign_type sa = get_sign(a);
    sign_type sb = get_sign(b);

    if (sa == sb) {
      c.sign = sa;
      c.magnitude = a.magnitude + get_magnitude(b);
    } else if (sa == sign_type::PLUS) {
      if (a.magnitude >= get_magnitude(b)) {
        c.sign = sign_type::PLUS;
        c.magnitude = a.magnitude - get_magnitude(b);
      } else {
        c.sign = sign_type::MINUS;
        c.magnitude = get_magnitude(b) - a.magnitude;
      }
    } else {
      if (a.magnitude > get_magnitude(b)) {
        c.sign = sign_type::MINUS;
        c.magnitude = a.magnitude - get_magnitude(b);
      } else {
        c.sign = sign_type::PLUS;
        c.magnitude = get_magnitude(b) - a.magnitude;
      }
    }
  }

  /**
   * Subtracts two big_integers into a third.
   * Parameters a and c may be the same, for a -= b.
   *
   * @param a The minuend.
   * @param b The subtrahend.
   * @param c The difference a - b.
   */
  template<typename T>
  static void subtract(const big_integer &a, const T &b, big_integer &c) {
    sign_type sa = get_sign(a);
    sign_type sb = get_sign(b);

    if (sb == sign_type::MINUS) add(a, -b, c);
    else {
      if (sa == sign_type::PLUS) {
        if (a.magnitude >= get_magnitude(b)) {
          c.sign = sign_type::PLUS;
          c.magnitude = a.magnitude - get_magnitude(b);
        } else {
          c.sign = sign_type::MINUS;
          c.magnitude = get_magnitude(b) - a.magnitude;
        }
      } else {
        c.sign = sign_type::MINUS;
        c.magnitude = a.magnitude + get_magnitude(b);
      }
    }
  }

  /**
   * Multiplies two big_integers into a third.
   * Parameters a and c may NOT be the same, as the content of c gets erased.
   *
   * @param a The first factor.
   * @param b The second factor.
   * @param c The product a * b.
   */
  template<typename T>
  static void mulitply(const big_integer &a, const T &b, big_integer &c) {
    c.magnitude = a.magnitude * get_magnitude(b);
    c.sign = a.sign == get_sign(b) ? sign_type::PLUS : sign_type:: MINUS;
  }

  /**
   * Divides a by a digit b and outputs the quotient.
   *
   * @param a The dividend.
   * @param b The divisor.
   * @param c The quotient a div b.
   */
  template<typename T>
  static void divide(const big_integer &a, const T &b, big_integer &c) {
    c.magnitude = a.magnitude / get_magnitude(b);
    c.sign = a.sign == get_sign(b) ? sign_type::PLUS : sign_type:: MINUS;
  }

  /**
   * Computes the modulus of a and b.
   *
   * @param a The dividend.
   * @param b The divisor.
   * @param c The remainder a mod b.
   */
  template<typename T>
  static void modulus(const big_integer &a, const T &b, big_integer &c) {
    c.magnitude = a.magnitude % get_magnitude(b);
    c.sign = a.sign;
  }

  /**
   * Bitwise and of a and b into c.
   * Parameters a and c may be the same.
   *
   * @param a The first operand.
   * @param b The second operand.
   * @param c The bitwise and c = a & b.
   */
  template<typename T>
  static void bitwise_and(const big_integer &a, const T &b, big_integer &c) {
    c.magnitude = a.magnitude & get_magnitude(b);
    c.sign = a.sign;
  }

  /**
   * Bitwise or of a and b into c.
   * Parameters a and c may be the same.
   *
   * @param a The first operand.
   * @param b The second operand.
   * @param c The bitwise or c = a | b.
   */
  template<typename T>
  static void bitwise_or(const big_integer &a, const T &b, big_integer &c) {
    c.magnitude = a.magnitude | get_magnitude(b);
    c.sign = a.sign;
  }

  /**
   * Bitwise xor of a and b into c.
   * Parameters a and c may be the same.
   *
   * @param a The first operand.
   * @param b The second operand.
   * @param c The bitwise or c = a ^ b.
   */
  template<typename T>
  static void bitwise_xor(const big_integer &a, const T &b, big_integer &c) {
    c.magnitude = a.magnitude ^ get_magnitude(b);
    c.sign = a.sign;
  }

  /**
   * Left shift of a by b binary positions into c.
   * Parameters a and c may be the same.
   *
   * @param a The number to be shifted.
   * @param b The amount of binary positions to shift.
   * @param c The result c = a << b.
   */
  static void left_shift(const big_integer &a, uint64_t b, big_integer &c) {
    c.magnitude = a.magnitude << b;
    c.sign = a.sign;
  }

  /**
   * Right shift of a by b binary positions into c.
   * Parameters a and c may be the same.
   *
   * @param a The number to be shifted.
   * @param b The amount of binary positions to shift.
   * @param c The result c = a >> b.
   */
  static void right_shift(const big_integer &a, int64_t b, big_integer &c) {
    c.magnitude = a.magnitude >> b;
    c.sign = a.sign;
  }
};

}

#include <cassert>
#include <cstdio>
#include <iostream>
#include <string>
#include <utility>

#include "big_unsigned.hpp"

void test_initialization() {
  // Default constructor.
  ntlib::big_unsigned a;

  // Construct by unsigned int.
  ntlib::big_unsigned b(5);

  // Construct by string.
  ntlib::big_unsigned c("12345678910111213141516171819");

  // Copy construction.
  ntlib::big_unsigned d(c);

  // Copy assignment.
  a = c;

  // Move construction.
  ntlib::big_unsigned f(std::move(b));

  // Move assignment.
  b = std::move(c);
}

void test_to_string() {
  ntlib::big_unsigned a("123456789101112");
  assert(a.to_string() == "123456789101112");
  assert(a.to_string(2) == "11100000100100010000110000011110011101000111000");
  assert(a.to_string(16) == "7048860F3A38");

  ntlib::big_unsigned b("11100000100100010000110000011110011101000111000", 2);
  assert(b.to_string() == "123456789101112");
  assert(b.to_string(2) == "11100000100100010000110000011110011101000111000");
  assert(b.to_string(16) == "7048860F3A38");
}

void test_comparison() {
 ntlib::big_unsigned a("12345678910111213141516171819");
 ntlib::big_unsigned b("19181716151413121110987654321");
 ntlib::big_unsigned c("12345678910111213141516171819");
 assert(a == a);
 assert(a == c);
 assert(a != b);
 assert(a < b);
 assert(b > a);
 assert(a <= b);
 assert(b >= a);
 assert(a <= c);
 assert(c >= a);
}

void test_addition() {
  // Exactly at the overflow boundary for a classical 32 bit unsigned int.
  ntlib::big_unsigned a(4294967295);
  ntlib::big_unsigned b(1);

  ntlib::big_unsigned a_plus_b = a + b;
  assert(a_plus_b.to_string() == "4294967296");
  ntlib::big_unsigned a_plus_1 = a + 1u;
  assert(a_plus_1.to_string() == "4294967296");
  a_plus_1 = 1u + a;
  assert(a_plus_1.to_string() == "4294967296");

  ntlib::big_unsigned c;
  c += a;
  assert(c.to_string() == "4294967295");
  c += 1;
  assert(c.to_string() == "4294967296");

  assert((a++).to_string() == "4294967295");
  assert(a.to_string() == "4294967296");
  assert((++a).to_string() == "4294967297");

  ntlib::big_unsigned d(0);
  ntlib::big_unsigned e(0);
  assert((d + e).to_string() == "0");
  assert((d + 0).to_string() == "0");
  assert((0 + e).to_string() == "0");
}

void test_subtraction() {
  // Exactly at the overflow boundary for a classical 32 bit unsigned int.
  ntlib::big_unsigned a("4294967296");
  ntlib::big_unsigned b(1);
  ntlib::big_unsigned c;

  assert((a - b).to_string() == "4294967295");
  assert((a - 1).to_string() == "4294967295");

  try {
    b - a;
    assert(false);
  } catch (const std::exception& e) {}

  try {
    c - b;
    assert(false);
  } catch (const std::exception& e) {}

  try {
    c - 1;
    assert(false);
  } catch (const std::exception& e) {}

  assert((a - 0).to_string() == "4294967296");
  assert((c - 0).to_string() == "0");

  assert((a -= c).to_string() == "4294967296");
  assert((b -= 1).to_string() == "0");

  assert((a--).to_string() == "4294967296");
  assert(a.to_string() == "4294967295");
  assert((--a).to_string() == "4294967294");
}

void test_multiplication() {
  ntlib::big_unsigned fac100 = 1;
  for (uint32_t i = 2; i <= 100; ++i) {
    fac100 *= i;
  }
  assert(fac100.to_string() == "93326215443944152681699238856266700490715968264"
      "381621468592963895217599993229915608941463976156518286253697920827223758"
      "251185210916864000000000000000000000000");

  assert((fac100 * fac100).to_string() == "870978248908948007941659016194448586"
      "556972064394084013421593253624337999634658332587796709633275492064469038"
      "076221960747636428941143592019057396067750788139460748990533172975801343"
      "299298718476460737588943431348338296680151515628085416269176619573749317"
      "3453603519594496000000000000000000000000000000000000000000000000");

  assert((fac100 * 0).to_string() == "0");
  assert((0 * fac100).to_string() == "0");
  ntlib::big_unsigned zero;
  assert((fac100 * zero).to_string() == "0");
  assert((zero * fac100).to_string() == "0");
  assert((zero * zero).to_string() == "0");
}

void test_division() {
  ntlib::big_unsigned fac50("30414093201713378043612608166064768844377641568960"
      "512000000000000");
  ntlib::big_unsigned fac100("8709782489089480079416590161944485865569720643940"
      "840134215932536243379996346583325877967096332754920644690380762219607476"
      "364289411435920190573960677507881394607489905331729758013432992987184764"
      "607375889434313483382966801515156280854162691766195737493173453603519594"
      "496000000000000000000000000000000000000000000000000");

  ntlib::big_unsigned a = fac50 / fac100;
  assert(a.to_string() == "0");
  ntlib::big_unsigned b = fac100 / fac50;
  assert(b * fac50 == fac100);

  try {
    fac100 / 0;
    assert(false);
  } catch (const std::exception& e) {}

  try {
    ntlib::big_unsigned zero;
    fac100 / zero;
    assert(false);
  } catch (const std::exception& e) {}
}

void test_modulo() {
  ntlib::big_unsigned fac50("30414093201713378043612608166064768844377641568960"
      "512000000000000");
  ntlib::big_unsigned fac100("8709782489089480079416590161944485865569720643940"
      "840134215932536243379996346583325877967096332754920644690380762219607476"
      "364289411435920190573960677507881394607489905331729758013432992987184764"
      "607375889434313483382966801515156280854162691766195737493173453603519594"
      "496000000000000000000000000000000000000000000000000");

  assert((fac100 % 17).to_string() == "0");
  assert((fac100 % fac50).to_string() == "0");
}

void test_logical_operators() {
  ntlib::big_unsigned zero(0);
  ntlib::big_unsigned one(1);

  assert(!zero);
  assert(!!one);
  assert(one && one);
  assert(one || zero);
  assert(!(zero || zero));
}

void test_bitwise_operations() {
  ntlib::big_unsigned a("12345678910111213141516171819");
  ntlib::big_unsigned zero(0);
  assert(a == ~~a);
  assert((a & zero).to_string() == "0");
  assert((a | zero) == a);
}

void test_shifts() {
  ntlib::big_unsigned a("12345678910111213141516171819");
  a <<= 1;
  assert(a.to_string() == "24691357820222426283032343638");
  a <<= 59;
  assert(a.to_string() == "14233598704438438575649355172504444699456569344");
  a >>= 1;
  assert(a.to_string() == "7116799352219219287824677586252222349728284672");
  a >>= 59;
  assert(a.to_string() == "12345678910111213141516171819");

  ntlib::big_unsigned b("19181716151413121110987654321");
  b <<= 64;
  assert(b.to_string() == "353840208739658780121431168289984594912514932736");
  b >>= 32;
  b >>= 32;
  assert(b.to_string() == "19181716151413121110987654321");
}

int main() {
  test_initialization();
  test_to_string();
  test_comparison();
  test_addition();
  test_subtraction();
  test_multiplication();
  test_division();
  test_modulo();
  test_logical_operators();
  test_bitwise_operations();
  test_shifts();

  return 0;
}

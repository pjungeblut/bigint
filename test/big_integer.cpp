#include <gtest/gtest.h>

#include "big_integer.hpp"

TEST(Initialization, DefaultConstructor) {
  bigint::big_integer a;
  EXPECT_EQ(a, 0);
}

TEST(Initialization, ConstructByInt) {
  bigint::big_integer a(5);
  EXPECT_EQ(a, 5);
}

TEST(Initialization, ConstructByString) {
  bigint::big_integer a("12345678910111213141516171819");
  EXPECT_EQ(a.to_string(), "12345678910111213141516171819");
}

TEST(Initialization, Copy) {
  bigint::big_integer a(5);
  bigint::big_integer b(a);
  EXPECT_EQ(b, 5);

  bigint::big_integer c;
  c = a;
  EXPECT_EQ(c, 5);
}

TEST(Initialization, Move) {
  bigint::big_integer a(5);
  bigint::big_integer b(std::move(a));
  EXPECT_EQ(b, 5);

  bigint::big_integer c(5);
  bigint::big_integer d;
  d = std::move(c);
  EXPECT_EQ(d, 5);
}

TEST(StringConversion, Base10) {
  bigint::big_integer a("-123456789101112");
  EXPECT_EQ(a.to_string(), "-123456789101112");
  EXPECT_EQ(a.to_string(2), "-11100000100100010000110000011110011101000111000");
  EXPECT_EQ(a.to_string(16), "-7048860F3A38");
}

TEST(StringConversion, Base2) {
  bigint::big_integer b("11100000100100010000110000011110011101000111000", 2);
  EXPECT_EQ(b.to_string(), "123456789101112");
  EXPECT_EQ(b.to_string(2), "11100000100100010000110000011110011101000111000");
  EXPECT_EQ(b.to_string(16), "7048860F3A38");
}

TEST(Arithmetic, Addition) {
  // Exactly at the overflow boundary for a classical 32 bit unsigned int.
  bigint::big_integer a(-4294967295);
  bigint::big_integer b(-1);

  bigint::big_integer a_plus_b = a + b;
  EXPECT_EQ(a_plus_b.to_string(), "-4294967296");
  bigint::big_integer a_plus_1 = a + 1u;
  EXPECT_EQ(a_plus_1.to_string(), "-4294967294");
  a_plus_1 = 1u + a;
  EXPECT_EQ(a_plus_1.to_string(), "-4294967294");

  bigint::big_integer c;
  c += a;
  EXPECT_EQ(c.to_string(), "-4294967295");
  c += 1;
  EXPECT_EQ(c.to_string(), "-4294967294");

  EXPECT_EQ((a++).to_string(), "-4294967295");
  EXPECT_EQ(a.to_string(), "-4294967294");
  EXPECT_EQ((++a).to_string(), "-4294967293");

  bigint::big_integer d(0);
  bigint::big_integer e(0);
  EXPECT_EQ((d + e).to_string(), "0");
  EXPECT_EQ((d + 0).to_string(), "0");
  EXPECT_EQ((0 + e).to_string(), "0");
}

TEST(Arithmetic, Subtraction) {
  // Exactly at the overflow boundary for a classical 32 bit unsigned int.
  bigint::big_integer a("4294967296");
  bigint::big_integer b(1);
  bigint::big_integer c;

  EXPECT_EQ((a - b).to_string(), "4294967295");
  EXPECT_EQ((a - 1).to_string(), "4294967295");

  EXPECT_EQ((b - a).to_string(), "-4294967295");
  EXPECT_EQ((c - b).to_string(), "-1");
  EXPECT_EQ((c - 1).to_string(), "-1");

  EXPECT_EQ((a - 0).to_string(), "4294967296");
  EXPECT_EQ((c - 0).to_string(), "0");

  EXPECT_EQ((a -= c).to_string(), "4294967296");
  EXPECT_EQ((b -= 1).to_string(), "0");

  EXPECT_EQ((a--).to_string(), "4294967296");
  EXPECT_EQ(a.to_string(), "4294967295");
  EXPECT_EQ((--a).to_string(), "4294967294");
}

TEST(Arithmetic, Multiplication) {
  bigint::big_integer fac100 = 1;
  for (uint32_t i = 2; i <= 100; ++i) {
    fac100 *= i;
  }
  EXPECT_EQ(fac100.to_string(), "9332621544394415268169923885626670049071596826"
      "438162146859296389521759999322991560894146397615651828625369792082722375"
      "8251185210916864000000000000000000000000");

  EXPECT_EQ((fac100 * fac100).to_string(), "87097824890894800794165901619444858"
      "655697206439408401342159325362433799963465833258779670963327549206446903"
      "807622196074763642894114359201905739606775078813946074899053317297580134"
      "329929871847646073758894343134833829668015151562808541626917661957374931"
      "73453603519594496000000000000000000000000000000000000000000000000");

  EXPECT_EQ((fac100 * 0).to_string(), "0");
  EXPECT_EQ((0 * fac100).to_string(), "0");
  bigint::big_integer zero;
  EXPECT_EQ((fac100 * zero).to_string(), "0");
  EXPECT_EQ((zero * fac100).to_string(), "0");
  EXPECT_EQ((zero * zero).to_string(), "0");
}

TEST(Arithmetic, Division) {
  bigint::big_integer fac50("3041409320171337804361260816606476884437764156896"
      "0512000000000000");
  bigint::big_integer fac100("870978248908948007941659016194448586556972064394"
      "084013421593253624337999634658332587796709633275492064469038076221960747"
      "636428941143592019057396067750788139460748990533172975801343299298718476"
      "460737588943431348338296680151515628085416269176619573749317345360351959"
      "4496000000000000000000000000000000000000000000000000");

  bigint::big_integer a = fac50 / fac100;
  EXPECT_EQ(a.to_string(), "0");
  bigint::big_integer b = fac100 / fac50;
  EXPECT_EQ(b * fac50, fac100);

  EXPECT_ANY_THROW(fac100 / 0);
  bigint::big_integer zero;
  EXPECT_ANY_THROW(fac100 / zero);
}

TEST(Arithmetic, Modulo) {
  bigint::big_integer fac50("3041409320171337804361260816606476884437764156896"
      "0512000000000000");
  bigint::big_integer fac100("870978248908948007941659016194448586556972064394"
      "084013421593253624337999634658332587796709633275492064469038076221960747"
      "636428941143592019057396067750788139460748990533172975801343299298718476"
      "460737588943431348338296680151515628085416269176619573749317345360351959"
      "4496000000000000000000000000000000000000000000000000");

  EXPECT_EQ((fac100 % 17).to_string(), "0");
  EXPECT_EQ((fac100 % fac50).to_string(), "0");

  EXPECT_ANY_THROW(fac100 % 0);
  bigint::big_integer zero;
  EXPECT_ANY_THROW(fac100 % zero);
}

TEST(Operators, Comparison) {
  bigint::big_integer a("12345678910111213141516171819");
  bigint::big_integer b("19181716151413121110987654321");
  bigint::big_integer c("12345678910111213141516171819");
  bigint::big_integer d("-12345678910111213141516171819");
  EXPECT_TRUE(a != d);
  EXPECT_TRUE(a == a);
  EXPECT_TRUE(a == c);
  EXPECT_TRUE(a != b);
  EXPECT_TRUE(a < b);
  EXPECT_TRUE(b > a);
  EXPECT_TRUE(a <= b);
  EXPECT_TRUE(b >= a);
  EXPECT_TRUE(a <= c);
  EXPECT_TRUE(c >= a);
}

TEST(Operators, Logic) {
  bigint::big_integer zero(0);
  bigint::big_integer one(1);

  EXPECT_TRUE(!zero);
  EXPECT_TRUE(!!one);
  EXPECT_TRUE(one && one);
  EXPECT_TRUE(one || zero);
  EXPECT_TRUE(!(zero || zero));
}

TEST(Operators, Bitwise) {
  bigint::big_integer a("12345678910111213141516171819");
  bigint::big_integer zero(0);
  EXPECT_EQ(a, ~~a);
  EXPECT_EQ((a & zero).to_string(), "0");
  EXPECT_EQ((a | zero), a);
}

TEST(Operators, Shifts) {
  bigint::big_integer a("12345678910111213141516171819");
  a <<= 1;
  EXPECT_EQ(a.to_string(), "24691357820222426283032343638");
  a <<= 59;
  EXPECT_EQ(a.to_string(), "14233598704438438575649355172504444699456569344");
  a >>= 1;
  EXPECT_EQ(a.to_string(), "7116799352219219287824677586252222349728284672");
  a >>= 59;
  EXPECT_EQ(a.to_string(), "12345678910111213141516171819");

  bigint::big_integer b("19181716151413121110987654321");
  b <<= 64;
  EXPECT_EQ(b.to_string(), "353840208739658780121431168289984594912514932736");
  b >>= 32;
  b >>= 32;
  EXPECT_EQ(b.to_string(), "19181716151413121110987654321");
}

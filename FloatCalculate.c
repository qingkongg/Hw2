#include "FloatCalculate.h"
#include <stdbool.h>
#include<math.h>

const size_t SIGN_BIT = 1;
const size_t EXPONENT_BITS = 8;
const size_t MANTISSA_BITS = 23;

static int32_t get_norm_bias(void) { return 1 - (1 << (EXPONENT_BITS - 1)); }

static int32_t get_denorm_bias(void) { return 1 + get_norm_bias(); }

static bool test_rightmost_all_zeros(uint32_t number, size_t bits) {
  uint32_t mask = (1ull << bits) - 1;
  return (number & mask) == 0;
}

static bool test_rightmost_all_ones(uint32_t number, size_t bits) {
  uint32_t mask = (1ull << bits) - 1;
  return (number & mask) == mask;
}

// You can also design a function of your own.
static void build_bitstring(Float input, char *output){
    output[0] = input.sign;
    if(input.type!=DENORMALIZED_T){     
      uint32_t quotient = input.exponent + 127;
      uint32_t remainder = quotient % 2;
      int round = 0;
      while(quotient>=2){
        
        if(remainder == 1){
          output[8-round] = 1;
        }
        round++;
        quotient /= 2;
        remainder = quotient % 2;
      }
      if(remainder == 1){
          output[8-round] = 1;
      }
    }
    float tail = input.mantissa;
    if(input.type == NORMALIZED_T){
      tail -= 1;
    }
    for(int i = 1;i < 24;i ++){
      if(input.mantissa >= pow(0.5,i)){
        output[i+8] = 1;
      }
    }
}

// You can also design a function of your own.
static Float parse_bitstring(const char *input){
  Float output;
  output.sign =(uint32_t)input[0];
  int ex = 0;
  for(int i = 0;i < 9;i++){
    ex += input[8-i] * pow(2,i);
  }
  if(test_rightmost_all_zeros(ex,8)){
    output.type = DENORMALIZED_T;
    output.exponent = ex - 126;
  }
  else{
    output.exponent = ex-127;
  }
   for(int i = 1;i<24;i++){
      output.mantissa += input[8+i] * pow(0.5,i);
  }
  if(output.mantissa == NORMALIZED_T){
    output.mantissa += 1;
  }
  return output;
}

// You can also design a function of your own.
static Float float_add_impl(Float a, Float b){
  Float output;
  int difference;
  if(a.sign != b.sign && a.exponent == b.exponent && a.mantissa && b.mantissa){
    output.type = ZERO_T;
    return output;
  }
  if((a.exponent < b.exponent) ||(a.exponent == b.exponent && a.mantissa < b.mantissa)){
    output = a;
    a = b;
    b = output;
  }
  output.sign = a.sign;
  if(a.sign == b.sign){
    if(a.type != DENORMALIZED_T && b.type != DENORMALIZED_T){
      difference = a.exponent - b.exponent;
      b.mantissa = b.mantissa / pow(2,difference);
      output.mantissa = (a.mantissa + b.mantissa) / 2;
      output.exponent = a.exponent + 1;
      output.type = a.type;
    }
    else if(b.type == DENORMALIZED_T){
      difference = a.exponent - get_denorm_bias();
      b.mantissa = b.mantissa / pow(2,difference);
      output.mantissa = a.mantissa + b.mantissa;
      if(output.mantissa >= 2){
        if(test_rightmost_all_ones(output.exponent - get_norm_bias(),8)){
          output.type = INFINITY_T;
          return output;
        }
        output.mantissa = output.mantissa / 2;
        output.exponent = a.exponent + 1;
      }
    }
  }
  else if(a.sign!=b.sign){
    if(a.type == NORMALIZED_T && b.type == NORMALIZED_T){
      difference = a.exponent - b.exponent;
      b.mantissa = b.mantissa / pow(2,difference);
      output.exponent = a.exponent;
      output.mantissa = a.mantissa - b.mantissa;
    }
    else if(b.type == DENORMALIZED_T){
      difference = a.exponent - get_denorm_bias();
      b.mantissa = b.mantissa / pow(2,difference);
      output.exponent = a.exponent;
      output.mantissa = a.mantissa - b.mantissa;
    }
    while(output.mantissa < 1){
      output.mantissa *= 2;
      output.exponent -= 1;
      if(output.mantissa < 1 && output.exponent == get_denorm_bias()){
        output.type = DENORMALIZED_T;
        break;
      }
    }
  }
  return output;
}

// You should not modify the signature of this function
void float_add(const char *a, const char *b, char *result) {
  // TODO: Implement this function
  // A possible implementation of the function:
  Float fa = parse_bitstring(a);
  Float fb = parse_bitstring(b);
  Float fresult = float_add_impl(fa, fb);
  build_bitstring(fresult, result);
}

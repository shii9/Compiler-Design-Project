/*
 * EXAMPLE 1: Function with Loop
 * ==============================
 *
 * This example demonstrates:
 * - Custom function definition with 'Fn' suffix
 * - Function parameters and return values
 * - Variable naming pattern: _[letters][digit][letters]
 * - Loop label syntax: loop_<name><2digits>:
 * - While loop with condition
 * - Printf statement
 * - Break statement
 * - Statement terminator: .. (double dot)
 *
 * Expected Token Sequence:
 * I T F B T V B B T V O V O N S R V S B
 * T M B B B T V O N S T V O F B V B S
 * L W B T V O N S B B P B V B S K S B R N S B
 *
 * Expected Result: ACCEPTED ✓
 */

#include <stdio.h>

dec computeValueFn(dec _val1a) { dec _temp2x = _val1a + 5.. return _temp2x.. }

int main() {
  dec _input3k = 10.. dec _result4m = computeValueFn(_input3k)..

                                      loop_main01
      : while (dec _loopin0x < 3..) {
    printf(_result4m)..break..
  }

  return 0..
}

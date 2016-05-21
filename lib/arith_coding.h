#pragma once

/** Arithmetic Coding state structure */
typedef struct 
{
  /** character probability table */
  int* prob_table;
  /** cumulative probabilities table */
  int* cumul_table;
  /** size of fractionnal part */ 
  int  frac_size;
  
  /** one-chain counter */
  int one_counter;

  /** last encoded symbol */
  int last_symbol;

  /** current symbol */
  unsigned char current_symbol;
  int current_index;

  int out_index;

  /** encoding range base value */
  int base;
  /** encoding range size */
  int length;
} ac_state_t;

/** Initialize arithmetic coding state stucture
 *  @p state structure to be initialized
 *  @p precision fixed-point precision to used in computation
 */
void init_state(ac_state_t* state, int precision);

/** build the probability table in @p state using a reference input
 *  @p state Arithmetic Coding state containing the probability table 
 *  @p in    reference input to be used for probability init
 *  @p size  number of byte to be read from @p in
 */
void build_probability_table(ac_state_t* state, unsigned char* in, int size); 

/** Display the probability table of @p state */
void display_prob_table(ac_state_t* state); 

/** Arithmetic coding of a byte-array 
 *  @p out byte-array used as output stream
 *  @p in input byte-array
 *  @p size number of bytes in @p in
 *  @p state arithmetic coder state and parameters
 */
void encode_value(unsigned char* out, unsigned char* in, size_t size, ac_state_t* state); 

/** Arithmetic decode the value in @p in according to the encoder defined by @p state, assuming @p 
 *  expected_size characters should be decoded, writting them to @p out
 */
void decode_value(unsigned char* out, unsigned char* in, ac_state_t* state, size_t expected_size); 

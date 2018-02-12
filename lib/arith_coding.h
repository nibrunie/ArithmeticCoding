#pragma once

/** \defgroup airht_coding arithmetic coding
 *  \brief Arithmetic coding encoding and decoding functions
 *   @{
 */

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
  /** deprecated index */
  int current_index;
  /** position of the next bit to be outputed */
  int out_index;
  /** last encoded symbol */
  int last_symbol;
  /** current symbol */
  unsigned char current_symbol;
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
void build_probability_table(ac_state_t* state, const unsigned char* in, int size);

/** Reset the probability table of the Arithmetic Coder state
 *  by giving equi-probable uniform probability to each byte
 *  @param state  arith coding state to reset
 */
void reset_uniform_probability(ac_state_t* state);

/** transform occurency count (in prob_table) to cumulative probability
 *  distribution, assuming the sum of occurences is equal to
 *  size + alphabet_size (where alphabet_size stands for the initial
 *  count of 1 for each alphabet symbol)
 *  @param state internal arithmetic coding state
 *  @param size number of occurences (+ alphabet_size) to account for
 */
void transform_count_to_cumul(ac_state_t* state, int size);

/** Reset the probability table (to be used as count table)
 *  so it contains a count equal to 1 for each entry (minimal possible
 *  value)
 *  @param state internal encoding state
 */
void reset_prob_table(ac_state_t* state);

/** Display the probability table of @p state */
void display_prob_table(ac_state_t* state);

/** Arithmetic coding of a byte-array
 *  @p out byte-array used as output stream
 *  @p in input byte-array
 *  @p size number of bytes in @p in
 *  @p state arithmetic coder state and parameters
 */
void encode_value(unsigned char* out, const unsigned char* in,
                  size_t size, ac_state_t* state);

/** Arithmetic Coding of one byte
 *  @p out byte array to be used as output stream
 *  @p in  byte to be coded
 *  @p state Arithmetic Coder state
 *  @return unused
 */
void encode_character(unsigned char* out, unsigned char in, ac_state_t* state);

/** Select a final numerical value to terminate encoding
 *  (flushing internal state)
 *  @param out output stream
 *  @param state internal arithmetic encoding state
 */
void select_value(unsigned char* out, ac_state_t* state);

/** Initialize arithmetic coding state to decode @p in
 *  @param in input value buffer
 *  @param internal arithmetic decoding state to be initialized
 */
void init_decoding(unsigned char* in, ac_state_t* state);

/** Decode a single character (byte) from the input buffer
 *  @param in input buffer (numerical encoded value)
 *  @param state arithmetic decoding internal state
 *  @return decoded byte value
 */
unsigned char decode_character( unsigned char* in, ac_state_t* state);

/** Arithmetic decode the value in @p in according to the encoder defined
 *  by @p state, assuming @p
 *  expected_size characters should be decoded, writting them to @p out
 */
void decode_value(unsigned char* out, unsigned char* in, ac_state_t* state,
                  size_t expected_size);

/** Arihtmetic coding of a byte-array with regular update to the probability
 *  table (initialized to uniform probabilities)
 *  @param out byte-array used as output stream for the numerical code value
 *  @param in input byte-array
 *  @param size number of bytes in @p in
 *  @param state arithmetic coder state (parameter + internal state)
 *  @param update_range number of input byte encoded between cumulative
 *                      probability update
 *  @param range_clear enable(1) / disable(0) the clear of probability count
 *                     when updating cumulative table
 */
void encode_value_with_update(unsigned char* out, unsigned char* in,
                              size_t size, ac_state_t* state, int update_range,
                              int range_clear);

/** Arihtmetic coding of a byte-array with regular update to the probability
 *  table (initialized to uniform probabilities)
 *  @param out byte-array used as output stream
 *  @param in input byte-array containing the numerical code value
 *  @param expected_size number of bytes expected to be decoded in @p in
 *  @param state arithmetic coder state (parameter + internal state)
 *  @param update_range number of input byte encoded between cumulative
 *                      probability update
 *  @param range_clear enable(1) / disable(0) the clear of probability count
                       when updating cumulative table
 */
void decode_value_with_update(unsigned char* out, unsigned char* in,
                              ac_state_t* state, size_t expected_size,
                              int update_range, int range_clear);

/** @} */

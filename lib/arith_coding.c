#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "arith_coding.h"

void init_state(ac_state_t* state, int precision) {
  state->prob_table = malloc(sizeof(int) * 256);
  state->cumul_table = malloc(sizeof(int) * 257);

  state->frac_size = precision;

  state->one_counter = 0;
  state->last_symbol = -1;

  state->current_symbol = 0;
  state->current_index  = 0;

  state->out_index = 0;

  state->base = 0;
  state->length = (1 << precision) - 1;

  assert(state->prob_table && state->cumul_table && "memeory allocation failed");
}

void transform_count_to_cumul(ac_state_t* state, int size) {
  int alphabet_size = 256;
  int i;

  for (i = 0; i < 256; ++i) {
    int count = state->prob_table[i];
    state->prob_table[i] = ((long long) count * (1 << state->frac_size)) / (size + alphabet_size);
    //if (i == 0) state->cumul_table[0] = state->prob_table[0];
    //else state->cumul_table[i] = state->cumul_table[i-1] + state->prob_table[i];
    if (i == 0) {
      state->cumul_table[0] = 0;
      state->cumul_table[1] = state->prob_table[0];
    }
    else state->cumul_table[i+1] = state->cumul_table[i] + state->prob_table[i];
  }
}

void build_probability_table(ac_state_t* state, unsigned char* in, int size) 
{
  int alphabet_size = 256;
  int count_weight = 16;
  int i;
  // reset 
  for (i = 0; i < 256; ++i) state->prob_table[i] = 1;

  // occurences counting
  for (i = 0; i < size; ++i) state->prob_table[in[i]] += count_weight;

  // normalization according to state format
  transform_count_to_cumul(state, count_weight * size);

}

void reset_uniform_probability(ac_state_t* state)
{
  int alphabet_size = 256;
  int size = 0;
  int i;

  for (i = 0; i < 256; ++i) state->prob_table[i] = 1;

  for (i = 0; i < 256; ++i) {
    int count = state->prob_table[i];
    state->prob_table[i] = ((long long) count * (1 << state->frac_size)) / (size + alphabet_size);
    if (i == 0) {
      state->cumul_table[0] = 0;
      state->cumul_table[1] = state->prob_table[0];
    }
    else state->cumul_table[i+1] = state->cumul_table[i] + state->prob_table[i];
  }
}

void display_prob_table(ac_state_t* state) 
{
  int i;
  double norm = (double) (1 << state->frac_size);
  for (i = 0; i < 256; i++) {
    printf("P[%i]=%.6f, C[%i]=%.6f\n", i, state->prob_table[i] / norm, i, state->cumul_table[i] / norm); 
  }
}

/** Set the bit of index @p index in @p out to the value @p bit_value */
void set_bit_value(unsigned char* out, int index, int bit_value) 
{
  if (bit_value) {
    out[index / 8] |= (1 << (7 - (index % 8)));
  } else {
    out[index / 8] &= ~(1 << ((7 - index % 8)));
  }
}

/** extract a bit value
  * @p out byte-array to read from
  * @p index bit index to be extracted
  * @p return 0-1, value of the requested bit
  */
int get_bit_value(unsigned char* out, int index) 
{
  return (out[index / 8] >> (7 - (index % 8))) & 0x1;
}

/** output a zero value bit value to AC state output
 *  @p out byte-array used as output stream
 *  @p state AC structure containing write pointer to @p out
 */ 
unsigned char* output_zero(unsigned char* out, ac_state_t* state) 
{
  assert(state->out_index >= 0 && "out_index must be positive");

  set_bit_value(out, state->out_index, 0);
  state->out_index++;

  return NULL;
}

/** output a one value bit value to AC state output
 *  @p out byte-array used as output stream
 *  @p state AC structure containing write pointer to @p out
 */ 
unsigned char* output_one(unsigned char* out, ac_state_t* state) 
{
  assert(state->out_index >= 0 && "out_index must be positive");

  set_bit_value(out, state->out_index, 1);
  state->out_index++;

  return NULL;
}

/** Output an arbitrary digit
 *  @p out output byte array
 *  @p state AC structure containing the write pointer
 *  @p digit bit value to be written (should be 0 or 1 )
 */
unsigned char* output_digit(unsigned char* out, ac_state_t* state, int digit)
{
  switch (digit) {
  case 0:
    output_zero(out, state);
    break;
  case 1:
    output_one(out, state);
    break;
  default:
    printf("unexpected digit=%d\n", digit);
    assert(0 && "unexpected digit value in output_digit\n");
    break;
  }
}

#define DISPLAY_VALUE(name, state, value) {\
  printf(name "= %.3f\n", (value) / (double) (1 << (state)->frac_size));\
}

/** Display a byte array as a bit string
 * @p out byte array to be displayed
 * @p size number of bits from @p out to display
 */
void display_bin(unsigned char* out, int bit_size) 
{
  int i;
  printf("bin_value=0.");
  for (i = 0; i < bit_size; ++i) printf("%01d", get_bit_value(out, i));
  printf("\n");
}

/** Process the step of carry propagation without an AC output stream
 *  @p out byte-array used as AC output stream
 *  @p state AC structure containing output parameters
 */
void propagate_carry(unsigned char* out, ac_state_t* state) 
{
  int index = state->out_index - 1;
  while (get_bit_value(out, index) == 1) {
    set_bit_value(out, index, 0);
    index--;
  }
  set_bit_value(out, index, 1);
}

/** Compute the half unit value for @p state
 *  @p state AC structure which contains computation paramaters
 *  @return fixed point value of 0.5 according to the fractionnal sized defined in @p state
 */
int state_half_length(ac_state_t* state)
{
  return 1 << (state->frac_size - 1);
}

/** Compute modulo precision in fixed-point system defined by @p state
 * @p state AC structure containing fixed-point system parameters
 * @p value input value
 * @p return (value % 1.0) in fixed-point system defined by @p state
 */
int modulo_precision(ac_state_t* state, int value) 
{
  return value % (1 << state->frac_size);
}

/** Arithmetic Coding of one byte 
 *  @p out byte array to be used as output stream
 *  @p in  byte to be coded
 *  @p state Arithmetic Coder state
 *  @return unused
 */
unsigned char* encode_character(unsigned char* out, unsigned char in, ac_state_t* state) 
{
  int in_cumul   = state->cumul_table[in];

  // DISPLAY_VALUE("begin base", state, state->base);
  // DISPLAY_VALUE("begin length", state, state->length);

  // interval update
  int Y = ((long long) state->length * state->cumul_table[in + 1]) >> state->frac_size;
  int base_increment = ((long long) state->length * in_cumul) >> state->frac_size;

  // DISPLAY_VALUE("Y", state, Y);
  // DISPLAY_VALUE("base_increment", state, base_increment);

  int new_base   = modulo_precision(state, state->base + base_increment);
  int new_length = Y - base_increment;

  assert(new_base > 0 && new_length > 0 && "intermediary values must be positive");

  //printf("pre propagate_carry and renormalization\n");
  //DISPLAY_VALUE("  base", state, new_base);
  //DISPLAY_VALUE("  length", state, new_length);

  if (new_base < state->base) {
    // propagate carry
    // printf("propagating carry\n");
    propagate_carry(out, state);
  }


  while (new_length < state_half_length(state)) {
    // renormalization
    int digit = (new_base * 2) >> state->frac_size;
    output_digit(out, state, digit);
    new_length = modulo_precision(state, 2 * new_length);
    new_base   = modulo_precision(state, 2 * new_base);
  }

  state->base   = new_base;
  state->length = new_length;

  //DISPLAY_VALUE("end base", state, state->base);
  //DISPLAY_VALUE("end length", state, state->length);
  //display_bin(out, state->out_index);
}

unsigned char decode_character( unsigned char* in, ac_state_t* state) 
{
  // input value
  int length = state->length;
  int V      = state->base;
  int t      = state->out_index;

  // interval selection
  int s = 0, n = 256, X = 0, Y = length;
  while (n - s > 1) {
    int m = (s + n) / 2;
    int Z = ((long long) length * state->cumul_table[m]) >> state->frac_size;
    //DISPLAY_VALUE("  Z", state, Z);

    if (Z > V) { n = m; Y = Z;}
    else { s = m; X = Z;};
  }
  V = V - X;
  length = Y - X;

  // printf("decoded %d/%c\n", s, s);

  while (length < state_half_length(state)) {
    // renormalization
    t++;
    V = modulo_precision(state, 2 * V) + get_bit_value(in, t);
    length = modulo_precision(state, 2 * length);
  }
  //DISPLAY_VALUE("new V", state, V);
  //DISPLAY_VALUE("length", state, length);

  //output
  state->length    = length;
  state->base      = V;
  state->out_index = t;

  return s;
}

void encode_value(unsigned char* out, unsigned char* in, size_t size, ac_state_t* state) 
{
  int i;
  
  // encoding each character
  for (i = 0; i < size; ++i) encode_character(out, in[i], state);

  // code value selection (flushing buffer)

  int base = state->base;
  int new_base = modulo_precision(state, state->base + state_half_length(state) / 2);
  int new_length = (1 << (state->frac_size - 2)) - 1;
  if (base > new_base) propagate_carry(out, state);

  // renormalization (output two symbols)
  while (new_length < state_half_length(state)) {
    int digit = (new_base * 2) >> state->frac_size;
    output_digit(out, state, digit);
    new_length = modulo_precision(state, 2 * new_length);
    new_base   = modulo_precision(state, 2 * new_base);
  }
}

void decode_value(unsigned char* out, unsigned char* in, ac_state_t* state, size_t expected_size) 
{
  int length = (1 << state->frac_size) - 1;
  int V = 0;
  int k, l;
  for (k = 0; k < state->frac_size; k++) {
    V |= get_bit_value(in, k) << (state->frac_size - 1 - k);
  }

  int t = state->frac_size - 1;
  //DISPLAY_VALUE("init V", state, V);
  //DISPLAY_VALUE("init length", state, length);

  state->out_index = t;
  state->base      = V;
  state->length    = length;

  int i;
  for (i = 0; i < expected_size; ++i) {
    *(out++) = decode_character(in, state);

  }

}


void encode_value_with_update(unsigned char* out, unsigned char* in, size_t size, ac_state_t* state, int update_range) 
{
  int i;
  int update_count = 0;

  // reseting count
  for (i = 0; i < 256; i++) state->prob_table[i] = 1;
  
  // encoding each character
  for (i = 0; i < size; ++i) {
    unsigned char input_char = in[i];
    encode_character(out, input_char, state);
    // updating prob
    state->prob_table[input_char]++;
    update_count++;

    // updating cumul table
    if (update_count >= update_range) {
      transform_count_to_cumul(state, update_count);
      update_count = 0;
      // reseting count
      int j;
      for (j = 0; j < 256; j++) state->prob_table[j] = 1;
    }
  }

  // code value selection (flushing buffer)

  int base = state->base;
  int new_base = modulo_precision(state, state->base + state_half_length(state) / 2);
  int new_length = (1 << (state->frac_size - 2)) - 1;
  if (base > new_base) propagate_carry(out, state);

  // renormalization (output two symbols)
  while (new_length < state_half_length(state)) {
    int digit = (new_base * 2) >> state->frac_size;
    output_digit(out, state, digit);
    new_length = modulo_precision(state, 2 * new_length);
    new_base   = modulo_precision(state, 2 * new_base);
  }
}

void decode_value_with_update(unsigned char* out, unsigned char* in, ac_state_t* state, size_t expected_size, int update_range) 
{
  int update_count = 0;
  int length = (1 << state->frac_size) - 1;
  int V = 0;
  int k, l;
  for (k = 0; k < state->frac_size; k++) {
    V |= get_bit_value(in, k) << (state->frac_size - 1 - k);
  }
  
  // int V = (in[0] << 8) | in[1];

  // reseting count
  int i;
  for (i = 0; i < 256; i++) state->prob_table[i] = 1;

  int t = state->frac_size - 1;
  //DISPLAY_VALUE("init V", state, V);
  //DISPLAY_VALUE("init length", state, length);

  state->out_index = t;
  state->base      = V;
  state->length    = length;

  for (i = 0; i < expected_size; ++i) {
    // printf("decoded %d/%c\n", s, s);
    unsigned char decoded_char = decode_character(in, state);
    *(out++) = decoded_char; 

    // updating prob
    state->prob_table[decoded_char]++;
    update_count++;

    // updating cumul table
    if (update_count >= update_range) {
      transform_count_to_cumul(state, update_count);
      update_count = 0;
      // reseting count
      int j;
      for (j = 0; j < 256; j++) state->prob_table[j] = 1;
    }

  }

}


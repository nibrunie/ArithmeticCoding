#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


/** Arithmetic Coding state structure */
typedef struct {
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
}

void build_probability_table(ac_state_t* state, unsigned char* in, int size) 
{
  int alphabet_size = 256;
  int i;
  // reset 
  for (i = 0; i < 256; ++i) state->prob_table[i] = 1;

  // occurences counting
  for (i = 0; i < size; ++i) state->prob_table[in[i]]++;


  // normalization according to state format
  for (i = 0; i < 256; ++i) {
    int count = state->prob_table[i];
    state->prob_table[i] = ((long long) count * (1 << state->frac_size)) / (size + alphabet_size);
    if (i == 0) state->cumul_table[0] = state->prob_table[0];
    else state->cumul_table[i] = state->cumul_table[i-1] + state->prob_table[i];
  }
}

void display_prob_table(ac_state_t* state) 
{
  int i;
  double norm = (double) (1 << state->frac_size);
  for (i = 0; i < 256; i++) {
    printf("P[%i]=%.3f, C[%i]=%.3f\n", i, state->prob_table[i] / norm, i, state->cumul_table[i] / norm); 
  }
}

void set_bit_value(unsigned char* out, int index, int bit_value) 
{
  if (bit_value) {
    out[index / 8] |= (1 << (7 - (index % 8)));
  } else {
    out[index / 8] &= ~(1 << ((7 - index % 8)));
  }
}

int get_bit_value(unsigned char* out, int index) {
  return (out[index / 8] >> (7 - (index % 8))) & 0x1;
}

unsigned char* output_zero(unsigned char* out, ac_state_t* state) 
{
  assert(state->out_index >= 0 && "out_index must be positive");

  set_bit_value(out, state->out_index, 0);
  state->out_index++;

  return NULL;
}

unsigned char* output_one(unsigned char* out, ac_state_t* state) 
{
  assert(state->out_index >= 0 && "out_index must be positive");

  set_bit_value(out, state->out_index, 1);
  state->out_index++;

  return NULL;
}

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

void display_bin(unsigned char* out, int bit_size) {
  int i;
  printf("bin_value=0.");
  for (i = 0; i < bit_size; ++i) printf("%01d", get_bit_value(out, i));
  printf("\n");
}


void propagate_carry(unsigned char* out, ac_state_t* state) {
  int index = state->out_index - 1;
  while (get_bit_value(out, index) == 1) {
    set_bit_value(out, index, 0);
    index--;
  }
  set_bit_value(out, index, 1);
}

int state_half_length(ac_state_t* state)
{
  return 1 << (state->frac_size - 1);
}

int modulo_precision(ac_state_t* state, int value) {
  return value % (1 << state->frac_size);
}

unsigned char* encode_character(unsigned char* out, unsigned char in, ac_state_t* state) 
{
  int in_prob    = state->prob_table[in];
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

  printf("pre propagate_carry and renormalization\n");
  DISPLAY_VALUE("  base", state, new_base);
  DISPLAY_VALUE("  length", state, new_length);

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

void decode_value(unsigned char* out, unsigned char* in, ac_state_t* state, size_t expected_size) 
{
  int length = (1 << state->frac_size) - 1;
  int V = 0;
  int k, l;
  for (k = 0; k < state->frac_size; k++) {
    V |= get_bit_value(in, k) << (state->frac_size - 1 - k);
  }
  
  // int V = (in[0] << 8) | in[1];

  int t = state->frac_size - 1;
  DISPLAY_VALUE("init V", state, V);
  DISPLAY_VALUE("init length", state, length);

  int i;
  for (i = 0; i < expected_size; ++i) {
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
    *(out++) = s;

    while (length < state_half_length(state)) {
      // renormalization
      t++;
      V = modulo_precision(state, 2 * V) + get_bit_value(in, t);
      length = modulo_precision(state, 2 * length);
    }
    //DISPLAY_VALUE("new V", state, V);
    //DISPLAY_VALUE("length", state, length);
  }

}

const unsigned char reference[] = "A reference work is a book or periodical (or its electronic equivalent) to which one can refer for confirmed facts.[1][not in citation given][unreliable source?] The information is intended to be found quickly when needed. Reference works are usually referred to for particular pieces of information, rather than read beginning to end. The writing style used in these works is informative; the authors avoid use of the first person, and emphasize facts. Many reference works are compiled by a team of contributors whose work is coordinated by one or more editors rather than by an individual author. Indices are commonly provided in many types of reference work. Updated editions are usually published as needed, in some cases annually (e.g. Whitaker's Almanack, Who's Who). Reference works include dictionaries, thesauruses, encyclopedias, almanacs, bibliographies, and catalogs (e.g. catalogs of libraries, museums or the works of individual artists).[2] Many reference works are available in electronic form and can be obtained as application software, CD-ROMs, DVDs, or online through the Internet.";

//const unsigned char input[] = "if this text appears completely, then it is a success";
const unsigned char input[] = "A reference work is a book or periodical (or its electronic equivalent) to which one can refer for confirmed facts.[1][not in citation given][unreliable source?] The information is intended to be found quickly when needed. Reference works are usually referred to for particular pieces of information, rather than read beginning to end. The writing style used in these works is informative; the authors avoid use of the first person, and emphasize facts. Many reference works are compiled by a team of contributors whose work is coordinated by one or more editors rather than by an individual author. Indices are commonly provided in many types of reference work. Updated editions are usually published as needed, in some cases annually (e.g. Whitaker's Almanack, Who's Who). Reference works include dictionaries, thesauruses, encyclopedias, almanacs, bibliographies, and catalogs (e.g. catalogs of libraries, museums or the works of individual artists).[2] Many reference works are available in electronic form and can be obtained as application software, CD-ROMs, DVDs, or online through the Internet.";

int main(void) 
{
  size_t  output_size = 10000;
  unsigned char* output = malloc(sizeof(unsigned char) * output_size);
  unsigned char* decomp = malloc(sizeof(unsigned char) * output_size);
  ac_state_t encoder_state;

  // initializing encoding state
  init_state(&encoder_state, 16);

  // building probability table from reference
  build_probability_table(&encoder_state, reference, sizeof(reference));

  display_prob_table(&encoder_state);

  printf("sizeof(reference) =%d\n", sizeof(reference));
  printf("sizeof(input)     =%d\n", sizeof(input));

  // computing arithmetic coding of input
  printf("encoding\n");
  int i;
  for (i = 0; i < sizeof(input); ++i) encode_character(output, input[i], &encoder_state);

  int compressed_size = (encoder_state.out_index + 7) / 8;
  double ratio = compressed_size / (double) sizeof(input);

  printf("compression ration is%.3f%\n", ratio * 100.0);

  printf("exit out_index=%d\n", encoder_state.out_index);


  printf("decoding\n");
  decode_value(decomp, output, &encoder_state, sizeof(input));

  printf("ouput= #%s#\n", decomp);

  DISPLAY_VALUE("state_half_length", &encoder_state, state_half_length(&encoder_state));

  return 0;
}

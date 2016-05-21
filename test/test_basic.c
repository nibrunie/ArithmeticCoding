#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "arith_coding.h"

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
  encode_value(output, input, sizeof(input), &encoder_state);

  int compressed_size = (encoder_state.out_index + 7) / 8;
  double ratio = compressed_size / (double) sizeof(input);

  printf("compression ration is%.3f%\n", ratio * 100.0);

  printf("exit out_index=%d\n", encoder_state.out_index);


  printf("decoding\n");
  decode_value(decomp, output, &encoder_state, sizeof(input));

  if (memcmp(decomp, reference, sizeof(reference))) {
    printf("failure: reference/decomp do not match\n");
    return 1;
  } else {
    printf("success\n");
  }



  return 0;
}

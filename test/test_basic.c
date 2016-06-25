#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "arith_coding.h"

const unsigned char reference[] = "A reference work is a book or periodical (or its electronic equivalent) to which one can refer for confirmed facts.[1][not in citation given][unreliable source?] The information is intended to be found quickly when needed. Reference works are usually referred to for particular pieces of information, rather than read beginning to end. The writing style used in these works is informative; the authors avoid use of the first person, and emphasize facts. Many reference works are compiled by a team of contributors whose work is coordinated by one or more editors rather than by an individual author. Indices are commonly provided in many types of reference work. Updated editions are usually published as needed, in some cases annually (e.g. Whitaker's Almanack, Who's Who). Reference works include dictionaries, thesauruses, encyclopedias, almanacs, bibliographies, and catalogs (e.g. catalogs of libraries, museums or the works of individual artists).[2] Many reference works are available in electronic form and can be obtained as application software, CD-ROMs, DVDs, or online through the Internet.";

//const unsigned char input[] = "if this text appears completely, then it is a success";
const unsigned char input[] = "A reference work is a book or periodical (or its electronic equivalent) to which one can refer for confirmed facts.[1][not in citation given][unreliable source?] The information is intended to be found quickly when needed. Reference works are usually referred to for particular pieces of information, rather than read beginning to end. The writing style used in these works is informative; the authors avoid use of the first person, and emphasize facts. Many reference works are compiled by a team of contributors whose work is coordinated by one or more editors rather than by an individual author. Indices are commonly provided in many types of reference work. Updated editions are usually published as needed, in some cases annually (e.g. Whitaker's Almanack, Who's Who). Reference works include dictionaries, thesauruses, encyclopedias, almanacs, bibliographies, and catalogs (e.g. catalogs of libraries, museums or the works of individual artists).[2] Many reference works are available in electronic form and can be obtained as application software, CD-ROMs, DVDs, or online through the Internet.";

int main(void) 
{

  // Random generated test
  const int test_size[] = {256, 128, 1024, 65536, 1 << 18, 1 << 20};
  int i;
  for (i = 0; i < 6; ++i) {
    int local_size = test_size[i];

    size_t  output_size = local_size * 2;
    unsigned char* input  = malloc(sizeof(unsigned char) * local_size);
    unsigned char* output = malloc(sizeof(unsigned char) * output_size);
    unsigned char* decomp = malloc(sizeof(unsigned char) * output_size);

    printf("testing AC on an almost uniform buffer of %d Bytes\n", local_size);

    // initializing test data
    int j;
    for (j = 0; j < local_size; ++j) input[j] = (rand() % 256);

    {
      ac_state_t encoder_state;
      // initializing encoding state
      init_state(&encoder_state, 16);

      // building probability table from reference
      build_probability_table(&encoder_state, input, local_size >= 256 ? 256 : local_size);

      //display_prob_table(&encoder_state);

      // computing arithmetic coding of input
      printf("encoding with static table\n");
      encode_value(output, input, local_size, &encoder_state);

      int compressed_size = (encoder_state.out_index + 7) / 8;
      double ratio = compressed_size / (double) local_size;

      printf("compression ratio is %.3f%\n", ratio * 100.0);

      printf("exit out_index=%d\n", encoder_state.out_index);


      printf("decoding with static table\n");
      decode_value(decomp, output, &encoder_state, local_size);

      if (memcmp(decomp, input, local_size)) {
        printf("failure: reference/decomp do not match\n");
        for (j = 0; j < local_size && decomp[j] == input[j]; ++j);
        printf("mismatch @ index %d, %x vs %x (expected) \n", j, decomp[j], input[j]);
        return 1;
      } else {
        printf("success\n");
      }
    }

    {
      ac_state_t encoder_state;
      // initializing encoding state
      init_state(&encoder_state, 16);
  
      reset_uniform_probability(&encoder_state);
      printf("encoding with dynamic table\n");
      const int update_range = 128, range_clear = 0;
      encode_value_with_update(output, input, local_size, &encoder_state, update_range, range_clear);

      int compressed_size = (encoder_state.out_index + 7) / 8;
      double ratio = compressed_size / (double) local_size;

      printf("compression ratio is %.3f%\n", ratio * 100.0);

      printf("exit out_index=%d\n", encoder_state.out_index);


      reset_uniform_probability(&encoder_state);
      printf("decoding with dynamic table\n");
      decode_value_with_update(decomp, output, &encoder_state, local_size, update_range, range_clear);

      if (memcmp(decomp, input, local_size)) {
        printf("failure: reference/decomp do not match\n");
        for (j = 0; j < local_size && decomp[j] == input[j]; ++j);
        printf("mismatch @ index %d, %x vs %x (expected) \n", j, decomp[j], input[j]);
        return 1;
      } else {
        printf("success\n");
      }

    }
  }

  {
    size_t  output_size = 10000;
    unsigned char* output = malloc(sizeof(unsigned char) * output_size);
    unsigned char* decomp = malloc(sizeof(unsigned char) * output_size);
    ac_state_t encoder_state;

    // initializing encoding state
    init_state(&encoder_state, 16);

    // building probability table from reference
    build_probability_table(&encoder_state, reference, sizeof(reference));

    //display_prob_table(&encoder_state);

    printf("sizeof(reference) =%u\n", sizeof(reference));
    printf("sizeof(input)     =%u\n", sizeof(input));

    // computing arithmetic coding of input
    printf("encoding\n");
    encode_value(output, input, sizeof(input), &encoder_state);

    int compressed_size = (encoder_state.out_index + 7) / 8;
    double ratio = compressed_size / (double) sizeof(input);

    printf("compression ratio is %.3f%\n", ratio * 100.0);

    printf("exit out_index=%d\n", encoder_state.out_index);


    printf("decoding\n");
    decode_value(decomp, output, &encoder_state, sizeof(input));

    if (memcmp(decomp, reference, sizeof(reference))) {
      printf("failure: reference/decomp do not match\n");
      return 1;
    } else {
      printf("success\n");
    }
  }


  return 0;
}

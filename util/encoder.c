#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "arith_coding.h"


#include <sys/stat.h>

off_t fsize(const char *filename) {
    struct stat st; 

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1; 
}

int main(int argc, char** argv) { 
  if (argc <= 2) {
    printf("usage: %s <filename> <update_range)\n", argv[0]);
    return 1;
  };

  const char* filename = argv[1];
  const int update_range = atoi(argv[2]);

  FILE*  input_stream = fopen(filename, "r");

  size_t input_size   = fsize(filename);
  size_t read_buffer_size = input_size;

  char* read_buffer    = malloc(input_size * sizeof(char));
  char* encoded_buffer = malloc((input_size + 100) * sizeof(char));
  char* decoded_buffer = malloc((input_size + 100) * sizeof(char));

  size_t read_size = fread(read_buffer, sizeof(char), read_buffer_size, input_stream);

  ac_state_t encoder_state;

  // initializing encoding state
  init_state(&encoder_state, 16);
  reset_uniform_probability(&encoder_state);
  //const char reference[] = "reference string";
  //build_probability_table(&encoder_state, reference, sizeof(reference));

  // encoding read buffer
  //encode_value(encoded_buffer, read_buffer, read_size, &encoder_state); 
  encode_value_with_update(encoded_buffer, read_buffer, read_size, &encoder_state, update_range, 0 /* range clear */); 

  size_t encoded_size = (encoder_state.out_index + 7) / 8;

  // decoding buffer 
  reset_uniform_probability(&encoder_state);
  //decode_value(decoded_buffer, encoded_buffer, &encoder_state, read_size);
  decode_value_with_update(decoded_buffer, encoded_buffer, &encoder_state, read_size, update_range, 0/* range clear */);

  if (memcmp(decoded_buffer, read_buffer, read_size)) {
    printf("Failure encoded and decoded buffer mismatch\n");
    return 1;
  }

  double compression_ratio = encoded_size / (double) read_size * 100.0;
  printf("success: compression ratio is %.3f \n", compression_ratio);

  return 0;
}

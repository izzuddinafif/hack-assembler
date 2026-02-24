#include "logisim.h"
void logisim_init(LogisimWriter *logisim_writer) {
  logisim_writer->current_word_count = 0;
  logisim_writer->words_per_line = 8;
}

void binary_to_hex(const char *bin16, char *hex4) {
  // run per nibble (4 bits)
  for (int i = 0; i < 4; i++) {
    int value = 0;
    // convert bit string to bits
    for (int j = 0; j < 4; j++) {
      value = (value << 1) | (bin16[i * 4 + j] == '1');
    }

    // convert to hex
    if (value < 10) {
      hex4[i] = '0' + (char)value;
    } else {
      hex4[i] = 'a' + ((char)value - 10);
    }
  }
  hex4[4] = '\0';
}

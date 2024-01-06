#ifndef PUZZLE_RECORD_H
#define PUZZLE_RECORD_H

#include <stdbool.h>
#include <stdint.h>

#include "hanoi.h"

struct hanoi_recorder
{
  int fd;
  uint64_t moves;
  char *path;
};

void
hanoi_set_records_directory (const char *path);

bool
hanoi_new_recorder (struct hanoi_recorder *recorder, const struct hanoi_puzzle *pzl);

bool
hanoi_delete_recorder_file (struct hanoi_recorder *recorder);

void
hanoi_free_recorder (struct hanoi_recorder *recorder);

bool
hanoi_push_move (struct hanoi_recorder *recorder, const uint32_t src_i, const uint32_t des_i,
                 const uint64_t duration);

#endif /* PUZZLE_RECORD_H */
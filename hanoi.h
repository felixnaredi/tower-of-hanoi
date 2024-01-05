#ifndef HANOI_H
#define HANOI_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
  HANOI_OK,
  HANOI_ERROR_ABORT,
  HANOI_INVALID_MOVE,
} hanoi_response_code;

struct hanoi_puzzle
{
  uint32_t n_rods;
  uint32_t n_disks;
  uint32_t **state;
};

hanoi_response_code hanoi_init (struct hanoi_puzzle *pzl, const uint32_t n_rods,
                                const uint32_t n_disks);

hanoi_response_code hanoi_free (struct hanoi_puzzle *pzl);

hanoi_response_code hanoi_move (struct hanoi_puzzle *pzl, const uint32_t src_i,
                                const uint32_t des_i);

bool hanoi_empty_rod (const struct hanoi_puzzle *pzl, const uint32_t i);

uint32_t hanoi_complete (const struct hanoi_puzzle *pzl);

#endif /* HANOI_H */
#ifndef HANOI_H
#define HANOI_H

#include <stdbool.h>
#include <stdint.h>

#define HANOI_INCOMPLETE ((uint32_t)-1)

struct hanoi_puzzle
{
  uint32_t n_rods;
  uint32_t n_disks;
  uint32_t **state;
};

enum hanoi_init_response
{
  HANOI_INIT_OK,
  HANOI_INIT_INVALID_N_RODS_VALUE,
  HANOI_INIT_SYSTEM_ERROR,
};

enum hanoi_init_response
hanoi_init (struct hanoi_puzzle *pzl, const uint32_t n_rods, const uint32_t n_disks);

void
hanoi_free (struct hanoi_puzzle *pzl);

bool
hanoi_move (struct hanoi_puzzle *pzl, const uint32_t src_i, const uint32_t des_i);

bool
hanoi_empty_rod (const struct hanoi_puzzle *pzl, const uint32_t i);

uint32_t
hanoi_complete (const struct hanoi_puzzle *pzl);

#endif /* HANOI_H */
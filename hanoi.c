#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "hanoi.h"

hanoi_response_code
hanoi_init (struct hanoi_puzzle *pzl, const uint32_t n_rods, const uint32_t n_disks)
{
  uint32_t *st = calloc (n_rods * n_disks + 1, sizeof (uint32_t));
  if (st == NULL)
    {
      return HANOI_ERROR_ABORT;
    }

  pzl->state = malloc (sizeof (pzl->state) * n_rods);
  if (st == NULL)
    {
      free (st);
      return HANOI_ERROR_ABORT;
    }

  for (uint32_t i = 0; i < n_rods; ++i)
    {
      pzl->state[i] = st + n_disks * i;
    }

  for (uint32_t i = 0; i < n_disks; ++i)
    {
      pzl->state[0][i] = n_disks - i;
    }

  pzl->n_rods = n_rods;
  pzl->n_disks = n_disks;

  return HANOI_OK;
}

hanoi_response_code
hanoi_free (struct hanoi_puzzle *pzl)
{
  free (pzl->state[0]);
  free (pzl->state);

  return HANOI_OK;
}

static uint32_t
top_index (const struct hanoi_puzzle *pzl, const uint32_t i)
{
  if (pzl->state[i][0] == 0)
    {
      return 0;
    }

  uint32_t j = 0;
  while (pzl->state[i][++j] != 0)
    {
    }

  return j;
}

hanoi_response_code
hanoi_move (struct hanoi_puzzle *pzl, const uint32_t src_i, const uint32_t des_i)
{
  const uint32_t src_j = top_index (pzl, src_i);

  if (src_j == 0)
    {
      return HANOI_INVALID_MOVE;
    }

  const uint32_t src = pzl->state[src_i][src_j - 1];
  const uint32_t des_j = top_index (pzl, des_i);

  if (des_j == 0)
    {
      pzl->state[src_i][src_j - 1] = 0;
      pzl->state[des_i][des_j] = src;
      return HANOI_OK;
    }

  const uint32_t des = pzl->state[des_i][des_j - 1];

  if (src < des)
    {
      pzl->state[src_i][src_j - 1] = 0;
      pzl->state[des_i][des_j] = src;
      return HANOI_OK;
    }

  return HANOI_INVALID_MOVE;
}

bool
hanoi_empty_rod (const struct hanoi_puzzle *pzl, const uint32_t i)
{
  return pzl->state[i][0] == 0;
}

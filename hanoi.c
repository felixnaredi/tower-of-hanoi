#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "hanoi.h"

/**
 * @brief Initializes a `struct hanoi_puzzle`. A succsessfull init must be freed using `hanoi_free`.
 *
 * @param pzl Target `struct hanoi_puzzle`.
 * @param n_rods Amount of rods in the puzzle.
 * @param n_disks Amount of disks in the puzzle.
 * @return HANOI_INIT_OK - Init successful.
 * @return HANOI_INIT_INVALID_N_RODS_VALUE - The value of `n_rods` was set to `HANOI_INCOMPLATE`.
 * @return HANOI_INIT_SYSTEM_ERROR - System failure during init. Check `errno`.
 */
enum hanoi_init_response
hanoi_init (struct hanoi_puzzle *pzl, const uint32_t n_rods, const uint32_t n_disks)
{
  if (n_rods == HANOI_INCOMPLETE)
    {
      return HANOI_INIT_INVALID_N_RODS_VALUE;
    }

  uint32_t *st = calloc (n_rods * n_disks + 1, sizeof (uint32_t));
  if (st == NULL)
    {
      return HANOI_INIT_SYSTEM_ERROR;
    }

  pzl->state = malloc (sizeof (pzl->state) * n_rods);
  if (pzl->state == NULL)
    {
      free (st);
      return HANOI_INIT_SYSTEM_ERROR;
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

  return HANOI_INIT_OK;
}

void
hanoi_free (struct hanoi_puzzle *pzl)
{
  free (pzl->state[0]);
  free (pzl->state);
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

/**
 * @brief Moves a disk to a new rod.
 *
 * @param pzl A `struct hanoi_puzzle` where the move should take place.
 * @param src_i Index of the source rod.
 * @param des_i Index of the destination rod.
 * @return true - State was updated.
 * @return false - State did not update.
 */
bool
hanoi_move (struct hanoi_puzzle *pzl, const uint32_t src_i, const uint32_t des_i)
{
  const uint32_t src_j = top_index (pzl, src_i);

  if (src_j == 0)
    {
      return false;
    }

  const uint32_t src = pzl->state[src_i][src_j - 1];
  const uint32_t des_j = top_index (pzl, des_i);

  if (des_j == 0)
    {
      pzl->state[src_i][src_j - 1] = 0;
      pzl->state[des_i][des_j] = src;
      return true;
    }

  const uint32_t des = pzl->state[des_i][des_j - 1];

  if (src < des)
    {
      pzl->state[src_i][src_j - 1] = 0;
      pzl->state[des_i][des_j] = src;
      return true;
    }

  return false;
}

bool
hanoi_empty_rod (const struct hanoi_puzzle *pzl, const uint32_t i)
{
  return pzl->state[i][0] == 0;
}

/**
 * @brief Checks if a `struct hanoi_puzzle` is completed. A complete state is defined as one where
 * all disks are on the same rod, the largest disk is at the bottom and all other disks are on top
 * of a disk of a size one greater than it.
 *
 * @param pzl
 * @return uint32_t The index of the rod where all disks resides or `HANOI_INCOMPLETE` if the puzzle
 * is not complete.
 */
uint32_t
hanoi_complete (const struct hanoi_puzzle *pzl)
{
  uint32_t filled_rod = HANOI_INCOMPLETE;

  for (uint32_t i = 0; i < pzl->n_rods; ++i)
    {
      if (!hanoi_empty_rod (pzl, i))
        {
          if (filled_rod != HANOI_INCOMPLETE)
            {
              return HANOI_INCOMPLETE;
            }
          else
            {
              filled_rod = i;
              uint32_t j = 0;

              for (; j < pzl->n_disks && pzl->state[i][j] == pzl->n_disks - j; ++j)
                {
                }
              if (j != pzl->n_disks)
                {
                  return HANOI_INCOMPLETE;
                }
            }
        }
    }

  return filled_rod;
}

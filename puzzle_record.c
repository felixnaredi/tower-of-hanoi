#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#include "hanoi.h"
#include "puzzle_record.h"

bool
hanoi_new_recorder (struct hanoi_recorder *recorder, const char *path,
                    const struct hanoi_puzzle *pzl)
{
  const int fd = creat (path, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if (fd == -1)
    {
      return false;
    }

  uint64_t zero = 0;
  if (write (fd, &zero, sizeof (uint64_t)) == -1)
    {
      close (fd);
      return false;
    }

  if (write (fd, &pzl->n_rods, sizeof (pzl->n_rods)) == -1)
    {
      close (fd);
      return false;
    }

  if (write (fd, &pzl->n_disks, sizeof (pzl->n_disks)) == -1)
    {
      close (fd);
      return false;
    }

  if (write (fd, pzl->state[0], sizeof (pzl->state[0][0]) * pzl->n_rods * pzl->n_disks) == -1)
    {
      close (fd);
      return false;
    }

  recorder->fd = fd;
  recorder->moves = 0;

  return true;
}

void
hanoi_free_recorder (struct hanoi_recorder *recorder)
{
  close (recorder->fd);
}

bool
hanoi_push_move (struct hanoi_recorder *recorder, const uint32_t src_i, const uint32_t des_i,
                 const uint64_t duration)
{
  const uint32_t buf[] = { src_i, des_i };

  if (lseek (recorder->fd, 0, SEEK_END) == -1)
    {
      return false;
    }

  if (write (recorder->fd, buf, sizeof (buf)) == -1)
    {
      return false;
    }
  if (write (recorder->fd, &duration, sizeof (duration)) == -1)
    {
      return false;
    }

  recorder->moves += 1;

  if (lseek (recorder->fd, 0, SEEK_SET) == -1)
    {
      return false;
    }

  if (write (recorder->fd, &recorder->moves, sizeof (recorder->moves)) == -1)
    {
      return false;
    }

  return true;
}
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "hanoi.h"
#include "puzzle_record.h"

#define FILENAME_LEN 12

static const char alphabet[]
    = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
        's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

static size_t recorder_path_filename_offset;
static char *recorder_path;

static char *
generate_recorder_path ()
{
  char *path = malloc (strlen (recorder_path));
  strcpy (path, recorder_path);

  for (int i = recorder_path_filename_offset; i < recorder_path_filename_offset + FILENAME_LEN; ++i)
    {
      path[i] = alphabet[rand () % (sizeof (alphabet) / sizeof (alphabet[0]))];
    }

  return path;
}

void
hanoi_set_records_directory (const char *path)
{
  size_t len = strlen (path);

  if (path[len - 1] == '/')
    {
      recorder_path = malloc (len + FILENAME_LEN + strlen (".hanoi-puzzle"));
      strcpy (recorder_path, path);
    }
  else
    {
      recorder_path = malloc (len + 1 + FILENAME_LEN + strlen (".hanoi-puzzle"));
      strcpy (recorder_path, path);
      recorder_path[len] = '/';
      ++len;
    }

  for (int i = len; i < len + FILENAME_LEN; ++i)
    {
      recorder_path[i] = 'X';
    }
  recorder_path[len + FILENAME_LEN] = '\0';

  strcat (recorder_path, ".hanoi-puzzle");

  recorder_path_filename_offset = len;
}

bool
hanoi_new_recorder (struct hanoi_recorder *recorder, const struct hanoi_puzzle *pzl)
{
  recorder->path = generate_recorder_path ();

  const int fd = creat (recorder->path, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if (fd == -1)
    {
      free (recorder->path);
      return false;
    }

  recorder->fd = fd;
  recorder->moves = 0;

  if (write (fd, &recorder->moves, sizeof (recorder->moves)) == -1)
    {
      free (recorder->path);
      close (fd);
      return false;
    }

  if (write (fd, &pzl->n_rods, sizeof (pzl->n_rods)) == -1)
    {
      free (recorder->path);
      close (fd);
      return false;
    }

  if (write (fd, &pzl->n_disks, sizeof (pzl->n_disks)) == -1)
    {
      free (recorder->path);
      close (fd);
      return false;
    }

  if (write (fd, pzl->state[0], sizeof (pzl->state[0][0]) * pzl->n_rods * pzl->n_disks) == -1)
    {
      free (recorder->path);
      close (fd);
      return false;
    }

  return true;
}

void
hanoi_free_recorder (struct hanoi_recorder *recorder)
{
  free (recorder->path);
  close (recorder->fd);
}

bool
hanoi_delete_recorder_file (struct hanoi_recorder *recorder)
{
  return remove (recorder->path) != -1;
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
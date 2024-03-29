#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "hanoi.h"
#include "record.h"

#define FILENAME_LEN 12
#define MAX_USERNAME_LEN 32

#define HEADER_SIZE                                                                                \
  (sizeof (uint64_t) + sizeof (uint64_t) + sizeof (uint32_t) + sizeof (uint32_t)                   \
   + sizeof (uint64_t) + MAX_USERNAME_LEN)

/* clang-format off */

static uint64_t * header_checksum (uint8_t *header) { return (uint64_t *)&header[0]; }
static uint64_t * header_moves    (uint8_t *header) { return (uint64_t *)&header[8]; }
static uint32_t * header_n_rods   (uint8_t *header) { return (uint32_t *)&header[16]; }
static uint32_t * header_n_disks  (uint8_t *header) { return (uint32_t *)&header[20]; }
static uint64_t * header_date     (uint8_t *header) { return (uint64_t *)&header[24]; }
static char     * header_username (uint8_t *header) { return (char *)&header[32]; }
// static uint32_t * header_puzzle   (uint8_t *header) { return (uint32_t *)&header[64]; }

// static const uint64_t * header_const_checksum (const uint8_t *header) { return (const uint64_t *)&header[0]; }
static const uint64_t * header_const_moves    (const uint8_t *header) { return (const uint64_t *)&header[8]; }
static const uint32_t * header_const_n_rods   (const uint8_t *header) { return (const uint32_t *)&header[16]; }
static const uint32_t * header_const_n_disks  (const uint8_t *header) { return (const uint32_t *)&header[20]; }
// static const uint64_t * header_const_date     (const uint8_t *header) { return (const uint64_t *)&header[24]; }
// static const char     * header_const_username (const uint8_t *header) { return (const char *)&header[32]; }
// static const uint32_t * header_const_puzzle   (const uint8_t *header) { return (const uint32_t *)&header[64]; }

/* clang-format on */

static const char alphabet[]
    = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
        's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

static size_t recorder_path_filename_offset;
static char *recorder_path = NULL;

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

/**
 * @brief Source: http://www.cse.yorku.ca/~oz/hash.html
 *
 * @param data
 * @return uint64_t Hash
 */
uint64_t
djb2 (uint64_t hash, uint8_t *data, size_t len)
{
  while (len--)
    {
      hash = ((hash << 5) + hash) + *data++; /* hash * 33 + c */
    }

  return hash;
}

void
hanoi_set_records_directory (const char *path)
{
  if (recorder_path != NULL)
    {
      free (recorder_path);
    }

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
      recorder_path[len++] = '/';
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
hanoi_new_recorder (struct hanoi_recorder *recorder, const struct hanoi_puzzle *pzl,
                    const char *username)
{
  recorder->path = generate_recorder_path ();

  const int fd
      = open (recorder->path, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

  if (fd == -1)
    {
      free (recorder->path);
      return false;
    }

  recorder->fd = fd;
  recorder->moves = 0;

  uint8_t buf[HEADER_SIZE];

  *header_checksum (buf) = 0;
  *header_moves (buf) = recorder->moves;
  *header_n_rods (buf) = pzl->n_rods;
  *header_n_disks (buf) = pzl->n_disks;
  *header_date (buf) = time (NULL);
  strncpy (header_username (buf), username, MAX_USERNAME_LEN);

  if (!(write (fd, buf, sizeof (buf)) != -1
        && write (fd, pzl->state[0], sizeof (pzl->state[0][0]) * pzl->n_rods * pzl->n_disks) != -1))
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
hanoi_recorder_remove_file (struct hanoi_recorder *recorder)
{
  return remove (recorder->path) != -1;
}

bool
hanoi_recorder_push_move (struct hanoi_recorder *recorder, const uint32_t src_i,
                          const uint32_t des_i, const uint64_t duration)
{
  const uint32_t buf[] = { src_i, des_i, ((uint32_t *)&duration)[0], ((uint32_t *)&duration)[1] };

  if (!(lseek (recorder->fd, 0, SEEK_END) != -1 && write (recorder->fd, buf, sizeof (buf)) != -1))
    {
      return false;
    }

  recorder->moves += 1;

  if (!(lseek (recorder->fd, 8, SEEK_SET) != -1
        && write (recorder->fd, &recorder->moves, sizeof (recorder->moves)) != -1))
    {
      return false;
    }

  return true;
}

bool
hanoi_recorder_write_checksum (struct hanoi_recorder *recorder)
{
  uint8_t header[HEADER_SIZE];

  if (!(lseek (recorder->fd, 0, SEEK_SET) != -1
        && read (recorder->fd, header, sizeof (header)) != -1))
    {
      return false;
    }

  uint64_t checksum = 142573;
  checksum = djb2 (checksum, (header + 8), HEADER_SIZE - 8);

  size_t len = *header_const_n_rods (header) * *header_const_n_disks (header) * sizeof (uint32_t)
               + *header_const_moves (header) * sizeof (uint32_t) * 2;

  uint8_t buf[512];

  while (true)
    {
      if (len > sizeof (buf))
        {
          if (read (recorder->fd, buf, sizeof (buf)) == -1)
            {
              return false;
            }
          checksum = djb2 (checksum, buf, sizeof (buf));
          len -= sizeof (buf);
        }
      else
        {
          if (read (recorder->fd, buf, len) == -1)
            {
              return false;
            }
          checksum = djb2 (checksum, buf, len);
          break;
        }
    }

  if (!(lseek (recorder->fd, 0, SEEK_SET) != -1
        && write (recorder->fd, &checksum, sizeof (checksum)) != -1))
    {
      return false;
    }

  return true;
}
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include <ncurses.h>

#include "hanoi.h"
#include "record.h"

#define error(...)                                                                                 \
  {                                                                                                \
    fprintf (stderr, "ERROR %s:%d - ", __FILE__, __LINE__);                                        \
    fprintf (stderr, __VA_ARGS__);                                                                 \
  }

static int
center (const struct hanoi_puzzle *pzl, const int i)
{
  return i * (1 + 2 * pzl->n_disks) + (1 + 2 * (pzl->n_disks - 1)) / 2;
}

static bool
init_puzzle (struct hanoi_puzzle *pzl, const uint32_t n_rods, const uint32_t n_disks)
{
  switch (hanoi_init (pzl, n_rods, n_disks))
    {
    case HANOI_INIT_OK:
      return true;
    case HANOI_INIT_INVALID_N_RODS_VALUE:
      error ("Value of `n_rods` may not be 0x%x\n", HANOI_INCOMPLETE);
      return false;
    case HANOI_INIT_SYSTEM_ERROR:
      error ("%s\n", strerror (errno));
      return false;
    }
}

static void
print_help (const char *program)
{
  printf ("usage: %s\n", program);
  printf ("    [ --size=<rods,disks> ]\n");
  printf ("    [ --username=<name> ]\n");
  printf ("    [ --help ]\n");
}

int
main (int argc, char **argv)
{
  srand (time (NULL));

  struct hanoi_puzzle pzl;
  bool puzzle_is_initialized = false;
  char username[32];

  strncpy (username, "John Doe", sizeof (username));

  for (int i = 1; i < argc; ++i)
    {
      uint32_t n_rods;
      uint32_t n_disks;

      if (sscanf (argv[i], "--size=%u,%u", &n_rods, &n_disks))
        {
          if (puzzle_is_initialized)
            {
              hanoi_free (&pzl);
            }

          if (!init_puzzle (&pzl, n_rods, n_disks))
            {
              return 1;
            }
          puzzle_is_initialized = true;
        }
      else if (sscanf (argv[i], "--username=%31s", username))
        {
        }
      else if (sscanf (argv[i], "--help"))
        {
          print_help (argv[0]);
          return 0;
        }
      else
        {
          error ("unknown option '%s'\n", argv[i]);
          print_help (argv[0]);

          if (puzzle_is_initialized)
            {
              hanoi_free (&pzl);
            }

          return 1;
        }
    }

  if (!puzzle_is_initialized)
    {
      if (!init_puzzle (&pzl, 3, 4))
        {
          return -1;
        }
    }

  if (mkdir ("records", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1 && errno != EEXIST)
    {
      error ("%s\n", strerror (errno));
      hanoi_free (&pzl);
      return 1;
    }

  hanoi_set_records_directory ("records");

  struct hanoi_recorder recorder;

  if (!hanoi_new_recorder (&recorder, &pzl, username))
    {
      error ("%s\n", strerror (errno));
      hanoi_free (&pzl);
      return 1;
    }

  initscr ();
  raw ();
  noecho ();
  halfdelay (3);
  curs_set (0);
  keypad (stdscr, TRUE);

  const int game_window_width = pzl.n_rods * (1 + pzl.n_disks * 2);

  WINDOW *window_game = newwin (pzl.n_disks, game_window_width, 2, 1);
  WINDOW *window_select = newwin (1, game_window_width, 1, 1);
  WINDOW *window_status = newwin (2, 32, pzl.n_disks + 3, 0);

  int last_complete_position = hanoi_complete (&pzl);
  int moves = 0;
  int selected_src = 0;
  int selected_des = -1;
  char *error_display = NULL;
  uint64_t duration = 0;
  bool active = false;

  struct timespec last_time;

  mvwprintw (window_status, 0, 0, "Time: %.1f", (double)duration / (double)1e9);
  mvwprintw (window_status, 0, 16, "Moves: %d", moves);

  while (1)
    {
      if (active)
        {
          struct timespec time;
          clock_gettime (CLOCK_MONOTONIC, &time);
          duration += (time.tv_sec - last_time.tv_sec) * 1000;
          duration += (time.tv_nsec - last_time.tv_nsec) / 1000000;
          last_time = time;

          wclear (window_status);
          mvwprintw (window_status, 0, 0, "Time: %.1f", (double)duration / (double)1e3);
          mvwprintw (window_status, 0, 16, "Moves: %d", moves);
        }

      wclear (window_game);
      wclear (window_select);

      const uint32_t current_complete_position = hanoi_complete (&pzl);

      if (current_complete_position != HANOI_INCOMPLETE
          && current_complete_position != last_complete_position)
        {
          active = false;
          last_complete_position = current_complete_position;
          mvwprintw (window_status, 1, 0, "Complete!");

          if (!hanoi_recorder_write_checksum (&recorder))
            {
              error ("%s\n", strerror (errno));
              delwin (window_game);
              delwin (window_select);
              delwin (window_status);
              endwin ();
              hanoi_free (&pzl);
              return 1;
            }

          hanoi_free_recorder (&recorder);

          if (!hanoi_new_recorder (&recorder, &pzl, username))
            {
              error ("%s\n", strerror (errno));
              delwin (window_game);
              delwin (window_select);
              delwin (window_status);
              endwin ();
              hanoi_free (&pzl);
              return 1;
            }
        }

      if (error_display)
        {
          mvwprintw (window_status, 1, 0, error_display);
        }

      for (int i = 0; i < pzl.n_rods; ++i)
        {
          const int cx = center (&pzl, i);

          for (int j = 0; j < pzl.n_disks; ++j)
            {
              const int y = pzl.n_disks - j - 1;

              const uint32_t disk = pzl.state[i][j];
              if (disk == 0)
                {
                  mvwaddch (window_game, y, cx, '|');
                }
              else
                {
                  for (int x = cx - disk + 1; x < cx + disk; ++x)
                    {
                      mvwaddch (window_game, y, x, 'O');
                    }
                }
            }
        }

      if (selected_des == -1)
        {
          mvwaddch (window_select, 0, center (&pzl, selected_src), 'v');
        }
      else
        {
          mvwaddch (window_select, 0, center (&pzl, selected_src), '+');
          mvwaddch (window_select, 0, center (&pzl, selected_des), 'V');
        }

      refresh ();
      wrefresh (window_game);
      wrefresh (window_select);
      wrefresh (window_status);

      const int c = getch ();

      if (c == 'q')
        {
          break;
        }
      else if (c == KEY_LEFT)
        {
          error_display = NULL;

          if (selected_des == -1)
            {
              selected_src += pzl.n_rods - 1;
              selected_src %= pzl.n_rods;
            }
          else
            {
              selected_des += pzl.n_rods - 1;
              selected_des %= pzl.n_rods;
            }
        }
      else if (c == KEY_RIGHT)
        {
          error_display = NULL;

          if (selected_des == -1)
            {
              selected_src += 1;
              selected_src %= pzl.n_rods;
            }
          else
            {
              selected_des += 1;
              selected_des %= pzl.n_rods;
            }
        }
      else if (c == ' ')
        {
          error_display = NULL;

          if (selected_des == -1)
            {
              if (hanoi_empty_rod (&pzl, selected_src))
                {
                  if (active)
                    {
                      error_display = "No can do... Empty rod";
                    }
                }
              else
                {
                  selected_des = selected_src;
                }
            }
          else
            {
              if (selected_src != selected_des)
                {
                  if (hanoi_move (&pzl, selected_src, selected_des))
                    {
                      if (!active)
                        {
                          active = true;

                          clock_gettime (CLOCK_MONOTONIC, &last_time);
                          moves = 0;
                          duration = 0;
                        }
                      hanoi_recorder_push_move (&recorder, selected_src, selected_des, duration);
                      selected_src = selected_des;
                      selected_des = -1;
                      ++moves;
                    }
                  else
                    {
                      error_display = "No can do... Invalid move";
                    }
                }
              else
                {
                  selected_des = -1;
                }
            }
        }
    }

  delwin (window_game);
  delwin (window_select);
  delwin (window_status);
  endwin ();
  hanoi_free (&pzl);

  if (!active)
    {
      if (!hanoi_recorder_remove_file (&recorder))
        {
          hanoi_free_recorder (&recorder);
          error ("%s\n", strerror (errno));
          return 1;
        }
    }
  hanoi_free_recorder (&recorder);

  return 0;
}
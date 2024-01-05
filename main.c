#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ncurses.h>

#include "hanoi.h"

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

int
main (int argc, char **argv)
{
  struct hanoi_puzzle pzl;

  if (argc == 1)
    {
      if (hanoi_init (&pzl, 3, 4) != HANOI_OK)
        {
          error ("%s\n", strerror (errno));
          return 1;
        }
    }
  else if (argc == 3)
    {
      char *end;
      const int n_rods = strtol (argv[1], &end, 10);

      if (*end != '\0')
        {
          error ("invalid numeric value '%s'\n", argv[1]);
          return 1;
        }

      const int n_disks = strtol (argv[2], &end, 10);

      if (*end != '\0')
        {
          error ("invalid numeric value '%s'\n", argv[2]);
          return 1;
        }

      if (hanoi_init (&pzl, n_rods, n_disks) != HANOI_OK)
        {
          error ("%s\n", strerror (errno));
          return 1;
        }
    }
  else
    {
      printf ("Usage: %s <RODS> <DISKS>\n", argv[0]);
      printf ("Default: %s 3 4\n", argv[0]);
      return 1;
    }

  initscr ();
  raw ();
  noecho ();
  curs_set (0);
  keypad (stdscr, TRUE);

  const int game_window_width = pzl.n_rods * (2 + (pzl.n_disks - 1) * 2) + 1;

  WINDOW *window_game = newwin (pzl.n_disks, game_window_width, 1, 1);
  WINDOW *window_select = newwin (1, game_window_width, 0, 1);
  WINDOW *window_status = newwin (2, 32, pzl.n_disks + 2, 0);

  int last_complete_position = hanoi_complete (&pzl);
  int moves = 0;
  int selected_src = 0;
  int selected_des = -1;
  char *error_display = NULL;

  while (1)
    {
      clear ();
      wclear (window_game);
      wclear (window_select);
      wclear (window_status);

      mvwprintw (window_status, 0, 0, "Moves: %d", moves);

      const int32_t current_complete_position = hanoi_complete (&pzl);

      if (current_complete_position > -1 && current_complete_position != last_complete_position)
        {
          mvwprintw (window_status, 1, 0, "Complete!");
          moves = 0;
          last_complete_position = current_complete_position;
        }

      if (error_display)
        {
          mvwprintw (window_status, 1, 0, error_display);
          error_display = NULL;
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
          if (selected_des == -1)
            {
              if (hanoi_empty_rod (&pzl, selected_src))
                {
                  error_display = "No can do... Empty rod";
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
                  if (hanoi_move (&pzl, selected_src, selected_des) == HANOI_INVALID_MOVE)
                    {
                      error_display = "No can do... Invalid move";
                    }
                  else
                    {
                      selected_src = selected_des;
                      selected_des = -1;
                      ++moves;
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

  return 0;
}
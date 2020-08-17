/****************************************************************************
 * examples/serloop/serrelay_main.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/ioctl.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: serial_in
 *
 * Description:
 *   Read data from serial and write to pts
 *
 ****************************************************************************/

static void serial_in(int fd)
{
  uint8_t ch;
  int ret;

  /* Run forever */

  while (true)
    {
      ssize_t len;

      len = read(fd, &ch, 1);
      if (len < 0)
        {
          fprintf(stderr,
                  "ERROR Failed to read from serial: %d\n",
                  errno);
        }
      else if (len > 0)
        {
          ret = write(1, &ch, len);
          if (ret < 0)
            {
              fprintf(stderr,
                      "Failed to write to serial: %d\n",
                      errno);
            }
        }
    }
}

/****************************************************************************
 * Name: serial_out
 *
 * Description:
 *   Read data from pts and write to serial
 *
 ****************************************************************************/

static void serial_out(int fd)
{
  uint8_t ch;
  int ret;

  /* Run forever */

  while (true)
    {
      ssize_t len;

      len = read(0, &ch, 1);
      if (len < 0)
        {
          fprintf(stderr,
                  "ERROR Failed to read from serial: %d\n",
                  errno);
        }
      else if (len > 0)
        {
          ret = write(fd, &ch, len);
          if (ret < 0)
            {
              fprintf(stderr,
                      "Failed to write to serial: %d\n",
                      errno);
            }
        }
    }
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * serrelay_main
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
  int fd = -1;
  int ret;
  pthread_t si_thread;
  pthread_t so_thread;

  if (argc == 2)
    {
      fd = open(argv[1], O_RDWR);
    }

  if (fd < 0)
    {
      return EXIT_FAILURE;
    }

  ret = pthread_create(&si_thread, NULL,
                       (pthread_startroutine_t)serial_in,
                       (pthread_addr_t)fd);
  if (ret != 0)
    {
      /* Can't do output because stdout and stderr are redirected */

      return EXIT_FAILURE;
    }

  /* Start a thread to write to serial */
  ret = pthread_create(&so_thread, NULL,
                       (pthread_startroutine_t)serial_out,
                       (pthread_addr_t)fd);
  if (ret != 0)
    {
      /* Can't do output because stdout and stderr are redirected */

      return EXIT_FAILURE;
    }

  for (;;)
    {
      usleep(100000);
    }


  return 0;
}

/****************************************************************************
 * examples/serloop/serproxy_main.c
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
#include <poll.h>

#include <sys/ioctl.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define POLL_WAIT 100

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct fd_pair
{
  int fd_in;
  int fd_out;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: serial_io
 *
 * Description:
 *   Transfer data from in to out
 *
 ****************************************************************************/

static void serial_io(pthread_addr_t arg)
{
  struct pollfd fdp;
  char buffer[64];
  int ret;
  int fdin;
  int fdout;

  fdin  = ((struct fd_pair *)arg)->fd_in;
  fdout = ((struct fd_pair *)arg)->fd_out;

  fdp.fd = fdin;
  fdp.events = POLLIN;

  /* Run forever */

  while (true)
    {
      ssize_t len;

      ret = poll(&fdp, 1, POLL_WAIT);
      if (ret > 0)
        {
          len = read(fdin, buffer, sizeof(buffer));
          if (len < 0)
            {
              fprintf(stderr,
                      "ERROR Failed to read from in: %d\n",
                      errno);
            }
          else if (len > 0)
            {
              ret = write(fdout, buffer, len);
              if (ret < 0)
                {
                  fprintf(stderr,
                          "Failed to write to out: %d\n",
                          errno);
                }
            }
        }
    }
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * serproxy_main
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
  int fd = -1;
  int ret;
  pthread_t si_thread;
  pthread_t so_thread;

  struct fd_pair fds[2];

  if (argc == 1)
    {
      fd = open("/dev/ttyUSB0", O_RDWR);
    }
  else if (argc == 2)
    {
      fd = open(argv[1], O_RDWR);
    }

  if (fd < 0)
    {
      return EXIT_FAILURE;
    }

  fds[0].fd_in  = fd;
  fds[0].fd_out = 1;

  /* Start a thread to read from serial */
  ret = pthread_create(&si_thread, NULL,
                       (pthread_startroutine_t)serial_io,
                       (pthread_addr_t)&fds[0]);
  if (ret != 0)
    {
      return EXIT_FAILURE;
    }

  fds[1].fd_in  = 0;
  fds[1].fd_out = fd;

  /* Start a thread to write to serial */
  ret = pthread_create(&so_thread, NULL,
                       (pthread_startroutine_t)serial_io,
                       (pthread_addr_t)&fds[1]);
  if (ret != 0)
    {
      return EXIT_FAILURE;
    }

  for (;;)
    {
      usleep(100000);
    }


  return 0;
}

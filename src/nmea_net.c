#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include <nmea.h>
#include <nmea/gpgll.h>
#include <nmea/gpgga.h>
#include <nmea/gprmc.h>

#include "nmea_net.h"

char buffer[BUF_SIZE];

static void parsing()
{
  char *str = NULL, *token;
  char *saveptr;
  char *sentence = malloc((NMEA_MAX_LENGTH + 2) * sizeof(char));

  str = buffer;
  for (int i = 1;; i++, str = NULL)
  {
    token = strtok_r(str, "\n", &saveptr);
    if (token == NULL)
      break;
    sprintf(sentence, "%s\n\n", token);
    // Pointer to struct containing the parsed data
    nmea_s *data;
    char buf[255];

    // Parse it...
    data = nmea_parse(sentence, strlen(sentence), 0);
    if (NULL == data)
    {
      //printf("No data\n");
    }
    else
    {

      if (NMEA_GPRMC == data->type)
      {
        printf("GPRMC sentence\n");
        nmea_gprmc_s *pos = (nmea_gprmc_s *)data;
        printf("Longitude:\n");
        printf("  Degrees: %d\n", pos->longitude.degrees);
        printf("  Minutes: %f\n", pos->longitude.minutes);
        printf("  Cardinal: %c\n", (char)pos->longitude.cardinal);
        printf("Latitude:\n");
        printf("  Degrees: %d\n", pos->latitude.degrees);
        printf("  Minutes: %f\n", pos->latitude.minutes);
        printf("  Cardinal: %c\n", (char)pos->latitude.cardinal);
        strftime(buf, sizeof(buf), "%H:%M:%S", &pos->time);
        printf("Time: %s\n", buf);
      }

      if (NMEA_GPGGA == data->type)
      {
        nmea_gpgga_s *gpgga = (nmea_gpgga_s *)data;

        printf("GPGGA Sentence\n");
        printf("Number of satellites: %d\n", gpgga->n_satellites);
        printf("Altitude: %d %c\n", gpgga->altitude, gpgga->altitude_unit);
      }

      if (NMEA_GPGLL == data->type)
      {
        nmea_gpgll_s *gpgll = (nmea_gpgll_s *)data;

        printf("GPGLL Sentence\n");
        printf("Longitude:\n");
        printf("  Degrees: %d\n", gpgll->longitude.degrees);
        printf("  Minutes: %f\n", gpgll->longitude.minutes);
        printf("  Cardinal: %c\n", (char)gpgll->longitude.cardinal);
        printf("Latitude:\n");
        printf("  Degrees: %d\n", gpgll->latitude.degrees);
        printf("  Minutes: %f\n", gpgll->latitude.minutes);
        printf("  Cardinal: %c\n", (char)gpgll->latitude.cardinal);
      }

      nmea_free(data);
    }
  }

  free(str);
  free(token);
  free(saveptr);
  free(sentence);
}

static void app(const char *address)
{
  SOCKET sock = init_connection(address);

  fd_set rdfs;

  while (1)
  {
    FD_ZERO(&rdfs);

    /* add STDIN_FILENO */
    FD_SET(STDIN_FILENO, &rdfs);

    /* add the socket */
    FD_SET(sock, &rdfs);

    if (select(sock + 1, &rdfs, NULL, NULL, NULL) == -1)
    {
      perror("select()");
      exit(errno);
    }

    if (FD_ISSET(sock, &rdfs))
    {
      int n = read_server(sock, buffer);
      /* server down */
      if (n == 0)
      {
        printf("Server disconnected !\n");
        break;
      }
      parsing();
    }
  }
  end_connection(sock);
}

static int init_connection(const char *address)
{
  SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
  SOCKADDR_IN sin = {0};
  struct hostent *hostinfo;

  if (sock == INVALID_SOCKET)
  {
    perror("socket()");
    exit(errno);
  }

  hostinfo = gethostbyname(address);
  if (hostinfo == NULL)
  {
    fprintf(stderr, "Unknown host %s.\n", address);
    exit(EXIT_FAILURE);
  }

  sin.sin_addr = *(IN_ADDR *)hostinfo->h_addr;
  sin.sin_port = htons(PORT);
  sin.sin_family = AF_INET;

  if (connect(sock, (SOCKADDR *)&sin, sizeof(SOCKADDR)) == SOCKET_ERROR)
  {
    perror("connect()");
    exit(errno);
  }

  return sock;
}

static void end_connection(int sock)
{
  closesocket(sock);
}

static int read_server(SOCKET sock, char *buffer)
{
  int n = 0;

  if ((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0)
  {
    perror("recv()");
    exit(errno);
  }

  buffer[n] = 0;

  return n;
}

int main(int argc, char **argv)
{
  if (argc < 1)
  {
    printf("Usage : %s [address]\n", argv[0]);
    return EXIT_FAILURE;
  }

  app(argv[1]);

  return 0;
}

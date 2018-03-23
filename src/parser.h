//
//  parser.h
//  NMEA_NET_Test
//
//  Created by Hugo Bidois on 23/03/2018.
//  Copyright © 2018 Hugo Bidois. All rights reserved.
//

#ifndef parser_h
#define parser_h

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> /* close */
#include <netdb.h>  /* gethostbyname */

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#define CRLF "\r\n"
#define PORT 55555

#define BUF_SIZE 1024

static void parsing();
void app(const char *address);
static int init_connection(const char *address);
static void end_connection(int sock);
static int read_server(SOCKET sock, char *buffer);

#endif /* parser_h */

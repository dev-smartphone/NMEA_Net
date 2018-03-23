//
//  parser.c
//  NMEA_NET_Test
//
//  Created by Hugo Bidois on 23/03/2018.
//  Copyright © 2018 Hugo Bidois. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include "minmea.h"
#include "parser.h"


char buffer[BUF_SIZE];

static void parsing()
{
    char *str = NULL, *token;
    char *saveptr;
    char *sentence = malloc((MINMEA_MAX_LENGTH + 2) * sizeof(char));
    
    str = buffer;
    for (int i = 1;; i++, str = NULL)
    {
        token = strtok_r(str, "\n", &saveptr);
        if (token == NULL)
            break;
        sprintf(sentence, "%s\n\n", token);
            switch (minmea_sentence_id(sentence, false)) {
                case MINMEA_SENTENCE_RMC: {
                    struct minmea_sentence_rmc frame;
                    if (minmea_parse_rmc(&frame, sentence)) {
                        printf("$RMC: raw coordinates and speed: (%d/%d,%d/%d) %d/%d\n",
                               frame.latitude.value, frame.latitude.scale,
                               frame.longitude.value, frame.longitude.scale,
                               frame.speed.value, frame.speed.scale);
                        printf("$RMC fixed-point coordinates and speed scaled to three decimal places: (%d,%d) %d\n",
                               minmea_rescale(&frame.latitude, 1000),
                               minmea_rescale(&frame.longitude, 1000),
                               minmea_rescale(&frame.speed, 1000));
                        printf("$RMC floating point degree coordinates and speed: (%f,%f) %f\n",
                               minmea_tocoord(&frame.latitude),
                               minmea_tocoord(&frame.longitude),
                               minmea_tofloat(&frame.speed));
                    }
                } break;
                    
                case MINMEA_SENTENCE_GGA: {
                    struct minmea_sentence_gga frame;
                    if (minmea_parse_gga(&frame, sentence)) {
                        printf("$GGA: fix quality: %d\n", frame.fix_quality);
                    }
                } break;
                    
                case MINMEA_SENTENCE_GSV: {
                    struct minmea_sentence_gsv frame;
                    if (minmea_parse_gsv(&frame, sentence)) {
                        printf("$GSV: message %d of %d\n", frame.msg_nr, frame.total_msgs);
                        printf("$GSV: sattelites in view: %d\n", frame.total_sats);
                        for (int i = 0; i < 4; i++)
                            printf("$GSV: sat nr %d, elevation: %d, azimuth: %d, snr: %d dbm\n",
                                   frame.sats[i].nr,
                                   frame.sats[i].elevation,
                                   frame.sats[i].azimuth,
                                   frame.sats[i].snr);
                    }
                } break;
                
                default: {
                    
                }
            }
    }
    
    free(str);
    free(token);
    free(saveptr);
    free(sentence);
}

void app(const char *address)
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
    
    if ((n = (int)recv(sock, buffer, BUF_SIZE - 1, 0)) < 0)
    {
        perror("recv()");
        exit(errno);
    }
    
    buffer[n] = 0;
    
    return n;
}
/*
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
*/

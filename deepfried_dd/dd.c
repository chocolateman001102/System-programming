/**
 * deepfried_dd
 * CS 341 - Fall 2022
 */
#include "format.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <signal.h>
#include <time.h>
#include <string.h>

extern char* optarg;
static FILE *i_file;
static FILE *o_file ;

static size_t block_size = 512;
static size_t total_copied_blocks = -1;
static size_t i_block_skipped = 0;
static size_t o_block_skipped = 0;

static size_t total_bytes_copied = 0;
static size_t f_b_i ;
static size_t p_b_i ;
static size_t f_b_o ;
static size_t p_b_o ;

static int status ;
static struct timespec start, end;
void Signal_Handler();

int main(int argc, char **argv) {


    signal(SIGUSR1, Signal_Handler);
    int opt = 0;

    i_file = stdin;
    o_file = stdout;

    while ((opt = getopt(argc, argv, "i:o:b:c:p:k:")) != -1) {
        switch (opt) {
            case 'i':
                i_file = fopen(optarg, "r");
                if (i_file == NULL) {
                    print_invalid_input(optarg);
                    if (i_file && i_file != stdin) {
                        fclose(i_file);
                    } 
                    if (o_file && o_file != stdout)  {
                        fclose(o_file);
                    }
                    exit(1);
                }
                break;
            case 'o':
                o_file = fopen(optarg, "r+");
                if (o_file == NULL) {
                    //不存在则建立新文件
                    if((o_file = fopen(optarg, "w")) == NULL) {
                        //建立新文件失败
                        print_invalid_output(optarg);
                        if (i_file && i_file != stdin)  {
                            fclose(i_file);
                        }
                        if (o_file && o_file != stdout) {
                            fclose(o_file);
                        }
                        exit(1);
                    }
                }
                break;
            case 'b':
                block_size = atoi(optarg);
                break;
            case 'c':
                total_copied_blocks = atoi(optarg);
                break;
            case 'p':
                i_block_skipped = atoi(optarg);
                break;
            case 'k':
                o_block_skipped = atoi(optarg);
                break;
            default:
                exit(1);
        }
    }
    if (i_file != stdin) {
        fseek(i_file, i_block_skipped * block_size, SEEK_SET);
    }
    if (o_file != stdout && o_block_skipped) {
        fseek(o_file, o_block_skipped * block_size, SEEK_SET);
    }

    clock_gettime(CLOCK_REALTIME, &start);
    char data_block[block_size];

    while (feof(i_file) == 0 && total_copied_blocks) {
            
        size_t bytes_read = fread(data_block, 1, block_size, i_file);

        if (bytes_read == block_size) {
            f_b_i ++;
        } else if (bytes_read > 0) {
            p_b_i ++;
        }

        size_t bytes_write = fwrite(data_block, 1, bytes_read, o_file);
        if (bytes_write == block_size) {
            f_b_o ++;
        } else if (bytes_write > 0) {
            p_b_o ++;
        }

        total_bytes_copied += bytes_write;
        total_copied_blocks --;

        if (status) {
            clock_gettime(CLOCK_REALTIME, &end);
            double time_ = (end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec) / 1000000000;
            print_status_report(f_b_i, p_b_i, f_b_o, p_b_o, total_bytes_copied, time_);
            status = 0;
        }

    }
    clock_gettime(CLOCK_REALTIME, &end);
    double time_ = (end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec) / 1000000000;
    print_status_report(f_b_i, p_b_i, f_b_o, p_b_o, total_bytes_copied, time_);
    
    if (i_file && i_file != stdin) {
        fclose(i_file);
    }
    if (o_file && o_file != stdout)  {
        fclose(o_file);
    }
    return 0;
}

void Signal_Handler() {
    status = 1;
}
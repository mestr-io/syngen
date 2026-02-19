#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "faker.h"
#include "generator.h"
#include "export_manager.h"

void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s -c <channels> -m <messages> -u <users> <output_filename>\n", prog_name);
    fprintf(stderr, "Defaults: -c 25 -m 1000 -u 10\n");
}

int main(int argc, char *argv[]) {
    int c_count = 25;
    int m_count = 1000;
    int u_count = 10;
    char *output_filename = NULL;
    
    int opt;
    while ((opt = getopt(argc, argv, "c:m:u:")) != -1) {
        switch (opt) {
            case 'c':
                c_count = atoi(optarg);
                break;
            case 'm':
                m_count = atoi(optarg);
                break;
            case 'u':
                u_count = atoi(optarg);
                break;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }
    
    if (optind < argc) {
        output_filename = argv[optind];
    } else {
        fprintf(stderr, "Error: Missing output filename.\n");
        print_usage(argv[0]);
        return 1;
    }
    
    if (c_count <= 0 || m_count <= 0 || u_count <= 0) {
        fprintf(stderr, "Error: Counts must be positive integers.\n");
        return 1;
    }
    
    printf("Syngen - Synthetic Slack Export Generator\n");
    printf("Configuration:\n");
    printf("  Users:    %d\n", u_count);
    printf("  Channels: %d\n", c_count);
    printf("  Messages: %d\n", m_count);
    printf("  Output:   %s\n", output_filename);
    
    faker_init();
    
    // Create temporary directory for generation
    char temp_dir[64];
    snprintf(temp_dir, sizeof(temp_dir), "syngen_temp_%d", getpid());
    
    printf("Generating data...\n");
    User *users = generate_users(u_count);
    Channel *channels = generate_channels(c_count, users, u_count);
    Message *messages = generate_messages(m_count, channels, c_count, users, u_count);
    
    printf("Exporting data to %s...\n", temp_dir);
    export_init(temp_dir);
    export_write_users(temp_dir, users, u_count);
    export_write_channels(temp_dir, channels, c_count);
    export_write_messages(temp_dir, messages, m_count, channels, c_count);
    
    export_finalize(temp_dir, output_filename);
    
    // Cleanup temporary directory
    printf("Cleaning up...\n");
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
    system(cmd);
    
    free_messages(messages, m_count);
    free_channels(channels, c_count);
    free_users(users, u_count);
    
    printf("Success!\n");

    return 0;
}

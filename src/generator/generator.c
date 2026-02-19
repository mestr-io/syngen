#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 500 // For M_PI usually, or just define it
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "generator.h"
#include "faker.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Box-Muller transform to generate standard normal distribution
static double rand_normal() {
    double u1 = (double)rand() / RAND_MAX;
    double u2 = (double)rand() / RAND_MAX;
    
    // Avoid log(0)
    if (u1 < 1e-9) u1 = 1e-9;
    
    return sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
}

// Generate an index with Gaussian distribution clamped to [0, max-1]
static int rand_gaussian_index(int max) {
    double mean = max / 2.0;
    // 3 sigma covers 99.7% of the range. So sigma = range / 6 ?
    // Let's say range is roughly 0 to max. So sigma = max / 6.
    double sigma = max / 6.0;
    if (sigma < 1.0) sigma = 1.0;
    
    double val = rand_normal() * sigma + mean;
    int idx = (int)round(val);
    
    if (idx < 0) idx = 0;
    if (idx >= max) idx = max - 1;
    return idx;
}

User *generate_users(int count) {
    User *users = malloc(sizeof(User) * count);
    for (int i = 0; i < count; i++) {
        faker_create_user(&users[i]);
    }
    return users;
}

Channel *generate_channels(int count, User *users, int user_count) {
    Channel *channels = malloc(sizeof(Channel) * count);
    for (int i = 0; i < count; i++) {
        // Pick a random creator
        int creator_idx = rand() % user_count;
        faker_create_channel(&channels[i], users[creator_idx].id);
        
        // Assign members (random subset)
        // For simplicity, let's say 20-80% of users are in each channel
        int num_members = (rand() % (user_count / 2 + 1)) + (user_count / 5);
        if (num_members > user_count) num_members = user_count;
        if (num_members < 1) num_members = 1; // At least creator
        
        channels[i].members = malloc(sizeof(char*) * num_members);
        channels[i].member_count = 0;
        
        // Always add creator first
        channels[i].members[0] = strdup(channels[i].creator);
        channels[i].member_count++;
        
        // Add others (simple shuffle or random pick avoiding duplicates is O(N^2) or O(N) with shuffle)
        // Since user_count is small (10-100), brute force check is fine.
        while (channels[i].member_count < num_members) {
            int u_idx = rand() % user_count;
            char *uid = users[u_idx].id;
            
            // Check existence
            int exists = 0;
            for (int k = 0; k < channels[i].member_count; k++) {
                if (strcmp(channels[i].members[k], uid) == 0) {
                    exists = 1;
                    break;
                }
            }
            
            if (!exists) {
                channels[i].members[channels[i].member_count] = strdup(uid);
                channels[i].member_count++;
            }
        }
    }
    return channels;
}

Message *generate_messages(int count, Channel *channels, int channel_count, User *users, int user_count) {
    (void)user_count;
    
    Message *messages = malloc(sizeof(Message) * count);
    
    // Sort channels? No, we access by index.
    
    // Time window: Last 30 days
    time_t now = time(NULL);
    time_t start = now - (30 * 24 * 3600);
    
    for (int i = 0; i < count; i++) {
        // Pick Channel using Gaussian
        int ch_idx = rand_gaussian_index(channel_count);
        Channel *ch = &channels[ch_idx];
        
        // Pick User from Channel Members
        // We can just pick uniformly from members for now, or Gaussian if we want "loud" users
        if (ch->member_count > 0) {
            int m_idx = rand() % ch->member_count;
            // Find the user object to verify? No need, we have the ID.
            strcpy(messages[i].user, ch->members[m_idx]);
        } else {
            // Fallback (shouldn't happen)
            strcpy(messages[i].user, users[0].id);
        }
        
        // Store Channel ID for grouping later
        strcpy(messages[i].channel, ch->id);
        
        strcpy(messages[i].type, "message");
        messages[i].ts = faker_get_timestamp(start, now);
        messages[i].text = faker_lorem_sentence(3, 20);
    }
    
    // Sort messages by timestamp?
    // It's better if they are sorted, but for daily files, we filter by date.
    // Let's sort them now to make file writing easier (sequential access).
    // Simple bubble sort or qsort.
    
    // qsort comparator
    int compare_msgs(const void *a, const void *b) {
        Message *ma = (Message *)a;
        Message *mb = (Message *)b;
        if (ma->ts < mb->ts) return -1;
        if (ma->ts > mb->ts) return 1;
        return 0;
    }
    
    qsort(messages, count, sizeof(Message), compare_msgs);
    
    return messages;
}

void free_users(User *users, int count) {
    (void)count; // unused
    free(users);
}

void free_channels(Channel *channels, int count) {
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < channels[i].member_count; j++) {
            free(channels[i].members[j]);
        }
        free(channels[i].members);
    }
    free(channels);
}

void free_messages(Message *messages, int count) {
    for (int i = 0; i < count; i++) {
        free(messages[i].text);
    }
    free(messages);
}

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "export_manager.h"
#include "cJSON.h"
#include "faker.h" // For timestamp helpers if needed, or just time.h

static void create_directory(const char *path) {
    if (mkdir(path, 0755) == -1) {
        if (errno != EEXIST) {
            perror("mkdir");
        }
    }
}

void export_init(const char *base_path) {
    create_directory(base_path);
}

void export_write_users(const char *base_path, User *users, int count) {
    cJSON *root = cJSON_CreateArray();
    
    for (int i = 0; i < count; i++) {
        cJSON *user = cJSON_CreateObject();
        cJSON_AddStringToObject(user, "id", users[i].id);
        cJSON_AddStringToObject(user, "name", users[i].name);
        cJSON_AddStringToObject(user, "real_name", users[i].real_name);
        cJSON_AddStringToObject(user, "team_id", "T012345678"); // Fake Team ID
        
        // Profile
        cJSON *profile = cJSON_CreateObject();
        cJSON_AddStringToObject(profile, "email", users[i].email);
        cJSON_AddStringToObject(profile, "real_name", users[i].real_name);
        cJSON_AddStringToObject(profile, "display_name", users[i].name);
        cJSON_AddStringToObject(profile, "image_original", users[i].image_original);
        // Add fake image sizes
        cJSON_AddStringToObject(profile, "image_24", users[i].image_original);
        cJSON_AddStringToObject(profile, "image_32", users[i].image_original);
        
        cJSON_AddItemToObject(user, "profile", profile);
        
        cJSON_AddBoolToObject(user, "is_admin", users[i].is_admin);
        cJSON_AddBoolToObject(user, "is_owner", users[i].is_admin); // Make admins owners for simplicity
        cJSON_AddBoolToObject(user, "is_bot", users[i].is_bot);
        cJSON_AddBoolToObject(user, "deleted", false);
        
        cJSON_AddItemToArray(root, user);
    }
    
    char path[512];
    snprintf(path, sizeof(path), "%s/users.json", base_path);
    
    char *json_str = cJSON_Print(root);
    FILE *fp = fopen(path, "w");
    if (fp) {
        fputs(json_str, fp);
        fclose(fp);
    } else {
        perror("fopen users.json");
    }
    
    free(json_str);
    cJSON_Delete(root);
}

void export_write_channels(const char *base_path, Channel *channels, int count) {
    cJSON *root = cJSON_CreateArray();
    
    for (int i = 0; i < count; i++) {
        cJSON *ch = cJSON_CreateObject();
        cJSON_AddStringToObject(ch, "id", channels[i].id);
        cJSON_AddStringToObject(ch, "name", channels[i].name);
        cJSON_AddNumberToObject(ch, "created", (double)channels[i].created);
        cJSON_AddStringToObject(ch, "creator", channels[i].creator);
        cJSON_AddBoolToObject(ch, "is_archived", false);
        cJSON_AddBoolToObject(ch, "is_general", false);
        
        cJSON *members = cJSON_CreateStringArray((const char **)channels[i].members, channels[i].member_count);
        cJSON_AddItemToObject(ch, "members", members);
        
        cJSON_AddItemToArray(root, ch);
        
        // Create directory for this channel
        char dir_path[512];
        snprintf(dir_path, sizeof(dir_path), "%s/%s", base_path, channels[i].name);
        create_directory(dir_path);
    }
    
    char path[512];
    snprintf(path, sizeof(path), "%s/channels.json", base_path);
    
    char *json_str = cJSON_Print(root);
    FILE *fp = fopen(path, "w");
    if (fp) {
        fputs(json_str, fp);
        fclose(fp);
    }
    
    free(json_str);
    cJSON_Delete(root);
}

static char *get_channel_name_by_id(Channel *channels, int count, const char *id) {
    for (int i = 0; i < count; i++) {
        if (strcmp(channels[i].id, id) == 0) {
            return channels[i].name;
        }
    }
    return "unknown";
}

void export_write_messages(const char *base_path, Message *messages, int count, Channel *channels, int channel_count) {
    // We need to group by channel and date.
    // Since implementing a hash map is complex in C, we will iterate and append to files.
    // This is inefficient but simple.
    // To make it slightly better, we process one message at a time, check if the file exists, read it (if we want valid JSON array), append...
    // But appending to a JSON array is hard with file I/O.
    // Better strategy:
    // 1. Sort messages by channel and date. (They are sorted by timestamp already).
    // 2. Iterate. When channel or date changes, close previous JSON array and start new one.
    
    // Sort logic requires creating a struct that holds all info. 
    // `Message` struct has everything except Channel Name (it has ID) and Date string.
    
    // Let's create a temporary index array and sort it.
    struct MsgRef {
        int index;
        char channel_id[12];
        time_t date; // Rounded to day
        double ts;
    };
    
    struct MsgRef *refs = malloc(sizeof(struct MsgRef) * count);
    for (int i = 0; i < count; i++) {
        refs[i].index = i;
        strcpy(refs[i].channel_id, messages[i].channel);
        refs[i].ts = messages[i].ts;
        
        // Calculate date
        time_t t = (time_t)messages[i].ts;
        struct tm *tm_info = localtime(&t);
        // Normalize to midnight
        tm_info->tm_sec = 0;
        tm_info->tm_min = 0;
        tm_info->tm_hour = 0;
        refs[i].date = mktime(tm_info);
    }
    
    // Sort by Channel ID, then Date, then Timestamp
    int compare_refs(const void *a, const void *b) {
        struct MsgRef *ra = (struct MsgRef *)a;
        struct MsgRef *rb = (struct MsgRef *)b;
        
        int ch_cmp = strcmp(ra->channel_id, rb->channel_id);
        if (ch_cmp != 0) return ch_cmp;
        
        if (ra->date < rb->date) return -1;
        if (ra->date > rb->date) return 1;
        
        if (ra->ts < rb->ts) return -1;
        if (ra->ts > rb->ts) return 1;
        
        return 0;
    }
    
    qsort(refs, count, sizeof(struct MsgRef), compare_refs);
    
    // Now iterate and write
    cJSON *current_array = NULL;
    char current_file_path[512] = {0};
    
    for (int i = 0; i < count; i++) {
        int original_idx = refs[i].index;
        Message *m = &messages[original_idx];
        
        // Determine file path
        char *ch_name = get_channel_name_by_id(channels, channel_count, refs[i].channel_id);
        
        time_t t = (time_t)m->ts;
        struct tm *tm_info = localtime(&t);
        char date_str[12];
        strftime(date_str, sizeof(date_str), "%Y-%m-%d", tm_info);
        
        char file_path[512];
        snprintf(file_path, sizeof(file_path), "%s/%s/%s.json", base_path, ch_name, date_str);
        
        // If file path changed, write current array and start new one
        if (strcmp(file_path, current_file_path) != 0) {
            if (current_array) {
                // Write previous file
                char *json_str = cJSON_Print(current_array);
                FILE *fp = fopen(current_file_path, "w");
                if (fp) {
                    fputs(json_str, fp);
                    fclose(fp);
                }
                free(json_str);
                cJSON_Delete(current_array);
            }
            
            strcpy(current_file_path, file_path);
            current_array = cJSON_CreateArray();
        }
        
        // Add message to current array
        cJSON *msg = cJSON_CreateObject();
        cJSON_AddStringToObject(msg, "user", m->user);
        cJSON_AddStringToObject(msg, "type", m->type);
        
        char ts_str[32];
        snprintf(ts_str, sizeof(ts_str), "%.6f", m->ts);
        cJSON_AddStringToObject(msg, "ts", ts_str);
        
        cJSON_AddStringToObject(msg, "text", m->text);
        
        cJSON_AddItemToArray(current_array, msg);
    }
    
    // Write last array
    if (current_array) {
        char *json_str = cJSON_Print(current_array);
        FILE *fp = fopen(current_file_path, "w");
        if (fp) {
            fputs(json_str, fp);
            fclose(fp);
        }
        free(json_str);
        cJSON_Delete(current_array);
    }
    
    free(refs);
}

void export_finalize(const char *base_path, const char *output_filename) {
    // System call to zip
    char command[1024];
    // Zip contents of base_path into output_filename
    // cd base_path && zip -r ../output_filename .
    
    // Ensure output filename is absolute or handles paths correctly.
    // For simplicity, we assume output_filename is in current dir or relative to CWD.
    // zip command: zip -r <zipfile> <directory>
    
    // Better: zip -r output.zip base_path
    // But then the zip root is base_path. Slack expects files at root.
    // So we must cd into base_path.
    
    snprintf(command, sizeof(command), "cd %s && zip -q -r ../%s .", base_path, output_filename);
    
    printf("Creating archive %s...\n", output_filename);
    int ret = system(command);
    if (ret != 0) {
        fprintf(stderr, "Zip command failed with code %d\n", ret);
    }
}

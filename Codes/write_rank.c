#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#define RANK_FILE "rank.txt"

void write_rank(const char *user_id, int score, const char *start_time) {
    FILE *file = fopen(RANK_FILE, "a");
    if (file == NULL) {
        perror("Unable to open rank.txt");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "%s,%d,%s\n", user_id, score, start_time);
    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <user_id> <score> <start_time>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *user_id = argv[1];
    int score = atoi(argv[2]);
    const char *start_time = argv[3];

    write_rank(user_id, score, start_time);

    return 0;
}

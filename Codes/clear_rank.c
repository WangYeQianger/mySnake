#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *file = fopen("rank.txt", "w"); // 确保路径正确
    if (file == NULL) {
        perror("Unable to open rank.txt to clear");
        return 1;
    }
    fclose(file);
    return 0;
}

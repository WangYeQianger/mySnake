#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *file = fopen("rank.txt", "w"); // ȷ��·����ȷ
    if (file == NULL) {
        perror("Unable to open rank.txt to clear");
        return 1;
    }
    fclose(file);
    return 0;
}

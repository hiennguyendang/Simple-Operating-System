#include <stdio.h>
#include <stdlib.h>

int main() {
    // Danh sách các lệnh cần chạy
    const char *commands[] = {
        "./os run_on_1_cpu",
        "./os run_on_many_cpu",
        "./os kill_process",
    };

    const int num_commands = sizeof(commands) / sizeof(commands[0]);

    for (int i = 0; i < num_commands; i++) {
        // Tạo tên file output tương ứng
        char filename[64];
        if (i == 0) {
            snprintf(filename, sizeof(filename), "./output/run_on_1_cpu.txt");
        } else if (i == 1) {
            snprintf(filename, sizeof(filename), "./output/run_on_many_cpu.txt");
        }
        else if (i == 2) {
            snprintf(filename, sizeof(filename), "./output/kill_process.txt");
        }

        // Mở tiến trình chạy lệnh
        FILE *fp = popen(commands[i], "r");
        if (fp == NULL) {
            perror("Failed to run command");
            continue;
        }

        // Mở file output để ghi kết quả
        FILE *out = fopen(filename, "w");
        if (out == NULL) {
            perror("Failed to open output file");
            pclose(fp);
            continue;
        }

        // Ghi kết quả đầu ra vào file
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            fputs(buffer, out);
        }

        // Đóng lại
        fclose(out);
        pclose(fp);
    }

    return 0;
}

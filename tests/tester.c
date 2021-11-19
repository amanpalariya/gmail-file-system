#include <stdbool.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>

#include "../logger.h"
#include "../configure.h"

int total_tests = 0;
int passed_tests = 0;
int failed_tests = 0;

void test_log(char* test_name, bool success) {
    total_tests++;
    if (success) {
        printf("\033[0;32m[✓]: %s\033[0m\n", test_name);
        passed_tests++;
    } else {
        printf("\033[0;31m[ ]: %s\033[0m\n", test_name);
        failed_tests++;
    }
}

void print_test_section(char* section) {
    printf("\n\033[0;1m%s\033[0m\n\n", section);
}

void test_config(){
    struct login_config* config = read_config_from_file("./test_config.txt");
    test_log("Correct URL formation", strcmp(config->url, "imaps://test.example.com:2021")==0);
    test_log("Proper username picked", strcmp(config->username, "uname")==0);
    test_log("Proper password picked", strcmp(config->password, "passphrase")==0);
}

int main(int argc, char** argv) {
    mute_logs();
    test_config();
    printf("\n%d/%d tests passed\n", passed_tests, total_tests);
    return 0;
}

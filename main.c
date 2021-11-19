#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "configure.h"
#include "fs.h"
#include "logger.h"

int main(int argc, char** argv) {
    if (argc<3){
	log_error("3 arguments expected (got %d)", argc);
	return 1;
    }
    char* mount_point = argv[1];
    char* config_file = argv[2];
    struct login_config* config = read_config_from_file(config_file);
    log_debug("Calling mount");
    mount_custom_fs(mount_point, config);
    return 0;
}

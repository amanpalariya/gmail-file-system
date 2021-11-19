#ifndef CS303_CONFIG_READER_H
#define CS303_CONFIG_READER_H

struct login_config{
    char* url;
    char* username;
    char* password;
};

struct login_config* read_config_from_file(char* filename);

#endif

#include "configure.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* remove_newline_if_present_at_end(char* str){
    unsigned long len = strlen(str);
    if (str[len-1]=='\n')
	str[len-1] = '\0';
    return str;
}

char* get_url_from_hostname_and_port(char* hostname, char* port){
    char* proto = "imaps";
    unsigned long len = strlen(proto) + strlen(hostname) + strlen(port) + 10;
    char* url = (char*)malloc((len+1)*sizeof(char));
    sprintf(url, "%s://%s:%s", proto, hostname, port);
    return url;
}

struct login_config* read_config_from_file(char* filename){
    struct login_config* config = (struct login_config*)malloc(sizeof(struct login_config));
    FILE* f = fopen(filename, "r");
    char* hostname = NULL;
    char* port = NULL;
    char* username = NULL;
    char* password = NULL;
    size_t len;
    getline(&hostname, &len, f);
    getline(&port, &len, f);
    getline(&username, &len, f);
    getline(&password, &len, f);
    fclose(f);

    remove_newline_if_present_at_end(hostname);
    remove_newline_if_present_at_end(port);
    remove_newline_if_present_at_end(username);
    remove_newline_if_present_at_end(password);

    config->url = get_url_from_hostname_and_port(hostname, port);
    config->username = username;
    config->password = password;
    return config;
}

#include <asm-generic/errno-base.h>
#define FUSE_USE_VERSION 31

#include "fs.h"
#include "configure.h"
#include "logger.h"
#include <fuse.h>
#include <curl/curl.h>
#include <string.h>
#include <curl/easy.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

struct __data__{
    CURL* curl;
    char* url;
    char* parent_label;
    char* temp_file;
};

struct __data__* get_private_data(){
    return ((struct __data__*)(fuse_get_context()->private_data));
}

unsigned int get_depth_of_path(const char* path){
    unsigned long len = strlen(path);
    unsigned int count = 0;
    for(int i = 0; i < len; i++){
	if (path[i]=='/') count++;
    }
    count -= (path[len-1]=='/'?1:0);
    return count;
}

char* get_dir_name(const char* path){
    switch (get_depth_of_path(path)) {
	case 2:
	    {
		char* dir_end = strstr(path+1, "/");
		unsigned long len = (dir_end - path - 1);
		char* dir_name = (char*)malloc((len+1)*sizeof(char));
		strncpy(dir_name, path+1, len);
		dir_name[len] = '\0';
		return dir_name;
	    }
	default:
	    return NULL;
    }
}

char* get_file_name(const char* path){
    switch (get_depth_of_path(path)) {
	case 2:
	    {
		char* dir_end = strstr(path+1, "/");
		return dir_end+1;
	    }
	default:
	    return NULL;
    }
}

size_t __null_callback__(char *ptr, size_t size, size_t nmemb, FILE *stream) { return size*nmemb; }

void* login_and_get_private_data(struct login_config* config){
    log_debug("Calling login_and_get_private_data()");
    log_debug("Initializing private data");
    struct __data__* data = (struct __data__*)malloc(sizeof(struct __data__));
    data->curl = curl_easy_init();
    data->url = config->url;
    data->parent_label = "Assignment5";
    data->temp_file = "/tmp/cs303-temp-file.txt";
    log_debug("Initialized private data");
    if(data->curl){
	char buff[1000];
	curl_easy_setopt(data->curl, CURLOPT_PROTOCOLS, CURLPROTO_IMAPS);
	curl_easy_setopt(data->curl, CURLOPT_USERNAME,config->username);
	curl_easy_setopt(data->curl, CURLOPT_PASSWORD,config->password);
	curl_easy_setopt(data->curl, CURLOPT_URL,config->url);
	curl_easy_setopt(data->curl, CURLOPT_WRITEFUNCTION, __null_callback__);
	log_info("Logging in...");
	CURLcode res = curl_easy_perform(data->curl);
	if (res!=CURLE_OK){
	    log_error("%s", curl_easy_strerror(res));
	    free(data);
	    exit(1);
	}
	log_info("Logged in successfully");
    }else{
	log_error("Unable to initialize curl");
	free(data);
	exit(1);
    }
    return (void*)data;
}

static int __create__(const char* path, mode_t mode, struct fuse_file_info* fi){
    log_debug("Calling create(\"%s\")", path);
    switch (get_depth_of_path(path)) {
	case 0:
	    {
		return -EPERM;
	    }
	case 1:
	    {
		return -EPERM;
	    }
	case 2:
	    {
		return 0;
	    }
	default:
	    break;
    }
    return -EPERM;
}

static int __getattr__(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    log_debug("Calling getattr(\"%s\")", path);
    memset(stbuf, 0, sizeof(struct stat));
    struct __data__* data = get_private_data();
    switch (get_depth_of_path(path)) {
	case 0:
	    {
	    stbuf->st_mode = S_IFDIR | 0755;
	    return 0;
	    }
	case 1:
	    {
	    char request[1000];
	    sprintf(request, "EXAMINE \"%s%s\"", data->parent_label, path);
	    curl_easy_setopt(data->curl, CURLOPT_CUSTOMREQUEST, request);
	    curl_easy_setopt(data->curl, CURLOPT_WRITEFUNCTION, __null_callback__);

	    CURLcode res = curl_easy_perform(data->curl);
	    if (res==CURLE_QUOTE_ERROR) return -ENOENT;
	    if (res!=CURLE_OK) return -EAGAIN;

	    stbuf->st_mode = S_IFDIR | 0755;
	    return 0;
	    }
	case 2:
	    {
		stbuf->st_mode = S_IFREG | 0655;
		char request[1000];
		char url[1000];
		FILE* f = fopen(data->temp_file, "w");
		sprintf(request, "SEARCH SUBJECT \"%s\"", get_file_name(path));
		sprintf(url, "%s/%s/%s", data->url, data->parent_label, get_dir_name(path));
		curl_easy_setopt(data->curl, CURLOPT_URL, url);
		curl_easy_setopt(data->curl, CURLOPT_CUSTOMREQUEST, request);
		curl_easy_setopt(data->curl, CURLOPT_WRITEDATA, f);
		curl_easy_setopt(data->curl, CURLOPT_WRITEFUNCTION, fwrite);
		log_debug("Finding mails...");
		{
		CURLcode curl_res = curl_easy_perform(data->curl);
		if (curl_res!=CURLE_OK){
		    log_error(curl_easy_strerror(curl_res));
		    return -ENOENT;
		}
		}
		char* line = NULL;
		size_t len = 0;
		fclose(f);
		f = fopen(data->temp_file, "r");
		int res = getline(&line, &len, f);
		if (res>0){
		    if (strlen(line)>strlen("* SEARCH\r\n")) return 0;
		}else{
		    return -ENOENT;
		}
		fclose(f);
		return -ENOENT;
	    }
	default:
	    break;
    }
    return -ENOENT;
}

static int __mkdir__(const char* path, mode_t mode){
    log_debug("Calling mkdir(\"%s\")", path);
    struct __data__* data = get_private_data();
    switch (get_depth_of_path(path)) {
	case 0:
	    {
		return -EEXIST;
	    }
	case 1:
	    {
		char request[1000];
		sprintf(request, "CREATE \"%s%s\"", data->parent_label, path);
		curl_easy_setopt(data->curl, CURLOPT_CUSTOMREQUEST, request);
		curl_easy_setopt(data->curl, CURLOPT_WRITEFUNCTION, __null_callback__);

		CURLcode res = curl_easy_perform(data->curl);
		if (res==CURLE_QUOTE_ERROR) return -EEXIST;
		if (res!=CURLE_OK) return -EAGAIN;

		return 0;
	    }
	case 2:
	    {
		return -EPERM;
	    }
	default:
	    break;
    }
    return -EPERM;
}

static int __readdir__(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags) {
    log_debug("Calling readdir(\"%s\")", path);
    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);
    struct __data__* data = get_private_data();
    switch (get_depth_of_path(path)) {
	case 0:
	    {
		char request[1000];
		FILE* f = fopen(data->temp_file, "w");
		sprintf(request, "LIST \"%s%s\" %%", data->parent_label, path);
		curl_easy_setopt(data->curl, CURLOPT_CUSTOMREQUEST, request);
		curl_easy_setopt(data->curl, CURLOPT_WRITEDATA, f);
		curl_easy_setopt(data->curl, CURLOPT_WRITEFUNCTION, fwrite);
		curl_easy_perform(data->curl);
		char* line = NULL;
		size_t len = 0;
		fclose(f);
		f = fopen(data->temp_file, "r");
		while(getline(&line, &len, f)>0){
		    char string_to_find[1000];
		    sprintf(string_to_find, "\"%s", data->parent_label);
		    char* starting_point = strstr(line, string_to_find) + strlen(string_to_find)+1;
		    if (starting_point==NULL) continue;
		    char* end_point = strstr(starting_point, "\"");
		    end_point[0] = '\0';
		    filler(buf, starting_point, NULL, 0, 0);
		}
		fclose(f);
		return 0;
	    }
	case 1:
	    {
		char request[1000];
		char url[1000];
		FILE* f = fopen(data->temp_file, "w");
		sprintf(request, "SEARCH ALL");
		sprintf(url, "%s/%s%s", data->url, data->parent_label, path);
		curl_easy_setopt(data->curl, CURLOPT_URL, url);
		curl_easy_setopt(data->curl, CURLOPT_CUSTOMREQUEST, request);
		curl_easy_setopt(data->curl, CURLOPT_WRITEDATA, f);
		curl_easy_setopt(data->curl, CURLOPT_WRITEFUNCTION, fwrite);
		log_debug("Finding mails...");
		{
		CURLcode curl_res = curl_easy_perform(data->curl);
		if (curl_res!=CURLE_OK){
		    log_error(curl_easy_strerror(curl_res));
		    break;
		}
		}
		char* line = NULL;
		size_t len = 0;
		fclose(f);
		f = fopen(data->temp_file, "r");
		int res = getline(&line, &len, f);
		fclose(f);
		if(res>0){
		    char string_to_find[15] = "SEARCH";
		    char* starting_point = strstr(line, string_to_find) + strlen(string_to_find)+1;
		    log_debug("Mails found %s", starting_point);
		    if (starting_point!=NULL){
			int uid = 0;
			unsigned long rem_len = strlen(starting_point);
			for (int i = 0; i < (rem_len-1); i++){
			    log_debug("Checking character '%c'", starting_point[i]);
			    if (starting_point[i] <= '9' && starting_point[i]>= '0'){
				uid = uid*10 + (starting_point[i] - '0');
			    }else{
				if (uid!=0){
				    log_debug("Fetching subject for uid=%d", uid);
				    f = fopen(data->temp_file, "w");
				    sprintf(url, "%s/%s%s;uid=%d", data->url, data->parent_label, path, uid);
				    curl_easy_setopt(data->curl, CURLOPT_URL, url);
				    curl_easy_setopt(data->curl, CURLOPT_CUSTOMREQUEST, NULL);
				    {
				    CURLcode curl_res = curl_easy_perform(data->curl);
				    if (curl_res!=CURLE_OK){
					log_error(curl_easy_strerror(curl_res));
					break;
				    }
				    }
				    fclose(f);
				    f = fopen(data->temp_file, "r");
				    char* __line = NULL;
				    while(getline(&__line, &len, f)>0){
					if (strncmp(__line, "Subject", strlen("Subject"))==0){
					    unsigned long n = strlen(__line);
					    __line[n-1] = '\0';
					    if(__line[n-2]=='\n' || __line[n-2]=='\r'){
						__line[n-2] = '\0';
					    }
					    filler(buf, __line+strlen("Subject")+2, NULL, 0, 0);
					    break;
					}
				    }
				    fclose(f);
				    uid = 0;
				}
			    }
			}
		    }
		}
		return 0;
	    }
	case 2:
	    {
		return -ENOTDIR;
	    }
	default:
	    break;
    }

    return 0;
} 

static int __read__(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    log_debug("Calling read(\"%s\")", path);
    struct __data__* data = get_private_data();
    switch (get_depth_of_path(path)) {
	case 0:
	    {
		return -EISDIR;
	    }
	case 1:
	    {
		return -EISDIR;
	    }
	case 2:
	    {
		char request[1000];
		char url[1000];
		FILE* f = fopen(data->temp_file, "w");
		sprintf(request, "SEARCH SUBJECT \"%s\"", get_file_name(path));
		sprintf(url, "%s/%s/%s", data->url, data->parent_label, get_dir_name(path));
		curl_easy_setopt(data->curl, CURLOPT_URL, url);
		curl_easy_setopt(data->curl, CURLOPT_CUSTOMREQUEST, request);
		curl_easy_setopt(data->curl, CURLOPT_WRITEDATA, f);
		curl_easy_setopt(data->curl, CURLOPT_WRITEFUNCTION, fwrite);
		log_debug("Finding mails...");
		{
		CURLcode curl_res = curl_easy_perform(data->curl);
		if (curl_res!=CURLE_OK){
		    log_error(curl_easy_strerror(curl_res));
		    break;
		}
		}
		char* line = NULL;
		size_t len = 0;
		fclose(f);
		f = fopen(data->temp_file, "r");
		int res = getline(&line, &len, f);
		fclose(f);
		if (res>0){
		    if (strlen(line)>strlen("* SEARCH\r\n")){
			line += strlen("* SEARCH ");
			unsigned long rem_len = strlen(line);
			int uid = 0;
			for (int i = 0; i < rem_len; i++){
			    if (line[i]<='9' && line[i]>='0'){
				uid = uid*10 + (line[i]-'0');
			    }else{
				break;
			    }
			}
			if (uid!=0){
			    log_debug("Fetching content for uid=%d", uid);
			    f = fopen(data->temp_file, "w");
			    sprintf(url, "%s/%s/%s;uid=%d;section=text", data->url, data->parent_label, get_dir_name(path), uid);
			    curl_easy_setopt(data->curl, CURLOPT_URL, url);
			    curl_easy_setopt(data->curl, CURLOPT_WRITEFUNCTION, fwrite);
			    curl_easy_setopt(data->curl, CURLOPT_CUSTOMREQUEST, NULL);
			    {
				CURLcode curl_res = curl_easy_perform(data->curl);
				if (curl_res!=CURLE_OK){
				    log_error(curl_easy_strerror(curl_res));
				    break;
				}
			    }
			    fclose(f);
			    f = fopen(data->temp_file, "r");
			    char* __line = NULL;
			    char* boundary = NULL;
			    log_debug("Whoa");
			    getline(&boundary, &len, f);
			    getline(&__line, &len, f);
			    unsigned int read_bytes = 0;
			    log_debug("Boundary: %s", boundary);
			    while(getline(&__line, &len, f)>0){
				log_debug("Line: %s", __line);
				if (strcmp(__line, boundary)==0){
				    break;
				}else{
				    sprintf(buf+read_bytes, "%s",__line);
				    read_bytes = strlen(buf);
				}
			    }
			    fclose(f);
			    log_debug("This is read (%d): %s", strlen(buf), buf);
			    return strlen(buf);
			}else{
			    return -EAGAIN;
			}
		    }else{
			return -ENOENT;
		    }
		}else{
		    return -EAGAIN;
		}
		return 0;
	    }
	default:
	    break;
    }
    return -EPERM;
}

static int __rename__(const char* path, const char* new_name, unsigned int flags){
    log_debug("Calling rename(\"%s\")", path);
    struct __data__* data = get_private_data();
    switch (get_depth_of_path(path)) {
	case 0:
	    {
		return -EPERM;
	    }
	case 1:
	    {
		char request[1000];
		sprintf(request, "RENAME \"%s%s\" \"%s%s\"", data->parent_label, path, data->parent_label, new_name);
		curl_easy_setopt(data->curl, CURLOPT_CUSTOMREQUEST, request);
		curl_easy_setopt(data->curl, CURLOPT_WRITEFUNCTION, __null_callback__);

		CURLcode res = curl_easy_perform(data->curl);
		if (res==CURLE_QUOTE_ERROR) return -ENOENT;
		if (res!=CURLE_OK) return -EAGAIN;

		return 0;
	    }
	case 2:
	    {
		return -ENOTDIR;
	    }
	default:
	    break;
    }
    return 0;
}

static int __rmdir__(const char* path){
    log_debug("Calling rmdir(\"%s\")", path);
    struct __data__* data = get_private_data();
    switch (get_depth_of_path(path)) {
	case 0:
	    {
		return -EPERM;
	    }
	case 1:
	    {
		char request[1000];
		sprintf(request, "DELETE \"%s%s\"", data->parent_label, path);
		curl_easy_setopt(data->curl, CURLOPT_CUSTOMREQUEST, request);
		curl_easy_setopt(data->curl, CURLOPT_WRITEFUNCTION, __null_callback__);

		CURLcode res = curl_easy_perform(data->curl);
		if (res==CURLE_QUOTE_ERROR) return -ENOENT;
		if (res!=CURLE_OK) return -EAGAIN;

		return 0;
	    }
	case 2:
	    {
		return -ENOTDIR;
	    }
	default:
	    break;
    }
    return -EPERM;
}

static int __unlink__(const char* path){
    log_debug("Calling unlink(\"%s\")", path);
    switch (get_depth_of_path(path)) {
	case 0:
	    {
		return -EISDIR;
	    }
	case 1:
	    {
		return -EISDIR;
	    }
	case 2:
	    {
		return 0;
	    }
	default:
	    break;
    }
    return -EPERM;
}

static int __write__(const char* path, const char* buf, size_t size, off_t offset, struct fuse_file_info* fi){
    log_debug("Calling write(\"%s\")", path);
    log_debug("Writing: \"%s\n\"", buf);
    switch (get_depth_of_path(path)) {
	case 0:
	    {
		return -EISDIR;
	    }
	case 1:
	    {
		return -EISDIR;
	    }
	case 2:
	    {
		return 0;
	    }
	default:
	    break;
    }
    return -EPERM;
}

static int __write_buf__(const char* path, struct fuse_bufvec* buf, off_t offset, struct fuse_file_info* fi){
    log_debug("Calling write_buf(\"%s\")", path);
    return 0;
}

static const struct fuse_operations operations = {
    .create = __create__,
    .getattr = __getattr__,
    .mkdir = __mkdir__,
    .readdir = __readdir__,
    .read = __read__,
    .rename = __rename__,
    .rmdir = __rmdir__,
    .unlink = __unlink__,
    .write = __write__,
    .write_buf = __write_buf__,
};

void mount_custom_fs(char *mount_point, struct login_config* config){
    char* args[] = {"NULL", "-f", mount_point};
    log_debug("Calling fuse_main()");
    fuse_main(3, args, &operations, login_and_get_private_data(config));
}

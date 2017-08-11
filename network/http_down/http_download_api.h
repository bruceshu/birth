#include "http_download.h"

void parse_url(const char *url, char *domain, int *port, char *file_name);
void get_ip_addr(const char *domain, char *ip_addr);
struct resp_header get_resp_header(const char *response);
void * download(void * socket_d);

extern struct resp_header resp;

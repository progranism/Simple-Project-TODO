#include <stdio.h>
#include <stdbool.h>
#include <windows.h>
#include <winsock.h>

const int SERVER_PORT = 8492;


void process_connection(int conn);


int main(int argc, char *argv[])
{
	int listener, conn;
	struct sockaddr_in servaddr;
	ssize_t rc;
	WSADATA wsaData;

	WSAStartup(MAKEWORD(1, 1), &wsaData);

	if ( (listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 ) {
		rc = WSAGetLastError();
		fprintf(stderr, "Could not create listening socket: %i", rc);
		return -1;
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family		= AF_INET;
	servaddr.sin_port		= htons(SERVER_PORT);
	servaddr.sin_addr.s_addr	= inet_addr("127.0.0.1");

	if ( bind(listener, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 ) {
		fprintf(stderr, "Could not bind listening socket.");
		return -1;
	}

	if ( listen(listener, 1) < 0) {
		fprintf(stderr, "Call to listen failed.");
		return -1;
	}

	system("start http://127.0.0.1:8492");

	while(1) {
		if ( (conn = accept(listener, NULL, NULL)) < 0 ) {
			fprintf(stderr, "Error calling accept()");
			return -1;
		}

		process_connection(conn);
	}

	WSACleanup();

	return 0;
}

void WriteLine(int conn, const char *line, size_t len)
{
	size_t rc;

	if(len == 0)
		len = strlen(line);

	while(len > 0) {
		rc = send(conn, line, len, 0);
		if(rc < 0) {
			fprintf(stderr, "Failed to write data to socket.\n");
			return;
		}
		len -= rc;
		line += rc;
	}
}

void process_connection(int conn)
{
	ssize_t rc, len;
	bool line_ended = false;
	char c;
	FILE *fp;
	char buf[1024];
	size_t requestheader_len = 0;
	char filename[256], requestheader[1024];


	// Wait until the request has finished sending
	while(1) {
		if ( (rc = recv(conn, &c, 1, 0)) == 1 ) {
			printf("%c", c);
			if ( c == '\n' ) {
				if ( line_ended )
					break;
				line_ended = true;
			}
			else if ( c != '\n' && c != '\r' )
				line_ended = false;

			if(requestheader_len < 1023) {
				requestheader[requestheader_len] = c;
				requestheader_len++;
			}
		}
		else if ( rc == 0 )
			break;
		else {
			fprintf(stderr, "Error reading request: %i.\n", rc);
			rc = WSAGetLastError();
			fprintf(stderr, "Error reading request: %i.\n", rc);
			return;
		}
	}

	if(requestheader_len > 0) {
		requestheader[requestheader_len-1] = 0;
		if(sscanf(requestheader, "GET %s HTTP", filename) != 1)
			strcpy(filename, "/index.html");
		if(filename[1] == 0)
			strcpy(filename, "/index.html");
	}
	else
		strcpy(filename, "/index.html");

	printf("Sending: '%s'\n", filename+1);

	if ( (fp = fopen(filename+1, "rb")) == NULL ) {
		fprintf(stderr, "Unable to open '%s' for reading.\n\n", filename);
		closesocket(conn);
		return;
	}


	// Send the file
	WriteLine(conn, "HTTP/1.0 200 OK\r\nServer: PGBWebServ v01.\r\nContent-Type: text/html\r\n\r\n", 0);
	while(1) {
		len = fread(buf, 1, 1024, fp);
		printf("Sending %i bytes.\n", len);
		WriteLine(conn, buf, len);

		if(len < 1024)
			break;
	}

	shutdown(conn, SD_SEND);
	closesocket(conn);


	fclose(fp);
}


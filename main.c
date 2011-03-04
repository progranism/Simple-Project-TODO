#include <stdio.h>
#include <stdbool.h>
#include <windows.h>
#include <winsock.h>

const int SERVER_PORT = 8492;

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


int main(int argc, char *argv[])
{
	int listener, conn;
	struct sockaddr_in servaddr;
	ssize_t rc, len;
	bool line_ended = false;
	char c;
	FILE *fp;
	char buf[1024], *writeptr;
	WSADATA wsaData;

	WSAStartup(MAKEWORD(1, 1), &wsaData);

	if ( (fp = fopen("index.html", "rb")) == NULL ) {
		fprintf(stderr, "Unable to open 'index.html' for reading.");
		return -1;
	}

	if ( (listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 ) {
		rc = WSAGetLastError();
		fprintf(stderr, "Could not create listening socket: %i", rc);
		return -1;
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family		= AF_INET;
	servaddr.sin_port		= htons(SERVER_PORT);
	servaddr.sin_addr.s_addr	= inet_addr("127.0.0.1");
	//inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);

	if ( bind(listener, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 ) {
		fprintf(stderr, "Could not bind listening socket.");
		return -1;
	}

	if ( listen(listener, 1) < 0) {
		fprintf(stderr, "Call to listen failed.");
		return -1;
	}

	system("start http://127.0.0.1:8492");

	if ( (conn = accept(listener, NULL, NULL)) < 0 ) {
		fprintf(stderr, "Error calling accept()");
		return -1;
	}

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
		}
		else if ( rc == 0 )
			break;
		else {
			fprintf(stderr, "Error reading request: %i.\n", rc);
			rc = WSAGetLastError();
			fprintf(stderr, "Error reading request: %i.\n", rc);
			return -1;
		}
	}


	// Send the file
	WriteLine(conn, "HTTP/1.0 200 OK\r\nServer: PGBWebServ v01.\r\nContent-Type: text/html\r\n\r\n", 0);
	while(1) {
		len = fread(buf, 1024, 1, fp);
		WriteLine(conn, buf, len);

		if(len < 1024)
			break;
	}

	shutdown(conn, SD_SEND);
	closesocket(conn);


	fclose(fp);
	WSACleanup();

	return 0;
}


#include <stdio.h>
#include <stdbool.h>
#include <windows.h>
#include <winsock.h>
#include "packer.h"
#include "datapack.h"

#ifndef MIN
	#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

const int SERVER_PORT = 8492;


void process_connection(int conn);


int main(int argc, char *argv[])
{
	int listener, conn;
	struct sockaddr_in servaddr;
	WSADATA wsaData;

	WSAStartup(MAKEWORD(1, 1), &wsaData);

	if ( (listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 ) {
		fprintf(stderr, "Could not create listening socket: %i", WSAGetLastError());
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

void ReadLine(int conn, char *buf, ssize_t max)
{
	int rc = 0;
	max--;	// Leave room for a NULL terminator

	while(max > 0) {
		if ( (rc = recv(conn, buf, 1, 0)) == 1 ) {
			printf("%c", *buf);
			if ( *buf == '\n' ) {
				*(buf+1) = 0;
				return;
			}

			buf++;
			max--;
		}
		else if ( rc == 0 )
		{
			*buf = 0;
			return;
		}
		else {
			fprintf(stderr, "Error reading request: %i.\n", rc);
			*buf = 0;
			return;
		}
	}
}

bool ReadData(int conn, char *buf, ssize_t max)
{
	int rc = 0;

	while(max > 0) {
		if ( (rc = recv(conn, buf, max, 0)) > 0 ) {
			buf += rc;
			max -= rc;
		}
		else if ( rc == 0 )
		{
			return false;
		}
		else {
			fprintf(stderr, "Error reading request: %i.\n", rc);
			return false;
		}
	}

	return true;
}

void process_connection(int conn)
{
	ssize_t len;
	FILE *fp;
	char buf[1024];
	char filename[256] = {0};
	bool GET = true;
	int content_length = 0;
	struct DATAPACK datapack;

	strcpy(filename, "/index.html");


	// Wait until the request has finished sending
	while(1) {
		ReadLine(conn, buf, 1024);
		if(buf[0] == '\r' && buf[1] == '\n')
			break;

		// TODO: These functions are unsafe.
		if(sscanf(buf, "GET %s HTTP", filename) == 1)
			GET = true;

		if(sscanf(buf, "POST %s HTTP", filename) == 1)
			GET = false;

		sscanf(buf, "Content-Length: %i\r\n", &content_length);
	}

	if(!strcmp(filename, "/save") && !GET)
	{
		fp = fopen("todo.json", "wb");

		while(content_length > 0) {
			if(!ReadData(conn, buf, MIN(content_length, 1024)))
				break;

			fwrite(buf, 1, MIN(content_length, 1024), fp);

			content_length -= 1024;
		}

		shutdown(conn, SD_SEND);
		closesocket(conn);
		fclose(fp);
		return;
	}

	if(filename[1] == 0)
		strcpy(filename, "/index.html");

	printf("Sending: '%s'\n", filename+1);

	datapack = find_datapack(datapacks, filename+1);

	if(datapack.name != NULL)
	{
		WriteLine(conn, datapack.data, datapack.len);
	}
	else
	{
		if ( (fp = fopen(filename+1, "rb")) == NULL ) {
			fprintf(stderr, "Unable to open '%s' for reading.\n\n", filename);

			if(!strcmp(filename, "/todo.json"))
			{
				WriteLine(conn, "\n", 1);

				shutdown(conn, SD_SEND);
			}

			closesocket(conn);
			return;
		}


		// Send the file
		WriteLine(conn, "HTTP/1.0 200 OK\r\nServer: PGBWebServ v01.\r\nContent-Type: text/html\r\n\r\n", 0);
		while(1) {
			len = fread(buf, 1, 1024, fp);
			printf("Sending %u bytes.\n", (unsigned int)len);
			WriteLine(conn, buf, len);

			if(len < 1024)
				break;
		}
	}

	shutdown(conn, SD_SEND);
	closesocket(conn);


	fclose(fp);
}


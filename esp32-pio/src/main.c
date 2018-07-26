/* RMT transmit control over http server.
	*
	* Based on: https://github.com/kimeckert/ESP32-RMT-server
	* 
	* HTTP server code modified from
	* https://github.com/feelfreelinux/myesp32tests/blob/master/examples/http_server.c
	* That software is distributed under GNU General Public License
	* 
	* RMT code modified from example: rmt_nec_tx_rx
	* That software is distributed under Public Domain (or CC0 licensed, at your option.)
	* 
	* Additional software included in this application is distributed under
	* Public Domain (or CC0 licensed, at your option.)
	* 
	* This software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
	* CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"


#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/api.h"

extern void initialise_wifi(void);
extern void rmt_tx_init(void);

// FreeRTOS function
#define INCLUDE_vTaskDelay 1

// HTTP responses
const static char http_html_Accepted[] = "HTTP/1.1 202 Accepted\r\nConnection: close\r\n\r\n";
const static char http_html_Not_Allowed[] = "HTTP/1.1 405 Method Not Allowed\r\nConnection: close\r\n\r\n";

// LED on Huzzah32 board
const int LED_BUILTIN = 13;

// used during debug
const static char http_debug1[] = "HTTP/1.1 202 Accepted\r\nContent-type: text/html\r\n\r\n";
const static char http_debug2[] = "<html><head><title>ESP32</title></head><body><pre>";
const static char http_debug3[] = "</pre></body></html>";

// used when parsing integers from character arrays
typedef struct {
	int num;
	bool good;
} returnIntVal;

// find the start of the request body, which follows a blank line
// a blank line is created by two consecutive \n's, ignoring \r's
// more than two consecutive newlines are accepted
// returns the index of the character following the newlines
u16_t find_body( char* buf, u16_t length ) {
	printf("Finding POST body, buffer size %d\n", length);
	// counts number of consecutive newlines
	u16_t newlines = 0;
	// character pointer
	u16_t this_char;
	// the response starts with 'POST ', so start at character 5
	for ( this_char=5; this_char<=length; this_char++ ) {
		if ( buf[this_char] == '\r' ) { continue; }
		if ( buf[this_char] == '\n' ) { newlines++; }
		// only received a single newline, reset counter
		else if ( newlines == 1 ) { newlines = 0; }
		if ( ( newlines > 1 ) && ( buf[this_char] != '\n' ) ) {
			printf("Found body start %d\n", this_char);
			return this_char;
		}
	}
	// body not found
	return 0;
}

// returns the number of comma-separated fields in this line,
// including the first field, the line type designation
u16_t count_values( char* buf, u16_t start, u16_t end ) {
	u16_t count = 0;
	u16_t pointer;
	for ( pointer=start; pointer<=end; pointer++ ) {
		if ( buf[pointer] == ',' ) { count++; }
	}
	return count + 1;
}

// get the n-th field from the comma-separated line
// the first numerical field is index = 0
returnIntVal get_number( char* buf, u16_t start, u16_t end, int count ) {
	returnIntVal n;
	n.num = 0;
	n.good = true;
	bool is_neg = false;
	u16_t this_char = start;
	u16_t commas = 0;
	printf("get_number: look for %d\n", count);
	
	// count values separated by commas
	if ( count > 0 ) {
		while( this_char++ <= end ) {
			if ( buf[this_char] == ',' ) { commas++; }
			if ( commas == count ) { break; }
		}
		// point to the character after the comma
		this_char++;
	}
	
	// did not find the value
	if ( commas < count ) {
		printf("get_number not found\n");
		n.good = false;
		return n;
	}
	
	// is the number negative?
	if ( buf[this_char] == '-' ) {
		is_neg = true;
		this_char++;
	}
	
	// get numerical characters
	while( (this_char <= end) && 
		(buf[this_char]>47) && 
		(buf[this_char]<58 ) ) {
		n.num = ( 10 * n.num ) + ( buf[this_char] - 48 );
		this_char++;
	}
	
	// return
	if ( is_neg ) { n.num = -1 * n.num; }
	// printf("get_number found %d\n", n.num);
	return n;
}

// FreeRTOS constant portTICK_PERIOD_MS
// example usage: vtaskDelay( 500 / portTICK_PERIOD_MS )
void delay_task( int ms ) {
	vTaskDelay( ms / portTICK_PERIOD_MS );
	printf("Delay %d\n", ms);
}

// Process an HTTP POST request
static void http_server_netconn_serve(struct netconn *conn) {
	struct netbuf *inbuf;
	char *buf;
	u16_t buflen;
	u16_t rmt_start;
	err_t err;
	
	// Read the data from the port, blocking if nothing yet there.
	err = netconn_recv(conn, &inbuf);
	
	printf("Start looking for POST request\n");
	
	if (err == ERR_OK) {
		netbuf_data(inbuf, (void**)&buf, &buflen);
		
		// Is this an HTTP POST command?
		if ( buflen>5 && 
			buf[0]=='P' && buf[1]=='O' && buf[2]=='S' && buf[3]=='T' ) {
			
			printf("Decoding POST request\n");
			
			// process the POST request
			rmt_start = find_body( buf, buflen );
			if ( rmt_start > 0 ) {
				// get_request_line( buf, rmt_start, buflen );
			}
			
			// HTTP Response to POST request
			netconn_write(conn, http_html_Accepted, sizeof(http_html_Accepted)-1, NETCONN_NOCOPY);
			
			// HTTP Debug response, instead of the normal response, echoes the entire request
			//netconn_write(conn, http_debug1, sizeof(http_debug1)-1, NETCONN_NOCOPY);
			//netconn_write(conn, http_debug2, sizeof(http_debug1)-1, NETCONN_NOCOPY);
			//netconn_write(conn, buf, buflen, NETCONN_NOCOPY);
			//netconn_write(conn, http_debug3, sizeof(http_debug1)-1, NETCONN_NOCOPY);
			
			// gpio_set_level(LED_BUILTIN,0);
		}
		// do not accept non-POST requests
		else {
			netconn_write(conn, http_html_Not_Allowed, sizeof(http_html_Not_Allowed)-1, NETCONN_NOCOPY);
		}
	}
	// Close the connection
	netconn_close(conn);
	
	// Delete the buffer
	netbuf_delete(inbuf);
}

// HTTP server
static void http_server(void *pvParameters) {
	struct netconn *conn, *newconn;
	err_t err;
	conn = netconn_new(NETCONN_TCP);
	netconn_bind(conn, NULL, 80);
	netconn_listen(conn);
	do {
		err = netconn_accept(conn, &newconn);
		if (err == ERR_OK) {
			http_server_netconn_serve(newconn);
			netconn_delete(newconn);
		}
	} while(err == ERR_OK);
	netconn_close(conn);
	netconn_delete(conn);
}

void app_main() {
	// set board built-in LED as an output
	gpio_pad_select_gpio(LED_BUILTIN);
	gpio_set_direction(LED_BUILTIN, GPIO_MODE_OUTPUT);
	
	// Initialize NVS flash storage
    nvs_flash_init();
    
    // Initialize the RMT peripheral for output
    rmt_tx_init();
    
    // Initialize WiFi
    initialise_wifi();
    
    // HTTP server task
    xTaskCreate(&http_server, "http_server", 2048, NULL, 5, NULL);
}

#include "driver/rmt.h"
#include "lwip/sys.h"

// RMT values
#define RMT_TX_CHANNEL RMT_CHANNEL_0
#define RMT_TX_GPIO    GPIO_NUM_26
// channel clock period = 0.8 uS?
#define RMT_CLK_DIV    100

// initialize RMT peripheral for output
// Note that some of these settings can be modified during operation
void rmt_tx_init(void) {
	rmt_config_t rmt_tx;
	rmt_tx.rmt_mode = RMT_MODE_TX;
	rmt_tx.channel  = RMT_TX_CHANNEL;
	rmt_tx.gpio_num = RMT_TX_GPIO;
	rmt_tx.mem_block_num = 1;
	rmt_tx.clk_div = RMT_CLK_DIV;
	rmt_tx.tx_config.loop_en = false;
	rmt_tx.tx_config.carrier_duty_percent = 30;
	rmt_tx.tx_config.carrier_freq_hz = 38000;
	rmt_tx.tx_config.carrier_level   = RMT_CARRIER_LEVEL_HIGH;
	rmt_tx.tx_config.carrier_en      = true;
	rmt_tx.tx_config.idle_level      = RMT_IDLE_LEVEL_LOW;
	rmt_tx.tx_config.idle_output_en  = true;
	
	ESP_ERROR_CHECK( rmt_config(&rmt_tx) );
	ESP_ERROR_CHECK( rmt_driver_install(rmt_tx.channel, 0, 0) );
	printf("RMT initialized\n");
}

// push item onto rmt_item32_t array
// value_index points to: duration0/level0, duration1/level1, ...
//       value_index: 0 1 2 3 4 5 6 7
//   value_index / 2: 0 0 1 1 2 2 3 3
//       items index: 0 0 1 1 2 2 3 3
void pushItem( rmt_item32_t* items, u16_t value_index, bool level, u16_t duration ) {
	u16_t i = value_index / 2;
	// lower 16 bits of the 32-bit item
	if ( value_index % 2 == 0 ) {
		items[i].level0 = level;
		items[i].duration0 = duration;
		// just in case this is the last item, fill up the next entry
		items[i].level1 = level;
		items[i].duration1 = 0;
	}
	// upper 16 bits of the 32-bit item
	else {
		items[i].level1 = level;
		items[i].duration1 = duration;
	}
	printf("Item pushed to RMT data RAM: level %d, duration %u\n", level, duration);
}

// set the clock divisor and carrier clock high/low durations
// set during the decode of RMT data
void set_rmt_frequency( uint16_t div, uint16_t high, uint16_t low ) {
	printf("Set frequency %i, %i, %i\n", div, high, low);
	
	// if clock divisor changes, set to new value
	uint8_t previous_div;
	rmt_get_clk_div( RMT_TX_CHANNEL, &previous_div );
	printf("previous div: %i\n", previous_div);
	if ( (uint8_t) div != previous_div ) {
		rmt_set_clk_div( RMT_TX_CHANNEL, (uint8_t) div );
		printf("set div = %i\n", div);
	}
	
	// set clock high and low times
	// args: channel, carrier_en, high_level, low_level, carrier_level
	rmt_set_tx_carrier( RMT_TX_CHANNEL, true, high, low, RMT_CARRIER_LEVEL_HIGH );
	printf("Reset RMT frequency\n");
}

// write frequency and clock data to the RMT peripheral
void send_freq( char* buf, u16_t start, u16_t end ) {
	// frequency settings: [Division Ratio, High Duration, Low Duration]
	int freq[3] = {0, 0, 0};
	bool good = true;
	// returnIntVal n;
	// printf("send_freq %d to %d\n", start, end);
	
	// // division ratio, 8-bit register
	// n = get_number( buf, start, end, 0 );
	// if ( (n.good) && (n.num>0) && (n.num<256) ) {
	// 	freq[0] = n.num;
	// 	// printf("get_num 0 %d\n", n.num);
	// }
	// else { good = false; }
	
	// // clock high duration, 16-bit register
	// n = get_number( buf, start, end, 1 );
	// if ( (n.good) && (n.num>0) && (n.num<65536) ) {
	// 	freq[1] = n.num;
	// 	// printf("get_num 1 %d\n", n.num);
	// }
	// else { good = false; }
	
	// // clock low duration, 16-bit register
	// n = get_number( buf, start, end, 2 );
	// if ( (n.good) && (n.num>0) && (n.num<65536) ) {
	// 	freq[2] = n.num;
	// 	// printf("get_num 2 %d\n", n.num);
	// }
	// else { good = false; }
	
	// set register values
	if ( good ) {
		set_rmt_frequency( freq[0], freq[1], freq[2] );
		//printf("Set frequency %d, %d, %d\n", freq[0], freq[1], freq[2]);
	}
}

// write duration data to the RMT RAM
void send_duration( char* buf, u16_t start, u16_t end ) {
	u16_t count = 0;//, c;
	// returnIntVal val;
	bool good = true;
	
	// // the number of comma-separated numeric values on this line
	// count = count_values(buf, start, end);
	
	// // each 32-bit RMT item holds 2 duration/level values
	// // divide count of numerical values by 2 get required number of 32-bit values
	// // multiply by 4 to get bytes
	// //            count: 0 1 2 3 4  5  6  7  8
	// //        count + 1: 1 2 3 4 5  6  7  8  9
	// //  (count + 1) / 2: 0 1 1 2 2  3  3  4  4
	// //    RMT RAM items: 0 1 1 2 2  3  3  4  4
	// //     malloc bytes: 0 4 4 8 8 12 12 16 16
	
	// // number of bytes in the 32-bit RMT item array
	size_t size = 4 * ( ( count + 1 ) / 2 );
	rmt_item32_t* items = (rmt_item32_t*) malloc(size);
	// printf("Row has %i values, allocated %i bytes\n", count, size);
	// printf("Starts with %c, ends with %c\n", buf[start], buf[end]);
	
	// // create an object for RMT RAM values
	// for ( c=0; c<count; c++ ) {
	// 	val = get_number( buf, start, end, c );
	// 	if ( val.good ) {
	// 		// negative values create no RMT output pulses
	// 		if ( val.num < 0 ) {
	// 			pushItem( items, c, false, -1*val.num );
	// 		}
	// 		// positive values create RMT output pulses
	// 		else {
	// 			pushItem( items, c, true, val.num );
	// 		}
	// 	}
	// 	// cannot write this value to RMT RAM
	// 	else { good = false; }
	// }
	if ( good ) {
		// args: channel, rmt_item, item_num wait_tx_done
		rmt_write_items(RMT_TX_CHANNEL, items, size, true);
		printf("Write %d durations\n", count);
	}
	else { printf("ERROR in transmit data\n"); }
	// free up memory
	free( items );
}

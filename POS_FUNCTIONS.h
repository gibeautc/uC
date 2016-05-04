#ifndef POS_FUNCTIONS_H_
#define POS_FUNCTIONS_H_

struct POS{
	unsigned long t;
	float a_x;
	float a_y;
	float a_z;
	
	float g_x;
	float g_y;
	float g_z;
	
	float m_x;
	float m_y;
	float m_z;
	
	};
	

volatile unsigned long count_t;


void record_window(void);
void live_pos(void);
void print_record(struct POS pos);

#endif /* SPI_H_ */
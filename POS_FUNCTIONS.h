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
unsigned long count;


void record_window(void);
void live_pos(void);
void print_record(struct POS pos);
int save_window(void);
void mem_reset(void);
void test_mem(void);
void read_back(int count);
void store_pos(struct POS pos);
struct POS read_mem_pos(void);
void print_mem(void);

#endif /* SPI_H_ */
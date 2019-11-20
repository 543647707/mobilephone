#ifndef __QDTFT_DEMO_H
#define __QDTFT_DEMO_H 


extern unsigned char Num[10];

void Redraw_Mainmenu(void);
void Num_Test(void);
void Font_Test(void);
void Color_Test(void);
void showimage(const unsigned char *p); 
//void QDTFT_Test_Demo(void);

extern void QDTFT_Test_Demo(void);
extern void showimage(const unsigned char *p); 
extern void showplay(const unsigned char *p); 
extern void QDTFT_Test_Demo2(void);

#endif

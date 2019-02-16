#define _CRT_SECURE_NO_WARNINGS    //(x*6+37,y*3+2) - перевод обычных координат в графический интерфейс
#include "wincon.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>
#define B 1
#define W 2
#define BQ 3
#define WQ 4
#define Q 5
#define ENEMY 6
#define notENEMY 7
#define HL 8
#define HR 9
#define DR 10
#define DL 11
#define EXIT -100000
/*****************************************************************______GLOBAL____******************************************************************************************/
int side = W;
int deep = 6;
int p_vs_p = 0, p_vs_comp = 0;
int human_clr = W, comp_clr = B;
int count_enemys = 0, q_check = 0;
int count_variants = 0;
int qwar = 0, strokes = 0;
int count_eat = 0;
enum x{ a = 0, b = 1, c = 2, d = 3, e = 4, f = 5, g = 6, h = 7 };
enum y{ I = 2, II = 5, III = 8, IV = 11, V = 14, VI = 17, VII = 20, VIII = 23 };
clock_t t_start, t_end;
unsigned res_time=0;
field[8][8] = { 
	{ -1, B, -1, 0, -1, W, -1, W, },
	{ B, -1, B, -1, 0, -1, W, -1, },
	{ -1, B, -1, 0, -1, W, -1, W, },
	{ B, -1, B, -1, 0, -1, W, -1, },
	{ -1, B, -1, 0, -1, W, -1, W, },
	{ B, -1, B, -1, 0, -1, W, -1, },
	{ -1, B, -1, 0, -1, W, -1, W, },
	{ B, -1, B, -1, 0, -1, W, -1, } };
FILE *save_name, *in, *rt;
enum priority{ QUEEN = 35, WAR = 10 };
enum end_game{ WIN = 1000, LOSE = -1000 };
///////////////////////////////////
void fprintf_field(int field[8][8], int side_abc)
{
	in = fopen("field_stat.txt", "a");

	fprintf(in, " SIDE: ");
	if (side_abc == B) fprintf(in, "BLACK\n");
	else if (side_abc == W) fprintf(in, "WHITE\n");

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if (field[j][i] == 0) fprintf(in, " . ");
			else if (field[j][i] == 1) fprintf(in, " B ");
			else if (field[j][i] == 2) fprintf(in, " W ");
			else if (field[j][i] == -1) fprintf(in, "  ");
			else if (field[j][i] == BQ) fprintf(in, "BQ");
			else if (field[j][i] == WQ) fprintf(in, "WQ");
		}
		fputc('\n', in);
	}
	
	fprintf(in,"-----Coordinate----\n");
	
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if (field[j][i] == 2) fprintf(in, "|W(%d,%d)|",j,i);
			else if (field[j][i] == 1) fprintf(in, "|B(%d,%d)|", j, i);
			else if (field[j][i] == 0) fprintf(in, "|.(%d,%d)|", j, i);
			else if (field[j][i] == BQ) fprintf(in, "|BQ(%d,%d)|", j, i);
			else if (field[j][i] == WQ) fprintf(in, "|WQ(%d,%d)|", j, i);
			else fprintf(in, "|   |", j, i);

		}
		fputc('\n', in);
	}

	fprintf(in, "--------------------\n");

	fclose(in);

}
void my_printf(const char *format, va_list arg_ptr,int w_or_a)
{
	if (w_or_a==1) in = fopen("field_stat.txt", "w");
	else if (w_or_a == 2) in = fopen("field_stat.txt", "a+");
	fprintf(in, format, arg_ptr);
	fclose(in);
}
///////////////////////////////////
struct BTree
{
	int field[8][8];
	int priority;
	struct BTree *next;
	struct BTree *down_tree;
};
///////////////////////////////////
struct coord{
	int x;
	int y;
};
struct WCheckers_Coord{  //все координаты белых фигур
	int queen;
	struct coord crd;
	struct WCheckers_Coord* WCNext;
	struct WCheckers_Coord* WCPrev;
};
struct BCheckers_Coord{   //все координаты черный фигур
	int queen;
	struct coord crd;
	struct BCheckers_Coord* BCNext;
	struct BCheckers_Coord* BCPrev;
};
struct BDots_Coord{       //все координаты черных полей
	struct coord crd;
	struct BDots_Coord* BDNext;
	struct BDots_Coord* BDPrev;
};
struct SaveBDots_Coord{   //все ДОСТУПНЫЕ координаты для данной шашки
	struct coord crd;
	struct SaveBDots_Coord* SBDNext;
	struct SaveBDots_Coord* SBDPrev;
};
struct Victims_Coord{
	struct coord crd;
	struct Victims_Coord* VCNext;
	struct Victims_Coord* VCPrev;
};
struct FreeCheckersCord
{
	int eat;
	int color;
	int queen;
	struct coord crd;
	struct FreeCheckersCord *FCNext;
	struct FreeCheckersCord *FCPrev;
};

typedef struct BTree btree;
typedef struct WCheckers_Coord wcc;
typedef struct BCheckers_Coord bcc;
typedef struct BDots_Coord bdc;
typedef struct SaveBDots_Coord sbdc;
typedef struct Victims_Coord vc;
typedef struct FreeCheckersCord fcc;
wcc *wcc_tail = NULL;
wcc *wcc_head = NULL;
wcc *wcc_cur = NULL;

bcc *bcc_tail = NULL;
bcc *bcc_head = NULL;
bcc *bcc_cur = NULL;

bdc *bdc_tail = NULL;
bdc *bdc_head = NULL;
bdc *bdc_cur = NULL;

sbdc *sbdc_tail = NULL;
sbdc *sbdc_head = NULL;
sbdc *sbdc_cur = NULL;

vc *vc_tail = NULL;
vc *vc_head = NULL;
vc *vc_cur = NULL;

fcc *fcc_head = NULL;
fcc *fcc_tail = NULL;
fcc *fcc_cur = NULL;

btree *tree_head = NULL;
btree *tree_cur = NULL;

void push_WCcord(int x, int y){
	wcc* tmp = NULL;

	tmp = (wcc*)malloc(sizeof(wcc));
	tmp->WCNext = NULL;
	tmp->crd.x = x;
	tmp->crd.y = y;
	tmp->queen = 0;

	if (wcc_head != NULL) //Если список не пуст
	{
		tmp->WCPrev = wcc_tail; //Указываем адрес на предыдущий элемент в соотв. поле
		wcc_tail->WCNext = tmp; //Указываем адрес следующего за хвостом элемента
		wcc_tail = tmp; //Меняем адрес хвоста
	}
	else //Если список пустой
	{
		tmp->WCPrev = NULL; //Предыдущий элемент указывает в пустоту
		wcc_head = wcc_tail = tmp; //Голова=Хвост=тот элемент, что сейчас добавили
	}
}
void destruct_WCcord(){
	while (wcc_head) //Пока по адресу на начало списка что-то есть
	{
		wcc_tail = wcc_head->WCNext; //Резервная копия адреса следующего звена списка
		free(wcc_head); //Очистка памяти от первого звена
		wcc_head = wcc_tail; //Смена адреса начала на адрес следующего элемента
	}
}
void pop_WCcord(wcc* pnt)
{
	wcc *temp, *temp2;
	if ((pnt == wcc_head) && (wcc_head->WCNext)){
		temp = wcc_head;
		pnt = pnt->WCNext;
		wcc_head = wcc_head->WCNext;
		wcc_head->WCPrev = NULL;
		free(temp);
		temp = NULL;
		return;
	}
	else
	{
		if ((pnt == wcc_head) && (wcc_head == wcc_tail)){

			wcc_head->WCNext = NULL;
			free(wcc_head);
			pnt = wcc_head = NULL;
			return;
		}
		else
		{
			if (pnt == wcc_tail)
			{
				temp = wcc_tail;
				pnt = pnt->WCNext;
				wcc_tail = wcc_tail->WCPrev;
				wcc_tail->WCNext = NULL;
				free(temp);
				temp = NULL;
				return;
			}
			else {
				temp = pnt;
				temp2 = temp;
				pnt = pnt->WCNext;
				temp2->WCPrev->WCNext = temp->WCNext;
				temp2->WCNext->WCPrev = temp->WCPrev;
				free(temp);
				temp = NULL;
			}
		}
	}
}
void fprintf_wcc()
{
	wcc* p = wcc_head;
	int i = 1;
	in = fopen("field_stat.txt", "a");
	fprintf(in, "\nWhite Checkers Cord List:\n");
	while (p)
	{	
		fprintf(in, "%d.(%d,%d)", i,((p->crd.x - 37) / 6), ((p->crd.y - 2) / 3));
		if (field[((p->crd.x - 37) / 6)][((p->crd.y - 2) / 3)] == W) fprintf(in," OK\n");
		else fprintf(in, " FAIL\n");
		i++;
		p = p->WCNext;
	}
	fclose(in);
}

void push_BCcord(int x, int y){
	bcc* tmp = NULL;

	tmp = (bcc*)malloc(sizeof(bcc));
	tmp->BCNext = NULL;
	tmp->crd.x = x;
	tmp->crd.y = y;
	tmp->queen = 0;

	if (bcc_head != NULL) //Если список не пуст
	{
		tmp->BCPrev = bcc_tail; //Указываем адрес на предыдущий элемент в соотв. поле
		bcc_tail->BCNext = tmp; //Указываем адрес следующего за хвостом элемента
		bcc_tail = tmp; //Меняем адрес хвоста
	}
	else //Если список пустой
	{
		tmp->BCPrev = NULL; //Предыдущий элемент указывает в пустоту
		bcc_head = bcc_tail = tmp; //Голова=Хвост=тот элемент, что сейчас добавили
	}

}
void destruct_BCcord(){
	while (bcc_head) //Пока по адресу на начало списка что-то есть
	{
		bcc_tail = bcc_head->BCNext; //Резервная копия адреса следующего звена списка
		free(bcc_head); //Очистка памяти от первого звена
		bcc_head = bcc_tail; //Смена адреса начала на адрес следующего элемента
	}
}
void pop_BCcord(bcc* pnt)
{
	bcc *temp, *temp2;
	if ((pnt == bcc_head) && (bcc_head->BCNext)){
		temp = bcc_head;
		pnt = pnt->BCNext;
		bcc_head = bcc_head->BCNext;
		bcc_head->BCPrev = NULL;
		free(temp);
		temp = NULL;
		return;
	}
	else
	{
		if ((pnt == bcc_head) && (bcc_head == bcc_tail)){

			bcc_head->BCNext = NULL;
			free(bcc_head);
			pnt = bcc_head = NULL;
			return;
		}
		else
		{
			if (pnt == bcc_tail)
			{
				temp = bcc_tail;
				pnt =pnt->BCNext;
				bcc_tail = bcc_tail->BCPrev;
				bcc_tail->BCNext = NULL;
				free(temp);
				temp = NULL;
				return;
			}
			else {
				temp = pnt;
				temp2 = temp;
				pnt = pnt->BCNext;
				temp2->BCPrev->BCNext = temp->BCNext;
				temp2->BCNext->BCPrev = temp->BCPrev;
				free(temp);
				temp = NULL;
			}
		}
	}
}
void fprintf_bcc()
{
	bcc* p = bcc_head;
	int i = 1;
	in = fopen("field_stat.txt", "a");
	fprintf(in, "\nBlack Checkers Cord List:\n");
	while (p)
	{
		fprintf(in, "%d.(%d,%d)", i, ((p->crd.x - 37) / 6), ((p->crd.y - 2) / 3));
		if (field[((p->crd.x - 37) / 6)][((p->crd.y - 2) / 3)] == B)fprintf(in, " OK\n");
		else fprintf(in," FAIL\n");
		i++;
		p = p->BCNext;
	}
	fclose(in);
}

void push_BDcord(int x, int y){
	bdc* tmp = NULL;

	tmp = (bdc*)malloc(sizeof(bdc));
	tmp->BDNext = NULL;
	tmp->crd.x = x;
	tmp->crd.y = y;

	if (bdc_head != NULL) //Если список не пуст
	{
		tmp->BDPrev = bdc_tail; //Указываем адрес на предыдущий элемент в соотв. поле
		bdc_tail->BDNext = tmp; //Указываем адрес следующего за хвостом элемента
		bdc_tail = tmp; //Меняем адрес хвоста
	}
	else //Если список пустой
	{
		tmp->BDPrev = NULL; //Предыдущий элемент указывает в пустоту
		bdc_head = bdc_tail = tmp; //Голова=Хвост=тот элемент, что сейчас добавили
	}

}
void destruct_BDcord(){
	while (bdc_head) //Пока по адресу на начало списка что-то есть
	{
		bdc_tail = bdc_head->BDNext; //Резервная копия адреса следующего звена списка
		free(bdc_head); //Очистка памяти от первого звена
		bdc_head = bdc_tail; //Смена адреса начала на адрес следующего элемента
	}
}
bdc* search_dot(int x, int y)
{
	bdc* pnt = bdc_head;
	while (pnt)
	{
		if ((pnt->crd.x == x) && (pnt->crd.y == y)) return pnt;
		pnt = pnt->BDNext;
	}
	free(pnt);
	return NULL;
}
void pop_BDcord(bdc* pnt)
{
	bdc *temp, *temp2;
	if ((pnt == bdc_head) && (bdc_head->BDNext)){
		temp = bdc_head;
		pnt = pnt->BDNext;
		bdc_head = bdc_head->BDNext;
		bdc_head->BDPrev = NULL;
		free(temp);
		temp = NULL;
		return;
	}
	else
	{
		if ((pnt == bdc_head) && (bdc_head == bdc_tail)){

			bdc_head->BDNext = NULL;
			free(bdc_head);
			pnt = bdc_head = NULL;
			return;
		}
		else
		{
			if (pnt == bdc_tail)
			{
				temp = bdc_tail;
				pnt = pnt->BDNext;
				bdc_tail = bdc_tail->BDPrev;
				bdc_tail->BDNext = NULL;
				free(temp);
				temp = NULL;
				return;
			}
			else {
				temp = pnt;
				temp2 = temp;
				pnt = pnt->BDNext;
				temp2->BDPrev->BDNext = temp->BDNext;
				temp2->BDNext->BDPrev = temp->BDPrev;
				free(temp);
				temp = NULL;
			}
		}
	}
}
void fprintf_bdc()
{
	bdc* p = bdc_head;
	int i = 1;
	in = fopen("field_stat.txt", "a+");
	fprintf(in, "\nBlack Dots Cord List:\n");
	while (p)
	{
		fprintf(in, "%d.(%d,%d)", i, ((p->crd.x - 37) / 6), ((p->crd.y - 2) / 3));
		if (field[((p->crd.x - 37) / 6)][((p->crd.y - 2) / 3)] == 0)fprintf(in, " OK\n");
		else fprintf(in," FAIL\n");
		i++;
		p = p->BDNext;
	}
	fclose(in);
}

void push_SBDcord(int x, int y){
	sbdc* tmp = NULL;

	tmp = (sbdc*)malloc(sizeof(sbdc));
	tmp->SBDNext = NULL;
	tmp->crd.x = x;
	tmp->crd.y = y;

	if (sbdc_head != NULL) //Если список не пуст
	{
		tmp->SBDPrev = sbdc_tail; //Указываем адрес на предыдущий элемент в соотв. поле
		sbdc_tail->SBDNext = tmp; //Указываем адрес следующего за хвостом элемента
		sbdc_tail = tmp; //Меняем адрес хвоста
	}
	else //Если список пустой
	{
		tmp->SBDPrev = NULL; //Предыдущий элемент указывает в пустоту
		sbdc_head = sbdc_tail = tmp; //Голова=Хвост=тот элемент, что сейчас добавили
	}

}
void destruct_SBDCcord(){
	while (sbdc_head) //Пока по адресу на начало списка что-то есть
	{
		sbdc_tail = sbdc_head->SBDNext; //Резервная копия адреса следующего звена списка
		free(sbdc_head); //Очистка памяти от первого звена
		sbdc_head = sbdc_tail; //Смена адреса начала на адрес следующего элемента
	}
}
void fprintf_sbdc()
{
	sbdc* p = sbdc_head;
	int i = 1;
	in = fopen("field_stat.txt", "a+");
	fprintf(in, "\nSave Black Dots Cord List:\n");
	while (p)
	{
		fprintf(in, "%d.(%d,%d)\n", i, ((p->crd.x - 37) / 6), ((p->crd.y - 2) / 3));
		i++;
		p = p->SBDNext;
	}
	fclose(in);
}

void push_VCcord(int x, int y){
	vc* tmp = NULL;

	tmp = (vc*)malloc(sizeof(vc));
	tmp->VCNext = NULL;
	tmp->crd.x = x;
	tmp->crd.y = y;

	if (vc_head != NULL) //Если список не пуст
	{
		tmp->VCPrev = vc_tail; //Указываем адрес на предыдущий элемент в соотв. поле
		vc_tail->VCNext = tmp; //Указываем адрес следующего за хвостом элемента
		vc_tail = tmp; //Меняем адрес хвоста
	}
	else //Если список пустой
	{
		tmp->VCPrev = NULL; //Предыдущий элемент указывает в пустоту
		vc_head = vc_tail = tmp; //Голова=Хвост=тот элемент, что сейчас добавили
	}
}
void destruct_VCcord(){
	while (vc_head) //Пока по адресу на начало списка что-то есть
	{
		vc_tail = vc_head->VCNext; //Резервная копия адреса следующего звена списка
		free(vc_head); //Очистка памяти от первого звена
		vc_head = vc_tail; //Смена адреса начала на адрес следующего элемента
	}
}
void fprintf_vc()
{
	vc* p = vc_head;
	int i = 1;
	in = fopen("field_stat.txt", "a+");
	fprintf(in, "\nVictims Cord List:\n");
	while (p)
	{
		fprintf(in, "%d.(%d,%d)\n", i, ((p->crd.x - 37) / 6), ((p->crd.y - 2) / 3));
		i++;
		p = p->VCNext;
	}
	fclose(in);
}

void push_FCcord(int x, int y)
{
	fcc* tmp = NULL;

	tmp = (fcc*)malloc(sizeof(fcc));
	tmp->FCNext = NULL;
	tmp->crd.x = x;
	tmp->crd.y = y;
	tmp->queen = 0;

	if (fcc_head != NULL) //Если список не пуст
	{
		tmp->FCPrev = fcc_tail; //Указываем адрес на предыдущий элемент в соотв. поле
		fcc_tail->FCNext = tmp; //Указываем адрес следующего за хвостом элемента
		fcc_tail = tmp; //Меняем адрес хвоста
	}
	else //Если список пустой
	{
		tmp->FCPrev = NULL; //Предыдущий элемент указывает в пустоту
		fcc_head = fcc_tail = tmp; //Голова=Хвост=тот элемент, что сейчас добавили
	}

}
void pop_FCcord(fcc *pnt)
{
	fcc *temp, *temp2;
	if ((pnt == fcc_head) && (fcc_head->FCNext)){
		temp = fcc_head;
		pnt = pnt->FCNext;
		fcc_head = fcc_head->FCNext;
		fcc_head->FCPrev = NULL;
		free(temp);
		temp = NULL;
		return;
	}
	else
	{
		if ((pnt == fcc_head) && (fcc_head == fcc_tail)){

			fcc_head->FCNext = NULL;
			free(fcc_head);
			pnt = fcc_head = NULL;
			return;
		}
		else
		{
			if (pnt == fcc_tail)
			{
				temp = fcc_tail;
				pnt = pnt->FCNext;
				fcc_tail = fcc_tail->FCPrev;
				fcc_tail->FCNext = NULL;
				free(temp);
				temp = NULL;
				return;
			}
			else {
				temp = pnt;
				temp2 = temp;
				pnt = pnt->FCNext;
				temp2->FCPrev->FCNext = temp->FCNext;
				temp2->FCNext->FCPrev = temp->FCPrev;
				free(temp);
				temp = NULL;
			}
		}
	}
}
void destruct_FCcord()
{
	while (fcc_head) //Пока по адресу на начало списка что-то есть
	{
		fcc_tail = fcc_head->FCNext; //Резервная копия адреса следующего звена списка
		free(fcc_head); //Очистка памяти от первого звена
		fcc_head = fcc_tail; //Смена адреса начала на адрес следующего элемента
	}
}
void fprintf_fcc()
{
	fcc* p = fcc_head;
	int i = 1;
	in = fopen("fieldstat.txt", "w");
	fprintf(in, "\nFree Checkers Cord List:\n");
	while (p)
	{
		fprintf(in, "%d.(%d,%d) ", i, ((p->crd.x - 37) / 6), ((p->crd.y - 2) / 3));
		if (p->color == W) fprintf(in, "White ");
		else if (p->color == B) fprintf(in, "Black ");
		if (p->queen == Q) fprintf(in, "Queen ");
		if (p->eat == 1) fprintf(in, "Eat ");
		fprintf(in,"\n");
		i++;
		p = p->FCNext;
	}
	fclose(in);
}
////////////////////////////////////
void fprintf_tree(btree* b)
{
	btree* a = b;
	FILE * in = fopen("field_stat.txt", "w");
	int i = 0, j = 0;
	while (a)
	{
		fprintf(in, "SCORE: %d\n", a->priority);
		for (int i = 0; i < 8; i++)
		{
			for (int j = 0; j < 8; j++)
			{
				if (a->field[j][i] == 0) fprintf(in, " . ");
				else if (a->field[j][i] == 1) fprintf(in, " B ");
				else if (a->field[j][i] == 2) fprintf(in, " W ");
				else if (a->field[j][i] == -1) fprintf(in, "  ");
				else if (a->field[j][i] == BQ) fprintf(in, "BQ");
				else if (a->field[j][i] == WQ) fprintf(in, "WQ");
			}
			fputc('\n', in);
		}
		fprintf(in, "------------------------------------------------------\n");
		a = a->next;
	}
	fclose(in);
}
btree* pop_btree(btree* ptr, btree* current_list, btree* last_current_list)
{
	btree* tmp = ptr;
	btree* prev = NULL;
	btree* cur = current_list;

	if (tmp == current_list && current_list->next==NULL)
	{
		free(tmp);
		tmp = NULL;
		current_list = NULL;
		last_current_list->down_tree = NULL;
	}
	else if (tmp == current_list)
	{
		current_list = current_list->next;
		last_current_list->down_tree = current_list;
		free(tmp);
		tmp = NULL;
	}
	else
	{
		while (cur)
		{
			if (cur == ptr) break;
			prev = cur;
			cur = cur->next;
		}
		if (cur->next)	prev->next = cur->next;		
		else	prev->next = NULL;	
		free(cur);
		cur = NULL;
	}
	return current_list;
}
//////////////////////////////////
int field_crdX(int x)
{
	return ((x - 37) / 6);
}
int field_crdY(int y)
{
	return ((y - 2) / 3);
}
int unfield_crdX(int x)
{
	return (x * 6 + 37);
}
int unfield_crdY(int y)
{
	return (y * 3 + 2);
}
///////////////////////////////////
void clrStars(sbdc *dont_clear_point,int value_esc)
{
	sbdc *pnt = sbdc_head;
	while (pnt)
	{
		if ((pnt == dont_clear_point)&&(!value_esc)) pnt = pnt->SBDNext;
		else{
			gotoxy(pnt->crd.x, pnt->crd.y);
			con_set_color(CON_CLR_BLACK, CON_CLR_YELLOW);
			printf(".");
			pnt = pnt->SBDNext;
		}
	}
	con_set_color(CON_CLR_CYAN_LIGHT, CON_CLR_YELLOW);
} // чистит "звездочки"
int scan_position(int x1, int y1, int side_abc, int field[8][8],int c)
{
	int x = field_crdX(x1); 
	int y = field_crdY(y1);
	int count = 0;
	if (side_abc == W){
		if (((field[x + 1][y + 1] == B)||(field[x + 1][y + 1] == BQ)) && (field[x + 2][y + 2] == 0) && (x + 2 < 8) && (y + 2 < 8))
		{
			if (c == 0){
				push_SBDcord((x + 2) * 6 + 37, (y + 2) * 3 + 2);
				push_VCcord(((x + 1) * 6 + 37), ((y + 1) * 3 + 2));
			}
			count++;
		}
		if (((field[x - 1][y + 1] == B) || (field[x - 1][y + 1] == BQ)) && (field[x - 2][y + 2] == 0) && (x - 2>-1) && (y + 2<8))
		{
			if (c == 0){
				push_SBDcord((x - 2) * 6 + 37, (y + 2) * 3 + 2);
				push_VCcord(((x - 1) * 6 + 37), ((y + 1) * 3 + 2));
			}
			count++;
		}
		if (((field[x - 1][y - 1] == B) || (field[x - 1][y - 1] == BQ)) && (field[x - 2][y - 2] == 0) && (x - 2>-1) && (y - 2>-1))
		{
			if (c == 0){
				push_SBDcord((x - 2) * 6 + 37, (y - 2) * 3 + 2);
				push_VCcord(((x - 1) * 6 + 37), ((y - 1) * 3 + 2));
			}
			count++;
		}
		if (((field[x + 1][y - 1] == B) || (field[x + 1][y - 1] == BQ)) && (field[x + 2][y - 2] == 0) && (x + 2<8) && (y - 2>-1))
		{
			if (c == 0){
				push_SBDcord((x + 2) * 6 + 37, (y - 2) * 3 + 2);
				push_VCcord(((x + 1) * 6 + 37), ((y - 1) * 3 + 2));
			}
			count++;
		}
	}
	else if (side_abc == B)
	{
		if (((field[x + 1][y + 1] == W) || (field[x + 1][y + 1] == WQ)) && (field[x + 2][y + 2] == 0) && (x + 2<8) && (y + 2<8))
		{
			if (c == 0){
				push_SBDcord((x + 2) * 6 + 37, (y + 2) * 3 + 2);
				push_VCcord(((x + 1) * 6 + 37), ((y + 1) * 3 + 2));
			}
			count++;
		}
		if (((field[x - 1][y + 1] == W) || (field[x - 1][y + 1] == WQ)) && (field[x - 2][y + 2] == 0) && (x - 2>-1) && (y + 2<8))
		{
			if (c == 0){
				push_SBDcord((x - 2) * 6 + 37, (y + 2) * 3 + 2);
				push_VCcord(((x - 1) * 6 + 37), ((y + 1) * 3 + 2));
			}
			count++;
		}
		if (((field[x - 1][y - 1] == W) || (field[x - 1][y - 1] == WQ)) && (field[x - 2][y - 2] == 0) && (x - 2>-1) && (y - 2>-1))
		{
			if (c == 0){
				push_SBDcord((x - 2) * 6 + 37, (y - 2) * 3 + 2);
				push_VCcord(((x - 1) * 6 + 37), ((y - 1) * 3 + 2));
			}
			count++;
		}
		if (((field[x + 1][y - 1] == W) || (field[x + 1][y - 1] == WQ)) && (field[x + 2][y - 2] == 0) && (x + 2<8) && (y - 2>-1))
		{
			if (c == 0){
				push_SBDcord((x + 2) * 6 + 37, (y - 2) * 3 + 2);
				push_VCcord(((x + 1) * 6 + 37), ((y - 1) * 3 + 2));
			}
			count++;
		}
	}
	return count;
}
void switch_side()
{
	int x, y;
	if (human_clr == side) strokes++;
	if (side == B)
	{
		side = W;
		if (wcc_head)
		{
			wcc_cur = wcc_head;
			x = wcc_head->crd.x;
			y = wcc_head->crd.y;
			gotoxy(x, y);
		}
	}
	else
	{
		side = B;
		if (bcc_head)
		{
			bcc_cur = bcc_head;
			x = bcc_head->crd.x;
			y = bcc_head->crd.y;
			gotoxy(x, y);
		}
	}
}
int skip_dots()        //переключение свободной клетки      
{

	int x = sbdc_head->crd.x;
	int y = sbdc_head->crd.y, xf, yf;
	int x_save, y_save;
	sbdc *p = sbdc_head;   
	sbdc_cur = sbdc_head;
	gotoxy(sbdc_cur->crd.x, sbdc_cur->crd.y);
	con_set_color(CON_CLR_CYAN_LIGHT, CON_CLR_YELLOW);

	if (side == B)
	{
		x_save = bcc_cur->crd.x;
		y_save = bcc_cur->crd.y;
	}
	else if (side == W)
	{
		x_save = wcc_cur->crd.x;
		y_save = wcc_cur->crd.y;
	}

	xf = field_crdX(x_save);
	yf = field_crdY(y_save);

	if ((field[xf][yf] == BQ) || (field[xf][yf] == WQ)||(qwar)) printf("D");
	else printf("O");

	while (p) // подсветка свободных полей
	{
		gotoxy(p->crd.x, p->crd.y);
		con_set_color(CON_CLR_CYAN_LIGHT, CON_CLR_YELLOW);
		printf("*");
		p = p->SBDNext;
	}
	
	gotoxy(x, y);
	con_set_color(CON_CLR_CYAN_LIGHT, CON_CLR_YELLOW);
	if ((field[xf][yf] == BQ) ||(field[xf][yf] == WQ)||(qwar)) printf("D");
	else printf("O");
	
	while (1){
		if (key_is_pressed())
		{
			int c = key_pressed_code();

			gotoxy(x, y);
			con_set_color(CON_CLR_CYAN_LIGHT, CON_CLR_YELLOW);
			printf("*");
			if (c == KEY_RIGHT)
			{
				if (sbdc_cur->SBDNext) sbdc_cur = sbdc_cur->SBDNext;
				else sbdc_cur = sbdc_head;
			}
			else  if (c == KEY_LEFT)
			{
				if (sbdc_cur->SBDPrev) sbdc_cur = sbdc_cur->SBDPrev;
				else sbdc_cur = sbdc_tail;
			}
			else if (c == KEY_SPACE)
			{
				clrStars(sbdc_cur,0);
				return c;
			}
			else if ((c == KEY_ESC) && (count_enemys==0)&&(qwar==0))
			{
				clrStars(sbdc_cur,1);
				destruct_SBDCcord();
				return c;
			}

			x = sbdc_cur->crd.x;
			y = sbdc_cur->crd.y;
			
			gotoxy(x, y);
			con_set_color(CON_CLR_CYAN_LIGHT, CON_CLR_YELLOW);

			if ((field[xf][yf] == BQ) || (field[xf][yf] == WQ)||(qwar)) printf("D");
			else printf("O");
		}
	}
}          
int search_free_dots()     //поиск свободных черных полей для хода шашки с координатами (x,y)
{
	int x_save, y_save,x_temp,y_temp;
	int count_dots = 0;
	int scan_p = 0;
	bdc *p=bdc_head;
	
	if (side == W)
	{
		x_save = wcc_cur->crd.x;
		y_save = wcc_cur->crd.y;
	}
	else if (side == B)
	{
		x_save = bcc_cur->crd.x;
		y_save = bcc_cur->crd.y;
	}
	gotoxy(x_save, y_save);

	if ((scan_p = scan_position(x_save, y_save,side,field,0)) != 0) return scan_p;
		while (p)
		{
			if (side == W)
			{
				if ((((p->crd.x + 6) == x_save)) && ((p->crd.y + 3) == y_save))
				{
					x_temp = ((p->crd.x) - 37) / 6;
					y_temp = ((p->crd.y) - 2) / 3;
					if (field[x_temp][y_temp] == 0)
					{
						count_dots++;
						push_SBDcord(p->crd.x, p->crd.y);
					}
				}
				if ((((p->crd.x - 6) == x_save)) && ((p->crd.y + 3) == y_save))
				{
					x_temp = ((p->crd.x) - 37) / 6;
					y_temp = ((p->crd.y) - 2) / 3;
					if (field[x_temp][y_temp] == 0)
					{
						push_SBDcord(p->crd.x, p->crd.y);
						count_dots++;
					}
				}
			}
			else if (side == B)
			{
				if ((((p->crd.x + 6) == x_save)) && ((p->crd.y - 3) == y_save))
				{
					x_temp = ((p->crd.x) - 37) / 6;
					y_temp = ((p->crd.y) - 2) / 3;
					if (field[x_temp][y_temp] == 0)
					{
						count_dots++;
						push_SBDcord(p->crd.x, p->crd.y);
					}
				}

				if ((((p->crd.x - 6) == x_save)) && ((p->crd.y - 3) == y_save))
				{
					x_temp = ((p->crd.x) - 37) / 6;
					y_temp = ((p->crd.y) - 2) / 3;
					if (field[x_temp][y_temp] == 0)
					{
						push_SBDcord(p->crd.x, p->crd.y);
						count_dots++;
					}
				}
			}
			p = p->BDNext;
		}
		return 0;
}            
void stroke()
{
	int i = 0, j = 0;
	bdc* dot_cord;
	int x = sbdc_cur->crd.x;
	int y = sbdc_cur->crd.y;
	if (side == W)
	{
		gotoxy(wcc_cur->crd.x, wcc_cur->crd.y);
		con_set_color(CON_CLR_BLACK, CON_CLR_YELLOW);
		printf(".");
		gotoxy(x, y);
		con_set_color(CON_CLR_WHITE, CON_CLR_YELLOW);
		printf("O");
		
		dot_cord=search_dot(sbdc_cur->crd.x, sbdc_cur->crd.y);
		dot_cord->crd.x = wcc_cur->crd.x;
		dot_cord->crd.y = wcc_cur->crd.y;

		x = (wcc_cur->crd.x - 37) / 6;   // установка значения в матрице 0 
		y = (wcc_cur->crd.y - 2) / 3;
		field[x][y] = 0;

		wcc_cur->crd.x = sbdc_cur->crd.x;
		wcc_cur->crd.y = sbdc_cur->crd.y;

		x = (sbdc_cur->crd.x - 37) / 6;   // установка значения в матрице.
		y = (sbdc_cur->crd.y - 2) / 3;
		field[x][y] = W;
		
	}
	else {
		gotoxy(bcc_cur->crd.x, bcc_cur->crd.y);
		con_set_color(CON_CLR_BLACK, CON_CLR_YELLOW);
		printf(".");
		gotoxy(x, y);
		con_set_color(CON_CLR_BLACK, CON_CLR_YELLOW);
		printf("O");
		
		dot_cord = search_dot(sbdc_cur->crd.x, sbdc_cur->crd.y);  // меняем координаты черной точки
		dot_cord->crd.x = bcc_cur->crd.x;
		dot_cord->crd.y = bcc_cur->crd.y;   
		

		x = (bcc_cur->crd.x - 37) / 6;   // установка значения в матрице 0 
		y = (bcc_cur->crd.y - 2) / 3;
		field[x][y] = 0;

		bcc_cur->crd.x = sbdc_cur->crd.x;
		bcc_cur->crd.y = sbdc_cur->crd.y;

		x = (sbdc_cur->crd.x - 37) / 6;   // установка значения в матрице.
		y = (sbdc_cur->crd.y - 2) / 3;
		field[x][y] = B;

	}
	destruct_SBDCcord();
}
//////Queen/////////
void queen_stroke()
{
	bdc* dot_cord;
	int x = sbdc_cur->crd.x;
	int y = sbdc_cur->crd.y;
	int xf, yf;
	xf = field_crdX(x);
	yf = field_crdY(y);

	clrStars(sbdc_cur, 0);
	if (side == W)
	{
		gotoxy(wcc_cur->crd.x, wcc_cur->crd.y);
		con_set_color(CON_CLR_BLACK, CON_CLR_YELLOW);
		printf(".");
		xf = field_crdX(wcc_cur->crd.x);
		yf = field_crdY(wcc_cur->crd.y);
        field[xf][yf] = 0;

		gotoxy(x, y);
		con_set_color(CON_CLR_WHITE, CON_CLR_YELLOW);
		printf("D");
		xf = field_crdX(x);
		yf = field_crdY(y);
		field[xf][yf] = WQ;


		dot_cord = search_dot(x, y);
		dot_cord->crd.x = wcc_cur->crd.x;
		dot_cord->crd.y = wcc_cur->crd.y;

		wcc_cur->crd.x = x;
		wcc_cur->crd.y = y;
	}
	else if (side == B)
	{
		gotoxy(bcc_cur->crd.x, bcc_cur->crd.y);
		con_set_color(CON_CLR_BLACK, CON_CLR_YELLOW);
		printf(".");
		xf = field_crdX(bcc_cur->crd.x);
		yf = field_crdY(bcc_cur->crd.y);
		field[xf][yf] = 0;

		gotoxy(x, y);
		con_set_color(CON_CLR_BLACK, CON_CLR_YELLOW);
		printf("D");
		xf = field_crdX(x);
		yf = field_crdY(y);
		field[xf][yf] = BQ;


		dot_cord = search_dot(x, y);
		dot_cord->crd.x = bcc_cur->crd.x;
		dot_cord->crd.y = bcc_cur->crd.y;

		bcc_cur->crd.x = x;
		bcc_cur->crd.y = y;

	}



}
int queen_check(int x1, int y1,int side_abc)
{
	int x = field_crdX(x1);
	int y = field_crdY(y1);
	
	if (field[x][y] == BQ || field[x][y] == WQ) return 2;
	
	if ((side_abc == W)&&(y == 0)&&(x == 1 || x == 3 || x == 5 || x == 7)) return 1;	
    if ((side_abc == B)&&(y == 7) && (x == 0 || x == 2 || x == 4 || x == 6)) return 1;
		
	return 0;
}
int scan_position_queen(int x1, int y1, int side_abc, int field[8][8],int c)
{
	int x = field_crdX(x1);
	int y = field_crdY(y1);
	int count_enemys = 0;
	int i = x, j = y, iunf, junf;
	i++; j++;//вправо вниз

	while (i < 7 && j < 7)
	{
		if (((field[i][j] == B || field[i][j] == BQ) && field[i + 1][j + 1] == 0 && side_abc == W) || ((field[i][j] == W||field[i][j]==WQ) && field[i + 1][j + 1] == 0 && side_abc == B))
		{
			iunf = unfield_crdX(i + 1);
			junf = unfield_crdY(j + 1);

			i = unfield_crdX(i);
			j = unfield_crdY(j);
			if (c == 0){
				push_VCcord(i, j);
				push_SBDcord(iunf, junf);
			}
			count_enemys++;
			break;
		}
		else if (field[i][j] != 0) break;
		i++; j++;
	}
	i = x - 1; j = y - 1;//влево вверх
	while (i >0 && j >0)
	{
		if (((field[i][j] == B || field[i][j] == BQ) && field[i - 1][j - 1] == 0 && side_abc == W) || ((field[i][j] == W || field[i][j] == WQ) && field[i - 1][j - 1] == 0 && side_abc == B))
		{
			iunf = unfield_crdX(i - 1);
			junf = unfield_crdY(j - 1);

			i = unfield_crdX(i);
			j = unfield_crdY(j);

			if (c == 0){
				push_VCcord(i, j);
				push_SBDcord(iunf, junf);
			}
			count_enemys++;
			break;
		}
		else if (field[i][j] != 0) break;
		i--; j--;
	}
	i = x - 1; j = y + 1;//влево вниз
	while (i >0 && j <7)
	{
		if (((field[i][j] == B || field[i][j] == BQ) && field[i - 1][j + 1] == 0 && side_abc == W) || ((field[i][j] == W || field[i][j] == WQ) && field[i - 1][j + 1] == 0 && side_abc == B))
		{
			iunf = unfield_crdX(i - 1);
			junf = unfield_crdY(j + 1);

			i = unfield_crdX(i);
			j = unfield_crdY(j);

			if (c == 0){
				push_VCcord(i, j);
				push_SBDcord(iunf, junf);
			}
			count_enemys++;
			break;
		}
		else if (field[i][j] != 0) break;
		i--; j++;
	}
	i = x + 1; j = y - 1;//вправо вверх
	while (i <7 && j >0)
	{
		if (((field[i][j] == B || field[i][j] == BQ) && field[i + 1][j - 1] == 0 && side_abc == W) || ((field[i][j] == W || field[i][j] == WQ) && field[i + 1][j - 1] == 0 && side_abc == B))
		{
			iunf = unfield_crdX(i + 1);
			junf = unfield_crdY(j - 1);

			i = unfield_crdX(i);
			j = unfield_crdY(j);

			if (c == 0){
				push_VCcord(i, j);
				push_SBDcord(iunf, junf);
			}
			count_enemys++;
			break;
		}
		else if (field[i][j] != 0) break;
		i++; j--;
	}

	return count_enemys;
}
int search_free_dots_queen()
{
	int x_temp, y_temp;
	int x = 0, y = 0;
	int unf_x, unf_y;
	int scan_p;

	if (side == B)
	{
		x_temp = bcc_cur->crd.x;
		y_temp = bcc_cur->crd.y;
	}
	else if (side == W)
	{
		x_temp = wcc_cur->crd.x;
		y_temp = wcc_cur->crd.y;
	}
	x = field_crdX(x_temp);
	y = field_crdY(y_temp);

	if ((scan_p = scan_position_queen(x_temp, y_temp,side,field,0))) return scan_p;

	while ((y>0)&&(x>0))   //левый верхний; 
	{
		y--;
		x--;
		unf_x = unfield_crdX(x);
		unf_y = unfield_crdY(y);
		if (field[x][y] == 0) push_SBDcord(unf_x, unf_y);
		else break;
	}
	x = field_crdX(x_temp);
	y = field_crdY(y_temp);

	while ((y>0) && (x<7))   //правый верхний; 
	{
		y--;
		x++;
		unf_x = unfield_crdX(x);
		unf_y = unfield_crdY(y);
		if (field[x][y] == 0) push_SBDcord(unf_x, unf_y);
		else break;

	}
	x = field_crdX(x_temp);
	y = field_crdY(y_temp);

	while ((y<7) && (x>0))   //левый нижний; 
	{
		y++;
		x--;
		unf_x = unfield_crdX(x);
		unf_y = unfield_crdY(y);
		if (field[x][y] == 0) push_SBDcord(unf_x, unf_y);
		else break;
	}
	x = field_crdX(x_temp);
	y = field_crdY(y_temp);

	while ((y < 7) && (x < 7))   //правый нижний; 
	{
		y++;
		x++;
		unf_x = unfield_crdX(x);
		unf_y = unfield_crdY(y);
		if (field[x][y] == 0) push_SBDcord(unf_x, unf_y);
		else break;
	}

	return 0;
}
int sbdcEQvc(int x_sbdc, int y_sbdc,int diffxf,int diffyf,int x_victim,int y_victim)     // проверяет совпали ли координаты точкИ, на которую ходим с координатой врага. Для дамки.
{
	int x_sbdcf = field_crdX(x_sbdc);
	int y_sbdcf = field_crdY(y_sbdc);
	int x_victimf = field_crdX(x_victim);
	int y_victimf = field_crdY(y_victim);

	if (((x_sbdcf + diffxf) == x_victimf) && ((y_sbdcf + diffyf) == y_victimf)) return 1;
	else return 0;

}
void war_queen()
{
	int x_save, y_save;
	int x_savef, y_savef;
	int sbdc_x, sbdc_y;   // координаты точки, находящейся на диагонали после врага.
	int x, y;
	int good_found = 0;   //флаг - если нашли подходящего врага для удаления
	int x_temp, y_temp;
	int diagonal = 0;       // значение диагонали, на которой нашли врага
	bcc *bcc_save = NULL;
	wcc *wcc_save=NULL;

	vc_cur = vc_head;
	sbdc_x = sbdc_cur->crd.x;
	sbdc_y = sbdc_cur->crd.y;

	if (side == B)
	{
		bcc_save = bcc_cur;
		x_save = bcc_cur->crd.x;
		y_save = bcc_cur->crd.y;
	}
	else if (side == W)
	{
		wcc_save = wcc_cur;
		x_save = wcc_cur->crd.x;
		y_save = wcc_cur->crd.y;
	}
	x = x_savef = field_crdX(x_save);
	y = y_savef = field_crdY(y_save);

	while (vc_cur)   //вычисление местоположения врага
	{
		while ((x > 0 && y > 0)&&(!good_found))   //левый верхний
		{
			x--;
			y--;
			if ((side == B) && ((field[x][y] == W) || (field[x][y] == WQ)))
			{
					if (sbdcEQvc(sbdc_x, sbdc_y, 1, 1, vc_cur->crd.x, vc_cur->crd.y)) good_found = 1;  //нашли
					diagonal = HL;
			}
			else if ((side == W)&&((field[x][y] == B)||(field[x][y]==BQ)))
			{
				
					if (sbdcEQvc(sbdc_x, sbdc_y, 1, 1, vc_cur->crd.x, vc_cur->crd.y)) good_found = 1;  //нашли	
					diagonal = HL;
			}
		}
		x = x_savef;
		y = y_savef;

		while ((x < 7 && y > 0) && (!good_found))    //правый верхний
		{
			x++;
			y--;
			if ((side == B) && ((field[x][y] == W) || (field[x][y] == WQ)))
			{
				if (sbdcEQvc(sbdc_x, sbdc_y, -1, 1, vc_cur->crd.x, vc_cur->crd.y))	good_found = 1;  //нашли	
				diagonal = HR;
				
			}
			else if ((side == W) && ((field[x][y] == B) || (field[x][y] == BQ)))
			{	
				if (sbdcEQvc(sbdc_x, sbdc_y, -1, 1, vc_cur->crd.x, vc_cur->crd.y)) good_found = 1;  //нашли	
				diagonal = HR;
			}
		}
		x = x_savef;
		y = y_savef;

		while ((x < 7 && y < 7) && (!good_found))    //правый нижний
		{
			x++;
			y++;
			if ((side == B) && ((field[x][y] == W) || (field[x][y] == WQ)))
			{
				if (sbdcEQvc(sbdc_x, sbdc_y, -1, -1, vc_cur->crd.x, vc_cur->crd.y)) good_found = 1;  //нашли
				diagonal = DR;

			}
			else if ((side == W) && ((field[x][y] == B) || (field[x][y] == BQ)))
			{
				if (sbdcEQvc(sbdc_x, sbdc_y, -1, -1, vc_cur->crd.x, vc_cur->crd.y))	good_found = 1;  //нашли			
				diagonal = DR;
			}
		}
		x = x_savef;
		y = y_savef;

		while ((x > 0 && y < 7) && (!good_found))    //левый нижний
		{
			x--;
			y++;
			if ((side == B) && ((field[x][y] == W) || (field[x][y] == WQ)))
			{
				if (sbdcEQvc(sbdc_x, sbdc_y, 1, -1, vc_cur->crd.x, vc_cur->crd.y)) good_found = 1;  //нашли
				diagonal = DL;
				
			}
			else if ((side == W) && ((field[x][y] == B) || (field[x][y] == BQ)))
			{	
				if (sbdcEQvc(sbdc_x, sbdc_y, 1, -1, vc_cur->crd.x, vc_cur->crd.y)) good_found = 1;  //нашли	
				diagonal = DL;
			}
		}
		x = x_savef;
		y = y_savef;
		if (good_found) break;
		vc_cur = vc_cur->VCNext;
	}
	bcc_cur = bcc_head;
	wcc_cur = wcc_head;
	x_temp = vc_cur->crd.x;
	y_temp = vc_cur->crd.y;

	while ((side == W) && (bcc_cur))   
	{
		if ((bcc_cur->crd.x == x_temp) && (bcc_cur->crd.y == y_temp)) break;
		bcc_cur = bcc_cur->BCNext;
	}
	while ((side == B) && (wcc_cur))
	{
		if ((wcc_cur->crd.x == x_temp) && (wcc_cur->crd.y == y_temp)) break;
		wcc_cur = wcc_cur->WCNext;
	}

	if (side == B)
	{
		x_temp = (wcc_cur->crd.x - 37) / 6;
		y_temp = (wcc_cur->crd.y - 2) / 3;
		field[x_temp][y_temp] = 0;   // ставим на месте положения удаленной фигуры черную точку
		gotoxy(wcc_cur->crd.x, wcc_cur->crd.y);
		con_set_color(CON_CLR_BLACK, CON_CLR_YELLOW);
		printf(".");
		push_BDcord(wcc_cur->crd.x, wcc_cur->crd.y);         
		pop_WCcord(wcc_cur);
	}
	else if (side == W)
	{
		x_temp = (bcc_cur->crd.x - 37) / 6;
		y_temp = (bcc_cur->crd.y - 2) / 3;
		field[x_temp][y_temp] = 0;
		gotoxy(bcc_cur->crd.x, bcc_cur->crd.y);
		con_set_color(CON_CLR_BLACK, CON_CLR_YELLOW);
		printf(".");
		push_BDcord(bcc_cur->crd.x, bcc_cur->crd.y);
		pop_BCcord(bcc_cur);
	}
	push_BDcord(x_save, y_save);  // ставим на первоначальном положении фигуры черную точку
	gotoxy(x_save, y_save);
	con_set_color(CON_CLR_BLACK, CON_CLR_YELLOW);
	printf(".");

	field[(x_save - 37) / 6][(y_save - 2) / 3] = 0;

	x_temp = field_crdX(sbdc_x);
	y_temp = field_crdY(sbdc_y);
	
	push_WCcord(sbdc_x, sbdc_y);
	push_BCcord(sbdc_x, sbdc_y);	//временная запись для bcc_cur
	bcc_cur = bcc_tail;
	wcc_cur = wcc_tail;
	destruct_SBDCcord();
	push_SBDcord(sbdc_x, sbdc_y);
	if (diagonal == HL)  // поиск возможных ходов для дамки после боя.
	{
		while (x_temp > 0 && y_temp>0)
		{
			if (field[x_temp - 1][y_temp - 1] != 0)
				break;
			x_temp--;
			y_temp--;
			x = unfield_crdX(x_temp);
			y = unfield_crdY(y_temp);
			push_SBDcord(x, y);
		}
	}
	if (diagonal == DR)
	{
		while (x_temp <7 && y_temp<7)
		{
			if (field[x_temp + 1][y_temp + 1] != 0)
				break;
			x_temp++;
			y_temp++;
			x = unfield_crdX(x_temp);
			y = unfield_crdY(y_temp);
			push_SBDcord(x, y);
		}
	}
	if (diagonal == DL)
	{
		while (x_temp >0 && y_temp<7)
		{
			if (field[x_temp - 1][y_temp + 1] != 0)
				break;
			x_temp--;
			y_temp++;
			x = unfield_crdX(x_temp);
			y = unfield_crdY(y_temp);
			push_SBDcord(x, y);
		}
	}
	if (diagonal == HR)
	{
		while (x_temp <7 && y_temp>0)
		{
			if (field[x_temp + 1][y_temp - 1] != 0)
				break;
			x_temp++;
			y_temp--;
			x = unfield_crdX(x_temp);
			y = unfield_crdY(y_temp);
			push_SBDcord(x, y);
		}
	}
	qwar = 1;
	if (skip_dots() == KEY_SPACE)
	{
		if (side == B)
		{
			gotoxy(sbdc_cur->crd.x, sbdc_cur->crd.y);
			pop_BDcord(search_dot(sbdc_cur->crd.x, sbdc_cur->crd.y));
			bcc_save->crd.x = sbdc_cur->crd.x;
			bcc_save->crd.y = sbdc_cur->crd.y;

			gotoxy(bcc_save->crd.x, bcc_save->crd.y);
			con_set_color(CON_CLR_BLACK, CON_CLR_YELLOW);
			printf("D");
			x_temp = (bcc_save->crd.x - 37) / 6;
			y_temp = (bcc_save->crd.y - 2) / 3;
			bcc_save->queen = Q;
			field[x_temp][y_temp] = BQ;
			x = bcc_save->crd.x;
			y = bcc_save->crd.y;
			bcc_cur = bcc_save;
		}
		else if (side == W)
		{
			gotoxy(sbdc_cur->crd.x, sbdc_cur->crd.y);
			pop_BDcord(search_dot(sbdc_cur->crd.x, sbdc_cur->crd.y));
			wcc_save->crd.x = sbdc_cur->crd.x;
			wcc_save->crd.y = sbdc_cur->crd.y;
			gotoxy(wcc_save->crd.x, wcc_save->crd.y);
			con_set_color(CON_CLR_WHITE, CON_CLR_YELLOW);
			printf("D");
			x_temp = (wcc_save->crd.x - 37) / 6;
			y_temp = (wcc_save->crd.y - 2) / 3;
			wcc_save->queen = Q;
			field[x_temp][y_temp] = WQ;
			x = wcc_save->crd.x;
			y = wcc_save->crd.y;
			wcc_cur = wcc_save;
		}

		pop_WCcord(wcc_tail);
		pop_BCcord(bcc_tail);
	}
	qwar = 0; 
	
	destruct_SBDCcord();
	destruct_VCcord();

	if (scan_position_queen(x, y,side,field,0))
	{
		search_free_dots_queen();
		if (skip_dots() == KEY_SPACE) war_queen();
	}

	destruct_SBDCcord();
	destruct_VCcord();

}
void war()
{
	vc_cur = vc_head;
	int x_cur, y_cur;
	int x_temp, y_temp;
	int x, y;
	wcc *wcc_save = NULL;
	bcc *bcc_save = NULL;

	if (side == B)
	{
		bcc_save = bcc_cur;
		x_cur = bcc_cur->crd.x;
		y_cur = bcc_cur->crd.y;
	}
	else if (side == W)
	{
		wcc_save = wcc_cur;
		x_cur = wcc_cur->crd.x;
		y_cur = wcc_cur->crd.y;
	}

	while (vc_cur)
	{
		if (((sbdc_cur->crd.x + 6) == (x_cur - 6)) && ((sbdc_cur->crd.y + 3) == (y_cur - 3)))
		{
			if (vc_cur->crd.x == (sbdc_cur->crd.x + 6) && vc_cur->crd.y == (sbdc_cur->crd.y + 3))
			{
				x_temp = vc_cur->crd.x;
				y_temp = vc_cur->crd.y;
			}
		}
		else if (((sbdc_cur->crd.x + 6) == (x_cur - 6)) && ((sbdc_cur->crd.y - 3) == (y_cur + 3)))
		{
			if (vc_cur->crd.x == (sbdc_cur->crd.x + 6) && vc_cur->crd.y == (sbdc_cur->crd.y - 3))
			{
				x_temp = vc_cur->crd.x;
				y_temp = vc_cur->crd.y;
			}
		}
		else if (((sbdc_cur->crd.x - 6) == (x_cur + 6)) && ((sbdc_cur->crd.y - 3) == (y_cur + 3)))
		{
			if (vc_cur->crd.x == (sbdc_cur->crd.x - 6) && vc_cur->crd.y == (sbdc_cur->crd.y - 3))
			{
				x_temp = vc_cur->crd.x;
				y_temp = vc_cur->crd.y;
			}
		}
		else if (((sbdc_cur->crd.x - 6) == (x_cur + 6)) && ((sbdc_cur->crd.y + 3) == (y_cur - 3)))
		{
			if (vc_cur->crd.x == (sbdc_cur->crd.x - 6) && vc_cur->crd.y == (sbdc_cur->crd.y + 3))
			{
				x_temp = vc_cur->crd.x;
				y_temp = vc_cur->crd.y;
			}
		}
		vc_cur = vc_cur->VCNext;
	}
	bcc_cur = bcc_head;
	wcc_cur = wcc_head;

	while ((side == W) && (bcc_cur))
	{
		if ((bcc_cur->crd.x == x_temp) && (bcc_cur->crd.y == y_temp)) break;
		bcc_cur = bcc_cur->BCNext;
	}
	while ((side == B) && (wcc_cur))
	{
		if ((wcc_cur->crd.x == x_temp) && (wcc_cur->crd.y == y_temp)) break;
		wcc_cur = wcc_cur->WCNext;
	}

	if (side == B)
	{
		x_temp = (wcc_cur->crd.x - 37) / 6;
		y_temp = (wcc_cur->crd.y - 2) / 3;
		field[x_temp][y_temp] = 0;
		gotoxy(wcc_cur->crd.x, wcc_cur->crd.y);
		con_set_color(CON_CLR_BLACK, CON_CLR_YELLOW);
		printf(".");
		push_BDcord(wcc_cur->crd.x, wcc_cur->crd.y);
		pop_WCcord(wcc_cur);
		gotoxy(sbdc_cur->crd.x, sbdc_cur->crd.y);
		pop_BDcord(search_dot(sbdc_cur->crd.x, sbdc_cur->crd.y));
		bcc_save->crd.x = sbdc_cur->crd.x;
		bcc_save->crd.y = sbdc_cur->crd.y;

		gotoxy(bcc_save->crd.x, bcc_save->crd.y);
		con_set_color(CON_CLR_BLACK, CON_CLR_YELLOW);
		printf("O");
		x_temp = (bcc_save->crd.x - 37) / 6;
		y_temp = (bcc_save->crd.y - 2) / 3;
		field[x_temp][y_temp] = 1;
		x = bcc_save->crd.x;
		y = bcc_save->crd.y;
		bcc_cur = bcc_save;
	}
	else if (side == W)
	{
		x_temp = (bcc_cur->crd.x - 37) / 6;
		y_temp = (bcc_cur->crd.y - 2) / 3;
		field[x_temp][y_temp] = 0;
		gotoxy(bcc_cur->crd.x, bcc_cur->crd.y);
		con_set_color(CON_CLR_BLACK, CON_CLR_YELLOW);
		printf(".");
		push_BDcord(bcc_cur->crd.x, bcc_cur->crd.y);
		pop_BCcord(bcc_cur);
		gotoxy(sbdc_cur->crd.x, sbdc_cur->crd.y);
		pop_BDcord(search_dot(sbdc_cur->crd.x, sbdc_cur->crd.y));
		wcc_save->crd.x = sbdc_cur->crd.x;
		wcc_save->crd.y = sbdc_cur->crd.y;
		gotoxy(wcc_save->crd.x, wcc_save->crd.y);
		con_set_color(CON_CLR_WHITE, CON_CLR_YELLOW);
		printf("O");
		x_temp = (wcc_save->crd.x - 37) / 6;
		y_temp = (wcc_save->crd.y - 2) / 3;
		field[x_temp][y_temp] = 2;
		x = wcc_save->crd.x;
		y = wcc_save->crd.y;
		wcc_cur = wcc_save;
	}
	push_BDcord(x_cur, y_cur);
	gotoxy(x_cur, y_cur);
	con_set_color(CON_CLR_BLACK, CON_CLR_YELLOW);
	printf(".");
	x_temp = (x_cur - 37) / 6;
	y_temp = (y_cur - 2) / 3;
	field[x_temp][y_temp] = 0;

	destruct_SBDCcord();
	destruct_VCcord();

	if ((q_check = queen_check(x, y,side)))
	{
		x_temp = x;
		y_temp = y;
		if (side == B) {
			bcc_cur->queen = Q;
			x = field_crdX(bcc_cur->crd.x);
			y = field_crdY(bcc_cur->crd.y);
			field[x][y] = BQ;
			con_set_color(CON_CLR_BLACK, CON_CLR_YELLOW);
			gotoxy(bcc_cur->crd.x, bcc_cur->crd.y);
			printf("D");
		}
		else if (side == W) {
			wcc_cur->queen = Q;
			x = field_crdX(wcc_cur->crd.x);
			y = field_crdY(wcc_cur->crd.y);
			field[x][y] = WQ;
			con_set_color(CON_CLR_WHITE, CON_CLR_YELLOW);
			gotoxy(wcc_cur->crd.x, wcc_cur->crd.y);
			printf("D");
		}
		x = x_temp;
		y = y_temp;

		if (scan_position_queen(x, y,side,field,0))
		{
			search_free_dots_queen();
			if (skip_dots() == KEY_SPACE) war_queen();
		}
	}
	else if (scan_position(x, y,side,field,0))
	{
		search_free_dots();
		if (skip_dots() == KEY_SPACE) war();
	}

	destruct_SBDCcord();
	destruct_VCcord();
}
/////COMPUTER//////////////////////
int delete_victim(int victim_x, int victim_y, int field[8][8],int score,int side_abc)
{	
	if (victim_x == -1 && victim_y == -1) return score;

	if ((side_abc == human_clr) && (field[victim_x][victim_y] != WQ && field[victim_x][victim_y] != BQ))
	{
		score = score - 10;
	}
	else if ((side_abc == comp_clr) && (field[victim_x][victim_y] != WQ && field[victim_x][victim_y] != BQ))
	{
		score = score + 10;
	}
	else if ((side_abc == human_clr) && (field[victim_x][victim_y] == WQ || field[victim_x][victim_y] == BQ))
	{
		score = score - 20;
	}
	else if ((side_abc == comp_clr) && (field[victim_x][victim_y] == WQ || field[victim_x][victim_y] == BQ))
	{
		score = score + 20;
	}
	field[victim_x][victim_y] = 0;
	return score;
}
void synchronization_field(int save_field[8][8])
{
	int x = 0, y = 0;

	destruct_VCcord();
	destruct_BCcord();
	destruct_WCcord();
	destruct_BDcord();
	destruct_SBDCcord();

	for (y = 0; y < 8; y++){
		for (x = 0; x < 8; x++){

			if (save_field[x][y] == 0){
				gotoxy(unfield_crdX(x), unfield_crdY(y));
				push_BDcord(unfield_crdX(x), unfield_crdY(y));
				con_set_color(CON_CLR_BLACK, CON_CLR_YELLOW);
				printf(".");
			}
			else if ((save_field[x][y] == B) || (save_field[x][y] == W)){
				gotoxy(unfield_crdX(x), unfield_crdY(y));
				if (save_field[x][y] == B)
				{
					push_BCcord(unfield_crdX(x), unfield_crdY(y));
					con_set_color(CON_CLR_BLACK, CON_CLR_YELLOW);
				}
				else if (save_field[x][y] == W)
				{
					push_WCcord(unfield_crdX(x), unfield_crdY(y));
					con_set_color(CON_CLR_WHITE, CON_CLR_YELLOW);
				}
				else con_set_color(CON_CLR_BLACK, CON_CLR_YELLOW);
				printf("O");
			}
			else if ((save_field[x][y] == BQ) || (save_field[x][y] == WQ)){
				gotoxy(unfield_crdX(x), unfield_crdY(y));
				if (save_field[x][y] == BQ)
				{
					push_BCcord(unfield_crdX(x), unfield_crdY(y));
					bcc_tail->queen = Q;
					con_set_color(CON_CLR_BLACK, CON_CLR_YELLOW);
				}
				else if (save_field[x][y] == WQ)
				{
					push_WCcord(unfield_crdX(x), unfield_crdY(y));
					wcc_tail->queen = Q;
					con_set_color(CON_CLR_WHITE, CON_CLR_YELLOW);
				}
				else con_set_color(CON_CLR_BLACK, CON_CLR_YELLOW);
				printf("D");
			}
			field[x][y] = save_field[x][y];
		}
	}
}
int scan_position_queen_comp(int x1, int y1, int side_abc, int field[8][8])
{
	int x = field_crdX(x1);
	int y = field_crdY(y1);
	int count_enemys = 0;
	int i = x, j = y, ivc = 0, jvc = 0;
	i++; j++;//вправо вниз

	while (i < 7 && j < 7)
	{
		if (((field[i][j] == B || field[i][j] == BQ) && field[i + 1][j + 1] == 0 && side_abc == W) || ((field[i][j] == W || field[i][j] == WQ) && field[i + 1][j + 1] == 0 && side_abc == B))
		{
			ivc = unfield_crdX(i);
			jvc = unfield_crdY(j);
			while (i < 7 && j < 7)
			{
				push_VCcord(ivc, jvc);
				push_SBDcord(unfield_crdX(i + 1), unfield_crdY(j + 1));
				i++; j++;
			}
				count_enemys++;
		}
		else if (field[i][j] != 0) break;
		i++; j++;
	}
	i = x - 1; j = y - 1;//влево вверх
	while (i >0 && j >0)
	{
		if (((field[i][j] == B || field[i][j] == BQ) && field[i - 1][j - 1] == 0 && side_abc == W) || ((field[i][j] == W || field[i][j] == WQ) && field[i - 1][j - 1] == 0 && side_abc == B))
		{
			ivc = unfield_crdX(i);
			jvc = unfield_crdY(j);
			while (i > 0 && j > 0)
			{
				push_VCcord(ivc, jvc);
				push_SBDcord(unfield_crdX(i - 1), unfield_crdY(j - 1));
				i--; j--;
			}
			count_enemys++;
		}
		else if (field[i][j] != 0) break;
		i--; j--;
	}
	i = x - 1; j = y + 1;//влево вниз
	while (i >0 && j <7)
	{
		if (((field[i][j] == B || field[i][j] == BQ) && field[i - 1][j + 1] == 0 && side_abc == W) || ((field[i][j] == W || field[i][j] == WQ) && field[i - 1][j + 1] == 0 && side_abc == B))
		{
			ivc = unfield_crdX(i);
			jvc = unfield_crdY(j);
			while (i > 0 && j < 7)
			{
				push_VCcord(ivc, jvc);
				push_SBDcord(unfield_crdX(i - 1), unfield_crdY(j + 1));
				i--; j++;
			}
			count_enemys++;
		}
		else if (field[i][j] != 0) break;
		i--; j++;
	}
	i = x + 1; j = y - 1;//вправо вверх
	while (i <7 && j >0)
	{
		if (((field[i][j] == B || field[i][j] == BQ) && field[i + 1][j - 1] == 0 && side_abc == W) || ((field[i][j] == W || field[i][j] == WQ) && field[i + 1][j - 1] == 0 && side_abc == B))
		{
			ivc = unfield_crdX(i);
			jvc = unfield_crdY(j);
			while (i < 7 && j > 0)
			{
				push_VCcord(ivc, jvc);
				push_SBDcord(unfield_crdX(i + 1), unfield_crdY(j - 1));
				i++; j--;
			}
			count_enemys++;
		}
		else if (field[i][j] != 0) break;
		i++; j--;
	}

	return count_enemys;

}
int switch_side_comp(int side_abc)
{
	if (side_abc == W)
	   return B;
	else return W;
}
int comp_stroke(int x, int y, int side_abc, int field[8][8],int Q_or_notQ)
{
	int x_temp,y_temp;
	int score = 0;
	x = field_crdX(x);
	y = field_crdY(y);
	x_temp = field_crdX(sbdc_cur->crd.x);
	y_temp = field_crdY(sbdc_cur->crd.y);

	
	if ((queen_check(unfield_crdX(x_temp), unfield_crdY(y_temp),side_abc) == 1)||(Q_or_notQ==Q))
	{
		if (field[x][y] != BQ && field[x][y] != WQ) score = 1;
		field[x][y] = 0;
		if (side_abc == B) field[x_temp][y_temp] = BQ;
		else field[x_temp][y_temp] = WQ;
		if ((score == 1)&&(side_abc==human_clr)) return -35;
		else if ((score == 1) && (side_abc == comp_clr)) return 35;
	}
	else
	{
		field[x][y] = 0;
		if (side_abc == B) field[x_temp][y_temp] = B;
		else field[x_temp][y_temp] = W;
	}
	return 0;
}
int get_opposite_side(int side_abc)
{
	if (side_abc == W) return B;
	return W;
}
int search_free_dots_comp(int x, int y, int side_abc, int field[8][8])
{
	int  x_temp, y_temp;
	int count_dots = 0;

	x = field_crdX(x);
	y = field_crdY(y);
	for (x_temp = 0; x_temp < 8; x_temp++){
		for (y_temp = 0; y_temp < 8; y_temp++){

			if (side_abc == W)
			{
				if ((field[x_temp][y_temp] == 0) && (((x_temp + 1) == x)) && ((y_temp + 1) == y))
				{
					count_dots++;
					push_SBDcord(unfield_crdX(x_temp), unfield_crdY(y_temp));
				}
				if ((field[x_temp][y_temp] == 0)&&(((x_temp - 1) == x)) && ((y_temp + 1) == y))
				{
					push_SBDcord(unfield_crdX(x_temp), unfield_crdY(y_temp));
					count_dots++;
				}
			}
			else if (side_abc == B)
			{
				if ((field[x_temp][y_temp] == 0) && (((x_temp + 1) == x)) && ((y_temp - 1) == y))
				{
					count_dots++;
					push_SBDcord(unfield_crdX(x_temp), unfield_crdY(y_temp));
				}
				if ((field[x_temp][y_temp] == 0) && (((x_temp - 1) == x)) && ((y_temp - 1) == y))
				{
					push_SBDcord(unfield_crdX(x_temp), unfield_crdY(y_temp));
					count_dots++;
				}
			}
		
		}
	}
	return count_dots;
}
int search_free_dots_queen_comp(int x1, int y1, int side_abc, int field[8][8],int c)
{
	int x = 0, y = 0;
	int unf_x, unf_y;
	int count = 0;

	x = field_crdX(x1);
	y = field_crdY(y1);

	while ((y>0) && (x>0))   //левый верхний; 
	{
		y--;
		x--;
		unf_x = unfield_crdX(x);
		unf_y = unfield_crdY(y);
		if (field[x][y] == 0) {
			count++;
			if (c==0) push_SBDcord(unf_x, unf_y);
		}
		else break;
	}
	x = field_crdX(x1);
	y = field_crdY(y1);

	while ((y>0) && (x<7))   //правый верхний; 
	{
		y--;
		x++;
		unf_x = unfield_crdX(x);
		unf_y = unfield_crdY(y);
		if (field[x][y] == 0) {
			count++;
			if (c == 0) push_SBDcord(unf_x, unf_y);
		}
		else break;

	}
	x = field_crdX(x1);
	y = field_crdY(y1);

	while ((y<7) && (x>0))   //левый нижний; 
	{
		y++;
		x--;
		unf_x = unfield_crdX(x);
		unf_y = unfield_crdY(y);
		if (field[x][y] == 0) {
			count++;
			if (c == 0) push_SBDcord(unf_x, unf_y);
		}
		else break;
	}
	x = field_crdX(x1);
	y = field_crdY(y1);

	while ((y < 7) && (x < 7))   //правый нижний; 
	{
		y++;
		x++;
		unf_x = unfield_crdX(x);
		unf_y = unfield_crdY(y);
		if (field[x][y] == 0) {
			count++;
			if (c == 0) push_SBDcord(unf_x, unf_y);
		}
		else break;
	}

	return count;
}
void approximate_field(int a[8][8], int b[8][8])
{
	int x = 0, y = 0;
	for (x = 0; x < 8; x++) {
		for (y = 0; y < 8;y++){
			a[x][y] = b[x][y];
		}
	}
}
int get_free_checkers_eat(int f[8][8], int side_abc)
{
	int x, y;
	int cnt = 0;
	int i = 0;
	int queen_stroke = 0;
	int scan_p = 0;

	destruct_FCcord();
	bcc_cur = bcc_head;
	wcc_cur = wcc_head;

	if (side_abc == W)
	{
		while (wcc_cur)
		{
			x = field_crdX(wcc_cur->crd.x);
			y = field_crdY(wcc_cur->crd.y);

			if ((f[x][y] == WQ) && ((scan_p = scan_position_queen(wcc_cur->crd.x, wcc_cur->crd.y, side_abc, f, 1))))
			{
				push_FCcord(wcc_cur->crd.x, wcc_cur->crd.y);
				fcc_tail->queen = wcc_cur->queen;
				fcc_tail->color = W;
				if (scan_p != 0)
				{
					cnt++;
					fcc_tail->eat = 1;
				}
				else fcc_tail->eat = 0;
				scan_p = 0;
				destruct_SBDCcord();
				destruct_VCcord();
			}
			else if (((scan_p = scan_position(wcc_cur->crd.x, wcc_cur->crd.y, side_abc, f,1))))
			{
				push_FCcord(wcc_cur->crd.x, wcc_cur->crd.y);
				fcc_tail->queen = wcc_cur->queen;
				fcc_tail->color = W;
				if (scan_p != 0)
				{
					cnt++;
					fcc_tail->eat = 1;
				}
				else fcc_tail->eat = 0;
				scan_p = 0;
				destruct_SBDCcord();
				destruct_VCcord();
			}
			wcc_cur = wcc_cur->WCNext;
		}
	}
	else if (side_abc == B)
	{
		while (bcc_cur)
		{
			x = field_crdX(bcc_cur->crd.x);
			y = field_crdY(bcc_cur->crd.y);

			if ((f[x][y] == BQ) && ((scan_p = scan_position_queen(bcc_cur->crd.x, bcc_cur->crd.y, side_abc, f, 1))))
			{
				push_FCcord(bcc_cur->crd.x, bcc_cur->crd.y);
				fcc_tail->queen = bcc_cur->queen;
				fcc_tail->color = B;
				if (scan_p != 0)
				{
					cnt++;
					fcc_tail->eat = 1;
				}
				else fcc_tail->eat = 0;
				scan_p = 0;
				destruct_SBDCcord();
				destruct_VCcord();
			}
			else if (((scan_p = scan_position(bcc_cur->crd.x, bcc_cur->crd.y, side_abc, f,1))))
			{
				push_FCcord(bcc_cur->crd.x, bcc_cur->crd.y);
				fcc_tail->queen = bcc_cur->queen;
				fcc_tail->color = B;
				if (scan_p != 0)
				{
					cnt++;
					fcc_tail->eat = 1;
				}
				else fcc_tail->eat = 0;
				scan_p = 0;
				destruct_SBDCcord();
				destruct_VCcord();
			}
			bcc_cur = bcc_cur->BCNext;
		}
	}
	return cnt;
}
int get_free_checkers_comp(int f[8][8], int side_abc,int check)
{
	int x, y;
	int i = 0;
	int cnt = 0;
	int scan_p = 0;
	int queen_stroke = 0;

	if (side_abc == W)
	{
		for (y = 0; y < 8; y++){
			for (x = 0; x < 8; x++){

				if ((f[x][y] == WQ) && ((scan_p = scan_position_queen(unfield_crdX(x), unfield_crdY(y), side_abc, f,check)) || ((queen_stroke = search_free_dots_queen_comp(unfield_crdX(x), unfield_crdY(y),side_abc,f,check)))))
				{
					cnt++;
					if (check == 0)
					{
						push_FCcord(unfield_crdX(x), unfield_crdY(y));
						fcc_tail->queen = Q;
						fcc_tail->color = W;
						if (scan_p != 0) fcc_tail->eat = 1;
						else fcc_tail->eat = 0;
						scan_p = 0;
						destruct_SBDCcord();
						destruct_VCcord();
					}
				}
				else if ((f[x][y] == W)&& (((scan_p = scan_position(unfield_crdX(x), unfield_crdY(y), side_abc, f,check))) || ((y > 0) && (x > 0) && (f[x - 1][y - 1] == 0)) || ((y > 0) && (x < 7) && (f[x + 1][y - 1] == 0))))
				{
					cnt++;
					if (check == 0)
					{
						push_FCcord(unfield_crdX(x), unfield_crdY(y));
						fcc_tail->queen = 0;
						fcc_tail->color = W;
						if (scan_p != 0) fcc_tail->eat = 1;
						else fcc_tail->eat = 0;
						scan_p = 0;
						destruct_SBDCcord();
						destruct_VCcord();
					}
				}
			}
		}
	}
	else if (side_abc == B)
	{
		for (y = 0; y < 8; y++){
			for (x = 0; x < 8; x++){

				if ((f[x][y] == BQ) && ((scan_p = scan_position_queen(unfield_crdX(x), unfield_crdY(y), side_abc, f,check)) || ((queen_stroke = search_free_dots_queen_comp(unfield_crdX(x), unfield_crdY(y), side_abc, f,check)))))
				{
					cnt++;
					if (check == 0)
					{
						push_FCcord(unfield_crdX(x), unfield_crdY(y));
						fcc_tail->queen = Q;
						fcc_tail->color = B;
						if (scan_p != 0) fcc_tail->eat = 1;
						else fcc_tail->eat = 0;
						scan_p = 0;
						destruct_SBDCcord();
						destruct_VCcord();
					}
				}
				else if ((f[x][y] == B) && (((scan_p = scan_position(unfield_crdX(x), unfield_crdY(y), side_abc, f,check))) || ((y < 7) && (x > 0) && (f[x - 1][y + 1] == 0)) || ((y < 7) && (x < 7) && (f[x + 1][y + 1] == 0))))
				{
					cnt++;
					if (check == 0)
					{
						push_FCcord(unfield_crdX(x), unfield_crdY(y));
						fcc_tail->queen = 0;
						fcc_tail->color = B;
						if (scan_p != 0) fcc_tail->eat = 1;
						else fcc_tail->eat = 0;
						scan_p = 0;
						destruct_SBDCcord();
						destruct_VCcord();
					}
				}
			}
		}
	}
	return cnt;
}
void push_variant_list(int field[8][8], btree *current_list, int score)
{
	if (count_variants==0){	
		current_list->down_tree = (btree*)malloc(sizeof(btree));
		current_list = current_list->down_tree;
		tree_cur = current_list;
		tree_cur->priority = score;
	    approximate_field(tree_cur->field, field);
	    tree_cur->next = NULL;
		tree_cur->down_tree = NULL;
		count_variants++;
	}
	else {
		tree_cur->next = (btree*)malloc(sizeof(btree));
		tree_cur = tree_cur->next;
		approximate_field(tree_cur->field, field);
		tree_cur->priority = score;
		tree_cur->next = NULL;
		tree_cur->down_tree = NULL;
		count_variants++;
	}
}
int ifWin_comp(int save_field[8][8])
{
	if (get_free_checkers_comp(save_field, B, 1) == 0)	return W;	
	if (get_free_checkers_comp(save_field, W, 1) == 0)	return B;	
	return 0;
}
void War_queen_comp(int x1, int y1, int side_abc, int field_g[8][8], int x_victim, int y_victim, btree* current_list, int score)
{
	int i;
	int koor[30];
	int victims_koord[30];
	int field_war[8][8];

	approximate_field(field_war, field_g);
	memset(koor, -1, 30 * sizeof(int));
	memset(victims_koord, -1, 30 * sizeof(int));

	destruct_VCcord();
	destruct_SBDCcord();
	score = delete_victim(x_victim, y_victim, field_war, score,side_abc);

	if (x_victim == -1 && y_victim == -1) field_war[x1][y1] = 0;
	if (scan_position_queen_comp(unfield_crdX(x1), unfield_crdY(y1), side_abc, field_war) == 0)
	{
		if (side_abc == B) field_war[x1][y1] = BQ;
		else if (side_abc == W) field_war[x1][y1] = WQ;

		if (ifWin_comp(field_war) == human_clr) score += LOSE;
		else if (ifWin_comp(field_war) == comp_clr) score += WIN;

		if ((side_abc == human_clr)&&(score!=0)) push_variant_list(field_war, current_list, (score - 4));
		else push_variant_list(field_war, current_list, score);

		if ((get_free_checkers_comp(field_war, get_opposite_side(side_abc), 1) == 0)) return;
		return;
	}

	sbdc_cur = sbdc_head;
	vc_cur = vc_head;
	i = 0;
	while (sbdc_cur)
	{
		koor[i] = field_crdX(sbdc_cur->crd.x);
		koor[i + 1] = field_crdY(sbdc_cur->crd.y);
		victims_koord[i] = field_crdX(vc_cur->crd.x);
		victims_koord[i + 1] = field_crdY(vc_cur->crd.y);
		i += 2;
		sbdc_cur = sbdc_cur->SBDNext;
		vc_cur = vc_cur->VCNext;
	}
	i = 0;
	while (koor[i] != -1)
	{
		War_queen_comp(koor[i], koor[i + 1], side_abc, field_war, victims_koord[i], victims_koord[i + 1], current_list, score);
		i += 2;
	}
}
void War_comp(int x1, int y1, int side_abc, int field_g[8][8], int x_victim, int y_victim, btree *current_list, int score)
{
	int i=0;
	int koor[10];
	int victims_koord[10];
	int field_war[8][8];

	approximate_field(field_war, field_g);
	memset(koor, -1, 10 * sizeof(int));
	memset(victims_koord, -1, 10 * sizeof(int));

	destruct_VCcord();
	destruct_SBDCcord();
	score = delete_victim(x_victim, y_victim, field_war, score, side_abc);

	if (x_victim == -1 && y_victim == -1) field_war[x1][y1] = 0;
	if (scan_position(unfield_crdX(x1), unfield_crdY(y1), side_abc, field_war,0) == 0)
	{
		if (side_abc == B){
		
			if (queen_check(unfield_crdX(x1), unfield_crdY(y1), side_abc) == 1)
			{
				field_war[x1][y1] = BQ;
				if (side_abc == comp_clr) score = score + 35;
				else if (side_abc == human_clr) score = score - 35;
			}
			else field_war[x1][y1] = B;
		}
		else if (side_abc == W)
		{
			if (queen_check(unfield_crdX(x1), unfield_crdY(y1), side_abc) == 1)
			{
				field_war[x1][y1] = WQ;
				if (side_abc == comp_clr) score = score + 35;
				else if (side_abc == human_clr) score = score - 35;
			}
			else field_war[x1][y1] = W;
		}
		if (field_war[x1][y1] == WQ || field_war[x1][y1] == BQ)
		{
			War_queen_comp(x1, y1, side_abc, field_war, -1, -1, current_list, score);
		}
		else
		{
			if (ifWin_comp(field_war) == human_clr) score += LOSE;
			else if (ifWin_comp(field_war) == comp_clr) score += WIN;

			if ((side_abc == human_clr)&&(score!=0)) push_variant_list(field_war, current_list, (score - 4));
			else push_variant_list(field_war, current_list, score);

			if ((get_free_checkers_comp(field_war, get_opposite_side(side_abc), 1) == 0)) return;
		}
		return;
	}

	if (queen_check(unfield_crdX(x1), unfield_crdY(y1), side_abc))
	{
		War_queen_comp(x1, y1, side_abc, field_war, -1, -1, current_list, 0);
		destruct_SBDCcord();
		destruct_VCcord();
		return;
	}

	sbdc_cur = sbdc_head;
	vc_cur = vc_head;
	while (sbdc_cur)
	{
		koor[i]=field_crdX(sbdc_cur->crd.x);
		koor[i+1] = field_crdY(sbdc_cur->crd.y);
		victims_koord[i] = field_crdX(vc_cur->crd.x);
		victims_koord[i+1] = field_crdY(vc_cur->crd.y);
		i += 2;
		sbdc_cur = sbdc_cur->SBDNext;
		vc_cur = vc_cur->VCNext;
	}
	i = 0;
	while (koor[i] != -1)
	{
		War_comp(koor[i], koor[i + 1], side_abc, field_war, victims_koord[i], victims_koord[i + 1], current_list, score);
		i += 2;
	}
}
int search_variants(int side_abc, int field[8][8], btree *current_list)
{
	int save_field[8][8];
	int score = 0;
	int WinScore = 0;
	approximate_field(save_field, field);
	get_free_checkers_comp(save_field, side_abc,0);

	if (fcc_head == NULL)	return 0;
	
	fcc_cur = fcc_head;
	
	while ((fcc_cur) && (fcc_cur->color != get_opposite_side(side_abc)))
	{
		if (fcc_cur->queen == Q)
		{
			if (scan_position_queen(fcc_cur->crd.x, fcc_cur->crd.y, side_abc, save_field,0))
			{
				destruct_SBDCcord();
				destruct_VCcord();
				War_queen_comp(field_crdX(fcc_cur->crd.x), field_crdY(fcc_cur->crd.y), side_abc, save_field, -1, -1, current_list, 0);
			}
			else
			{
				search_free_dots_queen_comp(fcc_cur->crd.x, fcc_cur->crd.y, side_abc, save_field,0);
				sbdc_cur = sbdc_head;
				while (sbdc_cur)
				{
					score = comp_stroke(fcc_cur->crd.x, fcc_cur->crd.y, side_abc, save_field, Q);
					sbdc_cur = sbdc_cur->SBDNext;

					if (ifWin_comp(save_field) == human_clr) score += LOSE;
					else if (ifWin_comp(save_field) == comp_clr) score += WIN;

					if ((side_abc == human_clr)&&(score!=0)) score = score - 4;

					push_variant_list(save_field, current_list, score);
					approximate_field(save_field, field);
					score = 0;
				}
			}
		}
		else if (fcc_cur->queen != Q)
		{
			if (scan_position(fcc_cur->crd.x, fcc_cur->crd.y, side_abc, save_field,0))
			{
				destruct_SBDCcord();
				destruct_VCcord();
				War_comp(field_crdX(fcc_cur->crd.x), field_crdY(fcc_cur->crd.y), side_abc, save_field, -1, -1, current_list, 0);
			}
			else
			{
				search_free_dots_comp(fcc_cur->crd.x, fcc_cur->crd.y, side_abc, save_field);
				sbdc_cur = sbdc_head;
				while (sbdc_cur)
				{
					score = comp_stroke(fcc_cur->crd.x, fcc_cur->crd.y, side_abc, save_field, 0);
					sbdc_cur = sbdc_cur->SBDNext;
					
					if (ifWin_comp(save_field) == human_clr) score += LOSE;
					else if (ifWin_comp(save_field) == comp_clr) score += WIN;

					if ((side_abc == human_clr) && (score != 0)) score = score - 4;

					push_variant_list(save_field, current_list,score);
					approximate_field(save_field, field);
					score = 0;
				}
			}
		}

		fcc_cur = fcc_cur->FCNext;
		destruct_SBDCcord();
		destruct_VCcord();
	}
	count_variants = 0;
	destruct_SBDCcord();
	destruct_FCcord();
	return 1;
}
void abc(int side_abc, int save_field[8][8],int deep, btree* current_list,btree* last_current_list)
{
	btree *current = NULL;
	btree* tmp = NULL;
	btree* save_ptr = NULL;
	long maximum = -999999;
	int eat_cnt = 0;
	int x = 0, y = 0;
	
	if (tree_head) --deep;
	if (deep < 0)	return; 
	
	if (tree_head == NULL)
	{
		tree_head = (btree*)malloc(sizeof(btree));
		approximate_field(tree_head->field, field);
		approximate_field(save_field, field);
		tree_head->next = NULL;
		tree_head->down_tree = NULL;
		last_current_list = tree_head;
		current_list = tree_head; 
		abc(side_abc, save_field,deep, current_list,last_current_list);
	}
	else if (tree_head != NULL)
	{
		side_abc = switch_side_comp(side_abc);
		
		if ((search_variants(side_abc, save_field, current_list)) == 0)	return; 
		else	current_list = current_list->down_tree;
		
		current = current_list;

		tmp = current_list;
		while (tmp)
		{
			if ((tmp->priority >= 10||tmp->priority<=-10) && (tmp->priority != 35&&tmp->priority!=-39))
			{
				eat_cnt = 1;
				break;
			}
			tmp = tmp->next;
		}
		if (eat_cnt == 1)
		{
			tmp = current_list;

			while (tmp)
			{
				if ((tmp->priority < 10 && tmp->priority > -10) || (tmp->priority == 35 || tmp->priority == -39))
				{
					current_list = pop_btree(tmp,current_list,last_current_list);	
					tmp = last_current_list->down_tree;
				}
				else tmp = tmp->next;
			}
				eat_cnt = 0;
		}
		current_list = last_current_list->down_tree;
		current = current_list;

		while (current)
		{
			 last_current_list = current;
			 approximate_field(save_field, current->field);
			 abc(side_abc, save_field, deep,current, last_current_list);
			 if (current->down_tree) current->priority = current->down_tree->priority + current->priority;
			 if (current->priority > maximum) maximum = current->priority;       

		     current = current->next;
	    }
		if (tree_head->down_tree == current_list)
		{	

			current = current_list;
			while (current)
			{
				if (current->priority == maximum) break;
				else current = current->next;
			}
			synchronization_field(current->field);
			return;
		}
		else current_list->priority = maximum;
		
	}
}
void destruct_tree(btree *cur)
{
	if (cur != NULL)
	{
		destruct_tree(cur->down_tree);
		destruct_tree(cur->next);
		free(cur);
		cur = NULL;
	}
	else tree_head = NULL;
}
int skip_checker()                          //переключение между шашками на доске.
{
	int x, y;
	int xf, yf, can_eat = 0;
	if (side == B){
		can_eat = get_free_checkers_eat(field, side);
		x = bcc_head->crd.x;
		y = bcc_head->crd.y;
	}
	else if (side == W)
	{
		can_eat = get_free_checkers_eat(field, side);
		x = wcc_head->crd.x;
		y = wcc_head->crd.y;
	}
	wcc_cur = wcc_head;
	bcc_cur = bcc_head;
	if (fcc_head) fcc_cur = fcc_head;

	while (1){

		if (key_is_pressed())
		{
			int c = key_pressed_code();

			gotoxy(x, y);

			if (side == B) con_set_color(CON_CLR_BLACK, CON_CLR_YELLOW);
			else if (side == W) con_set_color(CON_CLR_WHITE, CON_CLR_YELLOW);
			xf = field_crdX(x);
			yf = field_crdY(y);
			if ((field[xf][yf] == BQ) ||
				(field[xf][yf] == WQ)) printf("D");
			else printf("O");

			if (c == KEY_RIGHT)
			{
				if (side == B)
				{
					if (can_eat == 0){
						if (bcc_cur->BCNext) bcc_cur = bcc_cur->BCNext;
						else bcc_cur = bcc_head;
					}
					else if (can_eat)
					{
						if (fcc_cur->FCNext)	fcc_cur = fcc_cur->FCNext;
						else fcc_cur = fcc_head;
					}
				}
				else if (side == W)
				{
					if (can_eat == 0){
						if (wcc_cur->WCNext) wcc_cur = wcc_cur->WCNext;
						else wcc_cur = wcc_head;
					}
				    else if (can_eat)
			     	{
					    if (fcc_cur->FCNext)	fcc_cur = fcc_cur->FCNext;
					    else fcc_cur = fcc_head;
				    }
			    }
			}
			else  if (c == KEY_LEFT)
			{
				if (side == B)
				{
					if (can_eat == 0){
						if (bcc_cur->BCPrev) bcc_cur = bcc_cur->BCPrev;
						else bcc_cur = bcc_tail;
					}
					else if (can_eat)
					{
						if (fcc_cur->FCPrev) fcc_cur = fcc_cur->FCPrev;
						else fcc_cur = fcc_tail;
					}
				}
				else if (side == W)
				{
					if (can_eat == 0){
						if (wcc_cur->WCPrev) wcc_cur = wcc_cur->WCPrev;
						else wcc_cur = wcc_tail;
					}
					else if (can_eat)
					{
						if (fcc_cur->FCPrev) fcc_cur = fcc_cur->FCPrev;
						else fcc_cur = fcc_tail;
					}
				}
			}
			else if (c == 'q' || c == 'Q' || c == 'й' || c == 'Й')
			{
				destruct_VCcord();
				destruct_BCcord();
				destruct_FCcord();
				destruct_WCcord();
				destruct_BDcord();
				destruct_SBDCcord();
				return EXIT;
			}
			else
			{
				destruct_FCcord();
				return c;
			}

			if (side == B){
				if (can_eat)
				{
					x = fcc_cur->crd.x;
					y = fcc_cur->crd.y;
					bcc_cur = bcc_head;
					while (1)
					{
						if (bcc_cur->crd.x == fcc_cur->crd.x && bcc_cur->crd.y == fcc_cur->crd.y) break;
						bcc_cur = bcc_cur->BCNext;
					}
				}
				else
				{
					x = bcc_cur->crd.x;
					y = bcc_cur->crd.y;
				}
			}
			else if (side == W)
			{
				if (can_eat)
				{
					x = fcc_cur->crd.x;
					y = fcc_cur->crd.y;	
					wcc_cur = wcc_head;
					while (1)
					{
						if (wcc_cur->crd.x == fcc_cur->crd.x && wcc_cur->crd.y == fcc_cur->crd.y) break;
						wcc_cur = wcc_cur->WCNext;
					}
				}
				else
				{
					x = wcc_cur->crd.x;
					y = wcc_cur->crd.y;
				}
			}

			gotoxy(x, y);
			con_set_color(CON_CLR_CYAN_LIGHT, CON_CLR_YELLOW);
			xf = field_crdX(x);
			yf = field_crdY(y);
			if ((field[xf][yf] == BQ) ||
				(field[xf][yf] == WQ)) printf("D");
			else printf("O");
		}
	}
}
////////////COMPUTER STROKE/////////
void computer_stroke()
{
	abc(human_clr, field, deep,NULL, NULL);
	destruct_tree(tree_head);
	destruct_FCcord();
}
/////// //////////////////////////// 
void printf_WIN(int clr)
{
	if (clr == W)	printf("WHITE CHECKERS WIN!!!");	
	else if (clr == B)	printf("BLACK CHECKERS WIN!!!");
}
void field_draw(){
	int x_rh = 33, y_rh = 0;
	int x_rd = 33, y_rd = 25;
	int x_r = 33, y_r = 1;
	int x_l = 82, y_l = 1;
	while (x_rh < 83) {
		gotoxy(x_rh, y_rh);
		printf("-");
		x_rh++;
	}
	while (x_rd < 83) {
		gotoxy(x_rd, y_rd);
		printf("-");
		x_rd++;
	}
	while (y_r < 25) {
		gotoxy(x_r, y_r);
		printf("|");
		y_r++;
	}
	while (y_l < 25) {
		gotoxy(x_l, y_l);
		printf("|");
		y_l++;
	}

}
void field_restart(){
	int x = 0, y = 0;
	int save;
	memset(field, -1, 64 * sizeof(int));
	destruct_BCcord();
	destruct_VCcord();
	destruct_WCcord();
	destruct_BDcord();
	destruct_FCcord();
	destruct_SBDCcord();
	destruct_tree(tree_head);
	//black:
	field[b][0] = B;
	field[d][0] = B;
	field[f][0] = B;
	field[h][0] = B;
	field[a][1] = B;
	field[c][1] = B;
	field[e][1] = B;
	field[g][1] = B;
	field[b][2] = B;
	field[d][2] = B;
	field[f][2] = B;
	field[h][2] = B;
	//white:
	field[a][5] = W;
	field[c][5] = W;
	field[e][5] = W;
	field[g][5] = W;
	field[b][6] = W;
	field[d][6] = W;
	field[f][6] = W;
	field[h][6] = W;
	field[a][7] = W;
	field[c][7] = W;
	field[e][7] = W;
	field[g][7] = W;

	save = 0;   //-1 - белое поле, 0 - пустое поле , 2 - белая шашка, 1 - черная шашка

	for (y = 0; y < 8; y++){
		for (x = 0; x < 8; x++){
			if (save == 0){
				if (x != 7) save = 1;
				continue;
			}
			if (save == 1) {
				if ((field[y][x] != B) && (field[y][x] != W)) field[y][x] = 0;
				if (x != 7) save = 0;
			}

			if (field[y][x] == 0){
				gotoxy(y * 6 + 37, x * 3 + 2);
				push_BDcord(y * 6 + 37, x * 3 + 2);
				printf(".");
			}
			else if ((field[y][x] == B) || (field[y][x] == W)){
				gotoxy(y * 6 + 37, x * 3 + 2);
				if (field[y][x] == B)
				{
					push_BCcord(y * 6 + 37, x * 3 + 2);
					con_set_color(CON_CLR_BLACK, CON_CLR_YELLOW);
				}
				else if (field[y][x] == W)
				{
					push_WCcord(y * 6 + 37, x * 3 + 2);
					con_set_color(CON_CLR_WHITE, CON_CLR_YELLOW);
				}
				else con_set_color(CON_CLR_BLACK, CON_CLR_YELLOW);
				printf("O");
			}


		}
	}

	side = W;
	wcc_cur = wcc_head;
	x = wcc_head->crd.x;
	y = wcc_head->crd.y;
    gotoxy(x, y);
}
void inquiry()
{
	short clr_font = CON_CLR_WHITE_LIGHT;
	short clr_bg = CON_CLR_BLUE;

	con_set_color(clr_font, clr_bg);
	clrscr();
	gotoxy(50, 2);
	printf("СПРАВКА");
	gotoxy(2, 4);
	printf(
		"\n\n\
Стрелки влево, вправо - выбор нужной шашки для хода / выбор клетки для хода.\n\n\
Пробел - захватить шашку/завершить ход на выбранную клетку.\n\n\
q - выйти из игры.\n\n\
ESC - отмена хода.\n\n");

	key_pressed_code();
	con_set_color(clr_font, clr_bg);
	clrscr();
	gotoxy(53, 2);
	printf("ГЛАВНОЕ МЕНЮ");

	return;
}
void about()
{
	short clr_font = CON_CLR_WHITE_LIGHT;
	short clr_bg = CON_CLR_BLUE;

	con_set_color(clr_font, clr_bg);
	clrscr();

	gotoxy(0, 2);
	printf(
		"О программе:\n\nАвторы:\nСиняпкин Борис Григорьевич\nТретьяк Артем Вадимович\n\nГруппа: 13558/5\n\nСанкт-Петербургский Политехнический Университет\nИнститут компьютерных наук и технологий\nКафедра информационной безопасности компьютерных систем\n\n2017 год");

	key_pressed_code();
	con_set_color(clr_font, clr_bg);
	clrscr();
	gotoxy(53, 2);
	printf("ГЛАВНОЕ МЕНЮ");

	return;
}
void rang_table()
{
	int c, i = 0,y=4;
	int slash_n = 1;
	int j = 1;
	char *string = NULL;

	rt = fopen("rang_table.txt", "r");
	if (rt == NULL)
	{
		con_set_color(CON_CLR_WHITE_LIGHT, CON_CLR_BLUE);
		clrscr();
		gotoxy(53, 2);
		printf("ТАБЛИЦА РЕКОРДОВ ПУСТА");
	}
	else
	{
		con_set_color(CON_CLR_WHITE_LIGHT, CON_CLR_BLUE);
		clrscr();
		gotoxy(53, 2);
		printf("ТАБЛИЦА РЕКОРДОВ");
		gotoxy(2, 4);

		i = 0;
		while ((c = fgetc(rt)) != EOF)
		{
			string = (char*)realloc(string, sizeof(char)*(i + 2));
			string[i] = c;
			i++;
		}
		string[i] = '\0';
		i = 0;

		while (string[i] != '\0')
		{
			if (string[i] == '\n')
			{
				slash_n = 1;
				gotoxy(2, ++y);
				i++;
			}
			else if (slash_n)
			{
				slash_n = 0;
				printf("%d.", j);
				j++;
			}
			else
			{
				printf("%c", string[i]);
				i++;
			}
		}	
		free(string);
		string = NULL;
	}

	key_pressed_code();
	con_set_color(CON_CLR_WHITE_LIGHT, CON_CLR_BLUE);
	clrscr();
	gotoxy(53, 2);
	printf("ГЛАВНОЕ МЕНЮ");
	return;
}
void save_menu();
void demo_animation()
{
	int if_space = 0;
	int x = 0, y = 0;
	int speed = 40;
	int win = 0, sc = 0;
	int skip_dot = 0;
	strokes = 0;
	char fio[1000] = { 0 };


	con_set_color(CON_CLR_RED, CON_CLR_YELLOW);
	clrscr();

	field_draw();
	con_set_color(CON_CLR_BLACK, CON_CLR_YELLOW);
	gotoxy(40, 28);
	printf("Нажмите q для выхода. Нажмите ESC для отмены хода.");
	gotoxy(40, 29);
	printf("Перемещение: стрелки вправо/влево.");

	synchronization_field(field);
	t_start = clock();
	while (1)
	{
	
		if ((win = ifWin_comp(field)) != 0)	break;

		if (human_clr == side){   //ход человека
			if ((sc=skip_checker()) == KEY_SPACE)
			{
				if (side == B) {
					x = bcc_cur->crd.x;
					y = bcc_cur->crd.y;
				}
				else if (side == W)
				{
					x = wcc_cur->crd.x;
					y = wcc_cur->crd.y;
				}
				if (queen_check(x, y, side) == 2)
				{

					count_enemys = search_free_dots_queen();

					if ((sbdc_head) && ((skip_dot = skip_dots()) == KEY_SPACE))
					{

						if ((count_enemys == 0))
						{
							queen_stroke();
						}
						else if (count_enemys)
						{
							war_queen();
							destruct_VCcord();
							count_enemys = 0;
						}
						switch_side();
					}
					else if (skip_dot == KEY_ESC)
					{
						if (side == B)
						{
							gotoxy(bcc_cur->crd.x, bcc_cur->crd.y);
							con_set_color(CON_CLR_BLACK, CON_CLR_YELLOW);
							printf("D");
						}
						else if (side == W)
						{
							gotoxy(wcc_cur->crd.x, wcc_cur->crd.y);
							con_set_color(CON_CLR_WHITE, CON_CLR_YELLOW);
							printf("D");
						}
					}
					destruct_SBDCcord();
					count_enemys = 0;
					continue;
				}

				count_enemys = search_free_dots();

				if (sbdc_head)
				{
					if (skip_dots() == KEY_SPACE) // переключение свободных полей пользователем
					{
						if (count_enemys == 0)
						{
							stroke(); //xoд на выбранную клетку
						}
						else if (count_enemys != 0)
						{
							war();
							destruct_VCcord();
							count_enemys = 0;
						}
						destruct_SBDCcord();

						if (side == B) {
							x = bcc_cur->crd.x;
							y = bcc_cur->crd.y;
						}
						else if (side == W)
						{
							x = wcc_cur->crd.x;
							y = wcc_cur->crd.y;
						}
						if ((q_check = queen_check(x, y, side)))
						{
							if (side == B) {
								bcc_cur->queen = Q;
								x = field_crdX(bcc_cur->crd.x);
								y = field_crdY(bcc_cur->crd.y);
								field[x][y] = BQ;
								con_set_color(CON_CLR_BLACK, CON_CLR_YELLOW);
								gotoxy(bcc_cur->crd.x, bcc_cur->crd.y);
								printf("D");
							}
							else if (side == W) {
								wcc_cur->queen = Q;
								x = field_crdX(wcc_cur->crd.x);
								y = field_crdY(wcc_cur->crd.y);
								field[x][y] = WQ;
								con_set_color(CON_CLR_WHITE, CON_CLR_YELLOW);
								gotoxy(wcc_cur->crd.x, wcc_cur->crd.y);
								printf("D");
							}
						}
						switch_side();
					}
					else
					{
						if (side == B)
						{
							gotoxy(bcc_cur->crd.x, bcc_cur->crd.y);
							con_set_color(CON_CLR_BLACK, CON_CLR_YELLOW);
							printf("O");
						}
						else if (side == W)
						{
							gotoxy(wcc_cur->crd.x, wcc_cur->crd.y);
							con_set_color(CON_CLR_WHITE, CON_CLR_YELLOW);
							printf("O");
						}
					}
				}
			}
			else if (sc == EXIT)
			{
				res_time = clock() - t_start;
				save_menu();
				field_restart();

				con_set_color(CON_CLR_WHITE_LIGHT, CON_CLR_BLUE);
				clrscr();
				gotoxy(53, 2);
				printf("ГЛАВНОЕ МЕНЮ");

				return;
			}
		}
		else if ((comp_clr == side)&&(p_vs_comp))   //ход компа
		{
			computer_stroke();
			switch_side();	
		}
		if (p_vs_p)	human_clr = side;
     }
	t_end = clock();
	con_set_color(CON_CLR_WHITE_LIGHT, CON_CLR_BLUE);
	clrscr();
	while (1)
	{
		gotoxy(50, 12);
		printf_WIN(win);
		gotoxy(50, 17);
		if ((p_vs_comp) && (win==human_clr))
		{
			rt = fopen("rang_table.txt", "a+");
			printf("ВВЕДИТЕ ВАШЕ ИМЯ: ");
			gets(fio);
			fprintf(rt, "%s |Количество ходов=%d |Время партии:%0.4f sec |Сложность:", fio, strokes, ((double)res_time + ((double)t_end - t_start)/((double)CLOCKS_PER_SEC)));
			if (deep == 4)
			{
				fprintf(rt, "%s","Легкий\n");
			}
			else if (deep == 6)
			{
				fprintf(rt, "%s", "Средний\n");
			}
			else if (deep == 8)
			{
				fprintf(rt, "%s", "Тяжелый\n");
			}
			memset(fio, 0, sizeof(char) * 1000);
			fclose(rt);
			p_vs_comp = 0;
			field_restart();
			return;
		}
		else
	    {
			gotoxy(50, 17);
			printf("Нажмите q для выхода");

			if (key_is_pressed())
			{
				int c = key_pressed_code();
				if (c == 'q' || c == 'Q' || c == 'й' || c == 'Й')
				{
					field_restart();
					return;
				}
			}
		}	
	}
}
///MENU:
void save_game()
{
	int i = 0, j = 0;
	char buf[1000];
	const char *alph = "*/ |:<>?/";
	FILE *save_in;

	save_name = fopen("save_name.txt", "a++");
	con_set_color(CON_CLR_WHITE_LIGHT, CON_CLR_BLUE);
	clrscr();
	gotoxy(20, 5);
	printf("ВВЕДИТЕ НАЗВАНИЕ СОХРАНЕНИЯ: ");
	gets(buf);
	while (buf[i] != '\0')
	{
		if (strchr(alph, buf[i]))
		{
			clrscr();
			gotoxy(20, 5);
			con_set_color(CON_CLR_WHITE_LIGHT, CON_CLR_BLUE);
			printf("ИСПОЛЬЗОВАНЫ НЕДОПУСТИМЫЕ СИМВОЛЫ В НАЗВАНИИ ФАЙЛА");
			memset(buf, 0, 1000 * sizeof(char));
			pause(800);
			gotoxy(20, 5);
			clrscr();
			printf("ВВЕДИТЕ НАЗВАНИЕ СОХРАНЕНИЯ: ");
			gets(buf);
			i = 0;
		}
		i++;
	}
	fprintf(save_name, "%s.txt\n", buf);
	fclose(save_name);
	buf[i] = '.';
	buf[++i] = 't';
	buf[++i] = 'x';
	buf[++i] = 't';
	buf[++i] = '\0';

	save_in = fopen(buf, "w");
	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			fprintf(save_in, "%d ", field[j][i]);
		}
	}


	fprintf(save_in, "%d ", human_clr);
	fprintf(save_in, "%d ", comp_clr);
	fprintf(save_in, "%d ", p_vs_p);
	fprintf(save_in, "%d ", p_vs_comp);
	fprintf(save_in, "%d ", strokes);
	fprintf(save_in, "%d ", res_time);
	fclose(save_in);
}
void load_game()
{
	int q;
	FILE *load_name, *load_game;

	short clr_bg = CON_CLR_BLUE;
	short clr_bg_active = CON_CLR_GRAY;
	short clr_font = CON_CLR_WHITE_LIGHT;

	char *buf=NULL;
	char *menu_items[1000];

	int menu_active_idx = 0;
	int menu_items_count;
	int i = 0, b, cnt, j;

	load_name = fopen("save_name.txt", "r");

	if (load_name == NULL)
	{
		con_set_color(CON_CLR_WHITE_LIGHT, CON_CLR_BLUE);
		clrscr();
		gotoxy(53, 2);
		printf("СОХРАНЕНИЯ ОТСУТСТВУЮТ");

		key_pressed_code();
		con_set_color(clr_font, clr_bg);
		clrscr();
		gotoxy(53, 2);
		printf("ГЛАВНОЕ МЕНЮ");
	}
	else
	{
		con_set_color(CON_CLR_WHITE_LIGHT, CON_CLR_BLUE);
		clrscr();
		gotoxy(51, 2);
		printf("ВЫБЕРИТЕ СОХРАНЕНИЕ");
		gotoxy(50, 4);
		i = 0;
		j = 0;
		menu_items[0] = NULL;
		while (1)
		{
			q = fgetc(load_name);
			if (q == EOF) break;
			if (q == '\n')
			{
				menu_items[j] = (char*)realloc(menu_items[j], sizeof(char)*(i + 1));
				menu_items[j][i] = '\0';
				j++;
				menu_items[j] = NULL;
				i = 0;
			}
			else
			{
				menu_items[j] = (char*)realloc(menu_items[j], sizeof(char)*(i + 1));
				menu_items[j][i]  = q;
				i++;
			}
		}
		fclose(load_name);
		cnt = menu_items_count = j;
		while (1)
		{
			int left = 50;
			int top = 5;

			for (b = 0; b < menu_items_count; b++)
			{
				short btn_bg = clr_bg; // По умолчанию фон кнопки - как фон экрана
				if (b == menu_active_idx)
					btn_bg = clr_bg_active; // Если кнопка активна - то рисуется другим цветом

				gotoxy(left, top);
				con_set_color(clr_font, btn_bg);

				top++;
				gotoxy(left, top);

				gotoxy(left + 10 - strlen(menu_items[b]) / 2, top);
				printf("%s", menu_items[b]);

				con_set_color(clr_font, btn_bg);
				gotoxy(left + 19, top);
				top++;
				gotoxy(left, top);
				top += 2;
			}

			con_draw_release();
			while (!key_is_pressed()) // Если пользователь нажимает кнопку
			{
				int code = key_pressed_code();
				if (code == KEY_UP) // Если это стрелка вверх
				{
					// То переход к верхнему пункту (если это возможно)
					if (menu_active_idx > 0)
					{
						menu_active_idx--;
						break;
					}
				}
				else if (code == KEY_DOWN) // Если стрелка вниз
				{
					// То переход к нижнему пункту (если это возможно)
					if (menu_active_idx + 1 < menu_items_count)
					{
						menu_active_idx++;
						break;
					}
				}
				else if (code == KEY_ESC || code == 'q' || code == 'Q' || code == (unsigned char)'й' || code == (unsigned char)'Й') // ESC или 'q' - выход
				{
					con_set_color(clr_font, clr_bg);
					clrscr();
					gotoxy(53, 2);
					printf("ГЛАВНОЕ МЕНЮ");
					return;
				}
				else if (code == KEY_ENTER) // Нажата кнопка Enter
				{
					load_game = fopen(menu_items[menu_active_idx], "r");
					int j;
					for (i = 0; i < 8; i++){
						for (j = 0; j < 8; j++)
						{
							fscanf(load_game, "%d ", &field[j][i]);
						}
					}
					fscanf(load_game, "%d %d %d %d %d %u ", &human_clr, &comp_clr, &p_vs_p, &p_vs_comp,&strokes,&res_time);
					fclose(load_game);
					
					i = 0;
					while (i < cnt)
					{
						free(menu_items[i]);
						menu_items[i] = NULL;
						i++;
					}
					demo_animation();
					return;
				}


			}

		}
	}
}
void save_menu()
{
	const char *menu_items[] = { "Да", "Нет" };

	short clr_bg = CON_CLR_BLUE;
	short clr_bg_active = CON_CLR_GRAY;
	short clr_font = CON_CLR_WHITE_LIGHT;

	int menu_active_idx = 0;
	int menu_items_count = sizeof(menu_items) / sizeof(menu_items[0]);
	int left, top, b = 0;

	con_set_color(clr_font, clr_bg);
	clrscr();
	gotoxy(52, 5);
	printf("СОХРАНИТЬ ИГРУ?");
	while (1)
	{

		left = 49;
		top = 8;

		for (b = 0; b < 2; b++)
		{
			short btn_bg = clr_bg; // По умолчанию фон кнопки - как фон экрана
			if (b == menu_active_idx)
				btn_bg = clr_bg_active; // Если кнопка активна - то рисуется другим цветом

			gotoxy(left, top);
			con_set_color(clr_font, btn_bg);

			printf("********************");
			top++;
			gotoxy(left, top);
			printf("*                   ");

			gotoxy(left + 10 - strlen(menu_items[b]) / 2, top);
			printf("%s", menu_items[b]);

			con_set_color(clr_font, btn_bg);
			gotoxy(left + 19, top);
			printf("*");
			top++;
			gotoxy(left, top);
			printf("********************");
			top += 2;
		}
		con_draw_release();
		while (!key_is_pressed()) // Если пользователь нажимает кнопку
		{
			int code = key_pressed_code();
			if (code == KEY_UP) // Если это стрелка вверх
			{
				// То переход к верхнему пункту (если это возможно)
				if (menu_active_idx > 0)
				{
					menu_active_idx--;
					break;
				}
			}
			else if (code == KEY_DOWN) // Если стрелка вниз
			{
				// То переход к нижнему пункту (если это возможно)
				if (menu_active_idx + 1 < menu_items_count)
				{
					menu_active_idx++;
					break;
				}
			}
			else if (code == KEY_ESC || code == 'q' || code == 'Q' ||
				code == (unsigned char)'й' || code == (unsigned char)'Й') // ESC или 'q' - выход
			{
				con_set_color(clr_font, clr_bg);
				clrscr();
				return;
			}
			else if (code == KEY_ENTER)
			{
				if (menu_active_idx == 0)
				{
					save_game();

					con_set_color(clr_font, clr_bg);
					clrscr();
					gotoxy(53, 2);
					printf("ГЛАВНОЕ МЕНЮ");

					return;
				}
				else if (menu_active_idx == 1)
				{
					con_set_color(clr_font, clr_bg);
					clrscr();
					gotoxy(53, 2);
					printf("ГЛАВНОЕ МЕНЮ");

					return;
				}
			}

		}

	}
}
//////////////////////
void choose_clr_menu()
{
	const char *menu_items[] = { "Белый", "Черный","Назад" };

	short clr_bg = CON_CLR_BLUE;
	short clr_bg_active = CON_CLR_GRAY;
	short clr_font = CON_CLR_WHITE_LIGHT;

	int menu_active_idx = 0;
	int menu_items_count = sizeof(menu_items) / sizeof(menu_items[0]);
	int left, top, b = 0;

	clrscr();
	gotoxy(55, 5);
	printf("ВАШ ЦВЕТ");
	while (1)
	{
		
		left = 49;
		top = 8;

		for (b = 0; b < 3; b++)
		{
			short btn_bg = clr_bg; // По умолчанию фон кнопки - как фон экрана
			if (b == menu_active_idx)
				btn_bg = clr_bg_active; // Если кнопка активна - то рисуется другим цветом

			gotoxy(left, top);
			con_set_color(clr_font, btn_bg);

			printf("********************");
			top++;
			gotoxy(left, top);
			printf("*                   ");

			gotoxy(left + 10 - strlen(menu_items[b]) / 2, top);
			printf("%s", menu_items[b]);

			con_set_color(clr_font, btn_bg);
			gotoxy(left + 19, top);
			printf("*");
			top++;
			gotoxy(left, top);
			printf("********************");
			top += 2;
		}
		con_draw_release();
		while (!key_is_pressed()) // Если пользователь нажимает кнопку
		{
			int code = key_pressed_code();
			if (code == KEY_UP) // Если это стрелка вверх
			{
				// То переход к верхнему пункту (если это возможно)
				if (menu_active_idx > 0)
				{
					menu_active_idx--;
					break;
				}
			}
			else if (code == KEY_DOWN) // Если стрелка вниз
			{
				// То переход к нижнему пункту (если это возможно)
				if (menu_active_idx + 1 < menu_items_count)
				{
					menu_active_idx++;
					break;
				}
			}
			else if (code == KEY_ESC || code == 'q' || code == 'Q' ||
				code == (unsigned char)'й' || code == (unsigned char)'Й') // ESC или 'q' - выход
			{
				con_set_color(clr_font, clr_bg);
				clrscr();
				return;
			}
			else if (code == KEY_ENTER)
			{
				if (menu_active_idx == 0)
				{
					human_clr = W;
					comp_clr = B;
					demo_animation();
					return;
				}
				else if (menu_active_idx == 1)
				{
					human_clr = B;
					comp_clr = W;
					demo_animation();
					return;
				}
				else if (menu_active_idx == 2)
				{
					con_set_color(clr_font, clr_bg);
					clrscr();
					gotoxy(52, 5);
					printf("ВЫБОР СЛОЖНОСТИ");
					return;
				}	
			}

		}

	}
}
void demo_input()
{
	con_set_color(CON_CLR_WHITE_LIGHT, CON_CLR_GRAY);
	clrscr();

	while (1)
	{
		char buf[1024] = { 0 };
		int digit = -1;

		gotoxy(0, 1);
		printf("Введите число ('q' - выход):");

		gotoxy(0, 2);
		show_cursor(1);
		scanf_s("%s", buf, _countof(buf));
		show_cursor(0);

		// Весь остальной пользовательский ввод съедается
		while (key_is_pressed())
			key_pressed_code();

		if (!strcmp(buf, "q"))
			return;

		digit = atoi(buf);

		clrscr();
		gotoxy(0, 0);
		printf("Вы ввели: как число: %d (0x%x), как строка: '%s'\n", digit, digit, buf);
	}
}
void complexity_menu()
{
	const char *menu_items[] = { "Высокий", "Средний", "Низкий" ,"Назад"};

	short clr_bg = CON_CLR_BLUE;
	short clr_bg_active = CON_CLR_GRAY;
	short clr_font = CON_CLR_WHITE_LIGHT;

	int menu_active_idx = 0;
	int menu_items_count = sizeof(menu_items) / sizeof(menu_items[0]);
	int left, top, b = 0;

	clrscr();
	gotoxy(52, 5);
	printf("ВЫБОР СЛОЖНОСТИ");
	while (1)
	{
		left = 49;
		top = 8;

		for (b = 0; b < 4; b++)
		{
			short btn_bg = clr_bg; // По умолчанию фон кнопки - как фон экрана
			if (b == menu_active_idx)
				btn_bg = clr_bg_active; // Если кнопка активна - то рисуется другим цветом

			gotoxy(left, top);
			con_set_color(clr_font, btn_bg);

			printf("********************");
			top++;
			gotoxy(left, top);
			printf("*                   ");

			gotoxy(left + 10 - strlen(menu_items[b]) / 2, top);
			printf("%s", menu_items[b]);

			con_set_color(clr_font, btn_bg);
			gotoxy(left + 19, top);
			printf("*");
			top++;
			gotoxy(left, top);
			printf("********************");
			top += 2;
		}
		con_draw_release();
		while (!key_is_pressed()) // Если пользователь нажимает кнопку
		{
			int code = key_pressed_code();
			if (code == KEY_UP) // Если это стрелка вверх
			{
				// То переход к верхнему пункту (если это возможно)
				if (menu_active_idx > 0)
				{
					menu_active_idx--;
					break;
				}
			}
			else if (code == KEY_DOWN) // Если стрелка вниз
			{
				// То переход к нижнему пункту (если это возможно)
				if (menu_active_idx + 1 < menu_items_count)
				{
					menu_active_idx++;
					break;
				}
			}
			else if (code == KEY_ESC || code == 'q' || code == 'Q' ||
				code == (unsigned char)'й' || code == (unsigned char)'Й') // ESC или 'q' - выход
			{
				con_set_color(clr_font, clr_bg);
				clrscr();
				return;
			}
			else if (code == KEY_ENTER)
			{
				if (menu_active_idx == 0)
				{
					deep = 8;
					choose_clr_menu();
					return;
				}
				else if (menu_active_idx == 1)
				{
					deep = 6;
					choose_clr_menu();
					return;
				}
				else if (menu_active_idx == 2)
				{
					deep = 4;
					choose_clr_menu();
					return;
				}
				else if (menu_active_idx == 3)
				{
					con_set_color(clr_font, clr_bg);
					clrscr();
					gotoxy(50, 5);
					printf("ВЫБОР РЕЖИМА ИГРЫ");
					return;
				}

			}

		}
	}
}
void play_menu()
{
	const char *menu_items[] = { "Игрок vs Игрок", "Игрок vs Компьютер", "Назад" };
	
	short clr_bg = CON_CLR_BLUE;
	short clr_bg_active = CON_CLR_GRAY;
	short clr_font = CON_CLR_WHITE_LIGHT;

	int menu_active_idx = 0;
	int menu_items_count = sizeof(menu_items) / sizeof(menu_items[0]);
	int left, top, b = 0;

	clrscr();
	gotoxy(50, 5);
	printf("ВЫБОР РЕЖИМА ИГРЫ");
	while (1)
	{
		left = 49;
		top = 8;

		for (b = 0; b < 3; b++)
		{
			short btn_bg = clr_bg; // По умолчанию фон кнопки - как фон экрана
			if (b == menu_active_idx)
				btn_bg = clr_bg_active; // Если кнопка активна - то рисуется другим цветом

			gotoxy(left, top);
			con_set_color(clr_font, btn_bg);

			printf("********************");
			top++;
			gotoxy(left, top);
			printf("*                   ");

			gotoxy(left + 10 - strlen(menu_items[b]) / 2, top);
			printf("%s", menu_items[b]);

			con_set_color(clr_font, btn_bg);
			gotoxy(left + 19, top);
			printf("*");
			top++;
			gotoxy(left, top);
			printf("********************");
			top += 2;
		}
		con_draw_release();
		while (!key_is_pressed()) // Если пользователь нажимает кнопку
		{
			int code = key_pressed_code();
			if (code == KEY_UP) // Если это стрелка вверх
			{
				// То переход к верхнему пункту (если это возможно)
				if (menu_active_idx > 0)
				{
					menu_active_idx--;
					break;
				}
			}
			else if (code == KEY_DOWN) // Если стрелка вниз
			{
				// То переход к нижнему пункту (если это возможно)
				if (menu_active_idx + 1 < menu_items_count)
				{
					menu_active_idx++;
					break;
				}
			}
			else if (code == KEY_ESC || code == 'q' || code == 'Q' ||
				code == (unsigned char)'й' || code == (unsigned char)'Й') // ESC или 'q' - выход
			{
				con_set_color(clr_font, clr_bg);
				clrscr();
				return;
			}
			else if (code == KEY_ENTER)
			{
				if (menu_active_idx == 0)
				{
					p_vs_p = 1;
					p_vs_comp = 0;
					human_clr = W;
					demo_animation();

					con_set_color(clr_font, clr_bg);
					clrscr();
					gotoxy(53, 2);
					printf("ГЛАВНОЕ МЕНЮ");
					return;

				}
				else if (menu_active_idx == 1)
				{
					p_vs_p = 0;
					p_vs_comp = 1;
					complexity_menu();
					break;

				}
				else if (menu_active_idx == 2)
				{
					con_set_color(clr_font, clr_bg);
					clrscr();
					gotoxy(53, 2);
					printf("ГЛАВНОЕ МЕНЮ");
					return;
				}
	
			}

		}
	}

}
void main_menu()
{
	const char* menu_items[] = { "Играть", "Загрузить игру", "Справка", "О программе", "Таблица рекордов", "Выход" };

	int menu_active_idx = 0;
	int menu_items_count = sizeof(menu_items) / sizeof(menu_items[0]);

	short clr_bg = CON_CLR_BLUE;
	short clr_bg_active = CON_CLR_GRAY;
	short clr_font = CON_CLR_WHITE_LIGHT;

	con_set_color(clr_font, clr_bg);
	clrscr();
	gotoxy(53, 2);
	printf("ГЛАВНОЕ МЕНЮ");

	while (1)
	{
		int left = 50;
		int top = 5;
		int b;

		// Заблокировать отрисовку
		con_draw_lock();

		// Очистка экрана
		con_set_color(clr_font, clr_bg);
		//clrscr();

		// Цикл отрисовывает кнопки
		for (b = 0; b < menu_items_count; b++)
		{
			short btn_bg = clr_bg; // По умолчанию фон кнопки - как фон экрана
			if (b == menu_active_idx)
				btn_bg = clr_bg_active; // Если кнопка активна - то рисуется другим цветом

			gotoxy(left, top);
			con_set_color(clr_font, btn_bg);

			printf("********************");
			top++;
			gotoxy(left, top);
			printf("*                   ");

			gotoxy(left + 10 - strlen(menu_items[b]) / 2, top);
			printf("%s", menu_items[b]);

			con_set_color(clr_font, btn_bg);
			gotoxy(left + 19, top);
			printf("*");
			top++;
			gotoxy(left, top);
			printf("********************");
			top += 2;
		}

		// Данные подготовлены, вывести на экран
		con_draw_release();


		while (!key_is_pressed()) // Если пользователь нажимает кнопку
		{
			int code = key_pressed_code();
			if (code == KEY_UP) // Если это стрелка вверх
			{
				// То переход к верхнему пункту (если это возможно)
				if (menu_active_idx > 0)
				{
					menu_active_idx--;
					break;
				}
			}
			else if (code == KEY_DOWN) // Если стрелка вниз
			{
				// То переход к нижнему пункту (если это возможно)
				if (menu_active_idx + 1 < menu_items_count)
				{
					menu_active_idx++;
					break;
				}
			}
			else if (code == KEY_ESC || code == 'q' || code == 'Q' ||
				code == (unsigned char)'й' || code == (unsigned char)'Й') // ESC или 'q' - выход
			{
				return;
			}
			else if (code == KEY_ENTER) // Нажата кнопка Enter
			{
				if (menu_active_idx == menu_items_count - 1) // Выбран последний пункт - это выход
					return;

				if (menu_active_idx == 0) play_menu();
							
				if (menu_active_idx == 1) load_game();

				if (menu_active_idx == 2) inquiry();

				if (menu_active_idx == 3)
					about();
				if (menu_active_idx == 4) rang_table();

				if (menu_active_idx == 5) return;

				break;
			}


			pause(40); // Небольшая пауза (чтобы не загружать процессор)
		} // while (!key_is_pressed())


		// "Съедается" оставшийся ввод
		while (key_is_pressed())
			key_pressed_code();

	} // while(1)
}
//////////////////////////////
int main()
{
	con_init(80, 25);
	show_cursor(0);
	setlocale(LC_ALL, "RUS");

	main_menu();

	destruct_VCcord();
	destruct_BCcord();
	destruct_FCcord();
	destruct_WCcord();
	destruct_BDcord();
	destruct_SBDCcord();
	return 0;
}






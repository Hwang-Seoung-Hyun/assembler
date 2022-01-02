/*
 * 화일명 : my_assembler_20180637.c
 * 설  명 : 이 프로그램은 SIC/XE 머신을 위한 간단한 Assembler 프로그램의 메인루틴으로,
 * 입력된 파일의 코드 중, 명령어에 해당하는 OPCODE를 찾아 출력한다.
 * 파일 내에서 사용되는 문자열 "00000000"에는 자신의 학번을 기입한다.
 */

 /*
  *
  * 프로그램의 헤더를 정의한다.
  *
  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <ctype.h>

  // 파일명의 "00000000"은 자신의 학번으로 변경할 것.
#include "my_assembler_20180637.h"

/* ----------------------------------------------------------------------------------
 * 설명 : 사용자로 부터 어셈블리 파일을 받아서 명령어의 OPCODE를 찾아 출력한다.
 * 매계 : 실행 파일, 어셈블리 파일
 * 반환 : 성공 = 0, 실패 = < 0
 * 주의 : 현재 어셈블리 프로그램의 리스트 파일을 생성하는 루틴은 만들지 않았다.
 *		   또한 중간파일을 생성하지 않는다.
 * ----------------------------------------------------------------------------------
 */
int main(int args, char *arg[])
{
	if (init_my_assembler() < 0)
	{
		printf("init_my_assembler: 프로그램 초기화에 실패 했습니다.\n");
		return -1;
	}

	if (assem_pass1() < 0)
	{
		printf("assem_pass1: 패스1 과정에서 실패하였습니다.  \n");
		return -1;
	}
	// make_opcode_output("output_00000000");

	make_symtab_output(/*"symtab_20180637"*/NULL);
	make_literaltab_output(/*"literaltab_20180637"*/NULL);
	if (assem_pass2() < 0)
	{
		printf(" assem_pass2: 패스2 과정에서 실패하였습니다.  \n");
		return -1;
	}

	make_objectcode_output("output_20180637");

	return 0;
}

/* ----------------------------------------------------------------------------------
 * 설명 : 프로그램 초기화를 위한 자료구조 생성 및 파일을 읽는 함수이다.
 * 매계 : 없음
 * 반환 : 정상종료 = 0 , 에러 발생 = -1
 * 주의 : 각각의 명령어 테이블을 내부에 선언하지 않고 관리를 용이하게 하기
 *		   위해서 파일 단위로 관리하여 프로그램 초기화를 통해 정보를 읽어 올 수 있도록
 *		   구현하였다.
 * ----------------------------------------------------------------------------------
 */
int init_my_assembler(void)
{
	int result;

	if ((result = init_inst_file("inst.data")) < 0)
		return -1;
	if ((result = init_input_file("input.txt")) < 0)
		return -1;
	return result;
}

/* ----------------------------------------------------------------------------------
 * 설명 : 머신을 위한 기계 코드목록 파일을 읽어 기계어 목록 테이블(inst_table)을
 *        생성하는 함수이다.
 * 매계 : 기계어 목록 파일
 * 반환 : 정상종료 = 0 , 에러 < 0
 * 주의 : 기계어 목록파일 형식은 자유롭게 구현한다. 예시는 다음과 같다.
 *
 *	===============================================================================
 *		   | 이름 | 형식 | 기계어 코드 | 오퍼랜드의 갯수 | NULL|
 *	===============================================================================
 *
 * ----------------------------------------------------------------------------------
 */
int init_inst_file(char *inst_file)
{
	FILE *file;
	int errno;
	char buf[20];
	/* add your code here */
	file = fopen(inst_file, "r");
	inst_index = 0;
	while (!feof(file)) {
		fgets(buf, 20, file);
		inst_table[inst_index] = malloc(sizeof(inst));
		memset(inst_table[inst_index]->str, '\0', sizeof(inst_table[inst_index]->str));

		int i;
		unsigned char temp[5] = { '0', };
		int start = 0;
		///명령어 저장
		for (i = start; buf[i] != '|'; i++) {
			inst_table[inst_index]->str[i - start] = buf[i];
		}
		inst_table[inst_index]->str[i - start] = '\0';
		++i;
		///format 저장
		for (; buf[i] != '|'; i++)
			inst_table[inst_index]->format = (int)(buf[i] - 48);
		i++;
		///opcode 저장
		start = i;
		for (; buf[i] != '|'; i++) {
			temp[i - start] = buf[i];
		}
		inst_table[inst_index]->op = 0;
		/////////////////////////////
		///////16^1의자리 bit////////
		/////////////////////////////
		if ('A' <= temp[0] && temp[0] <= 'F') {
			inst_table[inst_index]->op += (unsigned char)(temp[0] - 55) * 16;
		}
		else if ('0' <= temp[0] && temp[0] <= '9')
			inst_table[inst_index]->op += (unsigned char)(temp[0] - 48) * 16;
		else
			errno = -1;
		/////////////////////////////
		///////16^1의자리 bit////////
		/////////////////////////////
		/////////////////////////////
		///////16^0의자리 bit////////
		/////////////////////////////
		if ('A' <= temp[1] && temp[1] <= 'F') {
			inst_table[inst_index]->op += (temp[1] - 55);
		}
		else if ('0' <= temp[1] && temp[1] <= '9')
			inst_table[inst_index]->op += (temp[1] - 48);
		else
			errno = -1;
		/////////////////////////////
		///////16^1의자리 bit////////
		/////////////////////////////
		++i;
		//operand개수 저장
		for (; buf[i] != '|'; i++)
			inst_table[inst_index]->ops = (int)(buf[i] - 48);
		++inst_index;
	}
	fclose(file);
	return errno;
}

/* ----------------------------------------------------------------------------------
 * 설명 : 어셈블리 할 소스코드를 읽어 소스코드 테이블(input_data)를 생성하는 함수이다.
 * 매계 : 어셈블리할 소스파일명
 * 반환 : 정상종료 = 0 , 에러 < 0
 * 주의 : 라인단위로 저장한다.
 *
 * ----------------------------------------------------------------------------------
 */
int init_input_file(char *input_file)
{
	FILE *file;
	int errno;

	/* add your code here */
	file = fopen(input_file, "r");
	line_num = 0;
	while (!feof(file)) {//eof전까지 read
		input_data[line_num] = malloc(sizeof(char) * 170);
		fgets(input_data[line_num], 170, file);
		++line_num;
	}
	fclose(file);
	return errno;
}

/* ----------------------------------------------------------------------------------
 * 설명 : 소스 코드를 읽어와 토큰단위로 분석하고 토큰 테이블을 작성하는 함수이다.
 *        패스 1로 부터 호출된다.
 * 매계 : 파싱을 원하는 문자열
 * 반환 : 정상종료 = 0 , 에러 < 0
 * 주의 : my_assembler 프로그램에서는 라인단위로 토큰 및 오브젝트 관리를 하고 있다.
 * ----------------------------------------------------------------------------------
 */
int token_parsing(char *str)
{
	/* add your code here */
	int label_size;
	int operator_size;
	int operand_num = 0;
	int strid = 0;//str index
	int i = 0;
	int operand_size[MAX_OPERAND] = { 0 };
	bool activeN = true;
	bool activeI = true;
	bool activeX = false;
	bool activeB = false;
	bool activeP = false;
	bool activeE = false;
	if (str[0] == '.') {//asemmbliy의 주석
		i = 1;
		label_size = 1;
		if (str[i++] == '\n') {
			operator_size = 0;
		}
		else {
			for (; str[i] != '\n'; i++);
			operator_size = i - (label_size + 1);
		}
	}
	else if (str[0] != '\t') {//label이 있는 경우
		for (i = 0; str[i] != '\t'; i++);

		label_size = i++;

		for (; str[i] != '\t'&& str[i] != '\n'; i++);
		operator_size = i - (label_size + 1);
	}
	else {//label이 없는 경우
		label_size = i++;
		for (; str[i] != '\t'&& str[i] != '\n'; i++);
		operator_size = i - (label_size + 1);
	}
	/////////////////////////////////////////////////////////////
	/*********token_table포인터 메모리 할당및 초기화************/
	/////////////////////////////////////////////////////////////
	token_table[token_line] = malloc(sizeof(char) * 162 + sizeof(char)*(label_size + 1) + sizeof(char)*(operator_size + 1));
	token_table[token_line]->label = malloc(sizeof(char)*(label_size + 1));
	token_table[token_line]->operator = malloc(sizeof(char)*operator_size + 1);
	memset(token_table[token_line]->label, '\0', sizeof(char)*(label_size + 1));
	memset(token_table[token_line]->operator, '\0', sizeof(char)*operator_size + 1);
	for (int k = 0; k < 3; k++)
		memset(token_table[token_line]->operand[k], '\0', 20);
	memset(token_table[token_line]->comment, '\0', sizeof(token_table[token_line]->comment));
	token_table[token_line]->nixbpe = 0;
	/////////////////////////////////////////////////////////////
	/************token_table LABEL,OPERATOR 할당****************/
	/////////////////////////////////////////////////////////////
	for (int k = 0; k < label_size; k++)
		token_table[token_line]->label[k] = str[k];
	token_table[token_line]->label[label_size] = '\0';
	for (int k = i - operator_size; k < i; k++)
		token_table[token_line]->operator[k - (i - operator_size)] = str[k];
	token_table[token_line]->operator[operator_size] = '\0';
	i++;
	/////////////////////////////////////////////////////////////
	/****************token_table OPERAND 할당*******************/
	/////////////////////////////////////////////////////////////
	if (str[0] != '.') {
		if (str[i] != '\0') {
			for (; str[i] != '\0'&&str[i] != '\t'&&str[i] != '\n'; i++) {
				if (str[i] == ',') {
					token_table[token_line]->operand[operand_num][strid] = '\0';
					++operand_num;
					strid = 0;
					continue;
				}
				if (operand_num >= 3) {
					fprintf(stderr, "too many operand: %d line\n", token_line);
					errno = -1;
				}
				token_table[token_line]->operand[operand_num][strid++] = str[i];
			}
			if (operand_num > 0)
				if (strcmp(token_table[token_line]->operand[1], "X") == 0) {

					if (strlen(token_table[token_line]->operand[0]) != 1)
						activeX = true;
				}
			/////////////////////////////////////////////////////////////
			/****************token_table comment 할당*******************/
			/////////////////////////////////////////////////////////////

		}
		strid = 0;
		if (str[i++] != '\0')
			for (; str[i] != '\0'&&str[i] != '\n'; i++) {
				token_table[token_line]->comment[strid++] = str[i];
				if (strid >= 100) {
					fprintf(stderr, "too long comment: %d line\n", token_line);
					errno = -1;
					break;
				}
			}


	}
	///////////////////////////////////////////////////////////////
	///////////////token_table nixbpe 할당/////////////////////////
	///////////////////////////////////////////////////////////////
	token_table[token_line]->nixbpe = 0b110000;
	if (token_table[token_line]->operand[0][0] == '@') {//n
		token_table[token_line]->nixbpe -= (1 << 4);
		activeI = false;
	}
	if (token_table[token_line]->operand[0][0] == '#') {//i
		token_table[token_line]->nixbpe -= (1 << 5);
		activeN = false;
	}
	if (activeX)//x
		token_table[token_line]->nixbpe += (1 << 3);
	char nixbpe = token_table[token_line]->nixbpe;
	if (token_table[token_line]->operator[0] == '+') {//e
		token_table[token_line]->nixbpe += 1;
		activeE = true;
	}
	else if (!activeN && activeI) {//n=0, i=1
		if (isalpha(token_table[token_line]->operand[0][1]))//p
			token_table[token_line]->nixbpe += (1 << 1);

	}
	else {//n=1
		if (token_table[token_line]->operand[0][0] != '\0')
			token_table[token_line]->nixbpe += (1 << 1);

	}


	++token_line;
}

/* ----------------------------------------------------------------------------------
 * 설명 : 입력 문자열이 기계어 코드인지를 검사하는 함수이다.
 * 매계 : 토큰 단위로 구분된 문자열
 * 반환 : 정상종료 = 기계어 테이블 인덱스, 에러 < 0
 * 주의 :
 *
 * ----------------------------------------------------------------------------------
 */
int search_opcode(char *str)
{
	/* add your code here */
	inst_index = 0;
	if (str[0] == '+')//4형식 명령어 59번째부터 table에 따로 정령
		inst_index = 59;

	for (; inst_index <= 100; inst_index++) {
		if (strcmp(inst_table[inst_index]->str, str) == 0)
			break;
	}
	if (inst_index == 101)
		inst_index = -1;
	return inst_index;
}

/* ----------------------------------------------------------------------------------
* 설명 : 어셈블리 코드를 위한 패스1과정을 수행하는 함수이다.
*		   패스1에서는..
*		   1. 프로그램 소스를 스캔하여 해당하는 토큰단위로 분리하여 프로그램 라인별 토큰
*		   테이블을 생성한다.
*
* 매계 : 없음
* 반환 : 정상 종료 = 0 , 에러 = < 0
* 주의 : 현재 초기 버전에서는 에러에 대한 검사를 하지 않고 넘어간 상태이다.
*	  따라서 에러에 대한 검사 루틴을 추가해야 한다.
*
* -----------------------------------------------------------------------------------
*/
static int assem_pass1(void)
{
	/* add your code here */
	int startadress = 0;
	int symline = 0;
	//int symfid = 0;//현재 section의 첫번째 simbol_table index
	int litline = 0;
	int litid = 0;//litfid의 index
	int opid;
	int id = 0;
	int prog_num = 0;
	locctr = 0;
	char buf[100];
	token_line = 0;
	token_parsing(input_data[0]);
	if (strcmp(token_table[0]->operator,"START") == 0) {
		startadress = atoi(token_table[0]->operand[0]);
		locctr = startadress;
		sym_table[symline].symbol = token_table[0]->label;
		sym_table[symline].addr = locctr;
		symline++;
		token_parsing(input_data[++id]);
	}
	for (; strcmp(token_table[id]->operator,"END") != 0; id++) {
		if (strcmp(token_table[id]->label, ".") != 0) {//if not commend line
			if (strcmp(token_table[id]->operator, "CSECT") == 0) {
				prog_length[prog_num++] = locctr - startadress;
				symfid[prog_num] = symline;
				locctr = 0;
			}
			//////////////////////////////////////
			///////////symbol table///////////////
			//////////////////////////////////////
			if (token_table[id]->label[0] != '\0') {
				bool exist = false;
				for (int i = symfid[prog_num]; i < symline; i++) {
					if (strcmp(sym_table[i].symbol, token_table[id]->label) == 0)
						exist = true;
				}
				if (exist) {
					fprintf(stderr, "twice define symbol: %s\n", token_table[id]->label);
					errno = -1;
					//return errno;
				}
				else {
					sym_table[symline].symbol = token_table[id]->label;
					sym_table[symline++].addr = locctr;
				}
			}
			/////////////////////////////////////
			/////////////literal table/////////////
			/////////////////////////////////////
			if (token_table[id]->operand[0][0] == '=') {
				bool exist = false;
				for (int i = litfid[litid]; i < litline; i++) {
					if (strcmp(literal_table[i].literal, token_table[id]->operand[0]) == 0)
						exist = true;
				}
				if (!exist) {
					literal_table[litline++].literal = token_table[id]->operand[0];
				}
			}

			///////////////////////////////////////
			////////////////EXTDEF/////////////////
			///////////////////////////////////////
			if (strcmp(token_table[id]->operator,"EXTDEF") == 0) {
				int length = 0;
				for (int i = 0; i < MAX_OPERAND; i++) {
					if (token_table[id]->operand[i][0] == '\0')
						break;
					for (int k = 0; token_table[id]->operand[i][k] != '\0'; k++)
						length++;
				}
				extdef[prog_num] = malloc(sizeof(char)*(length)+3);
				memset(extdef[prog_num], '\0', sizeof(char)*(length)+3);
				length = 0;
				for (int i = 0; i < MAX_OPERAND; i++) {
					if (token_table[id]->operand[i][0] == '\0')
						break;
					for (int k = 0; token_table[id]->operand[i][k] != '\0'; k++)
						extdef[prog_num][length++] = token_table[id]->operand[i][k];
					extdef[prog_num][length++] = ',';
				}
				extdef[prog_num][length] = '\0';
			}
			///////////////////////////////////////
			////////////////EXTREF/////////////////
			///////////////////////////////////////
			else if (strcmp(token_table[id]->operator,"EXTREF") == 0) {
				int length = 0;
				for (int i = 0; i < MAX_OPERAND; i++) {
					if (token_table[id]->operand[i][0] == '\0')
						break;
					for (int k = 0; token_table[id]->operand[i][k] != '\0'; k++)
						length++;
				}
				extref[prog_num] = malloc(sizeof(char)*(length)+4);
				memset(extref[prog_num], '\0', sizeof(char)*(length)+4);
				length = 0;
				for (int i = 0; i < MAX_OPERAND; i++) {
					if (token_table[id]->operand[i][0] == '\0')
						break;
					for (int k = 0; token_table[id]->operand[i][k] != '\0'; k++)
						extref[prog_num][length++] = token_table[id]->operand[i][k];
					extref[prog_num][length++] = ',';
				}
				extref[prog_num][length] = '\0';
			}
			///////////////////////////////////////////////////////
			else if (strcmp(token_table[id]->operator,"RESW") == 0) {

				locctr += 3 * atoi(token_table[id]->operand[0]);
			}
			else if (strcmp(token_table[id]->operator,"RESB") == 0) {

				locctr += atoi(token_table[id]->operand[0]);
			}
			else if (strcmp(token_table[id]->operator,"EQU") == 0) {
				if (strcmp(token_table[id]->operand[0], "*") != 0) {
					char*ptr;

					////////////////////////////////////////
					//////////////'-'caculation/////////////
					////////////////////////////////////////
					ptr = strchr(token_table[id]->operand[0], '-');
					if (ptr != NULL) {

						int operand1 = -1, operand2 = -1;
						ptr = strtok(token_table[id]->operand[0], "-");

						for (int i = symfid[prog_num]; i < symline; i++) {
							if (strcmp(sym_table[i].symbol, ptr) == 0)
								operand1 = sym_table[i].addr;
						}
						if (operand1 == -1) {
							fprintf(stderr, "EQU error: not defime symbol %s", ptr);
							errno = -1;
						}

						ptr = strtok(NULL, "\0");
						for (int i = symfid[prog_num]; i < symline; i++) {
							if (strcmp(sym_table[i].symbol, ptr) == 0)
								operand2 = sym_table[i].addr;
						}
						if (operand2 == -1) {
							errno = -1;
							fprintf(stderr, "EQU error: not defime symbol %s", ptr);
						}
						sym_table[symline - 1].addr = operand1 - operand2;

					}
					////////////////////////////////////////
					//////////////'+'caculation/////////////
					////////////////////////////////////////
					ptr = strchr(token_table[id]->operand[0], '+');
					if (ptr != NULL) {

						int operand1 = -1, operand2 = -1;
						ptr = strtok(token_table[id]->operand[0], "+");

						for (int i = symfid[prog_num]; i < symline; i++) {
							if (strcmp(sym_table[i].symbol, ptr) == 0)
								operand1 = sym_table[i].addr;
						}
						if (operand1 == -1)
							errno = -1;

						ptr = strtok(NULL, "\0");
						for (int i = symfid[prog_num]; i < symline; i++) {
							if (strcmp(sym_table[i].symbol, ptr) == 0)
								operand2 = sym_table[i].addr;
						}
						if (operand2 == -1)
							errno = -1;
						sym_table[symline - 1].addr = operand1 - operand2;

					}

				}//if not *

			}//if EQU
			else if (strcmp(token_table[id]->operator,"WORD") == 0) {
				locctr += 3;
			}
			else if (strcmp(token_table[id]->operator,"BYTE") == 0) {

				int length = 0;
				for (; token_table[id]->operand[0][length] != '\0'; length++);
				length -= 3;
				if (token_table[id]->operand[0][0] == 'X')
					locctr += (length + 1) % 2;
				else if (token_table[id]->operand[0][0] == 'C')
					locctr += length;
			}
			else if (strcmp(token_table[id]->operator, "LTORG") == 0) {
				for (int i = litfid[litid]; i < litline; i++) {
					literal_table[i].addr = locctr;
					int length = 0;
					for (; literal_table[i].literal[length] != '\0'; length++);
					length -= 4;
					if (literal_table[i].literal[1] == 'C')
						locctr += length;
					else if (literal_table[i].literal[1] == 'X')
						locctr += (length + 1) / 2;
					else
						locctr += 3;
				}
				litfid[++litid] = litline;
			}
			else if (strcmp(token_table[id]->operator, "CSECT") == 0) {

			}
			else {//
				if ((opid = search_opcode(token_table[id]->operator)) == -1) {
					errno = -1;
				}
				else {
					locctr += inst_table[opid]->format;
				}
			}
		}//if (strcmp(token_table[id]->label,".") != 0)
		else {

		}
		if (errno == -1)
			break;
		token_parsing(input_data[id + 1]);
	}//for not end
	////////////////////////////////
	/////flush literal table////////
	////////////////////////////////
	for (int i = litfid[litid]; i < litline; i++) {
		literal_table[i].addr = locctr;
		int length = 0;
		for (; literal_table[i].literal[length] != '\0'; length++);
		length -= 4;
		if (literal_table[i].literal[1] == 'C')
			locctr += length;
		else if (literal_table[i].literal[1] == 'X')
			locctr += (length + 1) / 2;
		else
			locctr += 3;
	}
	prog_length[prog_num] = locctr - startadress;
	/* input_data의 문자열을 한줄씩 입력 받아서
	 * token_parsing()을 호출하여 token_unit에 저장
	 */

	return errno;
}

/* ----------------------------------------------------------------------------------
* 설명 : 입력된 문자열의 이름을 가진 파일에 프로그램의 결과를 저장하는 함수이다.
*        여기서 출력되는 내용은 명령어 옆에 OPCODE가 기록된 표(과제 3번) 이다.
* 매계 : 생성할 오브젝트 파일명
* 반환 : 없음
* 주의 : 만약 인자로 NULL값이 들어온다면 프로그램의 결과를 표준출력으로 보내어
*        화면에 출력해준다.
*        또한 과제 4번에서만 쓰이는 함수이므로 이후의 프로젝트에서는 사용되지 않는다.
* -----------------------------------------------------------------------------------
*/
// void make_opcode_output(char *file_name)
// {
// 	/* add your code here */

// }

/* ----------------------------------------------------------------------------------
* 설명 : 입력된 문자열의 이름을 가진 파일에 프로그램의 결과를 저장하는 함수이다.
*        여기서 출력되는 내용은 SYMBOL별 주소값이 저장된 TABLE이다.
* 매계 : 생성할 오브젝트 파일명
* 반환 : 없음
* 주의 : 만약 인자로 NULL값이 들어온다면 프로그램의 결과를 표준출력으로 보내어
*        화면에 출력해준다.
*
* -----------------------------------------------------------------------------------
*/
void make_symtab_output(char *file_name)
{
	/* add your code here */
	int symline = 0;
	int symid = 0;
	FILE *fp;
	if (file_name == NULL) {
		fp = stdout;
		fprintf(fp, "symtab");
	}
	else
		fp = fopen(file_name, "w");


	for (int i = 0; sym_table[i].symbol != NULL; i++) {
		if (strcmp(sym_table[i].symbol, "FIRST") != 0 && sym_table[i].addr == 0)
			fprintf(fp, "\n");
		fprintf(fp, "%s\t\t%X\n", sym_table[i].symbol, sym_table[i].addr);
	}
}

/* ----------------------------------------------------------------------------------
* 설명 : 입력된 문자열의 이름을 가진 파일에 프로그램의 결과를 저장하는 함수이다.
*        여기서 출력되는 내용은 LITERAL별 주소값이 저장된 TABLE이다.
* 매계 : 생성할 오브젝트 파일명
* 반환 : 없음
* 주의 : 만약 인자로 NULL값이 들어온다면 프로그램의 결과를 표준출력으로 보내어
*        화면에 출력해준다.
*
* -----------------------------------------------------------------------------------
*/
void make_literaltab_output(char *file_name)
{
	/* add your code here */
	FILE *fp;
	if (file_name == NULL) {
		fp = stdout;
		fprintf(fp, "\nliteraltab\n");
	}
	else
		fp = fopen(file_name, "w");
	for (int i = 0; literal_table[i].literal != NULL; i++) {
		char buf[10];
		int length = strlen(literal_table[i].literal);
		
		int loop = 0;
		if (literal_table[i].literal[1] != 'C'&&literal_table[i].literal[1] != 'X') {
			loop = length - 1;
			for (int k = 0; k < loop; k++)
				buf[k] = literal_table[i].literal[k + 1];
		}
		else {
			loop = length - 4;
			for (int k = 0; k < loop; k++)
				buf[k] = literal_table[i].literal[k + 3];
		}
			
		
		buf[loop] = '\0';
		fprintf(fp, "%s\t\t%X\n", buf, literal_table[i].addr);
	}
}

/* ----------------------------------------------------------------------------------
* 설명 : 어셈블리 코드를 기계어 코드로 바꾸기 위한 패스2 과정을 수행하는 함수이다.
*		   패스 2에서는 프로그램을 기계어로 바꾸는 작업은 라인 단위로 수행된다.
*		   다음과 같은 작업이 수행되어 진다.
*		   1. 실제로 해당 어셈블리 명령어를 기계어로 바꾸는 작업을 수행한다.
* 매계 : 없음
* 반환 : 정상종료 = 0, 에러발생 = < 0
* 주의 :
* -----------------------------------------------------------------------------------
*/
static int assem_pass2(void)
{
	/* add your code here */
	int opid = 0;
	int obid = 0;
	int symid = 0;
	int litline = 0;
	int litid = 0;
	int prog_num = 0;
	int mline = 1;
	int obloc = 0;
	///////////////////////////
	/////input first line//////
	///////////////////////////
	if (strcmp(token_table[0]->operator,"START") == 0)
		locctr = atoi(token_table[0]->operand[0]);


	for (token_line = 1; strcmp(token_table[token_line]->operator,"END"); token_line++) {
		if (errno < 0)
			break;
		if (strcmp(token_table[token_line]->label, ".") != 0) {
			if (strcmp(token_table[token_line]->operator,"RESW") == 0) {

				locctr += 3 * atoi(token_table[token_line]->operand[0]);
			}
			else if (strcmp(token_table[token_line]->operator,"RESB") == 0) {

				locctr += atoi(token_table[token_line]->operand[0]);
			}
			else if ((opid = search_opcode(token_table[token_line]->operator)) >= 0) {
				locctr += inst_table[opid]->format;

				//////////////////////////////////////////
				///////////object code generation/////////
				//////////////////////////////////////////
				if (inst_table[opid]->format == 1) {
					ob_table[obid].ob_code = (inst_table[opid]->op / 16);
				}
				/////////////////////////////////////////
				//////////////format 2///////////////////
				/////////////////////////////////////////
				else if (inst_table[opid]->format == 2) {
					ob_table[obid].ob_code += (inst_table[opid]->op) << 8;//! ! _ _
					if (token_table[token_line]->operand[0][0] != '\0') {
						int i;
						for (i = 0; i < 9; i++) {
							if (strcmp(token_table[token_line]->operand[0], register_table[i].name) == 0) {
								ob_table[obid].ob_code += register_table[i].num << 4;
								break;
							}
						}
						if (i == 9) {
							fprintf(stderr, "wrong register name: %s\n", token_table[token_line]->operand[0]);
							errno = -1;
						}
						if (token_table[token_line]->operand[1][0] != '\0') {
							for (i = 0; i < 9; i++) {
								if (strcmp(token_table[token_line]->operand[1], register_table[i].name) == 0) {
									ob_table[obid].ob_code += register_table[i].num;
									break;
								}
							}
							if (i == 9) {
								fprintf(stderr, "wrong register name: %s\n", token_table[token_line]->operand[0]);
								errno = -1;
							}
						}
					}
					ob_table[obid].length = 2;
					obloc += ob_table[obid].length;
				}//if format=2
				/////////////////////////////////////////
				//////////////format 3///////////////////
				/////////////////////////////////////////
				else if (inst_table[opid]->format == 3) {
					int disp = 0;
					ob_table[obid].ob_code += (inst_table[opid]->op / 16) << 20;//! _ _ _ _ _
					ob_table[obid].ob_code += ((inst_table[opid]->op % 16) + (token_table[token_line]->nixbpe >> 4)) << 16;//_ ! _ _ _ _
					ob_table[obid].ob_code += (token_table[token_line]->nixbpe % 16) << 12;//_ _ ! _ _ _
					if (token_table[token_line]->operand[0][0] != '\0') {
						if ((token_table[token_line]->nixbpe >> 4) == 3) {//SIC/XE
							if (token_table[token_line]->operand[0][0] == '=') {
								////////////////////////////////
								////////////literal/////////////
								////////////////////////////////		
								if ((litline = search_literal(token_table[token_line]->operand[0], litfid[litid])) < 0) {
									fprintf(stderr, "non-existent literal : %s\n", token_table[token_line]->operand[0]);
									errno = -1;
								}
								disp = literal_table[litline].addr - locctr;
							}
							/////////////////////////////////////
							else {
								if ((symid = search_symbol(token_table[token_line]->operand[0], symfid[prog_num])) < 0) {

									fprintf(stderr, "non-existent symbol : %s\n", token_table[token_line]->operand[0]);
									errno = -1;

								}
								else {
									disp = sym_table[symid].addr - locctr;

								}
							}

						}
						else if ((token_table[token_line]->nixbpe >> 4) == 2) {//indirect adressing
							if ((symid = search_symbol(token_table[token_line]->operand[0] + 1, symfid[prog_num])) < 0) {

								fprintf(stderr, "non-existent symbol : %s\n", token_table[token_line]->operand[0]);
								errno = -1;

							}
							else {
								disp = sym_table[symid].addr - locctr;

							}
						}
						////////////////////////////////////////
						///////////immediate adressing//////////
						////////////////////////////////////////
						else if ((token_table[token_line]->nixbpe >> 4) == 1) {
							if (isalpha(token_table[token_line]->operand[0][1])) {//operand=symbol
								if ((symid = search_symbol(token_table[token_line]->operand[0] + 1, symfid[prog_num])) < -1) {
									fprintf(stderr, "non-existent symbol : %s\n", token_table[token_line]->operand[0]);
									errno = -1;
								}
								else {
									disp = sym_table[symid].addr - locctr;
								}
							}
							else {
								disp = atoi(token_table[token_line]->operand[0] + 1);
							}

						}
						///////////////////////////////////////
						ob_table[obid].ob_code += disp;
						if (disp < 0)
							ob_table[obid].ob_code += 1 << 12;
					}
					ob_table[obid].length = 3;
					obloc += ob_table[obid].length;
				}//if format=3
				/////////////////////////////////////////
				///////////////format 4//////////////////
				/////////////////////////////////////////
				else if (inst_table[opid]->format == 4) {
					int disp = 0;
					ob_table[obid].ob_code += (inst_table[opid]->op / 16) << 28;//! _ _ _ _ _ _ _
					ob_table[obid].ob_code += ((inst_table[opid]->op % 16) + (token_table[token_line]->nixbpe >> 4)) << 24;//_ ! _ _ _ _ _ _
					ob_table[obid].ob_code += (token_table[token_line]->nixbpe % 16) << 20;//_ _ ! _ _ _ _ _
					if ((symid = search_symbol(token_table[token_line]->operand[0], symfid[prog_num])) < 0) {
						//EXTREF에서 찾기구현
						char temp[30] = { 0 };
						memcpy(temp, extref[prog_num], strlen(extref[prog_num]) + 1);
						char *ptr = strtok(temp, ",");
						while (ptr != NULL) {

							if (strcmp(ptr, token_table[token_line]->operand[0]) == 0)
								break;
							ptr = strtok(NULL, ",");
						}
						if (ptr == NULL) {
							fprintf(stderr, "non-existent symbol : %s\n", token_table[token_line]->operand[0]);
							errno = -1;
						}
						else
							sprintf(m_record[mline++], "M%06X05+%s", obloc + 1, token_table[token_line]->operand[0]);
					}
					else {
						disp = sym_table[symid].addr;
					}
					ob_table[obid].ob_code += disp;
					if (disp < 0)
						ob_table[obid].ob_code += 1 << 20;
					ob_table[obid].length = 4;
					obloc += ob_table[obid].length;
				}
				ob_table[obid].pc = locctr;
				obid++;
			}//if opcode
			else if (strcmp(token_table[token_line]->operator,"WORD") == 0) {
				locctr += 3;
				if (isdigit(token_table[token_line]->operand[0][0]))
					ob_table[obid].ob_code = atoi(token_table[token_line]->operand[0]);
				else {
					char*ptr1, *ptr2, *ptr3;
					////////////////////////////////////////
					//////////////'-'caculation/////////////
					////////////////////////////////////////
					ptr1 = strchr(token_table[token_line]->operand[0], '-');//'-'인지 확인
					if (ptr1 != NULL) {

						int operand1 = -1, operand2 = -1;
						ptr1 = strtok(token_table[token_line]->operand[0], "-");
						ptr3 = strtok(NULL, "\0");
						operand1 = search_symbol(ptr1, symfid[prog_num]);
						if (operand1 == -1) {
							//EXTREF에서 찾기 구현
							char temp[30] = { 0 };
							memcpy(temp, extref[prog_num], strlen(extref[prog_num]) + 1);
							ptr2 = strtok(temp, ",");
							while (ptr2 != NULL) {

								if (strcmp(ptr2, ptr1) == 0)
									break;
								ptr2 = strtok(NULL, ",");
							}
							if (ptr2 == NULL) {
								fprintf(stderr, "non-existent symbol : %s\n", token_table[token_line]->operand[0]);
								errno = -1;
							}
							else
								sprintf(m_record[mline++], "M%06X06+%s", obloc, ptr1);
						}
						operand2 = search_symbol(ptr3, symfid[prog_num]);
						if (operand2 == -1) {
							//EXTREF에서 찾기 구현
							char temp[30] = { 0 };
							memcpy(temp, extref[prog_num], strlen(extref[prog_num]) + 1);
							ptr2 = strtok(temp, ",");
							while (ptr2 != NULL) {

								if (strcmp(ptr2, ptr3) == 0)
									break;
								ptr2 = strtok(NULL, ",");
							}
							if (ptr2 == NULL) {
								fprintf(stderr, "non-existent symbol : %s\n", token_table[token_line]->operand[0]);
								errno = -1;
							}
							else
								sprintf(m_record[mline++], "M%06X06-%s", obloc, ptr3);

						}
						//EXTREF가 아닌 경우는 EQU로 계산
					}
					////////////////////////////////////////
					//////////////'+'caculation/////////////
					////////////////////////////////////////
					ptr1 = strchr(token_table[token_line]->operand[0], '+');//'-'인지 확인
					if (ptr1 != NULL) {

						int operand1 = -1, operand2 = -1;
						ptr1 = strtok(token_table[token_line]->operand[0], "+");
						ptr3 = strtok(NULL, "\0");

						operand1 = search_symbol(ptr1, symfid[prog_num]);
						if (operand1 == -1) {
							//EXTREF에서 찾기 구현
							char temp[30] = { 0 };
							memcpy(temp, extref[prog_num], strlen(extref[prog_num]) + 1);
							ptr2 = strtok(temp, ",");
							while (ptr2 != NULL) {

								if (strcmp(ptr2, ptr1) == 0)
									break;
								ptr2 = strtok(NULL, ",");
							}
							if (ptr2 == NULL) {
								fprintf(stderr, "non-existent symbol : %s\n", token_table[token_line]->operand[0]);
								errno = -1;
							}
							else
								sprintf(m_record[mline++], "M%06X06+%s", obloc, ptr1);
						}


						operand2 = search_symbol(ptr3, symfid[prog_num]);
						if (operand2 == -1) {
							//EXTREF에서 찾기 구현
							char temp[30] = { 0 };
							memcpy(temp, extref[prog_num], strlen(extref[prog_num]) + 1);
							ptr2 = strtok(temp, ",");
							while (ptr2 != NULL) {

								if (strcmp(ptr2, ptr3) == 0)
									break;
								ptr2 = strtok(NULL, ",");
							}
							if (ptr2 == NULL) {
								fprintf(stderr, "non-existent symbol : %s\n", token_table[token_line]->operand[0]);
								errno = -1;
							}
							else
								sprintf(m_record[mline++], "M%06X06+%s", obloc, ptr3);
						}
						//EXTREF가 아닌 경우는 EQU로 계산
					}
				}
				ob_table[obid].length = 3;
				obloc += ob_table[obid].length;
				ob_table[obid].pc = locctr;
				obid++;
			}//if word
			else if (strcmp(token_table[token_line]->operator,"BYTE") == 0) {
				int length = strlen(token_table[token_line]->operand[0]) - 3;
				if (token_table[token_line]->operand[0][0] == 'X') {
					locctr += (length + 1) % 2;
					ob_table[obid].length = (length + 1) % 2;
					obloc += ob_table[obid].length;
					for (int i = 0; i < length; i++) {
						if (isalpha(token_table[token_line]->operand[0][i + 2]))
							ob_table[obid].ob_code += (int)(token_table[token_line]->operand[0][i + 2] - 0X37) << 4 * (length - 1 - i);
						else
							ob_table[obid].ob_code += (int)(token_table[token_line]->operand[0][i + 2] - 0X30) << 4 * (length - 1 - i);
					}
				}
				else if (token_table[token_line]->operand[0][0] == 'C') {
					locctr += length;
					ob_table[obid].length = length;
					obloc += ob_table[obid].length;
					for (int i = 0; i < length; i++) {
						if (isalpha(token_table[token_line]->operand[0][i + 2]))
							ob_table[obid].ob_code += (int)(token_table[token_line]->operand[0][i + 2]) << 8 * i;
						else
							ob_table[obid].ob_code += (int)(token_table[token_line]->operand[0][i + 2]) << 8 * i;
					}
				}
				ob_table[obid].pc = locctr;
				obid++;
			}//if operator = BYTE
			else if (strcmp(token_table[token_line]->operator,"CSECT") == 0) {
				secnum++;
				locctr = 0;
				prog_num++;
				mline++;
				obloc = 0;
			}
			else if (strcmp(token_table[token_line]->operator,"LTORG") == 0) {
			litnum[secnum] = litfid[litid + 1] - litfid[litid];
				for (int i = litfid[litid]; i < litfid[litid + 1]; i++) {
					int length = strlen(literal_table[i].literal) - 4;
					if (literal_table[i].literal[1] == 'X') {
						locctr += (length + 1) % 2;
						ob_table[obid].length = (length + 1) % 2;
						obloc += ob_table[obid].length;
						for (int k = 0; k < length; k++) {
							if (isalpha(literal_table[i].literal[k + 3]))
								ob_table[obid].ob_code += (int)(literal_table[i].literal[k + 3] - 0X37) << 4 * (length - 1 - k);
							else
								ob_table[obid].ob_code += (int)(literal_table[i].literal[k + 3] - 0X30) << 4 * (length - 1 - k);
						}
					}
					else if (literal_table[i].literal[1] == 'C') {
						locctr += length;
						ob_table[obid].length = length;
						obloc += ob_table[obid].length;
						for (int k = 0; k < length; k++) {
							if (isalpha(literal_table[i].literal[k + 3]))
								ob_table[obid].ob_code += (int)(literal_table[i].literal[k + 3]) << 8 * (length - 1 - k);
							else
								ob_table[obid].ob_code += (int)(literal_table[i].literal[k + 3]) << 8 * (length - 1 - k);
						}
					}
					else {
						locctr += 3;
						ob_table[obid].length = 3;
						obloc += ob_table[obid].length;
						for (int k = 0; k < strlen(literal_table[i].literal)-1; k++) {
								ob_table[obid].ob_code += (int)(literal_table[i].literal[k+1]-48) << 10*(2-(4- strlen(literal_table[i].literal))+k);
						}
					}
					ob_table[obid].pc = locctr;
					obid++;
				}
				litid++;
				
				
			}//if LOORG
		}//if not comment
	}//for not end
	////////////////////////////////////
	////////flush literal table/////////
	////////////////////////////////////
	for (int i = litfid[litid]; literal_table[i].literal != NULL; i++) {
		int length = strlen(literal_table[i].literal) - 4;
		if (literal_table[i].literal[1] == 'X') {
			locctr += (length + 1) % 2;
			ob_table[obid].length = (length + 1) % 2;
			obloc += ob_table[obid].length;
			for (int k = 0; k < length; k++) {
				if (isalpha(literal_table[i].literal[k + 3]))
					ob_table[obid].ob_code += (int)(literal_table[i].literal[k + 3] - 0X37) << 4 * (length - 1 - k);
				else
					ob_table[obid].ob_code += (int)(literal_table[i].literal[k + 3] - 0X30) << 4 * (length - 1 - k);
			}
		}
		else if (literal_table[i].literal[1] == 'C') {
			locctr += length;
			ob_table[obid].length = length;
			obloc += ob_table[obid].length;
			for (int k = 0; k < length; k++) {
				if (isalpha(literal_table[i].literal[k + 3]))
					ob_table[obid].ob_code += (int)(literal_table[i].literal[k + 3]) << 8 * k;
				else
					ob_table[obid].ob_code += (int)(literal_table[i].literal[k + 3]) << 8 * k;
			}
		}
		ob_table[obid].pc = locctr;
		obid++;
	}//end flush

	return errno;
}

/* ----------------------------------------------------------------------------------
* 설명 : 입력된 문자열의 이름을 가진 파일에 프로그램의 결과를 저장하는 함수이다.
*        여기서 출력되는 내용은 object code (프로젝트 1번) 이다.
* 매계 : 생성할 오브젝트 파일명
* 반환 : 없음
* 주의 : 만약 인자로 NULL값이 들어온다면 프로그램의 결과를 표준출력으로 보내어
*        화면에 출력해준다.
*
* -----------------------------------------------------------------------------------
*/
void make_objectcode_output(char *file_name)
{
	/* add your code here */
	FILE*fp;
	char buf[71];
	int prognum = 0;
	int buf_length = 0;
	int obid = 0;
	int mline = 0;
	int litout = 0;
	secnum = -1;
	if (file_name == NULL)
		fp = stdout;
	else
		fp = fopen(file_name, "w");
	/////////////////////////////////
	////////first section H//////////
	/////////////////////////////////
	sprintf(buf, "H%-6s%06X%06X\0", token_table[0]->label, 0, prog_length[prognum]);
	fprintf(fp, "%s\n", buf);
	if (extdef[prognum] != NULL)
		write_def(fp, prognum);
	if (extref[prognum] != NULL)
		write_ref(fp, prognum);
	secnum++;
	/////////////////////////////////
	/******T_ _ _ _ _ _ _ _ ~~******/
	/////////////////////////////////
	buf[0] = 'T';
	buf[1] = '\0';
	sprintf(buf, "%s%06X\0", buf, atoi(token_table[0]->operand[0]));
	buf[8] = '\0';
	buf[9] = '\0';
	/////////////////////////////////
	for (token_line = 1; ob_table[obid].length != 0; token_line++) {
		if (strcmp(token_table[token_line]->label, ".") != 0) {
			if (strcmp(token_table[token_line]->operator,"CSECT") == 0) {
				///////////////////////////
				////////flush buf//////////
				///////////////////////////
				if (buf_length > 0) {
					sprintf(buf, "%s%02X%s", buf, buf_length, buf + 9);
					fprintf(fp, "%s\n", buf);
				}
				///////////////////////////////
				///////////M RECORD////////////
				///////////////////////////////
				while (m_record[++mline][0] != '\0')
					fprintf(fp, "%s\n", m_record[mline]);
				///////////////////////////
				if (prognum == 0)
					sprintf(buf, "E%06X", atoi(token_table[0]->operand[0]));
				else
					sprintf(buf, "E");
				fprintf(fp, "%s\n", buf);



				///////////////////////////////
				//////new section program//////
				///////////////////////////////
				sprintf(buf, "H%-6s%06X%.6X\0", token_table[token_line]->label, 0, prog_length[++prognum]);
				fprintf(fp, "\n%s\n", buf);
				initial_bufT(buf, obid, &buf_length);
				if (extdef[prognum] != NULL)
					write_def(fp, prognum);
				if (extref[prognum] != NULL)
					write_ref(fp, prognum);
				secnum++;
				litout = 0;

			}//if CSECT
			else if (strcmp(token_table[token_line]->operator,"EXTDEF") == 0) {

			}
			else if (strcmp(token_table[token_line]->operator,"EXTREF") == 0) {

			}
			
			else if (strcmp(token_table[token_line]->operator,"RESW") == 0) {
				if (buf_length > 0) {
					sprintf(buf, "%s%02X%s", buf, buf_length, buf + 9);
					fprintf(fp, "%s\n", buf);
					initial_bufT(buf, obid, &buf_length);
				}
			}
			else if (strcmp(token_table[token_line]->operator,"RESB") == 0) {
				if (buf_length > 0) {
					sprintf(buf, "%s%02X%s", buf, buf_length, buf + 9);
					fprintf(fp, "%s\n", buf);
					initial_bufT(buf, obid, &buf_length);
				}
			}
			else if (strcmp(token_table[token_line]->operator,"EQU") == 0) {
				if (buf_length > 0) {
					sprintf(buf, "%s%02X%s", buf, buf_length, buf + 9);
					fprintf(fp, "%s\n", buf);
					initial_bufT(buf, obid, &buf_length);
				}
			}
			else {//write obcode
				if (strcmp(token_table[token_line]->operator,"LTORG") == 0) {
					litout++;
					if (litnum[secnum] > 1 && litout < litnum[secnum])
						--token_line;
				}
				int length = ob_table[obid].length;
				int object = ob_table[obid].ob_code;

				if (buf_length + length > 30) {//over max objectcode line
					sprintf(buf, "%s%02X%s", buf, buf_length, buf + 9);
					fprintf(fp, "%s\n", buf);
					initial_bufT(buf, obid, &buf_length);
				}
				buf_length += length;
				switch (length) {
				case 1:
					sprintf(buf + 9, "%s%02X\0", buf + 9, ob_table[obid].ob_code);
					break;
				case 2:
					sprintf(buf + 9, "%s%04X\0", buf + 9, ob_table[obid].ob_code);
					break;
				case 3:
					sprintf(buf + 9, "%s%06X\0", buf + 9, ob_table[obid].ob_code);
					break;
				case 4:
					sprintf(buf + 9, "%s%08X\0", buf + 9, ob_table[obid].ob_code);
					break;
				default:
					break;
				}
				obid++;
			}/**///write obcode end
		}//if not comment
	}//for not END
	///////////////////////////
	////////flush buf//////////
	///////////////////////////
	if (buf_length > 0) {
		sprintf(buf, "%s%02X%s", buf, buf_length, buf + 9);
		fprintf(fp, "%s\n", buf);
	}
	///////////////////////////////
	///////////M RECORD////////////
	///////////////////////////////
	while (m_record[++mline][0] != '\0')
		fprintf(fp, "%s\n", m_record[mline]);
	///////////////////////////////
	if (prognum == 0)
		sprintf(buf, "E%06X\n", atoi(token_table[0]->operand[0]));
	else
		sprintf(buf, "E\n");
	fprintf(fp, "%s\n", buf);

	fclose(fp);
}

/* ----------------------------------------------------------------------------------
 * 설명 : 입력 문자열이 심볼인지 검사하는 함수이다.
 * 매계 : 토큰 단위로 구분된 문자열
 * 반환 : 정상종료 = 심볼 테이블 인덱스, 에러 < 0
 * 주의 :
*
* -----------------------------------------------------------------------------------
*/
int search_symbol(char*str, int symfid) {
	for (int i = symfid; sym_table[i].symbol != NULL; i++) {
		if (i != symfid && sym_table[i].addr == 0 && strcmp(sym_table[i].symbol, "FIRST") != 0)
			break;
		if (strcmp(sym_table[i].symbol, str) == 0)
			return i;
	}

	return -1;
}

/* ----------------------------------------------------------------------------------
 * 설명 : 입력 문자열이 리터럴인지 검사하는 함수이다.
 * 매계 : 토큰 단위로 구분된 문자열
 * 반환 : 정상종료 = 심볼 테이블 인덱스, 에러 < 0
 * 주의 :
*
* -----------------------------------------------------------------------------------
*/
int search_literal(char*str, int litfid) {
	for (int i = litfid; literal_table[i].literal != NULL; i++) {
		if (strcmp(literal_table[i].literal, str) == 0)
			return i;
	}

	return -1;
}
/* ----------------------------------------------------------------------------------
 * 설명 : Define Record를 object program에 작성하는 함수이다.
 * 매계 : object program을 작성할 file pointer, program section number
 * 반환 : 없음
 * 주의 :
*
* -----------------------------------------------------------------------------------
*/
void write_def(FILE *fp, int prognum) {
	char buf[100] = { '\0' };
	int symid = 0;
	buf[0] = 'D';
	char*ptr = strtok(extdef[prognum], ",");
	sprintf(buf, "%s%-6s", buf, ptr);
	symid = search_symbol(ptr, symfid[prognum]);
	sprintf(buf, "%s%06X", buf, sym_table[symid].addr);
	while ((ptr = strtok(NULL, ",")) != NULL) {
		strcat(buf, ptr);
		symid = search_symbol(ptr, symfid[prognum]);
		sprintf(buf, "%s%06X", buf, sym_table[symid].addr);
	}
	fprintf(fp, "%s\n", buf);
}

/* ----------------------------------------------------------------------------------
 * 설명 : Refer Record를 object program에 작성하는 함수이다.
 * 매계 : object program을 작성할 file pointer, program section number
 * 반환 : 없음
 * 주의 :
*
* -----------------------------------------------------------------------------------
*/
void write_ref(FILE *fp, int prognum) {
	char buf[100] = { '\0' };
	int symid = 0;
	buf[0] = 'R';
	char*ptr = strtok(extref[prognum], ",");
	sprintf(buf, "%s%-6s", buf, ptr);
	while ((ptr = strtok(NULL, ",")) != NULL)
		strcat(buf, ptr);

	fprintf(fp, "%s\n", buf);
}
/* ----------------------------------------------------------------------------------
 * 설명 : Text Record를 작성하는데 사용될 buffer를 초기화한다.
 * 매계 : object program에 작성할 buffer, 첫번째 object code index, 현재 buffer에 있는 obcode의 length
 * 반환 : 없음
 * 주의 :
*
* -----------------------------------------------------------------------------------
*/
void initial_bufT(char* buf, int obid, int *buf_length) {
	sprintf(buf, "T%06X\0", ob_table[obid].pc - ob_table[obid].length);
	buf[8] = '\0';
	buf[9] = '\0';
	*buf_length = 0;
}
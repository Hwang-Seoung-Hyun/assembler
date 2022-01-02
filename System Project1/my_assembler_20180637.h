/*
 * my_assembler �Լ��� ���� ���� ���� �� ��ũ�θ� ��� �ִ� ��� �����̴�.
 *
 */
#define MAX_INST 256
#define MAX_LINES 5000
#define MAX_OPERAND 3

 /*
  * instruction ��� ���Ϸ� ���� ������ �޾ƿͼ� �����ϴ� ����ü �����̴�.
  * ������ ������ instruction set�� ��Ŀ� ���� ���� �����ϵ�
  * ���� ���� �ϳ��� instruction�� �����Ѵ�.
  */
struct inst_unit
{
	char str[10];
	unsigned char op;
	int format;
	int ops;
};

// instruction�� ������ ���� ����ü�� �����ϴ� ���̺� ����
typedef struct inst_unit inst;
inst *inst_table[MAX_INST];
int inst_index;

/*
 * ����� �� �ҽ��ڵ带 �Է¹޴� ���̺��̴�. ���� ������ ������ �� �ִ�.
 */
char *input_data[MAX_LINES];
static int line_num;

/*
 * ����� �� �ҽ��ڵ带 ��ū������ �����ϱ� ���� ����ü �����̴�.
 * operator�� renaming�� ����Ѵ�.
 * nixbpe�� 8bit �� ���� 6���� bit�� �̿��Ͽ� n,i,x,b,p,e�� ǥ���Ѵ�.
 */
struct token_unit
{
	char *label;
	char *operator;
	char operand[MAX_OPERAND][20];
	char comment[100];
	char nixbpe;
};

typedef struct token_unit token;
token *token_table[MAX_LINES];
static int token_line;

/*
 * �ɺ��� �����ϴ� ����ü�̴�.
 * �ɺ� ���̺��� �ɺ� �̸�, �ɺ��� ��ġ�� �����ȴ�.
 */
struct symbol_unit
{
	char *symbol;
	int addr;
};

/*
* ���ͷ��� �����ϴ� ����ü�̴�.
* ���ͷ� ���̺��� ���ͷ��� �̸�, ���ͷ��� ��ġ�� �����ȴ�.
* ���� ������Ʈ���� ���ȴ�.
*/
struct literal_unit
{
	char *literal;
	int addr;
};

typedef struct symbol_unit symbol;
symbol sym_table[MAX_LINES];

typedef struct literal_unit literal;
literal literal_table[MAX_LINES];

static int locctr;
//--------------

static char *input_file;
static char *output_file;
int init_my_assembler(void);
int init_inst_file(char *inst_file);
int init_input_file(char *input_file);
int token_parsing(char *str);
int search_opcode(char *str);
static int assem_pass1(void);
void make_opcode_output(char *file_name);

void make_symtab_output(char *file_name);
void make_literaltab_output(char *file_name);
static int assem_pass2(void);
void make_objectcode_output(char *file_name);


//-----additional define---------

struct register_unit {
	char name[3];
	int num;
};
typedef struct register_unit registers;

registers register_table[9] = {
	{"A",0},{"X",1},{"L",2},{"B",3},{"S",4},{"T",5},{"F",6},{"PC",8},{"SW",9}
};

struct object {
	int ob_code;
	int pc;
	int length;
};
struct object ob_table[MAX_LINES];
static char* extdef[10];
static char* extref[10];
static int prog_length[10];
static int pc[MAX_LINES];
static int symfid[MAX_LINES] = { 0 };
static int litfid[MAX_LINES] = { 0 };//���� ���ǵ��� ���� ù��° literal_table index
static char m_record[MAX_LINES][17];
static int secnum;
static int litnum[MAX_LINES];
int search_symbol(char*str, int symfid);
int search_literal(char*str, int litfid);
void write_def(FILE *fp, int prognum);
void write_ref(FILE *fp, int prognum);
void initial_bufT(char* buf, int obid, int *buf_length);
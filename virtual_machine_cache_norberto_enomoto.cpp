/********************************************************************************

M�quina Virtual - IOT010

	Este codigo implementa uma m�quina virtual (interpretador) capaz de buscar,
decodificar e executar um set de Instructionu��es criado exclusivamente para demonstra��o
durante as aulas de IOT010.

***********************************************************************************

Detalhes do set de Instructionu��es

	Tamanho das Instructionu��es 32 bits

	C�digo das intru��es:

		ADD: 	0000001
		SUB: 	0000011
		LOAD: 	0001000
		STORE:	0001001

	Instructionu��es Tipo 1:

		- Utilizado para opera��es aritm�ticas (soma, subtra��o, ...)

             MSB                                      LSB

		(End Reg Dest.)  (End. Reg 1)  (End. Reg 2)  (Tipo Instruction.)

           8 bits          8 bits        8 bits       8 bits


         - Exemplo: 0b00000010000000000000000100000001 >>> |00000010|00000000|00000001|00000001

         	Realiza a soma (00000001 >> tipo da Instructionu��o) do registro 0 (00000000
 	 	 	 >> end. Reg 1) com o registro 1 (00000001 >> end. Reg 2) e salva o resultado
 	 	 	 em registro 2 (00000010 >> end. Reg Dest.)


    Instructionu��es Tipo 2:

     	 - Uitlizado para opera��es de LOAD e STORE

     	       MSB                        LSB

     	  (End Mem�ria de dados)  (End Reg) (Tipo Instruction.)

		         16 bits            8 bits     8 bits

       - Exemplo: 00000000000010100000000000001000 >>> |0000000000001010|00000000|00001000|

         	Realiza o LOAD (00001000 >> tipo da Instructionu��o) do endere�o de
			mem�ria 10 (0000000000001010 >> end. mem�ria) para o registro 0
			(00000000 >> end. Reg )

*******************************************************************************

   Cache de instru��o


   Numero de linhas de cache: 2


    Cada linha da cache possui: (definida na struct LineMemoryCacheStruct)

    Bit de valida��o
    Campo para TAG
    2 words de data

********************************************************************************

    Calculo do BYTE, Word, Linha e TAG  a partir do endere�o de mem�ria solicitado

    Cada endere�o solicitado possui 2 bytes (16 bits - quantidades de bits usada
	nesta arquitetura)

	0b 0000 0000 0000 0000 (end. 0)

	BYTE: Para o campo BYTE n�o teremos nenhum bit reservado, pois como minha
	estrutura trabalha com 16 bits e cada posi��o da mem�ria de instru��o possui
	16 bits, ser� necess�ria a leitura de apenas um endere�o para obter uma
	WORD completa.

	WORD: Cada linha de cache suporta salvar duas WORDs, portanto 1 bit do endere�o
	sera utilizado para identificar qual WORD  a CPU deseja

	LINE: A cache projetada possui apenas 2 linhas, portanto ser� nences�rio apenas
	1 bit para endere�ar a linha

	TAG: � o que sobra.... 14 bits (16 bits (total) - 1 bit (LINE) - 1 bit (WORD)).


	0b 0000 0000 0000 00 |       0      |   0
         (TAG - 14 bits)  (LINE - 1 bit)  (WORD - 1 bit)

	Exemplo de endera�amento na cache:

	Suponha que a CPU solicitou a instru��o que esta no endere�o
	0b 0000 0000 0000 0011 (end. 3)

		0b 0000 0000 0000 00 |       1      |   1
         (TAG - 14 bits)      (LINE - 1 bit)  (WORD - 1 bit)


    Este endere�o deve ser procurado na linha 1 da cache e a TAG deve estar com o
    valor 0. Caso ocorra sucesso (a TAG presente na linha 1 seja igual a esperada)
    deve ser lido a WORD 1

********************************************************************************/


#include <iostream>

using namespace std;

// Definicao da Memoria cache
typedef struct LineMemoryCache
{
	bool 			bValid;
	unsigned int	Tag;
	unsigned int 	Data[2];
} LineMemoryCacheStruct;


// Memoria de programa
unsigned long ProgramMemory[] = {0b00000000000000000000000000001000,
								 0b00000000000000010000000100001000,
								 0b00000010000000000000000100000001,
								 0b00000000000000100000001000001001,
								 0b00000000000000100000000000001000,
								 0b00000000000000010000000100001000,
								 0b00000010000000000000000100000001,
								 0b00000000000000110000001000001001
								};


/*
unsigned long ProgramMemory[] = {0b00000000000000000000000000001000,
								 0b00000000000000010000000100001000,
								 0b00000010000000000000000100000001,
								 0b00000000000000100000001000001001,
								};
*/


// Memoria de dados
unsigned long DataMemory[] = {1, 2, 0, 0, 0, 0, 0, 0};

// Memoria cahe
LineMemoryCacheStruct  MemoryCache[2]; // numero de linhas da cache

// Registradores
unsigned long ProgramCounter;
unsigned long Instruction;
unsigned long InstructionType;
unsigned long RegisterSourceA;
unsigned long RegisterSourceB;
unsigned long RegisterDestination;
unsigned long RegisterAddressMemory;
unsigned long Register[10];

// Prototipos
void decode(void);
void evaluate(void);

unsigned long getInstructionType(unsigned long instruction);

unsigned int get_in_cache(unsigned long inst_addr);
unsigned int load_cache(unsigned long inst_addr);


/*
   carregar de um arquivo -> programa memory
*/

int main()
{
	unsigned char i;

	cout << "interpreter_norberto" << endl;

	// Inicializacao dos registros
	ProgramCounter = 0;
	for (i = 0; i < 10; i++)
	{
		Register[i] = 0;
	}

	for(i = 0; i < 2; i++)
	{
		MemoryCache[i].bValid = false;
	}

	while (ProgramCounter < 8)
	{
		// busca da Instru��o
		// Instruction = ProgramMemory[ProgramCounter];

		Instruction = get_in_cache(ProgramCounter);//ProgMemory[PC]; // busca da instru��o

		ProgramCounter = ProgramCounter + 1;
		// decodica��o
		decode();
		evaluate();
	}

	for (int i = 0; i < 10; i++)
	{
		cout << "Register[" << i << "]: " << Register[i] << endl;
	}

	return 0;
}

void decode(void)
{
	InstructionType = getInstructionType(Instruction);
	cout << "InstructionType: " << InstructionType << endl;

	switch( InstructionType )
	{
		// Soma
	case 1:
		RegisterSourceA = Instruction >> 16;
		RegisterSourceA = RegisterSourceA & 0b00000000000000000000000011111111;
		cout << "RegisterSourceA: " << RegisterSourceA << endl;

		RegisterSourceB = Instruction >> 8;
		RegisterSourceB = RegisterSourceB & 0b00000000000000000000000011111111;
		cout << "RegisterSourceB: " << RegisterSourceB << endl;

		RegisterDestination = Instruction >> 24;
		cout << "RegisterDestination: " << RegisterDestination << endl;
		break;

		// Substra��o
	case 3:
		RegisterSourceA = Instruction >> 16;
		RegisterSourceA = RegisterSourceA & 0b00000000000000000000000011111111;
		cout << "RegisterSourceA: " << RegisterSourceA << endl;

		RegisterSourceB = Instruction >> 8;
		RegisterSourceB = RegisterSourceB & 0b00000000000000000000000011111111;
		cout << "RegisterSourceB: " << RegisterSourceB << endl;

		RegisterDestination = Instruction >> 24;
		cout << "RegisterDestination: " << RegisterDestination << endl;
		break;

		// Load
	case 8:
		RegisterDestination = Instruction >> 8;
		RegisterDestination = RegisterDestination & 0b00000000000000000000000011111111;
		cout << "RegisterDestination: " << RegisterDestination << endl;

		RegisterAddressMemory = Instruction >> 16;
		RegisterAddressMemory = RegisterAddressMemory & 0b00000000000000001111111111111111;
		cout << "RegisterAddressMemory: " << RegisterAddressMemory << endl;
		break;

		// Store
	case 9:
		RegisterSourceA = Instruction >> 8;
		RegisterSourceA = RegisterSourceA & 0b00000000000000000000000011111111;
		cout << "RegisterSourceA: " << RegisterSourceA << endl;

		RegisterAddressMemory = Instruction >> 16;
		RegisterAddressMemory = RegisterAddressMemory & 0b00000000000000001111111111111111;
		cout << "RegisterAddressMemory: " << RegisterAddressMemory << endl;
		break;
	}
}

void evaluate(void)
{
	switch( InstructionType )
	{
	case 1:
		// Soma
		Register[RegisterDestination] = Register[RegisterSourceA] + Register[RegisterSourceB];
		break;
	case 3:
		// Subtracao
		Register[RegisterDestination] = Register[RegisterSourceA] - Register[RegisterSourceB];
		break;
	case 8:
		// Load
		Register[RegisterDestination] = DataMemory[RegisterAddressMemory];
		break;
	case 9:
		// Store
		DataMemory[RegisterAddressMemory] = Register[RegisterSourceA];
		break;
	}
}

unsigned long getInstructionType(unsigned long instruction)
{
	return instruction & 0b00000000000000000000000011111111;
}

unsigned int get_in_cache(unsigned long inst_addr)
{
	unsigned char Line, Word;
	unsigned long Tag;
	unsigned long InstAux;

	Word = inst_addr & 0x01;
	Line = inst_addr >> 1;
	Line &= 0x01;
	Tag = inst_addr >> 2;

	if(MemoryCache[Line].bValid)
	{
		if(MemoryCache[Line].Tag == Tag)
		{
			InstAux = MemoryCache[Line].Data[Word];
		}
		else InstAux = load_cache(inst_addr);
	}
	else InstAux = load_cache(inst_addr);

	return InstAux;
}

unsigned int load_cache(unsigned long inst_addr)
{
	unsigned char Line, Word, i;
	unsigned long Tag;
	unsigned long InstAux;
	unsigned long AuxInstAdd;


	Word = inst_addr & 0x01;
	Line = inst_addr >> 1;
	Line &= 0x01;
	Tag = inst_addr >> 2;

	MemoryCache[Line].bValid = true;
	MemoryCache[Line].Tag = Tag;
	AuxInstAdd = inst_addr - Word;
	for(i = 0; i < 2; i++)
	{
		MemoryCache[Line].Data[i] = ProgramMemory[AuxInstAdd + i];
		if((AuxInstAdd + i) == inst_addr)
		{
			InstAux = ProgramMemory[AuxInstAdd + i];
		}
	}

	return InstAux;
}


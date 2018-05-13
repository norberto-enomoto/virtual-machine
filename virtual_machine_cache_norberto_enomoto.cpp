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

********************************************************************************/

#include <iostream>

using namespace std;

// Definicao da Memoria cache
typedef struct LineMemoryCache
{
	bool 			bValid;
	unsigned int	Tag;
	unsigned int 	Data[2]; 
}LineMemoryCacheStruct;


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
unsigned int get_in_cache(unsigned int inst_addr);
unsigned int load_cache(unsigned int inst_addr);


/*
   getInstructionuction
   getRegisterSourceA
   getRegisterSourceB
   getRegisterDestination
   getRegisterAddressMemory

   trocar o if por switch
   
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
	// InstructionType = Instruction << 24;
	// InstructionType = Instruction & 0xFF;
	InstructionType = Instruction & 0b00000000000000000000000011111111;
	cout << "InstructionType: " << InstructionType << endl;

	if (InstructionType == 1 || InstructionType == 3)
	{
		// Soma, Subtracao
		RegisterSourceA = Instruction >> 16;
		RegisterSourceA = RegisterSourceA & 0b00000000000000000000000011111111;
		cout << "RegisterSourceA: " << RegisterSourceA << endl;
		RegisterSourceB = Instruction >> 8;
		RegisterSourceB = RegisterSourceB & 0b00000000000000000000000011111111;
		cout << "RegisterSourceB: " << RegisterSourceB << endl;
		RegisterDestination = Instruction >> 24;
		cout << "RegisterDestination: " << RegisterDestination << endl;
	}
	else if (InstructionType == 8)
	{
		/* Load */
		RegisterDestination = Instruction >> 8;
		RegisterDestination = RegisterDestination & 0b00000000000000000000000011111111;
		cout << "RegisterDestination: " << RegisterDestination << endl;
		RegisterAddressMemory = Instruction >> 16;
		RegisterAddressMemory = RegisterAddressMemory & 0b00000000000000001111111111111111;
		cout << "RegisterAddressMemory: " << RegisterAddressMemory << endl;
	}
	else if (InstructionType == 9)
	{
		/* Store */
		RegisterSourceA = Instruction >> 8;
		RegisterSourceA = RegisterSourceA & 0b00000000000000000000000011111111;
		cout << "RegisterSourceA: " << RegisterSourceA << endl;
		RegisterAddressMemory = Instruction >> 16;
		RegisterAddressMemory = RegisterAddressMemory & 0b00000000000000001111111111111111;
		cout << "RegisterAddressMemory: " << RegisterAddressMemory << endl;
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

	/*
	if (InstructionType == 1)
	{
	    // Soma
	    Register[RegisterDestination] = Register[RegisterSourceA] + Register[RegisterSourceB];
	}
	else if (InstructionType == 3)
	{
	    // Subtracao
	    Register[RegisterDestination] = Register[RegisterSourceA] - Register[RegisterSourceB];
	}
	else if (InstructionType == 8)
	{
	    // Load
	    Register[RegisterDestination] = DataMemory[RegisterAddressMemory];
	}
	else if (InstructionType == 9)
	{
	    // Store
	    DataMemory[RegisterAddressMemory] = Register[RegisterSourceA];
	}
	*/
}

unsigned int get_in_cache(unsigned int inst_addr)
{
	unsigned char Line, Word;
	unsigned int Tag;
	unsigned int InstAux;
	
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

unsigned int load_cache(unsigned int inst_addr)
{
	unsigned char Line, Word, i;
	unsigned int Tag;
	unsigned int InstAux;
	unsigned int AuxInstAdd;
	
	
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
		if((AuxInstAdd + i) == inst_addr)InstAux = ProgramMemory[AuxInstAdd + i];
	}

	return InstAux;
}


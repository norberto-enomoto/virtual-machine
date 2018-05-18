/********************************************************************************

Máquina Virtual - IOT010

	Este codigo implementa uma máquina virtual (interpretador) capaz de buscar,
decodificar e executar um set de instruções criado exclusivamente para demonstração
durante as aulas de IOT010.

***********************************************************************************

Detalhes do set de Instructionuções

	Tamanho das Instruções: 32 bits

	Código das intruções:

		ADD..: 	0000001
		SUB..: 	0000011
		OR...:  0000100
		AND..:  0000101
		LOAD.: 	0001000
		STORE:	0001001

	Instruções Tipo 1:

		- Utilizado para operações aritméticas (soma, subtração, ...)

             MSB                                      LSB

		(End Reg Dest.)  (End. Reg 1)  (End. Reg 2)  (Tipo Instruction.)

           8 bits          8 bits        8 bits       8 bits


         - Exemplo: 0b00000010000000000000000100000001 >>> |00000010|00000000|00000001|00000001

         	Realiza a soma (00000001 >> tipo da Instructionução) do registro 0 (00000000
 	 	 	 >> end. Reg 1) com o registro 1 (00000001 >> end. Reg 2) e salva o resultado
 	 	 	 em registro 2 (00000010 >> end. Reg Dest.)


    Instruções Tipo 2:

     	 - Uitlizado para operações de LOAD e STORE

     	       MSB                        LSB

     	  (End Memória de dados)  (End Reg) (Tipo Instruction.)

		         16 bits            8 bits     8 bits

       - Exemplo: 00000000000010100000000000001000 >>> |0000000000001010|00000000|00001000|

         	Realiza o LOAD (00001000 >> tipo da Instructionução) do endereço de
			memória 10 (0000000000001010 >> end. memória) para o registro 0
			(00000000 >> end. Reg )

*******************************************************************************

   Cache de instrução


   Numero de linhas de cache: 2


    Cada linha da cache possui: (definida na struct LineMemoryCacheStruct)

    Bit de validação
    Campo para TAG
    2 words de data

********************************************************************************

    Calculo do BYTE, Word, Linha e TAG  a partir do endereço de memória solicitado

    Cada endereço solicitado possui 2 bytes (16 bits - quantidades de bits usada
	nesta arquitetura)

	0b 0000 0000 0000 0000 (end. 0)

	BYTE: Para o campo BYTE não teremos nenhum bit reservado, pois como minha
	estrutura trabalha com 16 bits e cada posição da memória de instrução possui
	16 bits, será necessária a leitura de apenas um endereço para obter uma
	WORD completa.

	WORD: Cada linha de cache suporta salvar duas WORDs, portanto 1 bit do endereço
	sera utilizado para identificar qual WORD  a CPU deseja

	LINE: A cache projetada possui apenas 2 linhas, portanto será nencesário apenas
	1 bit para endereçar a linha

	TAG: É o que sobra.... 14 bits (16 bits (total) - 1 bit (LINE) - 1 bit (WORD)).


	0b 0000 0000 0000 00 |       0      |   0
         (TAG - 14 bits)  (LINE - 1 bit)  (WORD - 1 bit)

	Exemplo de enderaçamento na cache:

	Suponha que a CPU solicitou a instrução que esta no endereço
	0b 0000 0000 0000 0011 (end. 3)

		0b 0000 0000 0000 00 |       1      |   1
         (TAG - 14 bits)      (LINE - 1 bit)  (WORD - 1 bit)


    Este endereço deve ser procurado na linha 1 da cache e a TAG deve estar com o
    valor 0. Caso ocorra sucesso (a TAG presente na linha 1 seja igual a esperada)
    deve ser lido a WORD 1

********************************************************************************/


#include <iostream>

using namespace std;

// Constantes
const int __REGISTER_SIZE        = 10;
const int __PROGRAM_MEMORY_SIZE_ = 16;
const int __DATA_MEMORY_SIZE_    = 8;

const int __MEMORY_CACHE_SIZE_   = 2;
const int __WORD_SIZE_           = 2;
;
// Definicao da Memoria cache
typedef struct LineMemoryCache
{
	bool 			bValid;
	unsigned long	Tag;
	unsigned long 	Data[__WORD_SIZE_];
} LineMemoryCacheStruct;


// Memoria de programa
unsigned long ProgramMemory[] = {0b00000000000000000000000000001000,
								 0b00000000000000010000000100001000,
								 0b00000010000000000000000100000001,
								 0b00000000000000100000001000001001,
								 0b00000000000000100000000000001000,
								 0b00000000000000010000000100001000,
								 0b00000010000000000000000100000001,
								 0b00000000000000110000001000001001,
                                 0b00000000000001000000001100001000,
                                 0b00000000000001010000010000001000,
                                 0b00000101000000110000010000000100,
                                 0b00000000000001100000010100001001,
                                 0b00000000000001000000001100001000,
                                 0b00000000000001010000010000001000,
                                 0b00000101000000110000010000000101,
                                 0b00000000000001110000010100001001
								};

// Memoria de dados
unsigned long DataMemory[] = {1, 2, 0, 0, 3328, 15360, 0, 0};

// Memoria cahe
LineMemoryCacheStruct  MemoryCache[__MEMORY_CACHE_SIZE_]; // numero de linhas da cache

// Registradores
unsigned long ProgramCounter;
unsigned long Instruction;
unsigned long InstructionType;
unsigned long RegisterSourceA;
unsigned long RegisterSourceB;
unsigned long RegisterDestination;
unsigned long RegisterAddressMemory;
unsigned long Register[__REGISTER_SIZE];

// Prototipos
void initVariables(void);
void decode(void);
void evaluate(void);

unsigned long getInstructionType(unsigned long instruction);

unsigned long getFromCache(unsigned long inst_addr);
unsigned long loadCache(unsigned long inst_addr);

void printProgramMemory();
void printDataMemory();
void printRegister();


int main()
{
    initVariables();
    while (ProgramCounter < __PROGRAM_MEMORY_SIZE_)
	{
        Instruction = getFromCache(ProgramCounter);//ProgMemory[PC]; // busca da instrução
        ProgramCounter = ProgramCounter + 1;
		decode();
		evaluate();
		printDataMemory();
		printRegister();
	}

	return 0;
}

void initVariables()
{
    unsigned char i;

	// Inicialização dos registradores
	ProgramCounter = 0;
	for (i = 0; i < __REGISTER_SIZE; i++)
	{
		Register[i] = 0;
	}

	// Inicialização da memória cache
	for(i = 0; i < __MEMORY_CACHE_SIZE_; i++)
	{
		MemoryCache[i].bValid = false;
	}
	
	printProgramMemory();
}

void decode(void)
{
	InstructionType = getInstructionType(Instruction);
	cout << "decode->Instruction....: " << Instruction << endl;
	cout << "decode->InstructionType: " << InstructionType << endl;
	
	// Soma, Subtracao
	if (InstructionType == 1 || InstructionType == 3 
	    || InstructionType == 4 || InstructionType == 5)
	{   
		if (InstructionType == 1)
		{
			cout << "decode->Soma(1)" << endl;
		}
		else if (InstructionType == 3)
		{
			cout << "decode->Subracao(3)" << endl;
		}
		else if (InstructionType == 4)
        {
			cout << "decode->Or(4)" << endl;
		}
        else if (InstructionType == 5)
        {
			cout << "decode->And(5)" << endl;
		}
		
		RegisterSourceA = Instruction >> 16;
		RegisterSourceA = RegisterSourceA & 0b00000000000000000000000011111111;
		cout << "decode->RegisterSourceA: " << RegisterSourceA << endl;

		RegisterSourceB = Instruction >> 8;
		RegisterSourceB = RegisterSourceB & 0b00000000000000000000000011111111;
		cout << "decode->RegisterSourceB: " << RegisterSourceB << endl;

		RegisterDestination = Instruction >> 24;
		cout << "decode->RegisterDestination: " << RegisterDestination << endl;
	}
	
	// Load
    else if (InstructionType == 8)
	{
        cout << "decode->Load(8)" << endl;
		RegisterDestination = Instruction >> 8;
		RegisterDestination = RegisterDestination & 0b00000000000000000000000011111111;
		cout << "decode->RegisterDestination: " << RegisterDestination << endl;

		RegisterAddressMemory = Instruction >> 16;
		RegisterAddressMemory = RegisterAddressMemory & 0b00000000000000001111111111111111;
		cout << "decode->RegisterAddressMemory: " << RegisterAddressMemory << endl;
    }
    
	// Sotre
	else if (InstructionType == 9)
	{
        cout << "decode->Store(9)" << endl;
		RegisterSourceA = Instruction >> 8;
		RegisterSourceA = RegisterSourceA & 0b00000000000000000000000011111111;
		cout << "decode->RegisterSourceA: " << RegisterSourceA << endl;

		RegisterAddressMemory = Instruction >> 16;
		RegisterAddressMemory = RegisterAddressMemory & 0b00000000000000001111111111111111;
		cout << "decode->RegisterAddressMemory: " << RegisterAddressMemory << endl;
	}
}

void evaluate(void)
{
	switch( InstructionType )
	{
    // Soma
	case 1:
		Register[RegisterDestination] = Register[RegisterSourceA] + Register[RegisterSourceB];
		
		cout << "evaluate->Soma(1): Register[RegisterDestination] = Register[RegisterSourceA] + Register[RegisterSourceB]" << endl;
		cout << "evaluate->Register[" << RegisterDestination << 
		     "]: Register[" <<  RegisterSourceA << "] + Register[" <<
		     RegisterSourceB << "]" << endl;
		cout << "evaluate->Register[" << RegisterDestination << 
		     "]: " << Register[RegisterSourceA] << " + " << Register[RegisterSourceB]
			 << endl;
		     
		break;
	
	// Subtracao
	case 3:
		Register[RegisterDestination] = Register[RegisterSourceA] - Register[RegisterSourceB];

		cout << "evaluate->Subtracao(3): Register[RegisterDestination] = Register[RegisterSourceA] - Register[RegisterSourceB]" << endl;		
		cout << "evaluate->Register[" << RegisterDestination << 
		     "]: Register[" <<  RegisterSourceA << "] - Register[" <<
		     RegisterSourceB << "]" << endl;
		cout << "evaluate->Register[" << RegisterDestination << 
		     "]: " << Register[RegisterSourceA] << " - " << Register[RegisterSourceB]
			 << endl;
		
		break;
	
	// Or
	case 4:
		Register[RegisterDestination] = Register[RegisterSourceA] | Register[RegisterSourceB];
		
		cout << "evaluate->Or(4): Register[RegisterDestination] = Register[RegisterSourceA] | Register[RegisterSourceB]" << endl;
		cout << "evaluate->Register[" << RegisterDestination << 
		     "]: Register[" <<  RegisterSourceA << "] | Register[" <<
		     RegisterSourceB << "]" << endl;
		cout << "evaluate->Register[" << RegisterDestination << 
		     "]: " << Register[RegisterSourceA] << " | " << Register[RegisterSourceB]
			 << endl;
		     
		break;
	
    // And
	case 5:
		Register[RegisterDestination] = Register[RegisterSourceA] & Register[RegisterSourceB];
		
		cout << "evaluate->And(5): Register[RegisterDestination] = Register[RegisterSourceA] & Register[RegisterSourceB]" << endl;
		cout << "evaluate->Register[" << RegisterDestination << 
		     "]: Register[" <<  RegisterSourceA << "] & Register[" <<
		     RegisterSourceB << "]" << endl;
		cout << "evaluate->Register[" << RegisterDestination << 
		     "]: " << Register[RegisterSourceA] << " & " << Register[RegisterSourceB]
			 << endl;
		     
		break;
	
	// Load
	case 8:
		Register[RegisterDestination] = DataMemory[RegisterAddressMemory];
		
		cout << "evaluate->Load(8): Register[RegisterDestination] = DataMemory[RegisterAddressMemory]" << endl;;
		cout << "evaluate->Register[" << RegisterDestination << 
		     "]: DataMemory[" <<  RegisterAddressMemory << "]" << endl;
		cout << "evaluate->Register[" << RegisterDestination << 
		     "]: " << DataMemory[RegisterAddressMemory] << endl;
		
		break;
	
	// Store
	case 9:
		DataMemory[RegisterAddressMemory] = Register[RegisterSourceA];

		cout << "evaluate->Store(9): DataMemory[RegisterAddressMemory] = Register[RegisterSourceA]" << endl;
		cout << "evaluate->DataMemory[" << RegisterAddressMemory << 
		     "]: Register[" <<  RegisterSourceA << "]" << endl;
		cout << "evaluate->DataMemory[" << RegisterAddressMemory << 
		     "]: " << Register[RegisterSourceA] << endl;
		
		break;
	}
}

unsigned long getInstructionType(unsigned long instruction)
{
	return instruction & 0b00000000000000000000000011111111;
}

unsigned long getFromCache(unsigned long inst_addr)
{
	unsigned char Line, Word;
	unsigned long Tag;
	unsigned long InstAux;

	Word = inst_addr & 0x01;
	Line = inst_addr >> 1;
	Line &= 0x01;
	Tag = inst_addr >> 2;

	cout << "<---------------------------------------------------->" << endl;

	if(MemoryCache[Line].bValid)
	{
		if(MemoryCache[Line].Tag == Tag)
		{
			InstAux = MemoryCache[Line].Data[Word];
			cout << "getFromCache->InstAux: " << "MemoryCache[" << (int)Line <<
				 "].Data[" << int(Word) << "]" << ": " << InstAux << endl;
		}
		else
		{
			InstAux = loadCache(inst_addr);
			cout << "getFromCache->InstAux: " << "load_cache(" << inst_addr << ")"
				 << ": " << InstAux << endl;
		}

	}
	else
	{
		InstAux = loadCache(inst_addr);
		cout << "getFromCache->InstAux: " << "load_cache(" << inst_addr << ")"
			 << ": " << InstAux << endl;
	}

	return InstAux;
}

unsigned long loadCache(unsigned long inst_addr)
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
	for(i = 0; i < __WORD_SIZE_; i++)
	{
		MemoryCache[Line].Data[i] = ProgramMemory[AuxInstAdd + i];
		cout << "loadCache->MemoryCache[" << (int)Line << "].Data[" << (int)i << "]: " << ProgramMemory[AuxInstAdd + i] << endl;
		if((AuxInstAdd + i) == inst_addr)
		{
			InstAux = ProgramMemory[AuxInstAdd + i];
			cout << "loadCache->InstAux: ProgramMemory[" << AuxInstAdd + i << "]"<< endl;
		}
	}

	return InstAux;
}

void printProgramMemory()
{	
	cout << "<------------------ Program Memory ------------------>" << endl;
    for (int i = 0; i < __PROGRAM_MEMORY_SIZE_; i++)
	{
		cout << "ProgramMemory[" << i << "]: " << ProgramMemory[i] << endl;
	}
}

void printDataMemory()
{	
    for (int i = 0; i < __DATA_MEMORY_SIZE_; i++)
	{
		cout << "DataMemory[" << i << "]: " << DataMemory[i] << endl;
	}

}

void printRegister()
{	
	for (int i = 0; i < __REGISTER_SIZE; i++)
	{
		cout << "Register[" << i << "]: " << Register[i] << endl;
	}
}

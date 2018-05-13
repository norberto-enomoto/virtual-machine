/********************************************************************************

Máquina virtual IOT010

	Este codigo implementa uma máquina virtual (interpretador) capaz de buscar,
decodificar e executar um set de instrucão criado exclusivamente para demostrações 
durante as aulas de IOT010.   

***********************************************************************************

Detalhes do set de instrução

	Tamanho das instruções: 16 bits
	
	Código das intruções:
	
		ADD: 	0000001
		SUB: 	0000011
		LOAD: 	0001000
		STORE:	0001001

	Instruções Tipo 1: 
	
		- Utilizado para operações aritméticas (soma, subtração, ...)
	     
             MSB                                      LSB
		   
		(End Reg Dest.)  (End. Reg 1)  (End. Reg 2)  (Tipo instr.) 
          
           8 bits          8 bits        8 bits       8 bits
           
		   
         - Exemplo: 0b0001000000010010 >>> |00000010|00000000|00000001|00000001
         
         	Realiza a soma (00000001 >> tipo da instrução) do registro 0 (00000000 
 	 	 	 >> end. Reg 1) com o registro 1 (00000001 >> end. Reg 2) e salva o resultado
 	 	 	 em registro 2 (00000010 >> end. Reg Dest.)
 	 	 	 
 	 	 	 
    Instruções Tipo 2:
    
     	 - Uitlizado para operações de LOAD e STORE
     	 
     	       MSB                        LSB
     	 
     	  (End Memória de dados)  (End Reg) (Tipo instr.)

		         16 bits            8 bits     8 bits
		    
       - Exemplo: 00000000000010100000000000001000 >>> |0000000000001010|00000000|00001000|
         
         	Realiza o LOAD (00001000 >> tipo da instrução) do endereço de 
			memória 10 (0000000000001010 >> end. Memória) para o registro 0 
			(00000000 >> end. Reg )
 	 	 	 
********************************************************************************/

#include <iostream>

using namespace std;

// Memoria de programa
unsigned long ProgMemory[] = {0b00000000000000000000000000001000,
                              0b00000000000000010000000100001000,
                              0b00000010000000000000000100000001,
                              0b00000000000000100000001000001001,
                              };
// Memoria de dados
unsigned long DataMemory[] = {1, 2, 0, 0, 0, 0, 0, 0};

// Registradores
unsigned long PC;
unsigned long Instr;
unsigned long InstrType;
unsigned long RegSourceA;
unsigned long RegSourceB;
unsigned long RegDest;
unsigned long RegAddrMemory;
unsigned long Reg[10];

// Prototipos
void decode(void);
void execute(void);

int main()
{
    unsigned char i;

    cout << "interpreter_norberto" << endl;

    // Inicializacao dos registros
    PC = 0;
    for (i = 0; i < 10; i++)
    {
        Reg[i] = 0;
    }

    while (PC < 4)
    {
        Instr = ProgMemory[PC]; // busca da instrução
        PC = PC + 1;
        decode(); // decodificação
        execute();
    }

    for (int i = 0; i < 10; i++)
    {
        cout << "Reg[" << i << "]: " << Reg[i] << endl;
    }

    return 0;
}

void decode(void)
{
    // InstrType = Instr << 24;
    // InstrType = Instr & 0xFF;
    InstrType = Instr & 0b00000000000000000000000011111111;
    cout << "InstrType: " << InstrType << endl;

    if (InstrType == 1 || InstrType == 3)
    {
        // Soma, Subtracao
        RegSourceA = Instr >> 16;
        RegSourceA = RegSourceA & 0b00000000000000000000000011111111;
        cout << "RegSourceA: " << RegSourceA << endl;
        RegSourceB = Instr >> 8;
        RegSourceB = RegSourceB & 0b00000000000000000000000011111111;
        cout << "RegSourceB: " << RegSourceB << endl;
        RegDest = Instr >> 24;
        cout << "RegDest: " << RegDest << endl;
    }
    else if (InstrType == 8)
    {
        /* Load */
        RegDest = Instr >> 8;
        RegDest = RegDest & 0b00000000000000000000000011111111;
        cout << "RegDest: " << RegDest << endl;
        RegAddrMemory = Instr >> 16;
        RegAddrMemory = RegAddrMemory & 0b00000000000000001111111111111111;
        cout << "RegAddrMemory: " << RegAddrMemory << endl;
    }
    else if (InstrType == 9)
    {
        /* Store */
        RegSourceA = Instr >> 8;
        RegSourceA = RegSourceA & 0b00000000000000000000000011111111;
        cout << "RegSourceA: " << RegSourceA << endl;
        RegAddrMemory = Instr >> 16;
        RegAddrMemory = RegAddrMemory & 0b00000000000000001111111111111111;
        cout << "RegAddrMemory: " << RegAddrMemory << endl;
    }
}

void execute(void)
{
    if (InstrType == 1)
    {
        // Soma
        Reg[RegDest] = Reg[RegSourceA] + Reg[RegSourceB];
    }
    else if (InstrType == 3)
    {
        // Subtracao
        Reg[RegDest] = Reg[RegSourceA] - Reg[RegSourceB];
    }
    else if (InstrType == 8)
    {
        // Load
        Reg[RegDest] = DataMemory[RegAddrMemory];
    }
    else if (InstrType == 9)
    {
        // Store
        DataMemory[RegAddrMemory] = Reg[RegSourceA];
    }
}
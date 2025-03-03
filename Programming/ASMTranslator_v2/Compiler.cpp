#include "Compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/*******************************************************************************************************
********************************************************************************************************
********************************************************************************************************
********************************************************************************************************
*******************************************************************************************************/

char** CreateArrayOfStrings ( const char* FileName, char** FileBuffer )
{
    FILE* File = fopen ( FileName, "rb" );
	//ASSERT ( File );
	if ( File == NULL ) 
		return NULL;
	
	fseek ( File, 0, SEEK_END );
    uint32_t FileLength = ftell ( File );
	rewind ( File );

    *FileBuffer = ( char* ) calloc ( FileLength + 1, sizeof ( **FileBuffer ) );
    ASSERT ( *FileBuffer );

    fread ( *FileBuffer, sizeof ( **FileBuffer ), FileLength, File );
    fclose ( File );

	//-
    uint32_t FileLines = CountLines ( *FileBuffer, FileLength ) ;
    char** Strings = ( char** ) calloc ( FileLines + 1, sizeof ( Strings ) );
    ASSERT ( Strings );
	
    FillArrayOfAddressOnString ( Strings, *FileBuffer, FileLines );

	Strings[FileLines] = NULL;

    return Strings;
}

/*******************************************************************************************************
********************************************************************************************************
********************************************************************************************************
********************************************************************************************************
*******************************************************************************************************/

uint32_t CountLines ( char* FileBuffer, uint32_t Length )
{
    ASSERT ( FileBuffer != 0 );
    ASSERT ( Length != 0 );
    uint32_t n = 0;
    uint32_t i = 0;
    uint32_t LastEnter = 0;
    for ( i = 0; FileBuffer[i] != '\0'; i++ )
    {
        ASSERT ( i >= 0 && i < Length );
        if ( FileBuffer[i] == '\n' )
        {
            n++;
            LastEnter = i;
        }
    }
    if ( i - LastEnter > 1)
        n++;
    return n;
}

/*******************************************************************************************************
********************************************************************************************************
********************************************************************************************************
********************************************************************************************************
*******************************************************************************************************/

uint32_t FillArrayOfAddressOnString ( char** Strings, char* FileBuffer, uint32_t Lines )
{
    ASSERT ( Strings != NULL );
    ASSERT ( FileBuffer != NULL );
    ASSERT ( Lines != 0 );
    uint32_t NumberString = 1;
    char* OldPositionInFile = NULL;
    char* NewPositionInFile = FileBuffer;
    Strings[0] = FileBuffer;


    while ( NumberString < Lines )
    {
        OldPositionInFile = NewPositionInFile;

        NewPositionInFile = ( strchr ( OldPositionInFile, '\n' ) + 1 );
        ASSERT ( NewPositionInFile != NULL );
        *( NewPositionInFile - 1 ) = '\0';

        ASSERT ( NumberString >= 0 && NumberString < Lines );
        Strings[NumberString] = NewPositionInFile;

        NumberString++;
    }

    return OK;
}

/*******************************************************************************************************
********************************************************************************************************
********************************************************************************************************
********************************************************************************************************
*******************************************************************************************************/

uint32_t GetNumberLabel ( char** SourceCode )
{
    ASSERT ( SourceCode );
    uint32_t LabelCounter = 0;
    uint32_t i = 0;
    while ( SourceCode[i] != NULL )
    {
        if ( IsAlpha ( *( SourceCode[i] ) ) == OK || IsDigit ( *( SourceCode[i] ) ) == OK )
        {
            uint32_t j = 0;
            while ( *( SourceCode[i] + j ) != '\0' )
            {
                if ( *( SourceCode[i] + j ) == ':' )
                {
                    LabelCounter++;
                    break;
                }
                j++;
            }
        }
        i++;
    }
    return LabelCounter;
}

/*******************************************************************************************************
********************************************************************************************************
********************************************************************************************************
********************************************************************************************************
*******************************************************************************************************/

Label* CreateAndFillLabelTable ( char** SourceCode, Command* CommandsTable, uint32_t* ProgramSize )
{
    ASSERT ( SourceCode );
    ASSERT ( CommandsTable );
    uint32_t NumberLabel = GetNumberLabel ( SourceCode );
    Label* LabelTable = ( Label* ) calloc ( NumberLabel + 1, sizeof ( Label ) );
    ASSERT ( LabelTable );
    //====================
    uint32_t i = 0;
    uint32_t LabelCounter = 0;
    while ( SourceCode[i] != NULL )
    {
        char* NameLabel = GetLabelName ( SourceCode[i] );

        //Mistake in name of label
        if ( NameLabel == ( char* )( NULL + 1 ) )
        {
            printf( "   === Invalid name of label on line %d ===", i + 1 );
            errno = EINVAL;
            return NULL;
        }

        //It is right label
        if ( NameLabel != NULL )
        {
            LabelTable[LabelCounter].NameLabel = NameLabel;
            LabelTable[LabelCounter].ProgramAddress = *ProgramSize;
            LabelCounter++;
        }
        //It is command or empty string
        else
        {
            uint32_t j = 0;
            char Command[MaxLengthCommands] = {};

            while ( IsSpace ( *( SourceCode[i] + j ) ) == OK ) j++;

            if ( IsAlpha ( *( SourceCode[i] + j ) ) != OK )
            {
                if  ( *( SourceCode[i] + j ) == '\0' ||
                      *( SourceCode[i] + j ) == '\r' ||
                      *( SourceCode[i] + j ) == '\n' )
                {
                    i++;
                    continue;
                }
                printf( "   === Invalid command on line %d  ===", i + 1 );
                errno = EINVAL;
                return NULL;
            }

            if ( sscanf ( SourceCode[i] + j, "%[A-Z]", Command ) == 0 )
            {
                printf( "   === Invalid command on line %d  ===", i + 1 );
                errno = EINVAL;
                return NULL;
            } else
            {
                uint8_t ComSize = 0;
                if ( GetCommandCodeAndSize ( CommandsTable, Command, &ComSize ) == Error )
                {
                    printf( "   === This command on line %d not found ===", i + 1 );
                    errno = EINVAL;
                    return NULL;
                }
                *ProgramSize += ComSize;
            }
        }
        i++;
    }
    //*ProgramSize += SignatureSize; // Service information
    return LabelTable;
}

/*******************************************************************************************************
********************************************************************************************************
********************************************************************************************************
********************************************************************************************************
*******************************************************************************************************/

Command* CreateCommandsTable ( void )
{
    Command* CommandsTable = ( Command* ) calloc ( MaxCommands, sizeof ( Command ) );
    ASSERT ( CommandsTable );
    CommandsTable[0].Mnemonic  = "ADD" ;     CommandsTable[0].Size = 3;
    CommandsTable[1].Mnemonic  = "";			
    CommandsTable[2].Mnemonic  = "SUB" ;     CommandsTable[2].Size = 3;
    CommandsTable[3].Mnemonic  = "";
    CommandsTable[4].Mnemonic  = "DIV" ;     CommandsTable[4].Size = 3;
    CommandsTable[5].Mnemonic  = "";
    CommandsTable[6].Mnemonic  = "MUL" ;     CommandsTable[6].Size = 3;
    CommandsTable[7].Mnemonic  = "";
    CommandsTable[8].Mnemonic  = "MOV" ;     CommandsTable[8].Size = 3;
    CommandsTable[9].Mnemonic  = "";
    CommandsTable[10].Mnemonic = "CMP" ;     CommandsTable[10].Size = 3;
    CommandsTable[11].Mnemonic  = "";

    CommandsTable[12].Mnemonic = "POP" ;     CommandsTable[12].Size = 2;
    CommandsTable[13].Mnemonic = "PUSH";     CommandsTable[13].Size = 2;

    CommandsTable[14].Mnemonic = "CALL";     CommandsTable[14].Size = 2;
    CommandsTable[15].Mnemonic = "JP"  ;     CommandsTable[15].Size = 2;
    CommandsTable[16].Mnemonic = "JA"  ;     CommandsTable[16].Size = 2;
    CommandsTable[17].Mnemonic = "JB"  ;     CommandsTable[17].Size = 2;
    CommandsTable[18].Mnemonic = "JAE" ;     CommandsTable[18].Size = 2;
    CommandsTable[19].Mnemonic = "JBE" ;     CommandsTable[19].Size = 2;
    CommandsTable[20].Mnemonic = "JE"  ;     CommandsTable[20].Size = 2;
    CommandsTable[21].Mnemonic = "JZ"  ;     CommandsTable[21].Size = 2;
    CommandsTable[22].Mnemonic = "JNZ" ;     CommandsTable[22].Size = 2;

    CommandsTable[23].Mnemonic = "RET" ;     CommandsTable[23].Size = 1;

    CommandsTable[24].Mnemonic = "OUT" ;     CommandsTable[24].Size = 2;
    CommandsTable[25].Mnemonic = "IN"  ;     CommandsTable[25].Size = 2;

    CommandsTable[26].Mnemonic = "CLR" ;     CommandsTable[26].Size = 2;
    CommandsTable[27].Mnemonic = "DEC" ;     CommandsTable[27].Size = 2;
    CommandsTable[28].Mnemonic = "INC" ;     CommandsTable[28].Size = 2;
    CommandsTable[29].Mnemonic = "NOP" ;     CommandsTable[29].Size = 1;
    CommandsTable[30].Mnemonic = "END" ;     CommandsTable[30].Size = 1;
    return CommandsTable;
}

/*******************************************************************************************************
********************************************************************************************************
********************************************************************************************************
********************************************************************************************************
*******************************************************************************************************/

uint32_t GetCommandCodeAndSize ( Command* CommandsTable, char* Command, uint8_t* Size )
{
    ASSERT ( CommandsTable );
    ASSERT ( Command );
    for ( uint32_t i = 0; i < MaxCommands; i++ )
    {
        if ( strcmp ( CommandsTable[i].Mnemonic, Command ) == 0 && Command[0] != '\0' )
        {
            *Size = CommandsTable[i].Size;
            return i;
        }
    }
    return Error;
}

/*******************************************************************************************************
********************************************************************************************************
********************************************************************************************************
********************************************************************************************************
*******************************************************************************************************/

MemoryType* CreateCodeMachine ( char** SourceCode, Label* LabelTable, Command* CommandsTable, uint32_t ProgramSize )
{
    ASSERT ( SourceCode );
    ASSERT ( LabelTable );
    ASSERT ( CommandsTable );
    ASSERT ( ProgramSize );
    MemoryType* MachineCode = ( MemoryType* ) calloc ( ProgramSize + SignatureSize, sizeof ( MemoryType ) );
    ASSERT ( MachineCode );

    MachineCode[0] = 'E';
    MachineCode[1] = 'A';
    MachineCode[2] = 'S';
    MachineCode[3] = 'M';
    MachineCode[4] = 'D';
    MachineCode[5] = 'V';
    MachineCode[6] = '1';
    MachineCode[7] = '.';
    MachineCode[8] = '1';
    MachineCode[9] = ProgramSize;

    uint32_t ProgramCounter = SignatureSize - 1;
    uint32_t ProgramLine = 0;
	
    while ( SourceCode[ProgramLine] != NULL )
    {
        uint32_t j = 0;
        uint8_t ComSize = 0;

        if ( GetLabelName ( SourceCode[ProgramLine] ) == NULL )
        {
            while ( IsSpace ( *( SourceCode[ProgramLine] + j ) ) == OK ) j++;
            char Command[MaxLengthCommands] = {};

            if ( sscanf ( SourceCode[ProgramLine] + j, "%[A-Z]", Command ) == 0 )
            {
                ProgramLine++;
                continue;
            }
            MemoryType ReturnData = GetCommandCodeAndSize ( CommandsTable, Command, &ComSize );
            ProgramCounter++;
            MachineCode[ProgramCounter] = ReturnData;
            while ( IsSpace ( *( SourceCode[ProgramLine] + j ) ) != OK &&
                    *( SourceCode[ProgramLine] + j ) != '\0' ) j++;
        } else
        {
            ProgramLine++;
            continue;
        }

        /**************************************************************************************************/
        /**************************************************************************************************/
        if ( ComSize != 1 && ( MachineCode[ProgramCounter] > JNZ ||
                               MachineCode[ProgramCounter] < CALL ) )
        {
            uint32_t NumReg = GetFirstArg ( SourceCode[ProgramLine], &j, RegArg );
            if ( NumReg == Error )
            {
                printf ( "  === Error first argument on line: %d ===", ProgramLine + 1 );
                errno = EINVAL;
                return NULL;
            } else
            {
                ProgramCounter++;
                MachineCode[ProgramCounter] = NumReg;
            }
            if ( ComSize == 2 )
            {
                ProgramLine++;
                continue;
            }
        }

        /**************************************************************************************************/
        /**************************************************************************************************/
        if ( ComSize == 3 )
        {
            uint32_t StartPos = j;
            uint32_t SecArgReg = GetSecondArg ( SourceCode[ProgramLine], &j, RegArg );

            j = StartPos;
            uint32_t SecArgNum = GetSecondArg ( SourceCode[ProgramLine], &j, NumArg );
            if ( SecArgReg == Error && SecArgNum == Error && errno != 0 )
            {
                printf ( "  === Error second argument on line: %d ===", ProgramLine + 1 );
                errno = EINVAL;
                return NULL;
            } else
            {
                if ( SecArgNum == Error && errno != 0 )
                {
                    ProgramCounter++;
                    MachineCode[ProgramCounter] = SecArgReg;
                    errno = 0;
                }
                if ( SecArgReg == Error )
                {
                    ProgramCounter++;
                    MachineCode[ProgramCounter] = SecArgNum;
                    MachineCode[ProgramCounter - 2]++;
                }
            }
        }

        /**************************************************************************************************/
        /**************************************************************************************************/
        if ( ComSize == 2 && MachineCode[ProgramCounter] <= JNZ &&
                             MachineCode[ProgramCounter] >= CALL )
        {
            uint64_t LabelName = GetFirstArg ( SourceCode[ProgramLine], &j, LblArg );
            if ( LabelName == Error )
            {
                printf ( "  === Error name of label on line: %d ===", ProgramLine + 1 );
                errno = EINVAL;
                return NULL;
            }

            uint32_t AddressLabel = GetAddressLabel ( LabelTable, ( char* )LabelName );
            if ( AddressLabel == Error )
            {
                printf ( "  === Not found label on line %d ===", ProgramLine + 1 );
                errno = EINVAL;
                return NULL;
            } else
            {
                ProgramCounter++;
                MachineCode[ProgramCounter] = AddressLabel;
            }
        }
        ProgramLine++;
    }
    return MachineCode;
}

/*******************************************************************************************************
********************************************************************************************************
********************************************************************************************************
********************************************************************************************************
*******************************************************************************************************/

uint32_t GetAddressLabel ( Label* LabelTable, char* StringLabel )
{
    ASSERT ( LabelTable );
    ASSERT ( StringLabel );
    uint32_t i = 0;
    while ( LabelTable[i].NameLabel != NULL )
    {
        uint32_t j = 0;
        while ( *( LabelTable[i].NameLabel + j ) != ':' ) j++;
        *( LabelTable[i].NameLabel + j ) = '\0';
        if ( strcmp ( LabelTable[i].NameLabel, StringLabel ) == 0 )
        {
            *( LabelTable[i].NameLabel + j ) = ':';
            return LabelTable[i].ProgramAddress;
        }
        *( LabelTable[i].NameLabel + j ) = ':';
        i++;
    }
    return Error;
}

/*******************************************************************************************************
********************************************************************************************************
********************************************************************************************************
********************************************************************************************************
*******************************************************************************************************/

uint32_t PrintBinFile ( MemoryType* MachineCode, char* argv, uint32_t SizeFile )
{
    uint32_t i = 0;
    while ( argv[i] != '\0' ) i++;

    if  ( strcmp ( argv + i - LengthFileExtension, ".asm" ) != 0 )
    {
        printf( "   === Invalid inputting file extension! ===   " );
        return Error;
    }

    argv = strcpy ( argv + i - LengthFileExtension, DefaultOutputFile );

    FILE* File = fopen ( argv + LengthFileExtension - i, "wb" );
    ASSERT ( File );

    fwrite ( MachineCode, sizeof ( MemoryType ), SizeFile + SignatureSize, File ); //!!!!!
    fclose ( File );
    return OK;
}

/*******************************************************************************************************
********************************************************************************************************
********************************************************************************************************
********************************************************************************************************
*******************************************************************************************************/
uint32_t IsAlpha ( char Symbol )
{
    if ( ( 'A' <= Symbol && Symbol <= 'Z' ) || ( 'a' <= Symbol  && Symbol <= 'z' ) )
        return OK;
    else
        return Error;
}
/*******************************************************************************************************
********************************************************************************************************
********************************************************************************************************
********************************************************************************************************
*******************************************************************************************************/
uint32_t IsSpace ( char Symbol )
{
    char ASCIICodeSpace = 32;
    char ASCIICodeTab   =  9;
    if ( Symbol == ASCIICodeSpace || Symbol == ASCIICodeTab )
        return OK;
    else
        return Error;
}
/*******************************************************************************************************
********************************************************************************************************
********************************************************************************************************
********************************************************************************************************
*******************************************************************************************************/
uint32_t IsDigit ( char Symbol )
{
    if ( '1' <= Symbol && Symbol <= '9' )
        return OK;
    else
        return Error;
}
/*******************************************************************************************************
********************************************************************************************************
********************************************************************************************************
********************************************************************************************************
*******************************************************************************************************/
uint64_t GetFirstArg ( char* String, uint32_t* Position, uint8_t TypeArg )
{
    while ( IsSpace ( *( String + (*Position) ) ) == OK ) (*Position)++;

    switch ( TypeArg )
    {
    case RegArg:
    {
        uint32_t RegNum = 0;
        if ( sscanf ( String + (*Position), "R%u", &RegNum ) == 0 )
            return Error;

        if ( RegNum > MaxNumReg )
            return Error;

        if ( RegNum / 10 == 0 )
            (*Position) += 2;
        else
            (*Position) += 3;

        return RegNum;
    }
    case LblArg:
    {
        char* LabelBuf = String + (*Position);
        if ( sscanf ( String + (*Position), "%[A-z0-9]", LabelBuf ) == 0 )
            return Error;

        return ( uint64_t )LabelBuf;
    }
    }
    return Error;
}
/*******************************************************************************************************
********************************************************************************************************
********************************************************************************************************
********************************************************************************************************
*******************************************************************************************************/
uint32_t GetSecondArg ( char* String, uint32_t* Position, uint8_t TypeArg )
{
    while ( IsSpace ( *( String + (*Position) ) ) == OK ) (*Position)++;

    if ( *( String + (*Position) ) != ',' )
        return Error;
    else
        (*Position)++;
	
    while ( IsSpace ( *( String + (*Position) ) ) == OK ) (*Position)++;

    switch ( TypeArg )
    {
    case NumArg:
    {
        uint32_t Num = 0;
		if ( sscanf ( String + (*Position), "%i", &Num ) == 0 )
		{
			errno = EINVAL;
			return Error;
		} 
        return Num;
    }
    case RegArg:
    {
        uint8_t RegNum = 0;
        if ( sscanf ( String + (*Position), "R%u", &RegNum ) == 0 )
            return Error;

        if ( RegNum > MaxNumReg )
            return Error;

        if ( RegNum / 10 == 0 )
            (*Position) += 2;
        else
            (*Position) += 3;

        return RegNum;
    }
    }
    return Error;
}
/*******************************************************************************************************
********************************************************************************************************
********************************************************************************************************
********************************************************************************************************
*******************************************************************************************************/
char* GetLabelName ( char* String )
{
    if ( IsSpace ( *String ) == OK || *String == '\n' || *String == '\r' || *String == '\0' )
        return NULL;

    else
    {
        char* LblBuf = String;
        if ( sscanf ( String, "%s:", LblBuf ) == 0 )
            return ( char* )( NULL + 1 );
        else
        {
			uint32_t i = 0;
			while ( *( LblBuf + i ) != ':' )
            {
                if ( *( LblBuf + i ) == '\0' )
                     return ( char* )( NULL + 1 );
                i++;
            }
			*( LblBuf + i + 1 ) = '\0';
            return LblBuf;
        }
    }
    return NULL;
}
/*******************************************************************************************************
********************************************************************************************************
********************************************************************************************************
********************************************************************************************************
*******************************************************************************************************/

#pragma warning(disable:4996)

#include "utils.hpp"

int main ( )
{
	/* cool data object :dab: */
	data inject;
	ZeroMemory ( &inject, sizeof ( data ) );

	/* set all strings */
	strcpy ( inject.messageCh, "Hello, from target process!" );
	strcpy ( inject.titleCh, "Hakery.club" );
	strcpy ( inject.jvmCh, "jvm.dll" );
	strcpy ( inject.createdVMsCh, "JNI_GetCreatedJavaVMs" );

	/* load user32 so we can get access to messagebox*/
	auto user32 = LoadLibrary ( "user32.dll" );
	inject.paMessageBoxA = ( qword ) GetProcAddress ( user32, "MessageBoxA" );

	/* 
	   kernel is already loaded
	*/
	auto kernel32 = GetModuleHandleA ( "kernel32.dll" );
	inject.paGetModuleHandleA = ( qword ) GetProcAddress ( kernel32, "GetModuleHandleA" );
	inject.paGetProcAddress = ( qword ) GetProcAddress ( kernel32, "GetProcAddress" );

	/* free lib loaded */
	FreeLibrary ( user32 );

	auto pid = FindProcessIdByName ( "javaw.exe" );

	if ( pid )
	{

		/* open handle to process */
		auto hProcess = OpenProcess ( PROCESS_ALL_ACCESS, FALSE, pid );
		LPVOID remote = VirtualAllocEx ( hProcess, NULL, sizeof ( data ), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE );

		/* write our function to the process*/
		WriteProcessMemory ( hProcess, remote, ( LPVOID ) threads::InjectThread, sizeof ( data ), 0 );

		/* allocate memory for our data */
		data *pData = (data*)VirtualAllocEx ( hProcess, NULL, sizeof ( data ), MEM_COMMIT, PAGE_READWRITE );
		
		/* write data to the process */
		WriteProcessMemory ( hProcess, pData, &inject, sizeof ( data ), NULL );

		/* spawn thread in the process */
		auto thread = CreateRemoteThread ( hProcess, 0, 0, ( LPTHREAD_START_ROUTINE ) remote, pData, 0, 0 );

		/* wait for thread to finish */
		WaitForSingleObject ( thread, -1 );

		/* close thread handle */
		CloseHandle ( thread );

		/* set memory we allocated to 0's */
		WriteProcessMemory ( hProcess, remote, 0, sizeof ( data ), 0 );
		WriteProcessMemory ( hProcess, pData, 0, sizeof ( data ), 0 );

		/* close the handle to the process*/
		CloseHandle ( hProcess );
		std::cout << "Executed succesfully, data -> " << std::endl;
		std::cout << "Mem. of Function: 0x" << std::hex << remote << std::endl;
		std::cout << "Mem. of Data:     0x" << std::hex << (void*)pData << std::endl;
		std::cin.get ( );
	}
	else
	{
		std::cout << "Couldn't find Minecraft(?), are you sure it is running?" << std::endl;
		std::cin.get ( );
	}	
	return 0;
}
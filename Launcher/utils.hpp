#pragma once

#include <Windows.h>
#include <TlHelp32.h>

#include <iostream>
#include <string>

#include <jvmti.h>
#include <jni.h>


/* get all templates ready */
using qword = uint64_t;
using messageboxa_t = int ( __stdcall* ) ( HWND, LPCSTR, LPCSTR, UINT );
using get_module_handle_t = HMODULE ( __stdcall* ) ( LPCSTR );
using get_created_vms_t = jint ( JNICALL* )( JavaVM**, jsize, jsize* );
using get_proc_addr_t = FARPROC ( __stdcall* ) ( HMODULE, LPCSTR );

/* could also just set the strings here but idk */
class data {
public:
	char messageCh [ 256 ]; //here we store it's message!
	char titleCh [ 256 ]; //here we will store our messagebox's title
	char jvmCh [ 256 ]; //jvm.dll
	char createdVMsCh [ 256 ]; //JNI_GetCreatedJavaVMs

	qword paMessageBoxA; //pa = Procedure address in memory
	qword paGetModuleHandleA;
	qword paGetProcAddress;

	JavaVM *jvm;
	JNIEnv *jniEnv;
};

qword FindProcessIdByName ( std::string name )
{
	PROCESSENTRY32 pEntry;
	HANDLE hSnapshot;

	pEntry.dwSize = sizeof ( PROCESSENTRY32 );

	hSnapshot = CreateToolhelp32Snapshot ( TH32CS_SNAPPROCESS, NULL );
	if ( Process32First ( hSnapshot, &pEntry ) )
	{
		do {
			if ( std::string ( pEntry.szExeFile ).find ( name ) != std::string::npos )
				return pEntry.th32ProcessID;
		} while ( Process32Next ( hSnapshot, &pEntry ) );
	}

	return 0;
}
/* this is where we store all threads / functions that could be injected into the target process
   NOTE: You cannot inject from x64 -> x86 or from x86 -> x64
*/
namespace threads {

	qword __stdcall InjectThread ( data *cData )
	{
		messageboxa_t msgbox = ( messageboxa_t ) cData->paMessageBoxA;
		get_module_handle_t get_module_handle = ( get_module_handle_t ) cData->paGetModuleHandleA;
		get_proc_addr_t get_proc_addr = ( get_proc_addr_t ) cData->paGetProcAddress;

		HMODULE jvm;

		if ( ( jvm = get_module_handle ( cData->jvmCh ) ) != NULL )
		{
			get_created_vms_t get_created_vms = ( get_created_vms_t ) get_proc_addr ( jvm, cData->createdVMsCh );
			if ( get_created_vms ( &cData->jvm, 1, NULL ) == JNI_OK ) {
				if ( cData->jvm->AttachCurrentThread ( ( void** ) &cData->jniEnv, NULL ) == JNI_OK )
				{
					msgbox ( 0, cData->messageCh, cData->titleCh, MB_ICONINFORMATION );
				}
				else
				{
					return EXIT_FAILURE;
				}
			}
			else
			{
				return EXIT_FAILURE;
			}
		}
		else
		{
			return EXIT_FAILURE;
		}

		if ( cData->jvm )
			cData->jvm->DetachCurrentThread ( );
		return EXIT_SUCCESS;
	}

}
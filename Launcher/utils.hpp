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
	char nonForgeCh [ 256 ]; //here we store it's message!
	char titleCh [ 256 ]; //here we will store our messagebox's title
	char successCh [ 256 ]; //success message
	char jvmCh [ 256 ]; //jvm.dll
	char createdVMsCh [ 256 ]; //JNI_GetCreatedJavaVMs

	/* jni shit */
	char launchWrapperClass [ 256 ]; //net/minecraft/launchwrapper/Launch
	char launchClassLoader [ 256 ]; //net/minecraft/launchwrapper/LaunchClassLoader
	char chClassLoader [ 256 ]; //classLoader
	char chClassLoaderLaunch [ 256 ]; //Lnet/minecraft/launchwrapper/LaunchClassLoader;
	char entry [ 256 ]; //entry
	char voidd [ 256 ]; //()V
	char init [ 256 ]; //<init>
	char javaIoFile [ 256 ]; //java/io/File
	char strVoid [ 256 ]; //(Ljava/lang/String;)V
	char fileToURI [ 256 ]; //()Ljava/net/URI;
	char fileToURL [ 256 ]; // ()Ljava/net/URL;;
	char javaNetUri [ 256 ]; //java/net/URI
	char urlClassLoader [ 256 ]; //java/net/URLClassLoader
	char urlClassLoaderInit [ 256 ]; //([Ljava/net/URL;Ljava/lang/ClassLoader;)V
	char loadClassParams [ 256 ]; //(Ljava/lang/String;)Ljava/lang/Class;
	char toURI [ 256 ];
	char loadClass [ 256 ];
	char javaNetUrl [ 256 ];
	char toURL [ 256 ];
	char mainClass [ 256 ];


	char pathToJar [ 256 ]; //path to the jar to be loaded

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
					auto launchwrapperclass = cData->jniEnv->FindClass ( cData->launchWrapperClass );
					if ( launchwrapperclass )
					{

						/* starting from here won't be my code! */

						jclass clsFile;
						jmethodID midFileConst;
						jmethodID midFileToURI;

						jclass clsURLClsLoader;
						jmethodID midURLClsLoaderConst;
						jmethodID midLoadClass;

						jclass clsURL;
						jclass clsURI;
						jmethodID midURItoURL;

						jclass clsNmlLaunch;
						jclass clsNmlLaunchClsLoader;
						jobject objNmlClassLoader;

						jobject obj;
						jobject obj2;
						jstring str;

						jclass clsVapeEntryPoint;
						jmethodID midVapeEntryPoint;

						cData->jniEnv->PushLocalFrame ( 100 );

						clsFile =  cData->jniEnv->FindClass ( cData->javaIoFile);
						midFileConst = cData->jniEnv->GetMethodID ( clsFile, cData->init, cData->strVoid );
						midFileToURI = cData->jniEnv->GetMethodID ( clsFile, cData->toURI, cData->fileToURI );

						if ( cData->jniEnv->ExceptionCheck ( ) )
						{
							cData->jniEnv->ExceptionDescribe ( );
						}

						clsURLClsLoader = cData->jniEnv->FindClass ( cData->urlClassLoader );
						midURLClsLoaderConst = cData->jniEnv->GetMethodID ( clsURLClsLoader, cData->init, cData->urlClassLoaderInit );
						midLoadClass = cData->jniEnv->GetMethodID ( clsURLClsLoader, cData->loadClass, cData->loadClassParams );

						if ( cData->jniEnv->ExceptionCheck ( ) )
						{
							cData->jniEnv->ExceptionDescribe ( );
							msgbox ( 0, cData->successCh, cData->titleCh, 0 );
						}

						clsURL = cData->jniEnv->FindClass ( cData->javaNetUrl );
						clsURI = cData->jniEnv->FindClass ( cData->javaNetUri );
						midURItoURL = cData->jniEnv->GetMethodID ( clsURI, cData->toURL, cData->fileToURL );

						if ( cData->jniEnv->ExceptionCheck ( ) )
						{
							cData->jniEnv->ExceptionDescribe ( );
							msgbox ( 0, cData->successCh, cData->titleCh, 0 );
						}

						clsNmlLaunch = (jclass)cData->jniEnv->NewGlobalRef ( cData->jniEnv->FindClass ( cData->launchWrapperClass ) );
						//clsNmlLaunchClsLoader = (jclass) cData->jniEnv->NewGlobalRef ( cData->jniEnv->FindClass ( cData->launchClassLoader ) );
						objNmlClassLoader = cData->jniEnv->NewGlobalRef ( cData->jniEnv->GetStaticObjectField ( ( jclass ) clsNmlLaunch, cData->jniEnv->GetStaticFieldID ( ( jclass ) clsNmlLaunch, cData->chClassLoader, cData->chClassLoaderLaunch ) ) );

						if ( cData->jniEnv->ExceptionCheck ( ) )
						{
							cData->jniEnv->ExceptionDescribe ( );
							msgbox ( 0, cData->successCh, cData->titleCh, 0 );
						}

						jstring coolString = cData->jniEnv->NewStringUTF ( (char*) cData->pathToJar );

						// Create new java.io.File object with jar path as only argument
						// Folowed by converting the file to URI
						// Folowed by converting the URI to URL
						obj = cData->jniEnv->functions->NewObject (cData->jniEnv, clsFile, midFileConst, coolString );
						
						cData->jniEnv->DeleteLocalRef ( coolString );
						obj2 = cData->jniEnv->functions->CallObjectMethod (cData->jniEnv, obj, midFileToURI );
						obj = cData->jniEnv->functions->CallObjectMethod ( cData->jniEnv, obj2, midURItoURL );

						
						// Wrap URL object in an array and construct a new URLClassLoader obj
						obj2 = cData->jniEnv->functions->NewObjectArray ( cData->jniEnv, 1, clsURL, obj );
						obj = cData->jniEnv->functions->NewObject ( cData->jniEnv, clsURLClsLoader, midURLClsLoaderConst, obj2, objNmlClassLoader );
					
						
						// Create a new String obj with path to Vape's (OUR) entry point class
						// followed by calling loadClass on the above created URLClassLoader obj
						coolString = cData->jniEnv->functions->NewStringUTF ( cData->jniEnv, cData->mainClass );
						obj2 = cData->jniEnv->functions->CallObjectMethod ( cData->jniEnv, obj, midLoadClass, coolString );
						cData->jniEnv->functions->DeleteLocalRef ( cData->jniEnv, obj );
						clsVapeEntryPoint = (jclass)cData->jniEnv->functions->NewGlobalRef ( cData->jniEnv, obj2 );
						cData->jniEnv->functions->DeleteLocalRef ( cData->jniEnv, obj2 );
						cData->jniEnv->functions->DeleteLocalRef ( cData->jniEnv, coolString );
					
						midVapeEntryPoint = cData->jniEnv->functions->GetStaticMethodID ( cData->jniEnv, (jclass) clsVapeEntryPoint, cData->entry, cData->voidd );

						cData->jniEnv->functions->PopLocalFrame ( cData->jniEnv, 0 );
						
						cData->jniEnv->functions->CallStaticVoidMethod ( cData->jniEnv, (jclass) clsVapeEntryPoint, midVapeEntryPoint );
					}
					else
					{
						msgbox ( 0, cData->nonForgeCh, cData->titleCh, MB_ICONERROR );
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
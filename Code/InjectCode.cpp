//*************************************************************************
//  
//  Copyright (c) all 2014  All rights reserved
//  D a t e  : 2014.9.11
//  ��  �� : 
//  ��  �� : 0.1
//  ��  �� : ��ָ����dllע�뵽Ŀ�����
//  ˵  �� : 
//  ��  ע :
//
//  �޸ļ�¼:
//  ��   ��       �汾    �޸���              �޸�����
// 2014/9/11 0.1      EvilKnight        ����
// 2014/9/12 0.1      EvilKnight        ʵ��Inject by ProcessName����
//  YYYY/MM/DD    X.Y     <���߻��޸�����>    <�޸�����>
//
//*************************************************************************
#include "InjectCode.h"
#include "EnvironmentInformation.h"
#include "ErrorInformation.h"
#include "ProcessInformation.h"
#include <tchar.h>


/*******************************************************************************
*
*   �� �� �� : Inject
*  �������� : ��dllע�뵽ָ������
*  �����б� : dwPID          --             Ŀ�����ID
*                   pDllPath      --             ָ��Ҫע���dll
*   ˵      �� : 
*  ���ؽ�� :  ����ɹ�������TRUE��ʧ�ܷ���FALSE
*
*******************************************************************************/
BOOL Inject(__in CONST DWORD dwPID, 
                        __in_z CONST PTCHAR pDllPath)
{
        BOOL bResult(FALSE) ;
        HANDLE hProcess(NULL),
                       hThread(NULL);

        SIZE_T uSize(0) ;
        LPVOID   pAddr(NULL) ;

        if (0 == dwPID || NULL == pDllPath)
        {
                OutputDebugString(TEXT("Inject argv error!\r\n")) ;
                return FALSE ;
        }

        __try
        {
                ULONG   uMySelfBit(0),
                                uDestProcessBit(0),
                                uDestDllBit(0) ;

                // ��ȡ��Dll��λ������Ϊ������32λ���̻���64λ���̣�����Ҫȡ��
                uDestDllBit = GetPEFileBit(pDllPath) ;
                if (0 == uDestDllBit)
                {
                        OutputDebugString(TEXT("Inject::GetPEFileBit failed")) ;
                        __leave ;
                }

                // ����Ҫ�ж�ϵͳ�����������Ŀ������Լ�dll��λ��
                if (MACHINE64 == GetOSBit())
                {
                        uDestProcessBit = GetProcessBit(dwPID) ;
                        uMySelfBit = GetMyselfBit() ;
                        // �������Ŀ����̡���dllλ��Ҫһ��
                        if (uMySelfBit != uDestProcessBit
                                || uDestProcessBit != uDestDllBit)
                        {
                                OutputDebugString(TEXT("Process do not match the bits!\r\n")) ;
                                __leave ;
                        }
                }
                else
                {
                        if (MACHINE32 != uDestDllBit)
                        {
                                OutputDebugString(TEXT("Process do not match the bits!\r\n")) ;
                                __leave ;
                        }
                }

                // ��Ŀ�����
                hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_WRITE | PROCESS_VM_OPERATION,
                                                        FALSE,
                                                        dwPID) ;
                if (NULL == hProcess)
                {
                        OutputErrorInformation(TEXT("Inject"), TEXT("OpenProcess")) ;
                        __leave ;
                }

                // ������Ҫ��Ŀ�����д��dll·��������ڴ��С
                uSize = (_tcslen(pDllPath) + 1) * sizeof(TCHAR) ;

                if (0 == uSize)
                {
                        OutputDebugString(TEXT("Inject uSize can't zero!\r\n")) ;
                        __leave ;
                }
                // ��Ŀ����������ڴ�
                pAddr = VirtualAllocEx(hProcess, NULL, uSize, MEM_COMMIT,PAGE_READWRITE) ;
                if(NULL == pAddr)
                {
                        OutputErrorInformation(TEXT("Inject"), TEXT("VittualAllocEx")) ;
                        __leave ;
                }

                // д��dll·��
                SIZE_T uWritedSize(0) ;
                if (! WriteProcessMemory(hProcess, pAddr, pDllPath, uSize, &uWritedSize)
                        || 0 == uWritedSize)
                {
                        OutputErrorInformation(TEXT("Inject"), TEXT("WriteProcessMemory")) ;
                        __leave ;
                }

                // ����Զ���߳�
                hThread = CreateRemoteThread(hProcess, 
                                                                                        NULL,
                                                                                        0,
                                                                                        (LPTHREAD_START_ROUTINE)LoadLibrary,
                                                                                        pAddr,
                                                                                        0,
                                                                                        NULL) ;

                // �ȴ��߳̽���
                WaitForSingleObject(hThread, INFINITE) ;

                // ������Ի��Dll���صĻ���
                // ����������Ȳ�ȡ��
                // GetExitCodeThread(hThread, ) ;

                bResult = TRUE ;
        }
        __finally
        {
                if (NULL != hThread)
                {
                        CloseHandle(hThread) ;
                        hThread = NULL ;
                }
                if (NULL != pAddr)
                {
                        VirtualFreeEx(hProcess, pAddr, uSize, MEM_DECOMMIT) ;
                        pAddr = NULL ;
                }
                if (NULL != hProcess)
                {
                        CloseHandle(hProcess) ;
                        hProcess = NULL ;
                }
        }
  
        return bResult ;
}

// ��dllע�뵽ָ���������Ľ�����

/*******************************************************************************
*
*   �� �� �� : Inject
*  �������� : ��dllע�뵽ָ���������Ľ�����
*  �����б� : pProcessName     --             Ŀ�������
*                   pDllPath              --             ָ��Ҫע���dll
*   ˵      �� : ע��ʧ�ܣ�����ע����һ��
*  ���ؽ�� :  ���ȫ���ɹ�������TRUE����ʧ�ܷ���FALSE
*
*******************************************************************************/
BOOL Inject(__in_z CONST PTCHAR pProcessName,
            __in_z CONST PTCHAR pDllPath)
{
        ULONG uProcessCount(0) ;
        ULONG  uMemoryLength(0) ;
        BOOL bState(FALSE) ;
        PDWORD pArrayPID(NULL) ;
        const ULONG uFree(5) ;         // ������һЩ�ռ䣬��ֹ������ȡ�ý���id��ʱ��
                                                        // ͻȻ������һЩ���̣�����̫���ڼ��˵��������
                                                        // �Ͳ�������

        if (NULL == pProcessName)
        {
                OutputDebugString(TEXT("Inject:: pProcessName can't NULL! \r\n")) ;
                return FALSE ;
        }

        __try
        {
                uProcessCount = GetProcessListByProcessName(pProcessName, NULL, 0) ;
                // ���һ����ͬ�Ķ�û�У�ֱ���˳��ɣ�
                if (0 == uProcessCount)
                {
                        __leave ;
                }
                
                uMemoryLength = uProcessCount + uFree ;
                pArrayPID = new DWORD[uMemoryLength] ;
                uProcessCount = GetProcessListByProcessName(pProcessName, pArrayPID, uMemoryLength) ;

                // ���ĵ������ж��˰ɣ�Ҫ��Ȼ������ô��������ô���أ����һ���ͬ���ĳ���
                if (uProcessCount > uMemoryLength)
                {
                        __leave ;
                }

                bState = TRUE ;
                // ���ε���Inject��Ŀ�����ע��dll
                for (ULONG uIndex(0); uIndex < uProcessCount; ++ uIndex)
                {
                        // �����һ��ע��ʧ�ܣ��򷵻�״̬Ϊʧ��
                        if (! Inject(pArrayPID[uIndex], pDllPath))
                        {
                                bState = FALSE ;
                        }
                }
        }

        __finally
        {
                if (NULL != pArrayPID)
                {
                        delete [] pArrayPID ;
                        pArrayPID = NULL ;
                }
        }
        
        return bState ;
}

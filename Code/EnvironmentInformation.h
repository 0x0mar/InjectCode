//*************************************************************************
//  
//  Copyright (c) all 2014  All rights reserved
//  D a t e  : 2014.9.11
//  ��  �� : 
//  ��  �� : 0.1
//  ��  �� : ȡ����ص����л�����Ϣ
//  ˵  �� : 
//  ��  ע :
//
//  �޸ļ�¼:
//  ��   ��       �汾    �޸���              �޸�����
//  YYYY/MM/DD    X.Y     <���߻��޸�����>    <�޸�����>
//
//*************************************************************************

#ifndef ENVIRONMENTINFORMATION__H
#define ENVIRONMENTINFORMATION__H

#include <Windows.h>

// ȡ��ָ������λ����32λ����64λ���������ȡ��������ֵ������0
ULONG   GetProcessBit(__in DWORD dwPID) ;

// ȡ�õ�ǰ���̵�λ��
ULONG   GetMyselfBit(VOID) ;

// ȡ�ÿ�ִ���ļ���λ��
ULONG   GetPEFileBit(__in_z PTCHAR pPEFilePath) ;

// ͨ���ڴ��е�PEͷ��ȡ�ó���λ��
ULONG   GetBitByPEHeader(__in_bcount(uSize) PVOID *pPE,
                                                __in ULONG uSize) ;

#endif
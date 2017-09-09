#include <vcl.h>
#include <tlhelp32.h>
#pragma hdrstop

#include "main.h"
#include "frmrange.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

#include <fstream>

using namespace std;

TInterface *Interface;

//---------------------------------------------------------------------------
__fastcall TInterface::TInterface(TComponent* Owner)    : TForm(Owner)
{
    pHandle = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TInterface::btnQuitClick(TObject *Sender)
{
    Application->Terminate();
}
//---------------------------------------------------------------------------
void __fastcall TInterface::CBOpenProcessClick(TObject *Sender)
{
    DWORD pid;
    ListSegments->Clear();

    if(CBOpenProcess->Checked)
    {
        if( PID->Text.Length() == 0 )
        {
            Application->MessageBoxA("No PID Available ?!","ERROR",MB_OK);
            CBOpenProcess->Checked = false;
        }
        else
        {
            MODULEENTRY32 me;
            HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,vProcess[ListProcess->ItemIndex].pid);
            BOOL retval = Module32First(snap,&me);

            if(retval)
            {
                image_base = (DWORD)me.hModule; buf.sprintf("0x%08X",me.hModule);     Imagebase->Caption = buf;
                image_size = me.modBaseSize;    buf.sprintf("0x%08X",me.modBaseSize); Imagesize->Caption = buf;
            }
            else
            {
                image_base = 0; buf.sprintf("0x00000000");  Imagebase->Caption = buf;
                image_size = 0; buf.sprintf("0x00000000");  Imagesize->Caption = buf;

                Application->MessageBoxA("Can Not Create Toolhelp32 Snapshot !","ERROR",MB_OK);
            }

            CloseHandle(snap);

            pid     = Interface->PID->Text.ToInt();
            pHandle = OpenProcess(PROCESS_ALL_ACCESS,false,pid);

            if(pHandle != NULL)
            {

                if(!GetProcessSegments(pHandle)) Application->MessageBoxA("Can Not Find PE Header ?!","WARNING",MB_OK);

                PID->Enabled          = false;  btnDump->Enabled     = true;
                btnDumpRange->Enabled = true;   ListProcess->Enabled = false;
            }
            else
            {
                Application->MessageBoxA("Can Not Open Process !","ERROR",MB_OK);
                CBOpenProcess->Checked = false;
            }
        }
    }
    else
    {
        if(pHandle != NULL)
        {
            CloseHandle(pHandle);
            pHandle = NULL;
        }

        buf.sprintf("0x00000000");
        Imagebase->Caption  = buf;   Imagesize->Caption  = buf;
        N_EP->Text          = buf;   PEP->Caption        = buf;

        PID->Enabled            = true;   btnDump->Enabled      = false;
        btnDumpRange->Enabled   = false;  ListProcess->Enabled  = true;
    }
}
//---------------------------------------------------------------------------
bool __fastcall TInterface::GetProcessSegments(HANDLE h)
{
    // Read Segment information
    IMAGE_DOS_HEADER dos_header;
    IMAGE_NT_HEADERS pe_header;
    DWORD addr = image_base;
    bool ret = false;
    TListItem* item;

    memset(&pe_header,0,sizeof(IMAGE_NT_HEADERS));
    memset(&dos_header,0,sizeof(IMAGE_DOS_HEADER));

    if(ReadProcessMemory(pHandle, (void*)image_base, &dos_header, sizeof(IMAGE_DOS_HEADER), NULL ))
    {
        addr += dos_header.e_lfanew;
        if(ReadProcessMemory(pHandle, (void*)addr, &pe_header, sizeof(IMAGE_NT_HEADERS), NULL ))
            if(pe_header.Signature == 0x4550)
            {
                buf.sprintf("0x%08X",pe_header.OptionalHeader.AddressOfEntryPoint);
                PEP->Caption = buf; N_EP->Text = buf;

                addr += sizeof(IMAGE_NT_HEADERS);
                IMAGE_SECTION_HEADER* sections = new IMAGE_SECTION_HEADER[pe_header.FileHeader.NumberOfSections];
                if(ReadProcessMemory(pHandle, (void*)addr, sections, pe_header.FileHeader.NumberOfSections*sizeof(IMAGE_SECTION_HEADER), NULL ))
                {
                    for(int i=0; i<pe_header.FileHeader.NumberOfSections; i++)
                    {
                        item = ListSegments->Items->Add();
                        buf.sprintf("%s",sections[i].Name);                 item->Caption = buf;
                        buf.sprintf("%08X",sections[i].Misc.VirtualSize);   item->SubItems->Add(buf);
                        buf.sprintf("%08X",sections[i].VirtualAddress);     item->SubItems->Add(buf);
                        buf.sprintf("%08X",sections[i].SizeOfRawData);      item->SubItems->Add(buf);
                        buf.sprintf("%08X",sections[i].PointerToRawData );  item->SubItems->Add(buf);
                    }
                    ret = true;
                }
                delete sections;
            }
    }

    return ret;
}

//---------------------------------------------------------------------------
void __fastcall TInterface::PIDKeyPress(TObject *Sender, char &Key)
{
    if(Key < '0' || Key > '9') Key = 0x0;
}
//---------------------------------------------------------------------------

void __fastcall TInterface::btnDumpClick(TObject *Sender)
{
    if(SaveDialog->Execute())
    {
        ofstream* out = new ofstream(SaveDialog->FileName.c_str(),ios::binary);

        if(*out)
        {
            char* binary = new char[image_size];
            memset(binary,0,image_size);
            if(ReadProcessMemory(pHandle,(void*)image_base,binary,image_size,NULL)) out->write(binary,image_size);
            else Application->MessageBoxA("Dump Failt - Can Not Read Process Memory !","ERROR",MB_OK);

            out->close();   delete out;     delete binary;
        } else Application->MessageBoxA("Dump Failt - Can Not Create Output File !","ERROR",MB_OK);
    } else Application->MessageBoxA("Dump Failt !","ERROR",MB_OK);

    if(cbFixSection->Checked) RebuildPE(SaveDialog->FileName.c_str(),CBImageSize->Checked,N_EP->Text.ToInt());
}
//---------------------------------------------------------------------------
bool __fastcall TInterface::RebuildPE(const char* file, bool fix_size, long ep)
{
    IMAGE_NT_HEADERS nt_header;
    IMAGE_DOS_HEADER dos_header;
    bool ret = false;
    long fpos;

    memset(&nt_header,0,sizeof(IMAGE_NT_HEADERS));
    memset(&dos_header,0,sizeof(IMAGE_DOS_HEADER));

    ifstream* in = new ifstream(file,ios::binary|ios::in);

    if(*in)
    {
        in->read((char*)&dos_header,sizeof(IMAGE_DOS_HEADER));
        in->seekg(dos_header.e_lfanew,ios::beg);
        in->read((char*)&nt_header,sizeof(IMAGE_NT_HEADERS));

        fpos = in->tellg();

        if(nt_header.Signature == 0x4550)
        {
            IMAGE_SECTION_HEADER* sections = new IMAGE_SECTION_HEADER[nt_header.FileHeader.NumberOfSections];
            in->read((char*)sections,nt_header.FileHeader.NumberOfSections*sizeof(IMAGE_SECTION_HEADER));

            // Fixup Sections Addresses & Sizes
            for(int i=0; i<nt_header.FileHeader.NumberOfSections; i++)
            {
                sections[i].SizeOfRawData    = sections[i].Misc.VirtualSize;
                sections[i].PointerToRawData = sections[i].VirtualAddress;
            }
            in->close();    delete in;

            // Rebuild Executeable
            ofstream* out = new ofstream(file,ios::binary|ios::in|ios::out);
            if(*out)
            {
                // Fixup Sections ...
                out->seekp(fpos,ios::beg);
                out->write((char*)sections,nt_header.FileHeader.NumberOfSections*sizeof(IMAGE_SECTION_HEADER));

                if(ep != 0x00000000)
                {
                    nt_header.OptionalHeader.AddressOfEntryPoint = ep;
                    out->seekp(dos_header.e_lfanew,ios::beg);
                    out->write((char*)&nt_header,sizeof(IMAGE_NT_HEADERS));
                }

                ret = true; out->close();   delete out;
            }
            else Application->MessageBoxA("Can Not Write Executeable !","ERROR",MB_OK);

            delete sections;
        }
        else Application->MessageBoxA("Not A Vaild PE Dump !","ERROR",MB_OK);
    }
    else Application->MessageBoxA("Can Not Read Executeable !","ERROR",MB_OK);

    return ret;
}

//---------------------------------------------------------------------------
void __fastcall TInterface::btnDumpRangeClick(TObject *Sender)
{
    FormRange->Show();
}
//---------------------------------------------------------------------------

void __fastcall TInterface::RefreshListClick(TObject *Sender)
{
    PENTRY entry;
    vProcess.clear();    ListProcess->Clear();

    HANDLE hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	PROCESSENTRY32 pe;

	pe.dwSize=sizeof(PROCESSENTRY32);
	BOOL retval=Process32First(hSnapshot,&pe);

	while(retval)
	{
        entry.name.sprintf("%s",pe.szExeFile);
        entry.pid = pe.th32ProcessID;
        vProcess.push_back(entry);
        ListProcess->AddItem(entry.name,NULL);
        retval=Process32Next(hSnapshot,&pe);
	}

	CloseHandle(hSnapshot);
}
//---------------------------------------------------------------------------
void __fastcall TInterface::ListProcessClick(TObject *Sender)
{
    if(ListProcess->SelCount == 0 || CBOpenProcess->Checked) return;

    PID->Text = AnsiString(vProcess[ListProcess->ItemIndex].pid);
}
//---------------------------------------------------------------------------
void __fastcall TInterface::CB_NEPClick(TObject *Sender)
{
    N_EP->Enabled = CB_NEP->Checked;
}
//---------------------------------------------------------------------------

void __fastcall TInterface::TimerTimer(TObject *Sender)
{
    if(Interface->AlphaBlendValue<255) Interface->AlphaBlendValue += 5;
    else Timer->Enabled = false;    
}
//---------------------------------------------------------------------------

void __fastcall TInterface::FormClose(TObject *Sender, TCloseAction &Action)
{
    if(pHandle != NULL) CloseHandle(pHandle);
}
//---------------------------------------------------------------------------

void __fastcall TInterface::FormDestroy(TObject *Sender)
{
    if(pHandle != NULL) CloseHandle(pHandle);
}
//---------------------------------------------------------------------------


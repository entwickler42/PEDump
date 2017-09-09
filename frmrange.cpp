#include <vcl.h>
#pragma hdrstop

#include "frmrange.h"
#include "main.h"

#include <fstream>

using namespace std;

#pragma package(smart_init)
#pragma resource "*.dfm"

TFormRange *FormRange;

//---------------------------------------------------------------------------
__fastcall TFormRange::TFormRange(TComponent* Owner) : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TFormRange::BitChancelClick(TObject *Sender)
{
    Hide();
}
//---------------------------------------------------------------------------
void __fastcall TFormRange::BitBtn2Click(TObject *Sender)
{
    DWORD base, size;
    base = Start->Text.ToInt();    size = End->Text.ToInt();

    if(Interface->SaveDialog->Execute())
    {
        ofstream* out = new ofstream(Interface->SaveDialog->FileName.c_str(),ios::binary);
        if(*out)
        {
            char* binary = new char[size];
            memset(binary,0,size);

            if(ReadProcessMemory(Interface->pHandle,(VOID*)base,binary,size,NULL)) out->write(binary,size);
            else Application->MessageBoxA("Dump Failt - Can Not Read Process Memory !","ERROR",MB_OK);

            out->close();   delete out;     delete binary;
        } else Application->MessageBoxA("Dump Failt - Can Not Create Output File !","ERROR",MB_OK);
    } else Application->MessageBoxA("Dump Failt !","ERROR",MB_OK);
}
//---------------------------------------------------------------------------


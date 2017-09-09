#ifndef mainH
#define mainH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include <Menus.hpp>
#include <Dialogs.hpp>
#include <ImgList.hpp>
#include <ExtCtrls.hpp>
#include <Graphics.hpp>
#include <vector>

typedef struct _PENTRY
{
    AnsiString name;
    unsigned int pid;
} PENTRY;

using namespace std;

//---------------------------------------------------------------------------
class TInterface : public TForm
{
__published:	// Von der IDE verwaltete Komponenten
    TBitBtn *btnDump;
    TGroupBox *GroupBox2;
    TBitBtn *btnDumpRange;
    TGroupBox *GroupBox3;
    TListView *ListSegments;
    TLabel *Label1;
    TLabel *Label2;
    TLabel *Label3;
    TLabel *Label4;
    TLabel *Label5;
    TLabel *Label6;
    TLabel *Imagebase;
    TLabel *Imagesize;
    TLabel *PEP;
    TGroupBox *GroupBox4;
    TCheckBox *cbFixSection;
    TCheckBox *CheckBox2;
    TCheckBox *CB_NEP;
    TEdit *N_EP;
    TGroupBox *GroupBox5;
    TPopupMenu *PopupMenu;
    TMenuItem *RefreshList;
    TOpenDialog *OpenDialog;
    TSaveDialog *SaveDialog;
    TBitBtn *btnQuit;
    TListView *ListProcess;
    TImageList *ImageList;
    TTimer *Timer;
    TEdit *PID;
    TCheckBox *CBOpenProcess;
    TCheckBox *CBImageSize;
    void __fastcall btnQuitClick(TObject *Sender);
    void __fastcall CBOpenProcessClick(TObject *Sender);
    void __fastcall PIDKeyPress(TObject *Sender, char &Key);
    void __fastcall btnDumpClick(TObject *Sender);
    void __fastcall btnDumpRangeClick(TObject *Sender);
    void __fastcall RefreshListClick(TObject *Sender);
    void __fastcall ListProcessClick(TObject *Sender);
    void __fastcall CB_NEPClick(TObject *Sender);
    void __fastcall TimerTimer(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall FormDestroy(TObject *Sender);
private:	// Anwender-Deklarationen
    AnsiString buf;
    vector<PENTRY> vProcess;
    DWORD image_size, image_base;

    bool __fastcall GetProcessSegments(HANDLE h);
    bool __fastcall RebuildPE(const char* file, bool fix_size, long ep = 0x00000000);

public:		// Anwender-Deklarationen
    HANDLE pHandle;

    __fastcall TInterface(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TInterface *Interface;
//---------------------------------------------------------------------------
#endif

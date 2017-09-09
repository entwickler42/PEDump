//---------------------------------------------------------------------------

#ifndef frmrangeH
#define frmrangeH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
//---------------------------------------------------------------------------
class TFormRange : public TForm
{
__published:	// Von der IDE verwaltete Komponenten
    TGroupBox *GroupBox1;
    TGroupBox *GroupBox2;
    TEdit *Start;
    TEdit *End;
    TBitBtn *BitChancel;
    TBitBtn *BitBtn2;
    void __fastcall BitChancelClick(TObject *Sender);
    void __fastcall BitBtn2Click(TObject *Sender);
private:	// Anwender-Deklarationen
public:		// Anwender-Deklarationen
    __fastcall TFormRange(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFormRange *FormRange;
//---------------------------------------------------------------------------
#endif

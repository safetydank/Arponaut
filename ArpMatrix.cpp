#include "Arponaut.h"
#include "ArpMatrix.h"

#include <algorithm>

//  Arp matrix is 250px high, 10 octaves * 12 * 2 pixels == 240px representing the sequence
//  16 steps * 24px == 384px wide
ArpMatrix::ArpMatrix(IPlugBase* pPlug, Sequence* sequence, int x, int y) 
: IControl(pPlug, &IRECT(x, y, x+kMatrixWidth, y+kMatrixHeight)), sequence_(sequence)
{

}

bool ArpMatrix::Draw(IGraphics* pGraphics)
{
    IColor bgcolor;
    // pGraphics->DrawRect(&bgcolor, &mRECT);
    // pGraphics->FillIRect(&bgcolor, &mRECT);
    // pGraphics->DrawBitmap(background_, &mRECT, mRECT.L, mRECT.T);

    //  always display 16 steps
    int spos = sequence_->pos;
    int page = spos / 16;
    int upper = sequence_->playLength() - page*16;
    if (upper > 16) upper = 16;

    for (int i=page*16; i < page*16 + upper; ++i) {
        int nn = sequence_->get(i)->NoteNumber();
        if (nn) {
            DrawNote(pGraphics, i % 16, nn);
        }
    }

    // cursor
    int left = (sequence_->playPos() % 16)*20 + mRECT.L;
    int top = mRECT.T;
    IRECT notebox(left, top, left+20, top+4);
    IColor noteColor(255, 255, 255, 40);
    pGraphics->FillIRect(&noteColor, &notebox);

    // pGraphics->DrawBitmap(&mBackground, &mRECT, 1, &mBlend);
    // pGraphics->DrawBitmap(&mPuck, &IRECT(mMouseX, mMouseY, &mPuck), 1, &mBlend);

    return true;
}

void ArpMatrix::SetDirty(bool pushParamToPlug)
{
    mDirty = true;

	// mValueX = BOUNDED(mValueX, mClampXLo, mClampXHi);
	// mValueY = BOUNDED(mValueY, mClampYLo, mClampYHi);
    // mDirty = true;

	// if (pushParamToPlug && mPlug && mParamXIdx >= 0 && mParamYIdx >= 0) {
	// 	mPlug->SetParameterFromGUI(mParamXIdx, mValueX);
    //     mPlug->SetParameterFromGUI(mParamYIdx, mValueY);
	// }
}

void ArpMatrix::OnMouseDown(int x, int y, IMouseMod* pMod)
{
}

void ArpMatrix::OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod)
{
}

void ArpMatrix::DrawNote(IGraphics* pGraphics, int iseq, int note)
{
    int left = iseq*kMatrixPW + mRECT.L;
    int top = (kMatrixHeight - note*2) + mRECT.T;
    IRECT notebox(left, top, left+18, top+2);
    IColor noteColor(127, 255, 40, 40);
    if (sequence_->playPos() % 16 == iseq)
        noteColor.A = 255;
    pGraphics->FillIRect(&noteColor, &notebox);
};


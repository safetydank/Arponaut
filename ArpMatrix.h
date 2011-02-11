#pragma once

#include "IControl.h"

class Sequence;

class ArpMatrix : public IControl
{
public:
    /**
     * Constructor
     *
     * @param bgID background bitmap resource ID
     * @param cursorID cursor bitmap resource ID
     */
    ArpMatrix(IPlugBase* pPlug, Sequence* sequence, int x, int y);
    virtual ~ArpMatrix() { };

    virtual void OnMouseDown(int x, int y, IMouseMod* pMod);
    virtual void OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod);
    //virtual void OnMouseDblClick(int x, int y, IMouseMod* pMod);
    //virtual void OnMouseWheel(int x, int y, IMouseMod* pMod, int d);

    virtual bool Draw(IGraphics* pGraphics);
    virtual void SetDirty(bool pushParamToPlug);

    void Rebuild();

protected:
    //  Track two parameters
    // int mParamXIdx;
    // double mValueX, mClampXLo, mClampXHi;
    // double mMaxPadX;

    // int mParamYIdx;
    // double mValueY, mClampYLo, mClampYHi;
    // double mMaxPadY;

    int mMouseX;
    int mMouseY;
    
    void DrawNote(IGraphics* pGraphics, int iseq, int note);

private:
    Sequence* sequence_;
};


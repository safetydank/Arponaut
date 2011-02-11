#include "Arponaut.h"
#include "../IPlug_include_in_plug_src.h"
#include "../IControl.h"
#include "resource.h"
#include <math.h>
#include <algorithm>

#include "logger.h"

const int kNumPrograms = 1;

Arponaut::Arponaut(IPlugInstanceInfo instanceInfo)
:	IPLUG_CTOR(kNumParams, 6, instanceInfo), lastPos_(0), arpIndex_(0), playing_(0)
{
    TRACE;

    // Define parameter ranges, display units, labels.

    GetParam(kGainL)->InitDouble("Gain L", 0.0, -44.0, 12.0, 0.1, "dB");
    GetParam(kGainL)->NegateDisplay();
    GetParam(kGainR)->InitDouble("Gain R", 0.0, -44.0, 12.0, 0.1, "dB");
    GetParam(kPan)->InitInt("Pan", 0, -100, 100, "%");

    // Params can be enums.

    GetParam(kChannelSw)->InitEnum("Channel", kDefault, kNumChannelSwitchEnums);
    GetParam(kChannelSw)->SetDisplayText(kDefault, "default");
    GetParam(kChannelSw)->SetDisplayText(kReversed, "reversed");
    GetParam(kChannelSw)->SetDisplayText(kAllLeft, "all L");
    GetParam(kChannelSw)->SetDisplayText(kAllRight, "all R");
    GetParam(kChannelSw)->SetDisplayText(kOff, "mute");

    GetParam(kNoteLength)->InitEnum("Note length", kWhole, kNumNoteLengths);
    GetParam(kNoteLength)->SetDisplayText(kWhole, "1/4");
    GetParam(kNoteLength)->SetDisplayText(kHalf, "1/8");
    GetParam(kNoteLength)->SetDisplayText(kTriplet, "1/8T");
    GetParam(kNoteLength)->SetDisplayText(kQuarter, "1/16");

    GetParam(kOctaves)->InitEnum("Octaves", kOne8ve, kNumNoteLengths);
    GetParam(kOctaves)->SetDisplayText(kOne8ve,   "1 octave");
    GetParam(kOctaves)->SetDisplayText(kTwo8ve,   "2 octaves");
    GetParam(kOctaves)->SetDisplayText(kThree8ve, "3 octaves");
    GetParam(kOctaves)->SetDisplayText(kFour8ve,  "4 octaves");

    GetParam(kArpMode)->InitEnum("Arp modes", kUp, kNumArpModes);
    GetParam(kArpMode)->SetDisplayText(kUp, "Up");
    GetParam(kArpMode)->SetDisplayText(kDown, "Down");
    GetParam(kArpMode)->SetDisplayText(kUpDown, "Up/Down");
    GetParam(kArpMode)->SetDisplayText(kManual,  "Manual");
    GetParam(kArpMode)->SetDisplayText(kRandom,  "Random");

    GetParam(kInsertMode)->InitEnum("Insert modes", kInsertOff, kNumInsertModes);
    GetParam(kInsertMode)->SetDisplayText(kInsertOff, "Off");
    GetParam(kInsertMode)->SetDisplayText(kInsertLow, "Low");
    GetParam(kInsertMode)->SetDisplayText(kInsertHi,  "Hi");
    GetParam(kInsertMode)->SetDisplayText(kInsert31,  "3-1");
    GetParam(kInsertMode)->SetDisplayText(kInsert42,  "4-2");

    MakePreset("preset 1", -5.0, 5.0, 17, kReversed);
    MakePreset("preset 2", -15.0, 25.0, 37, kAllRight);
    MakeDefaultPreset("-", 4);

    // Instantiate a graphics engine.

    IGraphics* pGraphics = MakeGraphics(this, kW, kH); // MakeGraphics(this, kW, kH);
    pGraphics->AttachBackground(BG_ID, BG_FN);

    // Attach controls to the graphics engine.  Controls are automatically associated
    // with a parameter if you construct the control with a parameter index.

    // Attach a couple of meters, not associated with any parameter,
    // which we keep indexes for, so we can push updates from the plugin class.

    //IBitmap bitmap = pGraphics->LoadIBitmap(METER_ID, METER_FN, kMeter_N);
    //mMeterIdx_L = pGraphics->AttachControl(new IBitmapControl(this, kMeterL_X, kMeterL_Y, &bitmap));
    //mMeterIdx_R = pGraphics->AttachControl(new IBitmapControl(this, kMeterR_X, kMeterR_Y, &bitmap));

    // Attach a couple of faders, associated with the parameters GainL and GainR.

    //bitmap = pGraphics->LoadIBitmap(FADER_ID, FADER_FN);
    //pGraphics->AttachControl(new IFaderControl(this, kGainL_X, kGainL_Y, kFader_Len, kGainL, &bitmap, kVertical));
    //pGraphics->AttachControl(new IFaderControl(this, kGainR_X, kGainR_Y, kFader_Len, kGainR, &bitmap, kVertical));

    // Attach a 5-position switch associated with the ChannelSw parameter.

    IBitmap bitmap = pGraphics->LoadIBitmap(TOGGLE_ID, TOGGLE_FN, kNoteLength_N);
    pGraphics->AttachControl(new ISwitchControl(this, kNoteLength_X, kNoteLength_Y, kNoteLength, &bitmap));
    pGraphics->AttachControl(new ISwitchControl(this, kOctaves_X, kOctaves_Y, kOctaves, &bitmap));
    pGraphics->AttachControl(new ISwitchControl(this, kArpMode_X, kArpMode_Y, kArpMode, &bitmap));
    pGraphics->AttachControl(new ISwitchControl(this, kInsertMode_X, kInsertMode_Y, kInsertMode, &bitmap));

    // Attach a rotating knob associated with the Pan parameter.

    //bitmap = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN);
    //pGraphics->AttachControl(new IKnobRotaterControl(this, kPan_X, kPan_Y, kPan, &bitmap));

    // See IControl.h for other control types,
    // IKnobMultiControl, ITextControl, IBitmapOverlayControl, IFileSelectorControl, IGraphControl, etc.
    IText text;
    text.mAlign = IText::kAlignNear;
    textControl_ = new ITextControl(this, &IRECT(21, 0, 200, 100), &text, "Hello world");
    pGraphics->AttachControl(textControl_);

    sequence_ = new Sequence(keymap_, 16);
    matrix = new ArpMatrix(this, sequence_, kDisplay_X, kDisplay_Y);
    pGraphics->AttachControl(matrix);

    AttachGraphics(pGraphics);

    running_ = false;
}

void Arponaut::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
    // Mutex is already locked for us.
    ENoteLength noteLength = (ENoteLength) ((int(GetParam(kNoteLength)->Value())) + 1);
    EOctaves    octaves    = (EOctaves)    ((int(GetParam(kOctaves)->Value())) + 1);
    EArpMode    arpMode    = (EArpMode)    (int(GetParam(kArpMode)->Value()));
    EInsertMode insertMode = (EInsertMode) (int(GetParam(kInsertMode)->Value()));

    if (GetGUI()) {
        //GetGUI()->SetControlFromPlug(mMeterIdx_L, peakL);
        //GetGUI()->SetControlFromPlug(mMeterIdx_R, peakR);
    }

    int pos = GetSamplePos();
    running_ = (pos != lastPos_);
    int tnum, tden;
    GetTimeSig(&tnum, &tden);

    if (keymap_.held() == 0) {
        NoteOff(); // only sent if a note is already playing
    }
    
    if (running_ && keymap_.held()) {
        sequence_->setOctaves(octaves);
        sequence_->setArpMode(arpMode);
        sequence_->setInsertMode(insertMode);

        double perBeat = GetSamplesPerBeat() / noteLength;
        // trigger?
        int ibar = static_cast<int>(double(pos) / perBeat);
        int ilastBar = static_cast<int>(double(lastPos_) / perBeat);

        if ((pos == 0 && ibar == 0) || (ibar != ilastBar)) {
            // Log("pos %d pb %f Num %d Den %d ibar %d lastbar %d\n", pos, perBeat, tnum, tden, ibar, ilastBar);
            NoteOff();
            IMidiMsg* next = sequence_->next();

            if (next && next->StatusMsg() == IMidiMsg::kNoteOn) {
                SendMidiMsg(next);
                playing_ = *next;
            }

            matrix->SetDirty(false);
        }
    }

    lastPos_ = pos;
}

//  We're only ever playing one note (monophonic)
void Arponaut::NoteOff()
{
    if (playing_.StatusMsg() != IMidiMsg::kNoteOff) {
        IMidiMsg offMsg;
        offMsg.MakeNoteOffMsg(playing_.NoteNumber(), playing_.mOffset);
        SendMidiMsg(&offMsg);
        playing_ = offMsg;
    }
}

void Arponaut::ProcessMidiMsg(IMidiMsg* pMsg)
{
    char msgstr[256];
    IMidiMsg::EStatusMsg status = pMsg->StatusMsg();
    char *statstr = "";
    Log("Midi message received: nn %d prog %d status %d vel %d d1 %d d2 %d offset %d st %d\n", 
        pMsg->NoteNumber(), pMsg->Program(), status, pMsg->Velocity(),
        pMsg->mData1, pMsg->mData2, pMsg->mOffset, pMsg->mStatus);
    
    if (status == IMidiMsg::kNoteOn) {
        statstr = "Note on";
        if (keymap_.noteDown(pMsg) == false) {
            //  Terminate previous note if playing
            //  XXX should check that the same note is playing before calling noteoff
            // NoteOff();
            // playing_ = *pMsg;
        }
        sequence_->rebuild();
        matrix->SetDirty(false);
    }
    else if (status == IMidiMsg::kNoteOff) {
        statstr = "Note off";
        // NoteOff();
        keymap_.noteUp(pMsg);
        sequence_->rebuild();
        matrix->SetDirty(false);
    }

    sprintf(msgstr, "MIDI %s %d Vel %d %d held", statstr, pMsg->NoteNumber(), pMsg->Velocity(), 
            keymap_.held());
    textControl_->SetTextFromPlug(msgstr);
    // SendMidiMsg(pMsg);
}

//  Returns true if successfully added, false if key already held
bool KeyMap::noteDown(IMidiMsg *msg)
{
    for (std::vector<IMidiMsg>::iterator it = events.begin(); it != events.end(); ++it) {
        if (msg->NoteNumber() == it->NoteNumber()) {
            //  already have this note down
            return false;
        }
    }

    events.push_back(*msg);
    return true;
}

void KeyMap::noteUp(IMidiMsg *msg)
{
    for (std::vector<IMidiMsg>::iterator it = events.begin(); it != events.end(); ++it) {
        if (msg->NoteNumber() == it->NoteNumber()) {
            events.erase(it);
            return;
        }
    }
}

//  index wraps around
IMidiMsg* KeyMap::get(int index)
{
    int length = held();
    return &(events[index % length]);
}

Sequence::Sequence(KeyMap& keymap, int length) : keymap_(keymap), pos(0), playLength_(0), octaves_(1), arpMode_(kUp), insertMode_(kInsertOff)
{
    IMidiMsg off;
    off.MakeNoteOffMsg(0, 0);
    sequence.insert(sequence.begin(), length, off);
}

IMidiMsg* Sequence::get(int index)
{
    if (!(index >= 0 && index < sequence.size())) {
        Log("Index is %d seq size %d\n", index, sequence.size());
        return 0;
    }
    // assert(index >= 0 && index < sequence.size());
    return &(sequence[index]);
}

void Sequence::rebuild()
{
    int held = keymap_.held();

    playLength_ = held * octaves_;
    if (arpMode_ == kUpDown && (held*octaves_ > 2)) {
        playLength_ += playLength_ - 2;
    }

    if (insertMode_ == kInsertLow || insertMode_ == kInsertHi) {
        playLength_ *= 2;
    }

    if (playLength_ > length()) {
        sequence.resize(playLength_);
    }

    //  clear all notes, reset position if none held
    if (held == 0) {
        for (int i=0; i < length(); ++i) {
            get(i)->MakeNoteOffMsg(0, 0);        
        }
        pos = 0;
    }
    else {
        std::vector<IMidiMsg> input = keymap_.sortedEvents();

        IMidiMsg loNote = input.front();
        IMidiMsg hiNote = input.back();
        hiNote.MakeNoteOnMsg(hiNote.NoteNumber() + 12*(octaves_-1), hiNote.Velocity(), hiNote.mOffset);

        if (arpMode_ == kUp || arpMode_ == kDown || arpMode_ == kUpDown) {
            if (arpMode_ == kDown) {
                std::reverse(input.begin(), input.end());
            }
        }
        else {
            //  default others to manual for now
            input = keymap_.events;
        }

        int iLast;
        for (int oct=0; oct < octaves_; ++oct) {
            int noteOffset = (arpMode_ == kDown) ? ((octaves_-1)*12 - oct*12) : oct*12;

            for (int i=0; i < held; ++i) {
                IMidiMsg* inp = &(input[i % held]);
                iLast = i + oct*held;
                get(iLast)->MakeNoteOnMsg(inp->NoteNumber() + noteOffset, inp->Velocity(), inp->mOffset);
            }
        }

        //  Mirror the ascending sequence to produce an up/down sequence
        int iNext = iLast+1;
        if (arpMode_ == kUpDown) {
            while (--iLast > 0) {
                IMidiMsg* lastMsg = get(iLast);
                get(iNext++)->MakeNoteOnMsg(lastMsg->NoteNumber(), lastMsg->Velocity(), lastMsg->mOffset);
            }
        }

        if (insertMode_ == kInsertLow || insertMode_ == kInsertHi) {
            IMidiMsg& insertNote = (insertMode_ == kInsertLow) ? loNote : hiNote;
            // XXX change to using a member buffer instead of instantiating every time
            std::vector<IMidiMsg> seqcopy = sequence;
            int idx = 0;
            std::vector<IMidiMsg>::iterator prev = seqcopy.begin() + (playLength_/2 - 1);
            for (std::vector<IMidiMsg>::iterator it = seqcopy.begin(); 
              it != seqcopy.begin() + (playLength_/2); ++it) {
                  if (it->NoteNumber() != insertNote.NoteNumber() && prev->NoteNumber() != insertNote.NoteNumber()) {
                    sequence[idx++] = insertNote;
                  }
                  sequence[idx++] = *it;
                  prev = it;
            }
            playLength_ = idx;
        }
    }
}

IMidiMsg* Sequence::next()
{
    IMidiMsg* msg = get(pos);
    playPos_ = pos;
    if (++pos >= playLength_) 
        pos = 0;
    return msg;
}

void Sequence::setOctaves(int octaves)
{
    if (octaves != octaves_) {
        octaves_ = octaves;
        rebuild();
    }
}

void Sequence::setArpMode(EArpMode mode)
{ 
    if (mode != arpMode_) {
        // Log("Changed arp mode to %d\n", mode);
        arpMode_ = mode; 
        rebuild(); 
    }
}

void Sequence::setInsertMode(EInsertMode mode)
{
    if (mode != insertMode_) {
        insertMode_ = mode;
        rebuild();
    }
}

struct CmpNote {
    bool operator() (IMidiMsg& a, IMidiMsg& b) { return (a.NoteNumber() < b.NoteNumber()); }
} notecmp;

std::vector<IMidiMsg> KeyMap::sortedEvents()
{
    //  passing around the vector isn't so bad b.c. usually not many keys held
    std::vector<IMidiMsg> ret(events);
    sort(ret.begin(), ret.end(), notecmp);
    return ret;
}

void Arponaut::Reset()
{
    Log("Reset called\n");
}

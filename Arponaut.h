#ifndef ARPONAUT_H
#define ARPONAUT_H

// In the project settings, define either VST_API or AU_API.
#include "../IPlug_include_in_plug_hdr.h"

#include "ArpMatrix.h"
#include <vector>

enum EParams {
    kGainL = 0,
    kGainR,
    kPan,
    kChannelSw,
    kNoteLength,
    kOctaves,
    kArpMode,
    kInsertMode,
    kNumParams,
};

enum EChannelSwitch {
    kDefault = 0,
    kReversed,
    kAllLeft,
    kAllRight,
    kOff,
    kNumChannelSwitchEnums
};

enum ENoteLength {
    kWhole   = 0,
    kHalf    = 1,
    kTriplet = 2,
    kQuarter = 3,
    kNumNoteLengths
};

enum EOctaves {
    kOne8ve    = 0,
    kTwo8ve    = 1,
    kThree8ve  = 2,
    kFour8ve   = 3,
    kNumOctaves
};

enum EArpMode {
    kUp     = 0,
    kDown   = 1,
    kUpDown = 2,
    kManual = 3,
    kRandom = 4,
    kNumArpModes
};

enum EInsertMode {
    kInsertOff = 0,
    kInsertLow = 1,
    kInsertHi  = 2,
    kInsert31  = 3,
    kInsert42  = 4,
    kNumInsertModes
};

//  Tracks keydown midi event order
class KeyMap
{
public:
    std::vector<IMidiMsg> events;

    KeyMap() { };
    ~KeyMap() { };

    bool noteDown(IMidiMsg* msg);
    void noteUp(IMidiMsg* msg);

    int held() { return events.size(); };
    
    std::vector<IMidiMsg> sortedEvents();

    IMidiMsg* get(int index);
};

typedef enum {
    UP,
    DOWN,
    UPDOWN,
    MANUAL,
    RANDOM
} ArpMode_t;

typedef enum {
} octave_t;

typedef enum {
    WHOLE,
    HALF,
    HALFT,
    QUARTER,
    QUARTERT
} notelength_t;

enum ELayout
{
    kW = 423,
    kH = 582,

    kNoteLength_N = 5,	// # of sub-bitmaps.
    kMeter_N = 51,	// # of sub-bitmaps.

    kFader_Len = 150,
    kGainL_X = 80,
    kGainL_Y = 20,
    kGainR_X = 350,
    kGainR_Y = 20,

    kNoteLength_X = 10,
    kNoteLength_Y = 350,

    kOctaves_X = 50,
    kOctaves_Y = 350,

    kArpMode_X = 90,
    kArpMode_Y = 350,

    kInsertMode_X = 130,
    kInsertMode_Y = 350,

    kMatrixWidth = 384,
    kMatrixHeight = 250,
    kMatrixPW = 24,

    kPan_X = 225,
    kPan_Y = 145,

    kDisplay_X = 20,
    kDisplay_Y = 49,

    kMeterL_X = 135,
    kMeterL_Y = 20,
    kMeterR_X = 250,
    kMeterR_Y = 20
};


class Sequence
{
private:
    int playLength_;

public:
    explicit Sequence(KeyMap& keymap, int length);
    ~Sequence() {};
    std::vector<IMidiMsg> sequence;

    int length() { return sequence.size(); }
    int playLength() { return playLength_; }
    IMidiMsg* get(int index);
    IMidiMsg* next();

    //  Recalculate the arpeggiated sequence
    void rebuild();

    KeyMap& keymap_;
    
    int pos;

    // currently playing position, for display only
    int playPos_;
    int playPos() { return playPos_; };

    //  Arpeggio mode (up, down, manual)
    EArpMode arpMode_;
    void setArpMode(EArpMode mode);

    //  Insert mode
    EInsertMode insertMode_;
    void setInsertMode(EInsertMode mode);

    int octaves_;
    void setOctaves(int octaves);
};

class Arponaut : public IPlug
{
public:
	Arponaut(IPlugInstanceInfo instanceInfo);
	~Arponaut() {}	// Nothing to clean up.

	// Implement these if your audio or GUI logic requires doing something 
	// when params change or when audio processing stops/starts.
	void Reset();
	void OnParamChange(int paramIdx) {}

	void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);
    void ProcessMidiMsg(IMidiMsg* pMsg);

    void NoteOff();

private:
    ArpMatrix* matrix;

    //  True if the sequencer is running (clock ticking)
    bool running_;
    //  Last position
    int lastPos_;
    int arpIndex_;

    ITextControl* textControl_;

    Sequence* sequence_;
    //  keys held
    KeyMap keymap_;
    //  Currently playing message
    IMidiMsg playing_;
};

////////////////////////////////////////

#endif

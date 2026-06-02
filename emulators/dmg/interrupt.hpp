#ifndef VRITA_DMG_INTERRUPT_INCLUDES
#define VRITA_DMG_INTERRUPT_INCLUDES

class DMG_INTERRUPT {
public:
    bool IME;

    void setInterruptsEnabled(bool state);
    bool areInterruptsEnabled();
};

#endif
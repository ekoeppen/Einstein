#include <Includes/Frames/Objects.h>
#include <Includes/Frames/NewtonScript.h>
#include <Includes/Utilities/Unicode.h>
#include <SerialChipRegistry.h>
#include <CommErrors.h>

#define kCMOSerialEinsteinLoc 'eloc'

class THMOSerialEinsteinHardware : public TOption
{
    public:
        THMOSerialEinsteinHardware();
        ULong   fLocationID;    // hardware location
        ULong   fType;          // host side driver type
};

THMOSerialEinsteinHardware::THMOSerialEinsteinHardware()
{
    SetAsOption(kCMOSerialEinsteinLoc);
    SetLength(sizeof(THMOSerialEinsteinHardware));
}

extern "C" Ref Enable(RefArg inRcvr, RefArg inLocation, RefArg inType)
{
    THMOSerialEinsteinHardware option;
    ULong location = 0;
    char name[5];
    ULong type = RefToInt(inType);

    WITH_LOCKED_BINARY(inLocation, ptr)
        UniChar *l = static_cast<UniChar *>(ptr);
        for (size_t i = 0; i < 4; i++) {
            name[i] = l[i];
            location = (location << 8) + l[i];
        }
    END_WITH_LOCKED_BINARY(inLocation)
    name[4] = 0;
    TSerialChip *einsteinChip = TSerialChip::New("TSerialChipEinstein");
    if (einsteinChip == NULL) {
        return MakeInt(kCommErrMethodNotImplemented);
    }

    PSerialChipRegistry *registry = GetSerialChipRegistry();
    SerialChipID id = registry->FindByLocation(location);
    TSerialChip *existingChip = registry->GetChipPtr(id);
    if (existingChip != NULL) {
        registry->UnRegister(existingChip);
    }
    TUNameServer nameServer;
    nameServer.UnRegisterName(name, "TSerialChip");

    option.fLocationID = location;
    option.fType = type;
    einsteinChip->ProcessOption(&option);
    registry->Register(einsteinChip, location);

    return MakeInt(0);
}

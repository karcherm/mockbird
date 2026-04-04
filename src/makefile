CXXFLAGS=-d2 -xs -we -wx -wcd=433 -mm
OBJ=sndscape.obj sndauto.obj main.obj sndexcpt.obj ini.obj layer.obj util.obj

mockbird.exe: $(OBJ)
    %write mockbird.lnk NAME $@
    %write mockbird.lnk SYSTEM DOS
    %write mockbird.lnk DEBUG DWARF
    %write mockbird.lnk FILE {$(OBJ)}
    %write mockbird.lnk OPTION STACK=4096
    wlink  @mockbird.lnk

.cpp.obj: .AUTODEPEND
    wpp $(CXXFLAGS) -fo=$[*.obj $[*


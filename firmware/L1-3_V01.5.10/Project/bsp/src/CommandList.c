#include "CommandList.h"
#include "Command.h"

//*******************************************************************************************//
const cmdStructure cmdMessage[] = {
{"help",help,
 "help                       //all command\r\n"
},
{"rbc",RebackCharacter,
 "RBC x                      //reback every character x(1-true;0-false)\r\n"
},
{"ver",ReadVersion,
 "VER x                      //firmware version\r\n"
},
{"*idn",ReadWriteBoardIDN,
 "*IDN x x                   //read write IDN x(?/W) x(par)\r\n"
},
{"*pn",ReadWriteBoardPN,
 "*PN x x                    //read write PN x(?/W) x(par)\r\n"
},
{"*sn",ReadWriteBoardSN,
 "*SN x x                    //read write SN x(?/W) x(par)\r\n"
},
{"sw",ReadSelectSwitch,
 "SW x                      //read select switch x(?/channel)\r\n"
},
{"max",ReadWriteMaxCH,
 "Max x x                    //read write max channel x(?/W) x(par)\r\n"
},
{"lvd",LevelValueDispose,
 "LVD x x x x x x            //Set group voltage x(w/r) x(CH) x(X+) x(X-) x(Y+) x(Y-)\r\n"
},
{"ofs",OffsetChannel,
 "ofs x x x                  //offset channel voltage x(od-dac out/wk--coff/wl--level/rk/rl/cb--calculation k) x(index) x(par)\r\n"
},
{"srw",SetReadWrite,
 "SRW x x x                  //Set read write x(W/R) x[ls/bs/pps/ec/temp]\r\n"
},


};

//*******************************************************************************************//
const int cmdStrLen = sizeof(cmdMessage)/sizeof(cmdStructure);


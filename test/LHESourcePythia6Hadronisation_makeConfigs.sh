#!/bin/sh
cmsDriver.py GeneratorInterface/LHEInterface/LHESourcePythia6Hadronisation_cff.py -s GEN:ProducerFilterSequence --eventcontent RAWSIM --datatier GEN --conditions FrontierConditions_GlobalTag,IDEAL_V9::All -n 100 --no_exec
cmsDriver.py GeneratorInterface/LHEInterface/LHESourcePythia6Hadronisation_cff.py -s GEN:ProducerFilterSequence,SIM,DIGI,L1,DIGI2RAW,HLT --eventcontent RAWSIM --datatier GEN-SIM-RAW --conditions FrontierConditions_GlobalTag,IDEAL_V9::All -n 100 --no_exec

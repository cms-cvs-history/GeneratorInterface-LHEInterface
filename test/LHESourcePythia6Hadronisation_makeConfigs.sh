#!/bin/sh
cmsDriver.py GeneratorInterface/LHEInterface/LHESourcePythia6Hadronisation_cff.py -s GEN:ProducerSourceSequence --eventcontent RAWSIM --datatier GEN --conditions FrontierConditions_GlobalTag,IDEAL_V9::All -n 100 --no_exec --customise=GeneratorInterface/LHEInterface/generatorProducer.py
cmsDriver.py GeneratorInterface/LHEInterface/LHESourcePythia6Hadronisation_cff.py -s GEN:ProducerSourceSequence,SIM,DIGI,L1,DIGI2RAW,HLT --eventcontent RAWSIM --datatier GEN-SIM-RAW --conditions FrontierConditions_GlobalTag,IDEAL_V9::All -n 100 --no_exec --customise=GeneratorInterface/LHEInterface/generatorProducer.py

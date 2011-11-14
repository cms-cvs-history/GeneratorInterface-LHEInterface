import FWCore.ParameterSet.Config as cms

process = cms.Process("ScriptExample")

process.load("IOMC.RandomEngine.IOMC_cff")
process.load("FWCore.MessageService.MessageLogger_cfi")

process.source = cms.Source("EmptySource",
    firstLuminosityBlock = cms.untracked.uint32(23456),
    numberEventsInLuminosityBlock = cms.untracked.uint32(10)
)                            

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(100) )

process.load("GeneratorInterface/LHEInterface/ExternalLHEProducer_cfi")

process.out = cms.OutputModule("PoolOutputModule",
    fileName = cms.untracked.string('myOutputFile.root')
)

process.p = cms.Path(process.externalLHEProducer)

process.e = cms.EndPath(process.out)

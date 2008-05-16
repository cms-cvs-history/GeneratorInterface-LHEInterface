import FWCore.ParameterSet.Config as cms

process = cms.Process("LHE")
process.load("FWCore.MessageService.MessageLogger_cfi")

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(-1)
)
process.configurationMetadata = cms.untracked.PSet(
    version = cms.untracked.string('alpha'),
    name = cms.untracked.string('LHEF input'),
    annotation = cms.untracked.string('ttbar')
)
process.source = cms.Source("MCDBSource",
    articleID = cms.uint32(120),
    supportedProtocols = cms.vstring('gsiftp')
)

process.LHE = cms.OutputModule("PoolOutputModule",
    dataset = cms.untracked.PSet(
        dataTier = cms.untracked.string('LHE')
    ),
    fileName = cms.untracked.string('lhe.root')
)

process.outpath = cms.EndPath(process.LHE)
process.MessageLogger.cerr.threshold = 'INFO'



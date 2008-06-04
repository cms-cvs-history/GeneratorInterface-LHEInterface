#!/usr/bin/env cmsRun
import FWCore.ParameterSet.Config as cms

process = cms.Process("Reco")

process.source = cms.Source("PoolSource",
	fileNames = cms.untracked.vstring('file:raw.root')
)

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(-1))

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.threshold = 'INFO'

process.load("Configuration.StandardSequences.Geometry_cff")
process.load("Configuration.StandardSequences.MagneticField_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")

process.load("Configuration.StandardSequences.RawToDigi_cff")
process.load("Configuration.StandardSequences.Reconstruction_cff")

process.p0 = cms.Path(process.RawToDigi)
process.p1 = cms.Path(process.reconstruction)

process.load("Configuration.EventContent.EventContent_cff")

process.RECO = cms.OutputModule("PoolOutputModule",
	process.FEVTSIMEventContent,
	dataset = cms.untracked.PSet(dataTier = cms.untracked.string('RECO')),
	fileName = cms.untracked.string('reco.root')
)
process.RECO.outputCommands.append('keep *_source_*_*')
process.RECO.outputCommands.append('keep *_generator_*_*')

process.outpath = cms.EndPath(process.RECO)

process.schedule = cms.Schedule(
	process.p0,
	process.p1,
	process.outpath
)

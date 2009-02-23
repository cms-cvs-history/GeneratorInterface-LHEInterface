#!/usr/bin/env cmsRun
import FWCore.ParameterSet.Config as cms

process = cms.Process("Gen")

process.source = cms.Source("LHESource",
	fileNames = cms.untracked.vstring('file:events.lhe')
)

#process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(100))

process.configurationMetadata = cms.untracked.PSet(
	version = cms.untracked.string('alpha'),
	name = cms.untracked.string('LHEF input'),
	annotation = cms.untracked.string('ttbar')
)

process.load("Configuration.StandardSequences.Services_cff")

process.RandomNumberGeneratorService.generator = cms.PSet(
	initialSeed = cms.untracked.uint32(123456789),
	engineName = cms.untracked.string('HepJamesRandom')
)

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.threshold = 'INFO'

process.load("Configuration.Generator.PythiaUESettings_cfi")

process.generator = cms.EDProducer("LHEProducer",
	eventsToPrint = cms.untracked.uint32(1),

	hadronisation = cms.PSet(
		process.pythiaUESettingsBlock,

		generator = cms.string('Pythia6'),

		maxEventsToPrint = cms.untracked.int32(1),
		pythiaPylistVerbosity = cms.untracked.int32(2),

		parameterSets = cms.vstring(
			'pythiaUESettings', 
			'pythiaCMSDefaults'
		),

		 pythiaCMSDefaults = cms.vstring(
			'PMAS(5,1)=4.4  ! b quarks mass', 
			'PMAS(6,1)=172.4  ! t quarks mass', 
			'MSTJ(1)=1      !...Fragmentation/hadronization on or off', 
			'MSTP(61)=1     ! Parton showering on or off', 
			'MSEL=0         ! User defined processes/Full user control'
		)
	),

	jetMatching = cms.untracked.PSet(
		scheme = cms.string("Madgraph"),

		mode = cms.string("auto"),
		etaclmax = cms.double(5.0),
		qcut = cms.double(30.0),
		minjets = cms.int32(0),
		maxjets = cms.int32(3)
	)
)

process.load("Configuration.StandardSequences.Generator_cff")

process.p0 = cms.Path(
	process.generator *
	process.pgen
)

process.load("Configuration.StandardSequences.VtxSmearedGauss_cff")

process.genParticles.abortOnUnknownPDGCode = False

process.load("SimGeneral.HepPDTESSource.pythiapdt_cfi")

process.printList = cms.EDFilter("ParticleListDrawer",
	src = cms.InputTag("genParticles"),
	maxEventsToPrint = cms.untracked.int32(-1)
)

process.printTree = cms.EDFilter("ParticleTreeDrawer",
	src = cms.InputTag("genParticles"),
	printP4 = cms.untracked.bool(False),
	printPtEtaPhi = cms.untracked.bool(False),
	printVertex = cms.untracked.bool(True),
	printStatus = cms.untracked.bool(False),
	printIndex = cms.untracked.bool(False),
	status = cms.untracked.vint32(1, 2, 3)
)

process.p = cms.Path(
	process.printList *
	process.printTree
)

process.load("Configuration.EventContent.EventContent_cff")

process.GEN = cms.OutputModule("PoolOutputModule",
	process.FEVTSIMEventContent,
	dataset = cms.untracked.PSet(dataTier = cms.untracked.string('GEN')),
	SelectEvents = cms.untracked.PSet(SelectEvents = cms.vstring('p0')),
	fileName = cms.untracked.string('test.root')
)

process.outpath = cms.EndPath(process.GEN)

process.schedule = cms.Schedule(process.p0, process.outpath)

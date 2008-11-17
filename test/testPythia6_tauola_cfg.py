#!/usr/bin/env cmsRun
import FWCore.ParameterSet.Config as cms

process = cms.Process("Gen")

process.source = cms.Source("LHESource",
	fileNames = cms.untracked.vstring('file:ttbar.lhe')
)

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(100))

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
process.load("GeneratorInterface.Pythia6Interface.TauolaSettings_cff")

process.generator = cms.EDProducer("LHEProducer",
	eventsToPrint = cms.untracked.uint32(1),

	hadronisation = cms.PSet(
		process.pythiaUESettingsBlock,

		generator = cms.string('Pythia6'),

		maxEventsToPrint = cms.untracked.int32(1),
		pythiaPylistVerbosity = cms.untracked.int32(2),

		parameterSets = cms.vstring(
			'pythiaUESettings', 
			'pythiaAlpgen'
		),

		pythiaAlpgen = cms.vstring(
			'PMAS(5,1)=1.5   ! c quark mass', 
			'PMAS(5,1)=4.7   ! b quark mass', 
			'PMAS(6,1)=175.0 ! t quark mass', 
			'MSTP(32)=2      ! Q^2 = sum(m_T^2), iqopt = 1', 
			'MSEL=0          ! User defined processes/Full user control'
		),

		externalGenerators = cms.PSet(
			parameterSets = cms.vstring("Tauola"),

			Tauola = cms.PSet(
				process.TauolaPolar,
				process.TauolaDefaultInputCards
			)
		)
	)
)

process.load("Configuration.StandardSequences.Generator_cff")

process.p0 = cms.Path(
	process.generator *
	process.pgen
)

process.load("Configuration.StandardSequences.VtxSmearedGauss_cff")

process.VtxSmeared.src = 'generator'
process.genEventWeight.src = 'generator'
process.genEventScale.src = 'generator'
process.genEventPdfInfo.src = 'generator'
process.genEventProcID.src = 'generator'
process.genParticles.src = 'generator'
process.genParticleCandidates.src = 'generator'

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
process.GEN.outputCommands.append("keep *_generator_*_*")

process.outpath = cms.EndPath(process.GEN)

process.schedule = cms.Schedule(process.p0, process.outpath)

#ifndef GeneratorInterface_LHEInterface_LHEEvent_h
#define GeneratorInterface_LHEInterface_LHEEvent_h

#include <iostream>
#include <memory>
#include <utility>

#include <boost/shared_ptr.hpp>

#include <HepMC/GenEvent.h>
#include <HepMC/GenVertex.h>

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "GeneratorInterface/LHEInterface/interface/LesHouches.h"
#include "GeneratorInterface/LHEInterface/interface/LHECommon.h"

namespace lhef {

class LHEEvent {
    public:
	LHEEvent(const boost::shared_ptr<LHECommon> &common,
	         std::istream &in);
	~LHEEvent();

	struct PDF {
		std::pair<int, int>		id;
		std::pair<double, double>	x;
		std::pair<double, double>	xPDF;
		double				scalePDF;
	};

	const boost::shared_ptr<LHECommon> &getCommon() const { return common; }
	const HEPEUP *getHEPEUP() const { return &hepeup; }
	const HEPRUP *getHEPRUP() const { return common->getHEPRUP(); }
	const PDF *getPDF() const { return pdf.get(); }

	std::auto_ptr<HepMC::GenEvent> asHepMCEvent() const;

	static const HepMC::GenVertex *findSignalVertex(
			const HepMC::GenEvent *event, bool status3 = true);

    private:
	static bool checkHepMCTree(const HepMC::GenEvent *event);
	HepMC::GenParticle *makeHepMCParticle(unsigned int i) const;

	const boost::shared_ptr<LHECommon>	common;

	HEPEUP					hepeup;
	std::auto_ptr<PDF>			pdf;
};

} // namespace lhef

#endif // GeneratorEvent_LHEInterface_LHEEvent_h

#include <iostream>
#include <string>
#include <memory>

#include <boost/shared_ptr.hpp>

#include <HepMC/GenEvent.h>
#include <HepMC/SimpleVector.h>

#include "FWCore/Framework/interface/EDFilter.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/InputTag.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "SimDataFormats/HepMCProduct/interface/HepMCProduct.h"

class LHEFilter : public edm::EDFilter {
    public:
	explicit LHEFilter(const edm::ParameterSet &params);
	virtual ~LHEFilter();

    protected:
	virtual bool filter(edm::Event &event, const edm::EventSetup &es);

    private:
	edm::InputTag	sourceLabel;
};

LHEFilter::LHEFilter(const edm::ParameterSet &params) :
	sourceLabel(params.getParameter<edm::InputTag>("src"))
{
}

LHEFilter::~LHEFilter()
{
}

bool LHEFilter::filter(edm::Event &event, const edm::EventSetup &es)
{
	edm::Handle<edm::HepMCProduct> product;
	event.getByLabel(sourceLabel, product);

	return product->GetEvent();
}

DEFINE_ANOTHER_FWK_MODULE(LHEFilter);
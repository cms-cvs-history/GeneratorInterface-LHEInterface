#include <vector>

#include <fastjet/JetDefinition.hh>
#include <fastjet/PseudoJet.hh>
#include <fastjet/ClusterSequence.hh>

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/Exception.h"

#include "GeneratorInterface/LHEInterface/interface/JetInput.h"
#include "GeneratorInterface/LHEInterface/interface/JetClustering.h"

namespace lhef {

class JetClustering::Algorithm {
    public:
	typedef JetClustering::Jet		Jet;
	typedef JetClustering::ParticleVector	ParticleVector;

	Algorithm(const edm::ParameterSet &params, double jetPtMin) :
		jetPtMin(jetPtMin) {}
	virtual ~Algorithm() {}

	virtual std::vector<Jet> operator () (
				const ParticleVector &input) const = 0;

    protected:
	double	jetPtMin;
};

namespace {
	class KtAlgorithm : public JetClustering::Algorithm {
	    public:
		KtAlgorithm(const edm::ParameterSet &params,
		            double jetPtMin);
		~KtAlgorithm() {}

	    private:
		std::vector<Jet> operator () (
				const ParticleVector &input) const;

		fastjet::JetDefinition	jetDefinition;
	};
} // anonymous namespace

KtAlgorithm::KtAlgorithm(const edm::ParameterSet &params, double jetPtMin) :
	JetClustering::Algorithm(params, jetPtMin),
	jetDefinition(fastjet::kt_algorithm,
	              params.getParameter<double>("ktRParam"),
	              fastjet::Best)
{
}

std::vector<JetClustering::Jet> KtAlgorithm::operator () (
					const ParticleVector &input) const
{
	if (input.empty())
		return std::vector<JetClustering::Jet>();

	std::vector<fastjet::PseudoJet> jfInput;
	jfInput.reserve(input.size());
	for(ParticleVector::const_iterator iter = input.begin();
	    iter != input.end(); ++iter) {
		jfInput.push_back(fastjet::PseudoJet(
			(*iter)->momentum().px(), (*iter)->momentum().py(),
			(*iter)->momentum().pz(), (*iter)->momentum().e()));
		jfInput.back().set_user_index(iter - input.begin());
	}

	fastjet::ClusterSequence sequence(jfInput, jetDefinition);
	std::vector<fastjet::PseudoJet> jets =
				sequence.inclusive_jets(jetPtMin);

	std::vector<Jet> result;
	result.reserve(jets.size());
	ParticleVector constituents;
	for(std::vector<fastjet::PseudoJet>::const_iterator iter = jets.begin();
	    iter != jets.end(); ++iter) {
		std::vector<fastjet::PseudoJet> fjConstituents =
					sequence.constituents(*iter);
		unsigned int size = fjConstituents.size();
		constituents.resize(size);
		for(unsigned int i = 0; i < size; i++)
			constituents[i] =
				input[fjConstituents[i].user_index()];

		result.push_back(
			Jet(iter->px(), iter->py(), iter->pz(), iter->E(),
			    constituents));
	}

	return result;
}

JetClustering::JetClustering(const edm::ParameterSet &params)
{
	double jetPtMin = params.getParameter<double>("jetPtMin");
	init(params, jetPtMin);
}

JetClustering::JetClustering(const edm::ParameterSet &params,
                             double jetPtMin)
{
	init(params, jetPtMin);
}

JetClustering::~JetClustering()
{
}

void JetClustering::init(const edm::ParameterSet &params, double jetPtMin)
{
	edm::ParameterSet algoParams =
			params.getParameter<edm::ParameterSet>("algorithm");
	std::string algoName = algoParams.getParameter<std::string>("name");

	if (algoName == "KT")
		jetAlgo.reset(new KtAlgorithm(algoParams, jetPtMin));
	else
		throw cms::Exception("Configuration")
			<< "JetClustering algorithm \"" << algoName
			<< "\" unknown." << std::endl;
}

std::vector<JetClustering::Jet> JetClustering::operator () (
					const ParticleVector &input) const
{
	return (*jetAlgo)(input);
}

} // namespace lhef

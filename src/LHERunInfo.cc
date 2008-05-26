#include <algorithm>
#include <iostream>
#include <string>
#include <cctype>
#include <vector>
#include <memory>
#include <cmath>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>

#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "SimDataFormats/GeneratorProducts/interface/LesHouches.h"

#include "GeneratorInterface/LHEInterface/interface/LHERunInfo.h"

#include "XMLUtils.h"

XERCES_CPP_NAMESPACE_USE

static int skipWhitespace(std::istream &in)
{
	int ch;
	do {
		ch = in.get();
	} while(std::isspace(ch));
	if (ch != std::istream::traits_type::eof())
		in.putback(ch);
	return ch;
}

namespace lhef {

LHERunInfo::LHERunInfo(std::istream &in)
{
	in >> heprup.IDBMUP.first >> heprup.IDBMUP.second
	   >> heprup.EBMUP.first >> heprup.EBMUP.second
	   >> heprup.PDFGUP.first >> heprup.PDFGUP.second
	   >> heprup.PDFSUP.first >> heprup.PDFSUP.second
	   >> heprup.IDWTUP >> heprup.NPRUP;
	if (!in.good())
		throw cms::Exception("InvalidFormat")
			<< "Les Houches file contained invalid"
			   " header in init section." << std::endl;

	heprup.resize();

	for(int i = 0; i < heprup.NPRUP; i++) {
		in >> heprup.XSECUP[i] >> heprup.XERRUP[i]
		   >> heprup.XMAXUP[i] >> heprup.LPRUP[i];
		if (!in.good())
			throw cms::Exception("InvalidFormat")
				<< "Les Houches file contained invalid data"
				   " in header payload line " << (i + 1)
				<< "." << std::endl;
	}

	while(skipWhitespace(in) == '#') {
		std::string line;
		std::getline(in, line);
		comments.push_back(line + "\n");
	}

	if (!in.eof())
		edm::LogWarning("Generator|LHEInterface")
			<< "Les Houches file contained spurious"
			   " content after the regular data." << std::endl;

	init();
}

LHERunInfo::LHERunInfo(const HEPRUP &heprup) :
	heprup(heprup)
{
	init();
}

LHERunInfo::~LHERunInfo()
{
}

void LHERunInfo::init()
{
	for(int i = 0; i < heprup.NPRUP; i++) {
		Process proc;

		proc.process = heprup.LPRUP[i];
		proc.heprupIndex = (unsigned int)i;

		processes.push_back(proc);
	}

	std::sort(processes.begin(), processes.end());
}

bool LHERunInfo::operator == (const LHERunInfo &other) const
{
	return heprup == other.heprup;
}

void LHERunInfo::count(int process, CountMode mode, double eventWeight,
                      double matchWeight)
{
	std::vector<Process>::iterator proc =
		std::lower_bound(processes.begin(), processes.end(), process);
	if (proc == processes.end() || proc->process != process)
		return;

	switch(mode) {
	    case kAccepted:
		proc->accepted.add(eventWeight * matchWeight);
	    case kKilled:
		proc->killed.add(eventWeight * matchWeight);
	    case kSelected:
		proc->selected.add(eventWeight);
	    case kTried:
		proc->tried.add(eventWeight);
	}
}

LHERunInfo::XSec LHERunInfo::xsec() const
{
	double sigSelSum = 0.0;
	double sigSum = 0.0;
	double err2Sum = 0.0;

	for(std::vector<Process>::const_iterator proc = processes.begin();
	    proc != processes.end(); ++proc) {
		unsigned int idx = proc->heprupIndex;

		double sigmaSum, sigma2Sum, sigma2Err;
		if (std::abs(heprup.IDWTUP == 3)) {
			sigmaSum = proc->tried.n * heprup.XSECUP[idx];
			sigma2Sum = sigmaSum * heprup.XSECUP[idx];
			sigma2Err = proc->tried.n * heprup.XERRUP[idx]
			                          * heprup.XERRUP[idx];
		} else {
			sigmaSum = proc->tried.sum;
			sigma2Sum = proc->tried.sum2;
			sigma2Err = 0.0;
		}

		if (!proc->killed.n)
			continue;

		double sigmaAvg = sigmaSum / proc->tried.n;
		double fracAcc = (double)proc->killed.n / proc->selected.n;
		double sigmaFin = sigmaAvg * fracAcc;

		double deltaFin = sigmaFin;
		if (proc->killed.n > 1) {
			double sigmaAvg2 = sigmaAvg * sigmaAvg;
			double delta2Sig =
				(sigma2Sum / proc->tried.n - sigmaAvg2) /
				(proc->tried.n * sigmaAvg2);
			double delta2Veto =
				((double)proc->selected.n - proc->killed.n) /
				((double)proc->selected.n * proc->killed.n);
			double delta2Sum = delta2Sig + delta2Veto
			                   + sigma2Err / sigmaSum;
			deltaFin = sigmaFin * (delta2Sum > 0.0 ?
						std::sqrt(delta2Sum) : 0.0);
		}

		sigSelSum += sigmaAvg;
		sigSum += sigmaFin;
		err2Sum += deltaFin * deltaFin;
	}

	XSec result;
	result.value = 1.0e-9 * sigSum;
	result.error = 1.0e-9 * std::sqrt(err2Sum);

	return result;
}

LHERunInfo::Header::Header() :
	xmlDoc(0)
{
}

LHERunInfo::Header::Header(const std::string &tag) :
	LHERunInfoProduct::Header(tag), xmlDoc(0)
{
}

LHERunInfo::Header::Header(const Header &orig) :
	LHERunInfoProduct::Header(orig), xmlDoc(0)
{
}

LHERunInfo::Header::Header(const LHERunInfoProduct::Header &orig) :
	LHERunInfoProduct::Header(orig), xmlDoc(0)
{
}

LHERunInfo::Header::~Header()
{
	if (xmlDoc)
		xmlDoc->release();
}

namespace {
	class HeaderReader : public CBInputStream::Reader {
	    public:
		HeaderReader(const LHERunInfo::Header *header) :
			header(header), mode(kHeader),
			iter(header->begin())
		{
		}

		const std::string &data()
		{
			switch(mode) {
			    case kHeader:
				tmp = "<" + header->tag() + ">";
				mode = kBody;
				break;
			    case kBody:
				if (iter != header->end())
					return *iter++;
				tmp = "</" + header->tag() + ">";
				mode = kFooter;
				break;
			    case kFooter:
				tmp.clear();
			}

			return tmp;
		}

	    private:
		enum Mode {
			kHeader,
			kBody,
			kFooter
		};

		const LHERunInfo::Header		*header;
		Mode					mode;
		LHERunInfo::Header::const_iterator	iter;
		std::string				tmp;
	};
} // anonymous namespace

const DOMNode *LHERunInfo::Header::getXMLNode() const
{
	if (tag().empty())
		return 0;

	if (!xmlDoc) {
		XercesDOMParser parser;
		parser.setValidationScheme(XercesDOMParser::Val_Auto);
		parser.setDoNamespaces(false);
		parser.setDoSchema(false);
		parser.setValidationSchemaFullChecking(false);

		HandlerBase errHandler;
		parser.setErrorHandler(&errHandler);
		parser.setCreateEntityReferenceNodes(false);

		try {
			std::auto_ptr<CBInputStream::Reader> reader(
						new HeaderReader(this));
			CBInputSource source(reader);
			parser.parse(source);
			xmlDoc = parser.adoptDocument();
		} catch(const XMLException &e) {
			throw cms::Exception("Generator|LHEInterface")
				<< "XML parser reported DOM error no. "
				<< (unsigned long)e.getCode()
				<< ": " << XMLSimpleStr(e.getMessage()) << "."
				<< std::endl;
		} catch(const SAXException &e) {
			throw cms::Exception("Generator|LHEInterface")
				<< "XML parser reported: "
				<< XMLSimpleStr(e.getMessage()) << "."
				<< std::endl;
		}
	}

	return xmlDoc->getDocumentElement();
}

} // namespace lhef
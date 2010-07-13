/*
   (c) Copyright 2003-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

#include <string>
#include <algorithm>
#include <vector>
#include <iostream>

#include <boost/foreach.hpp>
#include <boost/format.hpp>

#include <Lintel/BoyerMooreHorspool.hpp>
#include <Lintel/LintelLog.hpp>
#include <Lintel/ProgramOptions.hpp>

#include <DataSeries/TypeIndexModule.hpp>
#include <DataSeries/DataSeriesFile.hpp>
#include <DataSeries/ParallelGrepModule.hpp>
#include <DataSeries/Extent.hpp>
#include <DataSeries/SimpleSourceModule.hpp>

using namespace std;
using lintel::BoyerMooreHorspool;

class StringFieldMatcher {
public:
    StringFieldMatcher(const string &needle)
        : matcher(new BoyerMooreHorspool(needle.c_str(), needle.size())) {
    }

    bool operator()(const Variable32Field &field) {
        return matcher->matches((const char*)field.val(), field.size());
    }

private:
    boost::shared_ptr<BoyerMooreHorspool> matcher;
};

lintel::ProgramOption<bool> countOnly("countOnly", "print match count but do not create output file");
lintel::ProgramOption<bool> noCopy("noCopy", "use a specialized input module for non-compressed"
                                             " data that does not memcpy");
lintel::ProgramOption<string> extentTypeName("extentTypeName", "the extent type", string("Text"));
lintel::ProgramOption<string> fieldName("fieldName", "the name of the field in which"
                                                     " to search for the needle", string("line"));
lintel::ProgramOption<string> needle("needle", "the substring to look for in each record");
lintel::ProgramOption<string> inputFile("inputFile", "the path of the input file");
lintel::ProgramOption<string> outputFile("outputFile", "the path of the output file", string());

lintel::ProgramOption<bool> help("help", "get help");

int main(int argc, char *argv[]) {
    LintelLog::parseEnv();
    vector<string> args = lintel::parseCommandLine(argc, argv, false);

    LintelLogDebug("parallelgrepanalysis", "Starting parallel grep analysis");

    INVARIANT(countOnly.get() == (outputFile.get().empty()),
              "Precisely one of outputFile and countOnly must be specified.");
    INVARIANT(!inputFile.get().empty(), "inputFile must be specified.");
    INVARIANT(!needle.get().empty(), "needle must be specified.");

    auto_ptr<DataSeriesModule> inputModule;
    if (noCopy.get()) {
        inputModule.reset(new SimpleSourceModule(inputFile.get()));
    } else {
        TypeIndexModule *typeInputModule = new TypeIndexModule(extentTypeName.get());
        typeInputModule->addSource(inputFile.get());
        inputModule.reset(typeInputModule);
    }


    StringFieldMatcher fieldMatcher(needle.get());
    ParallelGrepModule<Variable32Field, StringFieldMatcher>
        grepModule(*inputModule, fieldName.get(), fieldMatcher);

    size_t matches = 0;
    if (!countOnly.get()) {
        Extent *extent = grepModule.getExtent(); // the first extent
        if (extent != NULL) {
            DataSeriesSink sink(outputFile.get(), Extent::compress_none, 0);
            ExtentTypeLibrary library;
            library.registerType(extent->getType());
            sink.writeExtentLibrary(library);

            while (extent != NULL) {
                sink.writeExtent(*extent, NULL);
                matches += extent->getRecordCount();

                delete extent;
                extent = grepModule.getExtent();
            }
            sink.close();
        }
    } else {
        Extent *extent = grepModule.getExtent();
        while (extent != NULL) {
            matches += extent->getRecordCount();
            delete extent;
            extent = grepModule.getExtent();
        }
    }
    cerr << boost::format("Found %d occurrence(s) of the string '%s'") % matches % needle.get() << endl;
}
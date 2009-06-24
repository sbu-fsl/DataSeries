// -*-C++-*-
/*
   (c) Copyright 2003-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    A simple class for serializing temporary extents to temporary files. It is used in
    conjunction with ExtentReader.
    This class supports LZF compression.
*/

#ifndef __DATASERIES_EXTENTWRITER_H
#define __DATASERIES_EXTENTWRITER_H

#include <string>

#include <DataSeries/Extent.hpp>

class ExtentWriter {
public:
    ExtentWriter(const std::string &fileName, bool compress=false);
    ExtentWriter(int fd, bool compress=false);
    ExtentWriter(bool compress=false);

    virtual ~ExtentWriter();

    void setFileDescriptor(int fd);

    /** Write the specified extent to a file. */
    void writeExtent(Extent *extent);

    /** Close the file. */
    void close();

private:
    void writeExtentBuffers(bool fixedDataCompressed,
                            bool variableDataCompressed,
                            Extent::ByteArray &fixedData,
                            Extent::ByteArray &variableData);
    bool compressBuffer(Extent::ByteArray &source, Extent::ByteArray &destination);

    int fd;
    bool compress;
    size_t extent_index;

    size_t total_size;
    double total_time;
};

#endif

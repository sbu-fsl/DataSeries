// -*-C++-*-
/*
  (c) Copyright 2004-2005, Hewlett-Packard Development Company, LP

  See the file named COPYING for license details
*/

/** @file
    Convert DataSeries files to text
*/

/*
=pod

=head1 NAME

ds2txt - convert a dataseries file into text (or comma separated value)

=head1 SYNOPSIS

% ds2txt [OPTIONS] input.ds [input-2.ds] > output.txt

=head1 DESCRIPTION

ds2txt converts one or more input DataSeries files into text.  It has a variety of ways to select
sub-parts of a file to convert, and various options to control the output text format.  One of the
useful options is --csv to convert to a comma separated value outptu format.

=head1 OPTIONS

=over 4

=item --csv

Specify that the output should be in csv format.  Sets the separator to ',' and automatically skips
print the types index and extent type header.

=item --separator=I<string>

Specify that value that should be used.

=item --printSpec=I<xml specification|sub-part specification>

Specify the format that should be used for printing out a particular column.  Takes the options
supported by the DsToTextModule.  The format can either be <.../> to specify raw xml, or
type=I<type-name> ...; to specify the format more simply, in which case, ds2txt will automatically
wrap what you write in <printSpec I<your arguments> />.  Two arguments are required, type="..." and
name="..." in order to select the field that is being formatted.  Some standard optional formats
include print_format="I<a boost format like string>" units="units to use" epoch="epoch for a time
field".  TODO: document better.

=item --header=I<string>

Specify the header that should be printed at the start of each extent

=item --header-only-once

Specify that the header should be printed at most once.

=item --fields=I<field1,field2,field3>

Specify a list of fields that should be printed.

=item --skip-index

Skip printing the index extent.

=item --skip-types

Skip printing the types of all extents at the beginning of the output

=item --skip-extent-type

Skip printing the extent type header at the beginning of each extent.

=item --skip-extent-fieldnames

Skip printing the extent fieldnames in the header at the beginning of each extent.

=item --skip-all

Equivalent to specifying all of the --skip-* options.

=item --type=I<type-match>

Specify the type to print out, if it matches exactly that type is selected.  It it matches a unique
prefix, that type is printed, ortherwise if it matches a unique substring that type is selected.
Otherwise an error is generated.

=item --select I<extent-type-match|*> I<field,field,field>

Select a specific extent type and list of fields to print.  TODO: figure out if this is equivalent
to some of the other options. TODO: Change this documentation

=item --where I<expression>

Specify an expression to evaluate for each line.  If the expression returns true then print
out the matching row/record.

=back

=cut
*/

#include <iostream>
#include <DataSeries/DStoTextModule.hpp>
#include <DataSeries/TypeIndexModule.hpp>
#include <boost/program_options.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>

using namespace std;
namespace po = boost::program_options;

int main(int argc, char *argv[]) {
    /* Arguments */
    string input_file;
    string output_dir;

    /* Arguments Parsing */
    po::options_description desc("Allowed options");
    desc.add_options()
        ("input-file", po::value<string>(&input_file), "input file")
        ("output-dir", po::value<string>(&output_dir), "output dir")
    ;

    /* Positional Arguments*/
    po::positional_options_description p;
    p.add("input-file", 1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    /* TODO: Remove */
    if (vm.count("input-file")) {
        cout << "Input file is: " << input_file << "\n";
    }
    if (vm.count("output-dir")) {
        cout << "Output dir is: " << output_dir << "\n";
    }

    /* Output Directory Name */
    if (output_dir.empty()) {
        output_dir = input_file;
        output_dir += "_csv";
    }

    /* Create CSV output directory if it doesn't exist */
    struct stat output_dir_info;
    if (0 != stat(output_dir.c_str(), &output_dir_info)) {
        if (0 != mkdir(output_dir.c_str(), S_IRWXO | S_IRWXG | S_IRWXU)) {
            perror("mkdir");
            return -1;
        }
    }

    /* Output to CSV */
    TypeIndexModule source("");
    DStoTextModule toCSV(source);
    Extent::setReadChecksFromEnv(true); // TODO: See if we really need this
    source.addSource(input_file);
    toCSV.writeToCSV(input_file, output_dir);
    return 0;
}
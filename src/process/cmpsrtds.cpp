/*
  (c) Copyright 2003-2005, Hewlett-Packard Development Company, LP

  See the file named COPYING for license details
*/


#include <sys/time.h>
#include <sys/resource.h>
#include <math.h>
#include <arpa/inet.h>

#include <Lintel/AssertBoost.hpp>
#include <Lintel/StringUtil.hpp>
#include <Lintel/Clock.hpp>

#include <SRT/SRTTrace.H>
#include <SRT/SRTrecord.H>
#include <SRT/SRTTraceRaw.H>
#include <SRT/SRTTrace_Filter.H>

#include <DataSeries/DataSeriesFile.hpp>
#include <DataSeries/DataSeriesModule.hpp>
#include <DataSeries/TypeIndexModule.hpp>

using namespace std;
using boost::format;

int
main(int argc, char *argv[])
{
    SRTTrace *tracestream;

    Extent::setReadChecksFromEnv(true); // verifying things converted properly, should check
    typedef ExtentType::int32 int32;
    typedef ExtentType::int64 int64;
    INVARIANT(argc == 3 || argc == 4,
              format("Usage: %s in-srt in-ds [minor_version]") % argv[0]);
                  
    tracestream = new SRTTraceRaw(argv+1,1);
    INVARIANT(tracestream != NULL, 
              format("Unable to open %s for read") % argv[1]);

    TypeIndexModule srtdsin("Trace::BlockIO::HP-UX"); 
    //srtdsin.setSecondPrefix("I/O trace: SRT-V7"); // rename in progress...
    srtdsin.addSource(argv[2]);
    TypeIndexModule srtdsheaderin("Trace::BlockIO::SRTMetadata");
    srtdsheaderin.addSource(argv[2]);

    //Get start_time and offset from the info file.
    /*
      FILE* info_file_ptr = NULL;

      string info_file_name(argv[1]);
      info_file_name.append(".info");

      cout << format("%s\n") % info_file_name;
      info_file_ptr = fopen(info_file_name.c_str(), "r");
      INVARIANT(info_file_ptr != NULL,
      format("Unable to open %s for read: %s")
      % info_file_name % strerror(errno));
      char info_file_string[1024];
      char *ifs_ptr = info_file_string;
      *ifs_ptr = '\0';
      int str_size = fread(ifs_ptr, 1, 1024, info_file_ptr);
      int read_count = 0;
      while (read_count < str_size && *ifs_ptr != '\n') {
      ifs_ptr++;
      read_count++;
      }
      ifs_ptr++;
      read_count++;
      char *tmp_ptr = ifs_ptr;
      while (*tmp_ptr != '.' && *tmp_ptr != '\0') {
      tmp_ptr++;
      read_count++;
      }
      SINVARIANT(*tmp_ptr == '.');
      *tmp_ptr = '\0';
      tmp_ptr++;
      read_count--; //goes up to the . not including it
      cout << boost::format("hi '%s'\n") % maybehexstring(ifs_ptr);
      int64_t new_tfrac_base = stringToInt64(ifs_ptr, 10);
      ifs_ptr = tmp_ptr;
      while (read_count < str_size && *ifs_ptr != ' ') {
      ifs_ptr++;
      read_count++;
      }
      Clock::Tfrac base_time = 0, time_offset = 0;
      cout << "hi2\n";
      uint64_t new_tfrac_offset = stringToInt64(ifs_ptr, 10);
      base_time = (Clock::Tfrac)new_tfrac_base;
      time_offset = (Clock::Tfrac)new_tfrac_offset;
      Clock::Tfrac curtime = base_time;
      INVARIANT(curtime == base_time, "internal self check failed");
      printf("adjusted basetime %lld\n", base_time);
      printf("used time_offset %lld\n", time_offset);
    */

    int trace_major = tracestream->version().major_num();
    int trace_minor = tracestream->version().minor_num();
    
    printf ("inferred trace version %d.%d\n", trace_major, trace_minor);
    
    if (argc == 4) {
        trace_minor = atoi(argv[3]);
        printf ("overriding minor with %d\n", trace_minor);
    }
    ExtentSeries srtheaderseries;
    /*
      Int64Field time_adjustment(srtheaderseries, "time_adjustment", Field::flag_nullable);
      Int64Field first_return_to_driver(srtheaderseries, "first_return_to_driver", Field::flag_nullable);
      Int64Field last_return_to_driver(srtheaderseries, "last_return_to_driver", Field::flag_nullable);

      //Int64Field start_time_offset(srtheaderseries, "start_time_offset");
      //Int64Field start_time(srtheaderseries, "start_time");
      */
    Variable32Field header_text(srtheaderseries, "header_text", Field::flag_nullable);

    ExtentSeries srtseries;

    BoolField flag_synchronous(srtseries,"flag_synchronous");
    BoolField flag_raw(srtseries,"flag_raw");
    BoolField flag_no_cache(srtseries,"flag_nocache");
    BoolField flag_call(srtseries,"flag_call");
    BoolField flag_filesystemIO(srtseries,"flag_fsysio");
    BoolField flag_bufdata_invalid(srtseries,"flag_bufdata_invalid");
    BoolField flag_cache(srtseries,"flag_cache");
    BoolField flag_power_failure_timeout(srtseries,"flag_pftimeout");
    BoolField flag_write_verification(srtseries,"flag_writev");
    BoolField flag_rewrite(srtseries,"flag_rewrite");
    BoolField flag_delwrite(srtseries,"flag_delwrite");
    BoolField flag_async(srtseries,"flag_async");
    BoolField flag_ndelay(srtseries,"flag_ndelay");
    BoolField flag_wanted(srtseries,"flag_wanted");
    BoolField flag_end_of_data(srtseries,"flag_end_of_data");
    BoolField flag_phys(srtseries,"flag_phys");
    BoolField flag_busy(srtseries,"flag_busy");
    BoolField flag_error(srtseries,"flag_error");
    BoolField flag_done(srtseries,"flag_done");
    BoolField is_read(srtseries,"is_read");
    BoolField flag_ord_write(srtseries,"flag_ordwrite");
    BoolField flag_merged(srtseries,"flag_merged");
    BoolField flag_merged_from(srtseries,"flag_merged_from");
    BoolField act_release(srtseries,"act_release");
    BoolField act_allocate(srtseries,"act_allocate");
    BoolField act_free(srtseries,"act_free");
    BoolField act_raw(srtseries,"act_raw");
    BoolField act_flush(srtseries,"act_flush");
    BoolField net_buf(srtseries, "net_buf");

    Int64Field enter_kernel(srtseries,"enter_driver");
    Int64Field leave_driver(srtseries,"leave_driver");
    Int64Field return_to_driver(srtseries,"return_to_driver");
    BoolField is_suspect(srtseries,"is_suspect");
    BoolField suspect_bad_BFLAGS(srtseries, "suspect_bad_BFLAGS");
    BoolField suspect_bad_return_to_driver(srtseries, "suspect_bad_return_to_driver");
    BoolField suspect_bad_leave_driver(srtseries, "suspect_bad_leave_driver");
    BoolField suspect_bad_enter_driver(srtseries, "suspect_bad_enter_driver");
    BoolField suspect_no_return_to_driver(srtseries, "suspect_no_return_to_driver");
    BoolField suspect_no_leave_or_return(srtseries, "suspect_no_leave_or_return");
    BoolField suspect_no_enqueue_or_leave(srtseries, "suspect_no_enqueue_or_leave");
    BoolField suspect_no_leave_driver(srtseries, "suspect_no_leave_driver");
    BoolField suspect_no_enter_driver(srtseries, "suspect_no_enter_driver");
    Int32Field bytes(srtseries,"bytes");
    Int64Field disk_offset(srtseries,"disk_offset");
    ByteField device_major(srtseries, "device_major");
    ByteField device_minor(srtseries, "device_minor");
    ByteField device_controller(srtseries, "device_controller");
    ByteField device_disk(srtseries, "device_disk");
    ByteField device_partition(srtseries, "device_partition");
    Int32Field driver_type(srtseries,"driver_type", Field::flag_nullable);
    ByteField buffertype(srtseries,"buffertype");
    Variable32Field buffertype_text(srtseries, "buffertype_text");

    // could check for these things with hasColumn also, but that will be
    // implicitly checked by trying to create the fields
    Int32Field *cylinder_number = NULL;
    if (trace_minor >= 1 && trace_minor < 7) {
        cylinder_number = new Int32Field(srtseries,"cylinder_number");
    }
    Int32Field *queue_length = NULL;
    if (trace_minor >= 5) {
        queue_length = new Int32Field(srtseries,"queue_length");
    }
    Int32Field *pid = NULL;
    if (trace_minor >= 5) {
        pid = new Int32Field(srtseries,"pid", Field::flag_nullable);
    }
    Int32Field *logical_volume_number = NULL;
    if (trace_minor >= 6) {
        logical_volume_number = new Int32Field(srtseries,"logical_volume_number", Field::flag_nullable);
    }
    Int32Field *machine_id = NULL;
    Int32Field *thread_id = NULL;
    Int64Field *lv_offset = NULL;
    if (trace_minor >= 7) {
        machine_id = new Int32Field(srtseries,"machine_id");
        thread_id = new Int32Field(srtseries,"thread_id", Field::flag_nullable);
        lv_offset = new Int64Field(srtseries,"lv_offset", Field::flag_nullable);
    }
    Extent::Ptr srtheaderextent = srtdsheaderin.getSharedExtent();
    INVARIANT(srtheaderextent != NULL, "can't find srtheader extents in input file");
              
    srtheaderseries.setExtent(srtheaderextent);
    INVARIANT(strcmp(tracestream->header(), 
                     (const char*)header_text.val()) == 0,
              format("header's are NOT equal %s \n******************\n %s")
              % tracestream->header() % header_text.stringval());
    /*
      const char *header = tracestream->header();
      //printf("Header: %s\n", header);
      std::vector<std::string> lines;
      split(header, "\n", lines);
      for (std::vector<std::string>::iterator i = lines.begin(); 
      i != lines.end(); ++i) {
      // All headers have a tracedate
      const char* time_str = (*i).c_str();
      time_str = strstr(time_str, "tracedate");
      if (time_str == NULL)
      continue;
      const char* date_part = time_str;
      date_part = strstr(time_str, "tracedate    = \"");
      if (date_part != NULL) {
      //1992 Trace
      date_part = strstr(time_str, "\"");
      date_part++;
      } else {
      date_part = strstr(time_str, "tracedate = ");
      if (date_part != NULL) {
      //1996,1998,1999 Trace
      date_part = strstr(time_str, "= ");
      date_part++;
      date_part++;
      } else {
      FATAL_ERROR("Don't understand this trace format's timestamp");
      }
      }
      printf("Inferring start time from %s\n", date_part);
      struct tm tm;
      tm.tm_yday = -1;
      strptime(date_part, "%a %b %d %H:%M:%S %Y", &tm);
      SINVARIANT(tm.tm_yday != -1);
      time_t the_time = mktime(&tm);
      //printf("tfrac_start:%lld, offset:%lld\n", start_time.val(), start_time_offset.val());
      printf("SRT time:%ld, trace_base:%ld, info file base:%ld\n", the_time, Clock::TfracToSec(start_time.val()+start_time_offset.val()), Clock::TfracToSec(base_time + time_offset));
      printf("tfrac: SRT time:%lld trace_base:%lld info file base:%lld\n", Clock::secMicroToTfrac(the_time,0), start_time.val()+start_time_offset.val(),base_time+time_offset);
      INVARIANT(the_time == Clock::TfracToSec(start_time.val() 
      + start_time_offset.val()) 
      && the_time == Clock::TfracToSec(base_time + time_offset), 
      "Start times DO NOT MATCH!");
      break;
      }
    */

    Extent::Ptr srtextent = srtdsin.getSharedExtent();
    INVARIANT(srtextent != NULL, "can't find srt extents in input file");
              
    srtseries.setExtent(srtextent);

    int nrecords = 0;
    while (1) {
        SRTrawRecord *raw_tr = tracestream->record();
        if (srtseries.pos.morerecords() == false) {
            srtextent = srtdsin.getSharedExtent();
            if (srtextent != NULL) {
                srtseries.setExtent(srtextent);
            } else {
                srtseries.clearExtent();
            }
        }
        if (raw_tr == NULL || tracestream->eof() || tracestream->fail() || srtextent == NULL) {
            INVARIANT((raw_tr == NULL || tracestream->eof()) && 
                      srtextent == NULL,
                      "traces ended at different places");
            break;
        }
        
        SRTrecord *_tr = new SRTrecord(raw_tr, 
                                       SRTrawTraceVersion(trace_major, trace_minor));
        
        INVARIANT(_tr->type() == SRTrecord::IO,
                  "Only know how to handle I/O records");
        SRTio *tr = (SRTio *)_tr;
        INVARIANT(trace_minor == tr->get_version(), 
                  format("Version mismatch between header (minor version %d) and data (minor version %d).  Override header with data version to convert correctly!") % trace_minor % tr->get_version());
        ++nrecords;
        SINVARIANT(trace_minor < 7 || tr->noStart() == false);
        if (tr->is_suspect()) {
            //cout << "enter_kernel:" << enter_kernel.val() << " created:" << tr->tfrac_created() << "\n";
            INVARIANT(is_suspect.val(), "suspect SRT but no suspect flag in DS");
            if (trace_minor == 6) {
                /* The flags field appears to have been mangled somewhat.  Just looking at the 1999-01-14 directory of SRT files, the field appears to have been offset by 1 byte, and the bits seem to have been reversed!
                 */
                uint32_t uintSuspect = *(uint32_t*)&(((SRTrawSuspectIO_v6*)raw_tr)->suspect);
                uintSuspect = uintSuspect >> 8;
                uintSuspect = ntohl(uintSuspect);
                struct SRTrawSuspectFlags_t *newSuspect = (struct SRTrawSuspectFlags_t*)&uintSuspect;
                INVARIANT(newSuspect->badBFLAGS == suspect_no_enter_driver.val(), "flag noEnqueue??\n");
                INVARIANT(newSuspect->badCompletion == suspect_no_leave_driver.val(), "flag noStart\n");
                INVARIANT(newSuspect->badStart == suspect_no_enqueue_or_leave.val(), "noEnqueueStart\n");
                INVARIANT(newSuspect->badEnqueue == suspect_no_leave_or_return.val(), "noStartCompletion\n");
                INVARIANT(newSuspect->noCompletion == suspect_no_return_to_driver.val(), "noCompletion\n");
                INVARIANT(newSuspect->noStartCompletion == suspect_bad_enter_driver.val(), "badEnqueue\n");
                INVARIANT(newSuspect->noEnqueueStart == suspect_bad_leave_driver.val(), "badStart\n");
                INVARIANT(newSuspect->noStart == suspect_bad_return_to_driver.val(), "badCompletion\n");
                INVARIANT(newSuspect->noEnqueue == suspect_bad_BFLAGS.val(), "badBFlags\n");
                if (newSuspect->undefined) {
                    //printf ("undefined is set %X\n", newSuspect->undefined);
                    //printf("bytes is %X\n", tr->length());
                    //exit(1);
                }
                /*
                  double created_double = tr->created();
                  printf("create %f\n", created_double);
                  double started_double = tr->started();
                  printf("start %f\n", started_double);
                  double finished_double = tr->finished();
                  printf("finish %f\n", finished_double);
                */
                /*
                  } else if (trace_minor == 7) {
                  suspect_flags.set(((SRTrawSuspectIO_v7*)raw_tr)->suspect);
                */
            } else {
                INVARIANT(!(tr->is_suspect()), "got a version other than 6 or 7 and a suspect IO. Fix the comparer to look for those versions.\n");
            }

            INVARIANT(fabs(enter_kernel.val() - (tr->tfrac_created())) < 5e-7,
                      format("enter_kernel:%lld versus SRT created:%lld")
                      % enter_kernel.val() % tr->tfrac_created());
            // 0.4s == 1717986918 Tfrac
            INVARIANT(fabs(leave_driver.val() - (tr->tfrac_started())) <= 1717986918,
                      format("leave_driver:%lld versus SRT started:%lld\n")
                      % leave_driver.val() % tr->tfrac_started()); 
            SINVARIANT(fabs(return_to_driver.val() - (tr->tfrac_finished())) 
                       < 5e-7);
        } else {
            INVARIANT(fabs(enter_kernel.val() - 
                           (tr->tfrac_created())) < 5e-7,
                      format("enter_kernel:%lld versus SRT created:%lld")
                      % enter_kernel.val() % 
                      (tr->tfrac_created()));
          
            // 0.4s == 1717986918 Tfrac (2^-32 seconds)
            INVARIANT(fabs(leave_driver.val() 
                           - (tr->tfrac_started())) 
                      <= 1717986918,
                      format("leave_driver:%lld versus SRT started:%lld")
                      % leave_driver.val() 
                      % (tr->tfrac_started())); 
            SINVARIANT(fabs(return_to_driver.val() 
                            - (tr->tfrac_finished())) 
                       < 5e-7);
            //      INVARIANT(fabs(enter_kernel.val() - 

            //                     (tr->tfrac_created()+base_time+time_offset)) < 5e-7,
            //                format("enter_kernel:%lld versus SRT created:%lld")
            //                % enter_kernel.val() % 
            //                (base_time+time_offset+tr->tfrac_created()));
          
            //      // 0.4s == 1717986918 Tfrac (2^-32 seconds)
            //      INVARIANT(fabs(leave_driver.val() 
            //                     - (base_time+time_offset+tr->tfrac_started())) 
            //                <= 1717986918,
            //                format("leave_driver:%lld versus SRT started:%lld")
            //                % leave_driver.val() 
            //                % (base_time+time_offset+tr->tfrac_started())); 
            //      SINVARIANT(fabs(return_to_driver.val() 
            //                      - (tr->tfrac_finished()+base_time+time_offset)) 
            //                 < 5e-7);
        }

        SINVARIANT(bytes.val() == (int32)tr->length());
        INVARIANT(disk_offset.val() == (int64)tr->offset(), 
                  format("bad compare %d %lld %lld") % nrecords 
                  % disk_offset.val() % (int64)tr->offset());
        INVARIANT(device_major.val() 
                  == (((int32)tr->device_number() >> 24) & 0xFF), 
                  format("bad compare %d versus %d\n") % device_major.val() 
                  % (((int32)(tr->device_number()) >> 24) & 0xFF));
        SINVARIANT(device_minor.val() 
                   == ((int32)tr->device_number() >> 16 & 0xFF));
        SINVARIANT(device_controller.val() 
                   == ((int32)tr->device_number() >> 12 & 0xF));
        SINVARIANT(device_disk.val() 
                   == ((int32)tr->device_number() >> 8 & 0xF));
        SINVARIANT(device_partition.val() 
                   == ((int32)tr->device_number() & 0xFF));
        SINVARIANT(buffertype.val() == (int32)tr->buffertype());
        SINVARIANT(tr->is_synchronous() == flag_synchronous.val());
        SINVARIANT(tr->is_DUXaccess() == false);
        SINVARIANT(tr->is_raw() == flag_raw.val());
        SINVARIANT(tr->is_no_cache() == flag_no_cache.val());
        SINVARIANT(tr->is_call() == flag_call.val());
        SINVARIANT(tr->is_filesystemIO() == flag_filesystemIO.val());
        SINVARIANT(tr->is_invalid_info() == flag_bufdata_invalid.val());
        SINVARIANT(tr->is_cache() == flag_cache.val());
        SINVARIANT(tr->is_power_failure_timeout() 
                   == flag_power_failure_timeout.val());
        SINVARIANT(tr->is_write_verification() 
                   == flag_write_verification.val());
        SINVARIANT(tr->is_private() == false);
        SINVARIANT(tr->is_rewrite() == flag_rewrite.val());
        SINVARIANT(tr->is_ord_write() == flag_ord_write.val());
        SINVARIANT(tr->is_write_at_exit() == flag_delwrite.val());
        SINVARIANT(tr->is_asynchronous() == flag_async.val());
        SINVARIANT(tr->is_no_delay() == flag_ndelay.val());
        SINVARIANT(tr->is_wanted() == flag_wanted.val());
        SINVARIANT(tr->is_end_of_data() == flag_end_of_data.val());
        SINVARIANT(tr->is_physical_io() == flag_phys.val());
        SINVARIANT(tr->is_busy() == flag_busy.val());
        SINVARIANT(tr->is_error() == flag_error.val());
        SINVARIANT(tr->is_transaction_complete() == flag_done.val());
        SINVARIANT(tr->is_read() == is_read.val());
        // is_readahead is a composite test, so we don't propogate it as a flag
        // SINVARIANT(tr->is_readahead() == false);
        SINVARIANT(tr->is_merged() == flag_merged.val());
        SINVARIANT(tr->is_merged_from() == flag_merged_from.val());
        SINVARIANT(tr->is_flush() == act_flush.val());
        SINVARIANT(tr->is_release() == act_release.val());
        SINVARIANT(tr->is_allocate() == act_allocate.val());
        SINVARIANT(tr->is_free() == act_free.val());
        SINVARIANT(tr->is_character_dev_io() == act_raw.val());
        SINVARIANT(tr->is_netbuf() == net_buf.val());

        if (trace_minor >= 7 && tr->noDriver()) {
            SINVARIANT(driver_type.isNull());
        } else {
            SINVARIANT(driver_type.val() == (int32)tr->driverType());
        }
        if (cylinder_number) {
            SINVARIANT(cylinder_number->val() == (int32)tr->cylno());
        }
        if (queue_length) {
            SINVARIANT(trace_minor < 7 || tr->noQueueLen() == false);
            SINVARIANT(queue_length->val() == (int32)tr->qlen());
        }
        if (pid) {
            if (trace_minor >= 7 && tr->noPid()) {
                //cout << "got a noPid\n";
                SINVARIANT(pid->isNull());
            } else {
                //cout << "pidval:"<<pid->val()<<" trpid:"<< (int32)tr->pid()<< "\n";
                SINVARIANT(pid->val() == (int32)tr->pid());
            }
        }
        if (logical_volume_number) {
            if (trace_minor >= 7 && tr->noLvDevNo()) {
                SINVARIANT(logical_volume_number->isNull());
            } else {
                SINVARIANT(logical_volume_number->val() == (int32)tr->lvdevno());
            }
        }
        if (machine_id) {
            SINVARIANT(tr->noMachineID() == false);
            SINVARIANT(machine_id->val() == (int32)tr->machineID());
        }
        if (thread_id) {
            if (tr->noThread()) {
                SINVARIANT(thread_id->isNull());
            } else {
                SINVARIANT((ExtentType::int32)tr->thread() >= 0);
                SINVARIANT(thread_id->val() == (int32)tr->thread());
            }
        }
        if (lv_offset) {
            if (tr->noLvOffset()) {
                SINVARIANT(lv_offset->isNull());
            } else {
                SINVARIANT((ExtentType::int64)tr->lv_offset() >= 0);
                SINVARIANT(lv_offset->val() == (int64)tr->lv_offset());
            }
        }
        delete _tr;
        ++srtseries.pos;
    }
    printf("%d records successfully compared\n",nrecords);
    delete cylinder_number;
    delete queue_length;
    delete pid;
    delete logical_volume_number;
    delete machine_id;
    delete thread_id;
    delete lv_offset;
}

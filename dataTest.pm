#!/usr/bin/perl
#-------------------------------------------------------
# dataTest.pm  - inspect a myIOT binary datalog file
#
# Usage: perl dataTest.pm <file> <rec_size>
#
# The file is a sequence of fixed-size binary records.
# The first 4 bytes of every record are a uint32_t unix
# timestamp (little-endian).  The remaining bytes are
# opaque application data.
#
# Reports: record count, first/last timestamp, time span,
#          and any ordering or future-date anomalies.
#-------------------------------------------------------

use strict;
use warnings;
use POSIX qw(strftime);

my $filename = shift or die "Usage: $0 <datalog_file> <rec_size>\n";
my $rec_size = shift or die "Usage: $0 <datalog_file> <rec_size>\n";

die "rec_size must be >= 4\n" if $rec_size < 4;
die "File not found: $filename\n" unless -f $filename;

my $file_size = -s $filename;

print "File:        $filename\n";
print "File size:   $file_size bytes\n";
print "Record size: $rec_size bytes\n";

if ($file_size % $rec_size != 0)
{
    my $extra = $file_size % $rec_size;
    print "WARNING: file size is not a multiple of rec_size ($extra extra bytes)\n";
}

my $num_recs = int($file_size / $rec_size);
print "Records:     $num_recs\n";

if ($num_recs == 0)
{
    print "\nFile is empty.\n";
    exit 0;
}


#-----------------------------------------------
# read all timestamps
#-----------------------------------------------

open(my $fh, '<:raw', $filename) or die "Cannot open $filename: $!\n";

my @timestamps;
for my $i (0 .. $num_recs - 1)
{
    my $buf;
    my $got = read($fh, $buf, $rec_size);
    die "Read error at record $i (got $got, expected $rec_size)\n"
        unless defined $got && $got == $rec_size;
    my $dt = unpack('V', substr($buf, 0, 4));   # little-endian uint32
    push @timestamps, $dt;
}
close($fh);


#-----------------------------------------------
# helpers
#-----------------------------------------------

sub fmtDt
    # format a unix timestamp as a readable local-time string
{
    my ($dt) = @_;
    return strftime("%Y-%m-%d %H:%M:%S", localtime($dt)) . "  (unix=$dt)";
}

sub fmtSpan
    # format a duration in seconds as "Xd Xh Xm Xs"
{
    my ($secs) = @_;
    my $d = int($secs / 86400);
    my $h = int(($secs % 86400) / 3600);
    my $m = int(($secs % 3600) / 60);
    my $s = $secs % 60;
    return "${d}d ${h}h ${m}m ${s}s";
}


#-----------------------------------------------
# summary
#-----------------------------------------------

my $first_dt = $timestamps[0];
my $last_dt  = $timestamps[$num_recs - 1];
my $span     = $last_dt > $first_dt ? $last_dt - $first_dt : 0;

print "\n";
print "First record [0]:         " . fmtDt($first_dt) . "\n";
print "Last  record [" . ($num_recs-1) . "]:  " . fmtDt($last_dt) . "\n";
print "Span:                     " . fmtSpan($span) . "\n";


#-----------------------------------------------
# scan for anomalies
#-----------------------------------------------

my $now = time();
my @anomalies;

for my $i (0 .. $num_recs - 1)
{
    my $dt = $timestamps[$i];

    # future: more than 1 hour ahead of host clock
    if ($dt > $now + 3600)
    {
        push @anomalies,
        {
            type     => 'future',
            idx      => $i,
            dt       => $dt,
            delta    => $dt - $now,
        };
    }

    # out of order: strictly less than previous timestamp
    if ($i > 0 && $dt < $timestamps[$i - 1])
    {
        push @anomalies,
        {
            type     => 'out_of_order',
            idx      => $i,
            dt       => $dt,
            prev_idx => $i - 1,
            prev_dt  => $timestamps[$i - 1],
        };
    }

    # duplicate: same timestamp as previous
    if ($i > 0 && $dt == $timestamps[$i - 1])
    {
        push @anomalies,
        {
            type     => 'duplicate',
            idx      => $i,
            dt       => $dt,
            prev_idx => $i - 1,
        };
    }
}

print "\n";
if (@anomalies)
{
    print scalar(@anomalies) . " anomaly(s) found:\n\n";
    for my $a (@anomalies)
    {
        if ($a->{type} eq 'future')
        {
            printf "  [%d]  FUTURE      %s  (%.1f hours ahead)\n",
                $a->{idx},
                fmtDt($a->{dt}),
                $a->{delta} / 3600;
        }
        elsif ($a->{type} eq 'out_of_order')
        {
            printf "  [%d]  OUT-OF-ORDER  %s\n",
                $a->{idx}, fmtDt($a->{dt});
            printf "        prev [%d]:    %s\n",
                $a->{prev_idx}, fmtDt($a->{prev_dt});
        }
        elsif ($a->{type} eq 'duplicate')
        {
            printf "  [%d]  DUPLICATE  %s  (same as [%d])\n",
                $a->{idx}, fmtDt($a->{dt}), $a->{prev_idx};
        }
    }
}
else
{
    print "No anomalies found.\n";
}

print "\n";

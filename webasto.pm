#!/usr/bin/perl
#-------------------------------------------------------------------------
# Uses Win32::CONSOLE commands to implement a scrolling portion
# 	and a fixed portions.  The left side of the screen are PANEL
# 	messages and the right side are PCB messages.  The top
# 	are fixed state fields and the bottom is the scrolling portions
# Initially developed to read RS485 using the USB dongle and frame packets,
# 	now modified to use COM or TELNET to receive packets streamed by the
# 	ESP32 RS485 myIOT "airco_device".
# This program is specifically intended to analyze and identify changes in
#   the messages sent by the PCB and PANEL with regards to state changes
#   in the airco. It DOES NOT send bytes to the ESP32.  Instead it implements
#   it's own command scheme for logging the messaeges and changes and interjecting
#	text into the logfile to help understand those state changes in relation
#	to external real world events (i.e. "Change setpoint to N")
#
# Usage:  perl webasto.pm COM_PORT | TELNET_IP_ADDRESS
#
# Note: By default, in win11, the shortcut target "%windir%\system32\cmd.exe"
# 	that I use will bring up the tabbed "Windows Terminal" application.
# 	To use the old classic, standalone cmd.exe window, you change the default
# 	terminal application in Settings - Privacy & security - For developers - Terminal
# 	(or search for “Default terminal application”). There you can set it back to
# 	Windows Console Host, but it doesnt really make much difference.
# 	In any case you must manually resize the window to a big enough size before
# 	starting this application.


package webasto_console;
use strict;
use warnings;
use threads;
use threads::shared;
use Time::HiRes qw(sleep time);
use Win32::SerialPort;
use Win32::Console;
use IO::Socket::INET;
use Pub::Utils;
Pub::Utils::initUtils();

$logfile = "/junk/webasto.log";

my $BAUD_RATE = 9600;
my $TELNET_PORT = 23;
my $IP_ADDRESS = '';
my $COM_PORT = '';

my $in = Win32::Console->new(STD_INPUT_HANDLE);
$in->Mode(ENABLE_WINDOW_INPUT);
my ($tleft, $ttop, $tright, $tbottom) = $CONSOLE->Window();
# print "$tleft,$ttop,$tright,$tbottom\n";

my $TOP   	= 10;
my $BOTTOM = $tbottom-4;

my $MAX_COMMENT = 20;
	# maximum size of a comment
my $SIDE_WIDTH = 4 + 3*16 + 10;
	# PAN/PCB space upto 20 3 character spaces and 15 characters for crc(XX) error and separation
my $FULL_WIDTH = $tright;
	# sized to the window
$FULL_WIDTH = $SIDE_WIDTH + $MAX_COMMENT if $FULL_WIDTH < $SIDE_WIDTH + $MAX_COMMENT;
	#

my $EMPTY_CHAR = pack('S S', ord(' '), 0);
my $EMPTY_LINE = $EMPTY_CHAR x $FULL_WIDTH;		# used to clear bottom line
my $EMPTY_SIDE = $EMPTY_CHAR x $SIDE_WIDTH;		# used to clear state lines
my $EMPTY_COMMENT = $EMPTY_CHAR x $MAX_COMMENT;

my $sock;
my $port;
my $error_string = '';



#------------------------------------
# utilities
#------------------------------------

sub parseParams
{
	my $param = $ARGV[0];
	usage() if !$param;

	if ($param =~ /^\d+$/)
	{
		$COM_PORT = $param;
		print("COM_PORT=$COM_PORT\n");
	}
	elsif ($param =~ /^\d+\.\d+\.\d+\.\d+/)
	{
		$IP_ADDRESS = $param;
		print("IP_ADDRESS=$IP_ADDRESS\n");
	}
	else
	{
		usage();
	}

	showTitle();
}


sub usage
{
	print("You must specify a COM_PORT or IP_ADDRESS on the command line\n");
	exit 0;
}


sub showTitle
{
	my $title = "webasto.pm (";
	if ($COM_PORT)
	{
		$title .= "COM".$COM_PORT.") ";
		$title .= $port ? "open" : "closed";
	}
	else	# IP_ADDRESS
	{
		$title .= $IP_ADDRESS.") ";
		$title .= $sock ? "open" : "closed";
	}
	$CONSOLE->Title($title);
}




sub exitConsole
{
	my ($msg) = @_;
	print "Error: $msg\n" if $msg;

	if ($port)
	{
		$port->close();
		$port = undef;
	}
	if ($sock)
	{
		$sock->close();
		$sock = undef;
	}
	kill 6,$$;
}


sub isEventCtrlC
{
    my (@event) = @_;
    if ($event[0] &&
        $event[0] == 1 &&      # key event
        $event[5] == 3)        # char = 0x03
    {
        print("ctrl-C pressed ...\n");
        return 1;
    }
    return 0;
}


sub getChar
{
    my (@event) = @_;
    if ($event[0] &&
        $event[0] == 1 &&       # key event
        $event[1] == 1 &&       # key down
        $event[5])              # char
    {
        return chr($event[5]);
    }
    return undef;
}


sub myLog
{
	my ($str) = @_;
	if (open(MYLOGFILE,">>$logfile"))
	{
		print MYLOGFILE $str."\n";
		close MYLOGFILE;
	}
}


sub setChar
{
    my ($line_ref, $col, $char, $attr) = @_;
    my $ord = ord($char);
    my $offset = $col * 4;
    substr($$line_ref, $offset, 4, pack("S S", $ord, $attr));
}


sub setString
{
	my ($line_ref, $col, $string, $attr) = @_;
	for (my $i=0; $i<length($string); $i++)
	{
		setChar($line_ref,$col+$i,substr($string,$i,1),$attr);
	}
}

sub sideX
{
	my ($type) = @_;
	return ($type eq 'PAN') ? $SIDE_WIDTH : 0;
}

sub showOneState
{
	my ($type,$y,$msg,$changed) = @_;
	my $x = sideX($type);
	my $line = $EMPTY_SIDE;
	setString(\$line,0,$msg,$changed?$UTILS_COLOR_RED:$DISPLAY_COLOR_NONE);
	$CONSOLE->WriteRect($line,$x,$y,$x+$SIDE_WIDTH-1,$y);
}

sub showComment
{
	my ($comment) = @_;
	$comment = substr($comment,0,$MAX_COMMENT) if length($comment) > $MAX_COMMENT;
	my $line = $EMPTY_COMMENT;
	setString(\$line,0,$comment,$DISPLAY_COLOR_NONE);
	$CONSOLE->WriteRect($line,2*$SIDE_WIDTH,$BOTTOM,2*$SIDE_WIDTH+$MAX_COMMENT-1,$BOTTOM);
}


sub showCommentArea
{
	my ($row,$msg) = @_;
	my $line = $EMPTY_COMMENT;
	setString(\$line,0,$msg,$DISPLAY_COLOR_NONE);
	$CONSOLE->WriteRect($line,$SIDE_WIDTH*2,$row,$SIDE_WIDTH*2+$MAX_COMMENT-1,$row);
}


sub reportError
{
	my ($msg) = @_;
	if (index($error_string,$msg) <  0)
	{
		$error_string .= "; " if $error_string;
		$error_string .= $msg;
		$CONSOLE->Cursor(0,0);
		$CONSOLE->Attr($DISPLAY_COLOR_WARNING);
		$CONSOLE->Write($error_string);
		$CONSOLE->Attr($DISPLAY_COLOR_NONE);
	}
	myLog("\nIMPLEMENTATION ERROR: $msg\n");
}

#---------------------------------
# communications
#---------------------------------

sub initComPort
{
	myLog("connect COM$COM_PORT");
	showCommentArea(0,"connect COM$COM_PORT");
	sleep(1);

    $port = Win32::SerialPort->new("COM$COM_PORT",1);

    if ($port)
    {
        # This code modifes Win32::SerialPort to allow higher baudrates

        $port->{'_L_BAUD'}{78440} = 78440;
        $port->{'_L_BAUD'}{230400} = 230400;
        $port->{'_L_BAUD'}{460800} = 460800;
        $port->{'_L_BAUD'}{921600} = 921600;
        $port->{'_L_BAUD'}{1843200} = 1843200;

        $port->baudrate($BAUD_RATE);
        $port->databits(8);
        $port->parity("none");
        $port->stopbits(1);

        $port->buffers(60000,8192);

        $port->read_interval(100);    # max time between read char (milliseconds)
        $port->read_char_time(5);     # avg time between read char
        $port->read_const_time(100);  # total = (avg * bytes) + const
        $port->write_char_time(5);
        $port->write_const_time(100);

        $port->handshake("none");   # "none", "rts", "xoff", "dtr".
			# handshaking needed to be turned off for uploading binary files
            # or else sending 0's, for instance, would freeze

        exitConsole("Could not call $port->write_settings()") if !$port->write_settings();
		$port->binary(1);	# probably not needed
    }

	showCommentArea(0,"");
	showTitle();
}




sub initSocket
{
	myLog("connect $IP_ADDRESS");
	showCommentArea(0,"connect $IP_ADDRESS");
	sleep(1);

	my @psock = (
		PeerAddr => $IP_ADDRESS,
		PeerPort => $TELNET_PORT,
		Proto    => 'tcp',
		Timeout  => 5,
		Blocking => 0,
		# KeepAlive  => 1,
	);
	$sock = IO::Socket::INET->new(@psock);
	if ($sock)
	{
		binmode $sock;
		$sock->blocking(0);
	}

	showCommentArea(0,"");
	showTitle();
}



#---------------------------------------------
# processFrame
#---------------------------------------------
# We first determine the PACKET_TYPE, and then add the frame {packet}
# 	members. In the most detailed experiments we found that
# 	the PCB packet comes before the PAN packet, with a separation of about 10ms.
# The comment is set from $in_buf when the user types
# 	some text and a carriage return
# Lurking in here is some bitwise interpretation that makes sense.

my %frame;
$frame{comment} = '';
$frame{PCB} = {};
$frame{PAN} = {};


my $SIG_OFFSET = 0;
my $TYPE_OFFSET = 2;
my $PAYLOAD_OFFSET = 4;

my %PACKET_TYPES = (
	'00ff' => "PCB",
	'88ff' => "BUS",
	'ff00' => "PAN", );

# We have determined that the BUS message is always
# 7e 7e 11 04 10 09 82 f9 so we never display it.

my $PANEL_ONOFF = {
	0x00 => "off",
	0xff => "on",
};

my $PANEL_MODES = {
	0x3f => "cool",
	0x7e => "dehum",
	0xbd => "fan",
	0xce => "heat",
	0xdd => "auto",
};

my $PCB_MODES = {
	0x3f => "cool",
	0x7e => "dehum",
	0xbd => "fan",
	0xce => "heat",
	0xdd => "auto",
};

# this is actually best interpreted bitwise
# where low order nibble is 0x0 for off or 0x8 for compressor on,
# and high order nibble is 0x80 for 'user' or not, with the speeds

# 	my $PCB_FAN_SPEEDS = {
# 		0x98 => "high",
# 		0xa8 => "med",
# 		0xc8 => "low",
# 		0x40 => "auto_low",
# 		0x20 => "auto_med",
# 		0x10 => "auto_high",
# 	};

my $PANEL_FAN_SPEEDS = {
	0xce => "auto",
	0xbd => "high",
	0x7e => "med",
	0x3f => "low",
};


my %STATE_FIELDS = (
	PCB => {
		5 => {name=>'on_off', y=>1, discrete=>$PCB_MODES },
		6 => {name=>'cond',	  y=>2, bias=>40, },
		7 => {name=>'intake', y=>4, bias=>40, },
		8 => {name=>'fan', 	  y=>5, fxn=>\&pcbFanSpeed,},
	},
	PAN => {
		5 => {name=>'on', 	  y=>1, discrete=>$PANEL_ONOFF },
		6 => {name=>'mode',	  y=>2, discrete=>$PANEL_MODES },
		7 => {name=>"setp",   y=>3},
		8 => {name=>'intake', y=>4, bias=>40, },
		9 => {name=>'fan',	  y=>5, discrete=>$PANEL_FAN_SPEEDS, },
	},
);


# The low nibble of PCB{fan} is a current candidate for whether
# 	the compressor is running or not.
# PCB{6} is a good candidate for compressor temperature biased
# 	by 40 typcally showing zub-zero temperatures in C at this time.
# I used to think the 1st two bytes of PCB were the best candidates
#   for AC voltage, but now I suspect that last two, before the checksum
#	with low byte first are candidates as it changes somewhat randomly around
#	7c->7f, and in fact, went to 80 the one time I ran the genset.
# I need to do more experimentation to nail mode behaior.
#	Currently I am using the same enum for both, but I suspect there
#	are modes that don't fit that mold.  The mode has not yet been
#	built into the ESP32.  Most of the rest of these values have.

sub CtoF
{
	my ($c) = @_;
 	my $f = ($c * 9/5) + 32;
	return sprintf("%.0fF",$f);
}


sub pcbFanSpeed
{
	my ($val) = @_;
	my $comp_nibble = $val & 0xf;
	my $comp = $comp_nibble ? " comp_on" : " comp_off";
	reportError("unknown PCB comp nibble($comp_nibble)")
		if $comp_nibble & 0x3;
	my $auto = !($val & 0x80) ? "auto_" : "" ;
	my $speed_flag = $val & 0x70;
	my $speed =
		($speed_flag == 0x10) ? 'high' :
		($speed_flag == 0x20) ? 'med' :
		($speed_flag == 0x40) ? 'low' : 'off';
	return $auto.$speed.$comp;
}


sub processSideState
	# displays states, with changed ones in red
	# and adds changed ones to a list of changed_states for the side.
	# THERE IS A BUG.  I saw the fan state change to 0x18 then 0x98
	# but the log file does not show the latter "state".
{
	my ($type,$side,$payload,$last_payload) = @_;
	my $len = length($payload);
	my $last_len = length($last_payload);
	my $fields = $STATE_FIELDS{$type};

	my $any_changes = 0;
	for my $offset (keys %$fields)
	{
		my $field = $fields->{$offset};
		my $val = ($offset < $len)? unpack('C',substr($payload,$offset,1)) : 0xffff;
		my $old = ($offset < $last_len) ? unpack('C',substr($last_payload,$offset,1)) : 0xffff;
		my $changed = ($val == $old) ? 0 : 1;

		# build the "$show" string

		my $tempC;
		my $extra_str = '';
		if ($field->{name} eq 'setp')
		{
			$tempC = $val;
		}
		elsif ($field->{bias})
		{
			$tempC = $val-$field->{bias};
			$extra_str = sprintf("-%d=%d",$field->{bias},$tempC)
		}
		elsif ($field->{discrete})
		{
			$extra_str = $field->{discrete}->{$val};

			reportError(sprintf("UNKNOWN_DISCRETE(0x%02x) type($type) field($field->{name}) offset($offset))",$val))
				if !$extra_str;

			$extra_str = 'unknown' if !$extra_str;
			$extra_str = ' => '.$extra_str;
		}
		elsif ($field->{fxn})
		{
			$extra_str = " => ".$field->{fxn}->($val);
		}
		if (defined($tempC))
		{
			$extra_str .= "=".CtoF($tempC);
		}

		my $show = "$field->{name}"."[".$offset."]".sprintf("(0x%02x=%d)",$val,$val).$extra_str;

		# add it to the side's state_changes if it changed

		push @{$side->{state_changes}},$show if $changed;

		# always show it on the screen

		showOneState($type,$field->{y},$show,$changed);
		$any_changes = 1 if $changed;
	}
	return $any_changes;
}





sub crcError
{
	my ($packet) = @_;
	my $sum = 0;
	my $len = length($packet);
	my $crc =  unpack('C',substr($packet,$len-1,1));
	for (my $i=2; $i<$len-1; $i++)
	{
		my $c =  unpack('C',substr($packet,$i,1));
		$sum ^= $c;
	}
	return ($sum == $crc) ? "" : sprintf("crc(%02x) ",$sum);
}


sub processFrameSide
{
	my ($type) = @_;
	my $side = $frame{$type};
	my $packet = $side->{packet};
	my $payload = substr($packet,4);
	my $last_payload = $side->{last_payload} || '';
	my $crc_error = crcError($packet);

	$side->{state_changes} = [];
	my $any_changes = 1;


	$any_changes = processSideState($type,$side,$payload,$last_payload)
		if !$crc_error;

	my $log_string = $type.' ';
	my $show_string  = $EMPTY_SIDE;
	setString(\$show_string,0,$type,$DISPLAY_COLOR_NONE);

	my $show_off = 4;
	my $len = length($payload);
	my $last_len = length($last_payload);
	for (my $i=0; $i<$len; $i++)			# 3 * len
	{
		my $c1 = substr($payload,$i,1);
		my $c2 = ($i < $last_len) ? substr($last_payload,$i,1) : undef;
		my $changed =  !defined($c2) || ($c1 ne $c2) ? 1 : 0;
		my $color = $changed ? $UTILS_COLOR_RED : $DISPLAY_COLOR_NONE;
		my $byte = unpack('H*',$c1);

		$log_string .= ($changed?">":" ").$byte;
		setString(\$show_string,$show_off,$byte,$color);
		$show_off += 3;
	}
	$log_string .= " ".$crc_error;
	setString(\$show_string,$show_off,$crc_error,$DISPLAY_COLOR_NONE);
	my $x = sideX($type);
	$CONSOLE->WriteRect($show_string,$x,$BOTTOM,$x+$SIDE_WIDTH-1,$BOTTOM);
	$side->{log_string} = $log_string;
	$side->{last_payload} = $payload;

	# for now I am skipping the last two bytes (possible AC voltage fluctuations)
	# and the checksum with regards to whether or not there are any significant changes
	#
	# return $last_payload ne $payload;
	#
	# The standard payload is 15 bytes, so we will only look the first 12

	$any_changes = 1 if substr($last_payload,0,12) ne substr($payload,0,12);
	$any_changes = 1 if @{$side->{state_changes}};

	return $any_changes;
}



my $skip_count = 0;

sub processFrame
{
	# scroll the bottom line
	
	my $buf = $CONSOLE->ReadRect(0,$TOP+1,$FULL_WIDTH-1,$BOTTOM);
	$CONSOLE->WriteRect($buf,0,$TOP,$FULL_WIDTH-1,$BOTTOM-1);

	# process and display the console
	
	my $any_changes = processFrameSide("PCB");
	$any_changes = 1 if processFrameSide("PAN");
	showComment($frame{comment});

	if ($frame{comment} || $any_changes)
	{
		myLog("\ncomment: $frame{comment}\n") if $frame{comment};
		my $log_line =
			pad($skip_count,6).
			pad($frame{PCB}->{log_string},$SIDE_WIDTH).
			$frame{PAN}->{log_string};
		myLog($log_line);
		$skip_count = 0;

		my $indent = '  state: ';
		my @state_pcb = @{$frame{PCB}->{state_changes}};
		my @state_pan = @{$frame{PAN}->{state_changes}};
		while (@state_pcb || @state_pan)
		{
			my $log_line = '';
			my $left = @state_pcb ? shift @state_pcb : '';
			my $right = @state_pan ? shift @state_pan : '';
			$log_line .= $indent.$left if $left;
			$log_line .= pad('',$SIDE_WIDTH-length($log_line)) if $right;
			$log_line .= $indent.$right if $right;
			myLog(pad('',6).$log_line);
		}
	}
	else
	{
		$skip_count++;
	}

	
	# finished; prep for next frame

	$frame{PCB}->{packet} = '';
	$frame{PAN}->{packet} = '';
	$frame{comment} = '';
}


#------------------------------------------------
# processPacket
#------------------------------------------------


sub sigTypeError
{
	my ($sig,$type_byte,$type) = @_;
	reportError(sprintf("SIG_TYPE_ERROR(%s,%s,%s)",$sig,$type_byte,$type));
}


sub processPacket
{
	my ($packet) = @_;
	my $sig = unpack('H*',substr($packet,0,2));
	my $type_byte = unpack('H*',substr($packet,2,2));
	my $type = $PACKET_TYPES{$type_byte} || '';

	if ($sig ne '7e7e' || !$type)
	{
		sigTypeError($sig,$type_byte,$type);
		return;
	}

	# synchronize start to PCB packet
	# and process the entire frame on the PAN packet

	return if $type eq 'PAN' && !$frame{PCB}->{packet};
	$frame{$type}->{packet} = $packet;
	processFrame() if $type eq 'PAN';
}


#------------------------------
# main
#------------------------------

parseParams();

my $now = now(0,1);
myLog("");
myLog("---------------------------------------------------------------------");
myLog("webasto.pm starting at $now on ".($COM_PORT?"COM$COM_PORT":$IP_ADDRESS));
myLog("---------------------------------------------------------------------");
myLog("");

$CONSOLE->Attr($DISPLAY_COLOR_NONE);
$CONSOLE->Cursor(0,0);
$CONSOLE->Cls();
$CONSOLE->Display();


my $line_buf = '';
my $in_buf = '';

my $last_mem = 0;
my $last_low = 0;




sub processLine
{
	my ($line) = @_;

	# memory debugging

	if ($line =~ /\s+(\d+):(\d+)\s+/)
	{
		my ($mem,$low) = ($1,$2);
		if ($last_mem != $mem ||
			$last_low != $low)
		{
			$last_mem = $mem;
			$last_low = $low;
			showCommentArea(2,"mem($mem:$low)");
		}
	}

	if ($line =~ /addHistoryRecord\((\d+)\)/)
	{
		showCommentArea(3,"num_hist($1)");
	}

	# ACTUAL PACKET LINE
	# print "line=$line\n";
	# 2025-12-12  10:43:06.580 170:142   packet(157,19) 7e 7e 00 ff 11 0d 10 09 82 3f 29 40 c8 00 02 00 7c 00 98
	if ($line =~ s/^.*packet\((\d+),(\d+)\) //)
	{
		my $packet_num = $1;
		my $len = $2;
		showCommentArea(1,$packet_num);
		my $data = substr($line,0,$len*3);
		$data =~ s/ //g;
		# print "line($packet_num,$len)=$data\n";
		my $packet = pack('H*',$data);
		processPacket($packet);
	}


}


sub processBytes
{
	my ($buf) = @_;
	my $len = length($buf);
	for (my $i=0; $i<$len; $i++)
	{
		my $c = substr($buf,$i,1);
		if (ord($c) == 0x0a)
		{
			processLine($line_buf);
			$line_buf = '';
		}
		elsif (ord($c) && ord($c) != 0x0d)
		{
			$line_buf .= $c;
		}
	}
}


sub showInBuf
{
	my $line = $EMPTY_LINE;
	setString(\$line,0,$in_buf,$DISPLAY_COLOR_NONE);
	$CONSOLE->WriteRect($line,0,$BOTTOM+2,$FULL_WIDTH-1,$BOTTOM+2);
}



#------------------------
# loop
#------------------------

my $RECONNECT_WAIT = 5;
my $SOCK_KEEP_ALIVE = 3;

my $reconnect_time = int(time());
my $sock_send_time = 0;
my $show_reconnect = 0;

sub setShowReconnect
{
	my ($start_it) = @_;
	my $what = $COM_PORT ? "PORT" : "SOCK";
	my $now = int(time());
	if ($start_it)
	{
		$reconnect_time = $now + $RECONNECT_WAIT;
		$show_reconnect = 0;
		showTitle();
		myLog("LOST $what");
	}
	my $show = $reconnect_time - $now;
	if ($show_reconnect != $show)
	{
		$show_reconnect = $show;
		showCommentArea(0,"LOST $what($show)");
	}
}




while (1)
{
	# read pending $port or $sock

	my $now = int(time());

	if ($port)
	{
		my ($BlockingFlags, $InBytes, $OutBytes, $LatchErrorFlags) = $port->status();
		if (!defined($BlockingFlags))
		{
			$port = undef;
			setShowReconnect(1);
		}
		elsif ($InBytes)
		{
			my ($bytes, $buf) = $port->read($InBytes);
			processBytes($buf) if $bytes;
		}
		else
		{
			sleep(0.01);
		}
	}
	elsif ($sock)	# $sock
	{
		my $buf;
		my $bytes = sysread($sock,$buf,1024);
		if ($bytes)
		{
			processBytes($buf) if $bytes;
		}
		elsif ($! && $! !~ /A non-blocking socket operation could not be completed immediately/)
		{
			close($sock);
			$sock = undef;
			setShowReconnect(1);
		}
		elsif ($now - $sock_send_time > $SOCK_KEEP_ALIVE)
		{
			$sock_send_time = $now;
			$sock->write(0);
		}
		else
		{
			sleep(0.01);
		}
	}
	elsif ($reconnect_time)
	{
		if ($now >= $reconnect_time)
		{
			$reconnect_time = 0;
			$show_reconnect = 0;
			if ($COM_PORT)
			{
				initComPort();
				setShowReconnect(1) if !$port;
			}
			else	# $IP_ADDRESS
			{
				initSocket();
				setShowReconnect(1) if !$sock;
			}
		}
		else
		{
			setShowReconnect(0);
		}
	}



    # keyboard events

    if ($in->GetEvents())
    {
        my @event = $in->Input();
        if (@event && isEventCtrlC(@event))			# CTRL-C
        {
			exitConsole(1);
        }
        my $char = getChar(@event);
		if (defined($char))
		{
			if (ord($char) == 4)            # CTRL-D
			{
				$error_string = '';
				$CONSOLE->Cls();    # manually clear the screen
			}
			elsif (ord($char) == 8)
			{
				if ($in_buf)
				{
					$in_buf = substr($in_buf,0,length($in_buf)-1);
					showInBuf();
				}
			}
			elsif (ord($char) == 0x0d)
			{
				if ($in_buf)
				{
					$frame{comment} = $in_buf;
					$in_buf = '';
					showInBuf();
				}
			}
			elsif (ord($char) != 0x0a)
			{
				$in_buf .= $char;
				showInBuf();
			}
		}
    }

}	# while (1)



1;

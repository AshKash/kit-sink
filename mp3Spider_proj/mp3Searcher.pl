#!/usr/bin/perl

use CGI qw/:standard/;
use CGI::Carp qw/fatalsToBrowser/;
use Text::ParseWords;
use Storable;
use Text::Soundex;
use Lingua::Ident;

# takes a text and wraps url around it

my %html;

unless (scalar stat("songs.db")) {

	open (FH, "<songs") or die "bad file";
	# Lang ident
	$ident = new Lingua::Ident('data.en', 'data.hindi');

	while (<FH>) {
		chomp;
		$_ =~ m/\\\\(.+?)\\(.+?)\\(.*\.mp3)/i;

		my $mach = $1;
		my $share = $2;
		my $cat = $3;
		my $mp3;
		if ($cat =~ m/(.*)\\(.*\.mp3)/i) {
			$cat = $1;
			$mp3 = $2;
		} else { 
			$mp3 = $cat;
			$cat = "";
		}

		push(@{$html{$mach}->{$share}->{$cat}->[0]}, $mp3);
		push(@{$html{$mach}->{$share}->{$cat}->[1]}, $ident->identify($mp3));
	}
	store(\%html, "songs.db");
	close(FH);
}
%html = %{retrieve("songs.db")};

# Print out a nice HTML file

print header();
print start_html(-title => 'The MindTree mp3 Collection', -bgcolor=>'black',
		 -text=>'white', -background=>'/songs/chabkgde.gif',
		 -link=>'lightyellow');
print ('<span style="background-color: #000000">');
print b('Hi!<BR>Welcome to the The MindTree Songs Archive! This is a comprehensive '),
   b("listing of all shared songs on our intranet.<BR>"),
   b('This list gets updated twice a week, please mail '),
   b('<A href="mailto:ashwinsk@mindtree.com"> Ashwin</A> in case of any '),
   b('mistakes, or if you want the script, '),
   b('or if u think the page sucks and have mega planz to make it rock!<BR>'),
   b('Also, if you want to help me index better, <font color=#ffff00>PLEASE DON\'T SWITCH OFF YOUR '),
   b('MACHINES</font><BR>');
print 'Note: This works best with IE and badly with Netscape(*on Windoze only*) ',
   'Linux fans checkout KDE 2.0, which has support for SMB built in<BR>';

($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
      $atime,$mtime,$ctime,$blksize,$blocks)
           = stat("songs.db");          
print "[ Database last modified: ", scalar localtime $mtime, " ]<HR>";

print h2('Search!');

print startform();

print 'Search in Machine:&nbsp;', textfield(-name => 'mach'), '<BR>',
   'Search under Share:&nbsp;', textfield(-name => 'share'), '<BR>',
   'Search for Category/Song:&nbsp;', textfield(-name => 'cat'), '<BR>',
#   'Enter Song:&nbsp;', textfield(-name => 'mp3'), '<BR>',
#   checkbox(-name => 'soundex', -label => 'Do Soundex Match'), '<BR>',
   'Language (Very buggy):', radio_group(-name=>'ident', -values=>['Any', 'English', 'Hindi'],
		-default=>'Any'), '<BR>',
   submit('Search!');
print '<HR>', endform();
print '</span>';

# Get list of args passed
$f_mach = param('mach');
$f_share = param('share');
$f_cat = param('cat');
$f_mp3 = $cat;
$f_soundex = param('soundex');
$f_ident = param('ident');

$f_mach =~ s/^\s+|\s+$//g;
$f_share =~ s/^\s+|\s+$//g;
$f_cat =~ s/^\s+|\s+$//g;
$f_mp3 =~ s/^\s+|\s+$//g;

$f_mach = $f_mach || '.*';
$f_share = $f_share || '.*';
$f_cat = $f_cat || '.*';
$f_mp3 = $f_mp3 || '.*';

@f_mach = parse_line(' ', 0, $f_mach);
@f_share = parse_line(' ', 0, $f_share);
@f_cat = parse_line(' ', 0, $f_cat);
@f_mp3 = @f_cat;

exit unless ("@f_mach" || "@f_share" || "@f_cat" || "@f_mp3");

#print "@f_mach\n", "@f_share\n", "@f_cat\n", "@f_mp3\n", "<BR>\n";
foreach my $mach (sort keys %html) {
#	print ('<span style="background-color: #000000">');

	next unless (grep($mach =~ m/$_/i, @f_mach));
	$d_mach = 1;

	foreach my $share (sort keys %{$html{$mach}}) {

		next unless (grep($share =~ m/$_/i, @f_share));
		$d_share = 1;

		foreach my $cat (sort keys %{$html{$mach}->{$share}}) {

			$d_cat = 1;

			my @mp3_list = @{$html{$mach}->{$share}->{$cat}->[0]};
			my @ident_list = @{$html{$mach}->{$share}->{$cat}->[1]};

			foreach (0 .. $#mp3_list) {
			my $mp3 = $mp3_list[$_];
			my $ident = $ident_list[$_];
			if($ident =~ m/en_US/) {
				$ident = "English???";
			} elsif ($ident =~ m/hindi/) {
				$ident = "Hindi???";
			} else {
				$ident = "Other";
			}

			next unless (grep($cat =~ m/$_/i, @f_cat) || 
				( grep($mp3 =~ m/$_/i, @f_mp3) &&
				 ( $ident =~ m/$f_ident/i || $f_ident =~ m/any/i)));

			$d_cat = 1 if ((grep($cat =~ m/$_/i, @f_cat) || grep($mp3 =~ m/$_/i, @f_mp3)) && $d_cat);

			print "<BR><BR>" if $d_share || $d_cat || $d_mach;

			print ("Where: <A HREF = \"file:\/\/\\\\$mach\">\\<font size=4 color=#ff3300>$mach<\/A></font>") if $d_mach || $d_share || $d_cat;

			$d_mach = 0;
			print b("\\<A HREF = \"file:\/\/\\\\$mach\\$share\"><font size=3 color=#ff3300>$share<\/A></font>") if $d_share || $d_mach || $d_cat;
			$d_share = 0;

			print ("\\<A HREF = \"file:\/\/\\\\$mach\\$share\\$cat\"><font color=#ff3300>$cat<\/A></font>") if ($d_cat && $cat);
			$d_cat = 0;

			print "<BR>Play! <A HREF = \"file:\/\/\\\\$mach\\$share\\",
			($cat?"$cat\\":""),"$mp3\">$mp3<\/A> ($ident)"
			 if (($mp3 =~ m/($f_mp3)/i) && ($ident =~ m/$f_ident/i || $f_ident =~ m/any/i));

#|| grep(soundex($f_mp3) eq soundex($_), split(' ', $mp3)));


			}
		}
	}
	print "\n" if $d_mach == 0;
#	print '</span>';
}
print end_html();

#!/usr/bin/perl

use Socket;

# This works only in windowze

my @list = split('\n', `net view`);
#my @list = split('\n', $list);
@list = grep(s/\\\\(\w+).*/$1/, @list);


`copy \/y h:\\httpd\\perl\\songs h:\\httpd\\perl\\songs.old`;
open (DB, ">h:\\httpd\\perl\\songs") or die "bad file: $!";

grep($_=uc($_), @list);
my %html;

@list = @ARGV if (@ARGV);

foreach my $mach (@list) {

	next if $mach=~m/aladin|usnj|hqbng|helpdesk|arista|fabmart|nhawk|avis/i;
	next if $mach=~m/webrid|wayto|wsp|agni/i;

	print "pinging $mach\n";
	my $ping = `ping -w 50 $mach`;
	next if ($ping =~ m/(timed out)|(unknown)/im);
	#print "$ping\n";
	print "'$mach'\n";

	my @share=split('\n', `net view $mach`);
	@share = grep(s/(.*?)(Disk.*|$)/$1/, @share);
	foreach my $share (@share) {
		$share =~ s/\s+$/$1/;
		#next unless ($share =~ m/(down)|(share)|(music)|(public)|(song)|(soft)|(mp3)|(media)/ig);
		
		next if $share=~m/^$/;
                next if $share=~m/-----------------|Shared resources at|Share name   Type/i;
                next if $share=~m/The command completed|There are no entries/i;
                next if $share=~m/error|apple|^java|^cvs$|weblogic|^jsdk|^jdk/i;

		print "\t'$share' ";

		my @res = split('\n', `dir /a/s/b \"\\\\$mach\\$share\\*.mp3\"`);
		
		my $hit=0;
		foreach (@res) {
			$hit++;
			# Buffer the output, dont print now
			print DB "$_\n";
		}
		print "Found $hit mp3z\n";
	}
}


`del h:\\httpd\\perl\\songs.db`;


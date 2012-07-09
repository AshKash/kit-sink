#!/usr/bin/perl

# this perl script will take a txt file with the following format:
# FUNC NUMBER	FUNC NAME	FUNC ADDRESS
# it then generates the mapping which includes teh #defines and the 
# table array of pointers.
# the first command line argument is the name of the file to process.
# files can have comments at the begining of the like.
# the  output will be a vaild header file (it must be redirected to 
# appropriate file)

%h;
$max = 0;

open FD, "<$ARGV[0]" or die $!;

# temp code!
print "#include <math.h>\n";
print "#define START_ARG2_FUNCS 128\n";

print "inline double my_add(double, double);\n";
print "inline double my_mul(double, double);\n";
print "inline double my_div(double, double);\n";
print "inline double my_sub(double, double);\n";

while (<FD>) {
    chomp;
    # rid of empty lines
    next unless $_;
    next if /^#/;
    ($a, $b, $c) = split /\s+/, $_;
    if ($a > $max) {$max = $a;}
    print "#define $b\t $a\n";
    $h{$a} = $c;
}

$s = "{";
$m = "{";
for ($i= 0; $i<=$max; $i++) {
    if ($h{$i})  {
	$s .= $h{$i} . ",";
	$m .= "\"" . $h{$i} . "\",";
    } else {
	$s .= "NULL,";
	$m .= '"NULL",';
    }
}
$s .= "};";
$m .= "};";

print "#define TABLE_SIZE $a\n";
print "#ifdef TBF_MAIN\n";
print "const void *f_table[] = $s\n";
print "const char *s_table[] = $m\n";
print "#else\n";
print "extern void *f_table[];\n";
print "extern char *s_table[];\n";
print "#endif\n";

